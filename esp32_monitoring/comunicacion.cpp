#include "comunicacion.h"
#include "sensores.h"
#include <Arduino.h>
#include <WiFi.h>
#include <NetworkClientSecure.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <time.h>
#include <math.h>
#include "config.h"


namespace {

constexpr size_t SENSOR_COUNT = sizeof(SENSOR_CONFIGURATION) / sizeof(SENSOR_CONFIGURATION[0]);
static_assert(SENSOR_COUNT > 0 && SENSOR_COUNT <= 3, "Configurar entre uno y tres sensores.");
constexpr uint32_t NORMAL_INTERVAL_MS = 30000;
constexpr uint32_t MAX_LEASE_MS = 60000;

/** Estado local; las concesiones se miden con millis(), independientemente de NTP. */
struct SensorState {
  bool live = false;
  bool hasValue = false;
  double value = 0;
  uint32_t sampledAt = 0;
  uint32_t leaseUntil = 0;
  uint32_t intervalMs = NORMAL_INTERVAL_MS;
  uint32_t sequence = 0;
  String requestId;
  String readingId;
  String measuredAt;
};

SensorState sensors[SENSOR_COUNT];
WebSocketsClient channel;
SemaphoreHandle_t cacheMutex;
String authorizationHeader;
String bootId;
uint32_t readingCounter = 0;

/** Compara plazos cortos y soporta el desbordamiento de millis(). */
bool reached(uint32_t now, uint32_t deadline) {
  return static_cast<int32_t>(now - deadline) >= 0;
}

/** Encuentra únicamente sensores configurados para esta placa. */
int sensorIndex(const String& id) {
  for (size_t index = 0; index < SENSOR_COUNT; index++) if (id == SENSOR_CONFIGURATION[index].id) return static_cast<int>(index);
  return -1;
}

/** Obtiene hora UTC; una hora desconocida se omite y nunca se inventa. */
String utcNow() {
  time_t epoch = time(nullptr);
  if (epoch < 1700000000) return String();
  struct tm utc;
  gmtime_r(&epoch, &utc);
  char text[25];
  strftime(text, sizeof(text), "%Y-%m-%dT%H:%M:%SZ", &utc);
  return String(text);
}

/** Crea un identificador de arranque aleatorio para no reutilizar lecturas después de reiniciar. */
String createBootId() {
  uint8_t bytes[16];
  esp_fill_random(bytes, sizeof(bytes));
  char text[33];
  for (size_t index = 0; index < sizeof(bytes); index++) snprintf(text + index * 2, 3, "%02x", bytes[index]);
  return String(text);
}

/** Vuelve al muestreo normal sin alterar las últimas lecturas almacenadas en caché. */
void stopLive(size_t index) {
  sensors[index].live = false;
  sensors[index].requestId = String();
  sensors[index].intervalMs = NORMAL_INTERVAL_MS;
}

/** Procesa órdenes conocidas, confirma la frecuencia real y limita toda concesión a 60 segundos. */
void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_DISCONNECTED) {
    Serial.println("Conexion en vivo desconectada; se intentara reconectar.");
    for (size_t index = 0; index < SENSOR_COUNT; index++) stopLive(index);
    return;
  }
  if (type != WStype_TEXT || length > 2048) return;
  JsonDocument message;
  if (deserializeJson(message, payload, length)) return;
  const String kind = message["type"] | "";
  if (kind == "monitoring.ready") {
    Serial.println("Conexion en vivo autenticada y lista.");
    return;
  }
  if (kind != "monitoring.start" && kind != "monitoring.stop") return;
  const String sensorId = message["sensorId"] | "";
  const String requestId = message["requestId"] | "";
  const int index = sensorIndex(sensorId);
  if (kind == "monitoring.stop") { if (index >= 0) stopLive(index); return; }

  const uint32_t requested = message["intervalMs"] | 0U;
  const uint32_t duration = message["durationSeconds"] | 0U;
  const uint32_t minimum = index >= 0 ? SENSOR_CONFIGURATION[index].minimumIntervalMs : 30000;
  const uint32_t applied = requested > minimum ? requested : minimum;
  const bool accepted = index >= 0 && requestId.length() == 36 && requested >= 1000 && requested <= 10000 && applied <= 30000 && duration > 0 && duration <= MAX_LEASE_MS / 1000 && sensorAvailable(SENSOR_CONFIGURATION[index].id);

  JsonDocument ack;
  ack["type"] = "monitoring.ack";
  ack["sensorId"] = sensorId;
  ack["requestId"] = requestId;
  ack["accepted"] = accepted;
  if (accepted) ack["intervalMs"] = applied;
  String response;
  serializeJson(ack, response);
  const bool sent = channel.sendTXT(response);
  if (!accepted || !sent) return;
  sensors[index].requestId = requestId;
  sensors[index].intervalMs = applied;
  sensors[index].sequence = 0;
  sensors[index].leaseUntil = millis() + duration * 1000U;
  sensors[index].live = true;
}

/** Copia la caché; las lecturas rápidas no modifican la frecuencia de escritura histórica. */
String historicalPayload() {
  JsonDocument document;
  JsonArray readings = document["readings"].to<JsonArray>();
  xSemaphoreTake(cacheMutex, portMAX_DELAY);
  for (size_t index = 0; index < SENSOR_COUNT; index++) {
    if (!sensors[index].hasValue) continue;
    JsonObject reading = readings.add<JsonObject>();
    reading["sensorId"] = SENSOR_CONFIGURATION[index].id;
    reading["readingId"] = sensors[index].readingId;
    reading["value"] = sensors[index].value;
    if (sensors[index].measuredAt.length()) reading["measuredAt"] = sensors[index].measuredAt;
  }
  xSemaphoreGive(cacheMutex);
  if (readings.size() == 0) return String();
  String output;
  serializeJson(document, output);
  return output;
}

/**
 * Mantiene HTTPS periódico en otra tarea para no bloquear el canal de órdenes.
 * Reintenta el mismo JSON ante errores temporales; conserva sus identificadores.
 */
void normalSender(void*) {
  String pending;
  uint32_t nextAttempt = millis() + NORMAL_INTERVAL_MS;
  for (;;) {
    const uint32_t now = millis();
    if (WiFi.status() == WL_CONNECTED && time(nullptr) >= 1700000000 && reached(now, nextAttempt)) {
      if (!pending.length()) pending = historicalPayload();
      if (pending.length()) {
        NetworkClientSecure secure;
        secure.setCACert(ROOT_CA);
        HTTPClient http;
        http.setConnectTimeout(5000);
        http.setTimeout(5000);
        const String url = String("https://") + API_HOST + ":" + API_PORT + "/telemetry";
        if (http.begin(secure, url)) {
          http.addHeader("Content-Type", "application/json");
          http.addHeader("Authorization", String("Bearer ") + ESP32_CREDENTIAL);
          const int status = http.POST(pending);
          if (status == 200) { Serial.println("Lecturas historicas enviadas."); pending = String(); }
          else if (status >= 400 && status < 500 && status != 429) {
            Serial.printf("Telemetría rechazada (%d): revisar clave, sensores y contrato.\n", status);
            pending = String();
          }
          http.end();
        }
      }
      nextAttempt = millis() + (pending.length() ? 5000 : NORMAL_INTERVAL_MS);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

}  // Los detalles de comunicación son privados de este módulo.

/** Inicializa Wi-Fi, TLS, NTP y tareas sin exponer las credenciales por Serial. */
void initializeCommunication() {
  if (String(WIFI_SSID).indexOf("CONFIGURAR") >= 0 || String(WIFI_PASSWORD).indexOf("CONFIGURAR") >= 0 || String(ROOT_CA).indexOf("-----BEGIN CERTIFICATE-----") < 0 || String(ESP32_CREDENTIAL).indexOf("CONFIGURAR") >= 0) {
    Serial.println("Completar config.h con Wi-Fi, credencial y certificado CA válidos.");
    for (;;) delay(1000);
  }
  for (size_t index = 0; index < SENSOR_COUNT; index++) {
    if (String(SENSOR_CONFIGURATION[index].id).indexOf("CONFIGURAR") >= 0) {
      Serial.println("Configurar los identificadores reales de sensores en config.h.");
      for (;;) delay(1000);
    }
  }
  cacheMutex = xSemaphoreCreateMutex();
  if (!cacheMutex) { Serial.println("No se pudo crear la caché."); for (;;) delay(1000); }
  bootId = createBootId();
  for (size_t index = 0; index < SENSOR_COUNT; index++) sensors[index].sampledAt = millis() - NORMAL_INTERVAL_MS;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  authorizationHeader = String("Authorization: Bearer ") + ESP32_CREDENTIAL;
  channel.beginSslWithCA(API_HOST, API_PORT, "/controllers/socket", ROOT_CA);
  channel.setExtraHeaders(authorizationHeader.c_str());
  channel.onEvent(onWebSocketEvent);
  channel.setReconnectInterval(5000);
  channel.enableHeartbeat(20000, 10000, 2);
  if (xTaskCreate(normalSender, "telemetria-normal", 12288, nullptr, 1, nullptr) != pdPASS) {
    Serial.println("No se pudo iniciar HTTPS periódico."); for (;;) delay(1000);
  }
}

/** Atiende órdenes y toma lecturas físicas; un vencimiento local siempre desactiva el modo rápido. */
void processCommunication() {
  if (WiFi.status() == WL_CONNECTED && time(nullptr) >= 1700000000) channel.loop();
  else for (size_t index = 0; index < SENSOR_COUNT; index++) stopLive(index);
  for (size_t index = 0; index < SENSOR_COUNT; index++) {
    const uint32_t now = millis();
    if (sensors[index].live && reached(now, sensors[index].leaseUntil)) stopLive(index);
    const uint32_t normalMinimum = SENSOR_CONFIGURATION[index].minimumIntervalMs > NORMAL_INTERVAL_MS ? SENSOR_CONFIGURATION[index].minimumIntervalMs : NORMAL_INTERVAL_MS;
    const uint32_t interval = sensors[index].live ? sensors[index].intervalMs : normalMinimum;
    if (now - sensors[index].sampledAt < interval || !sensorAvailable(SENSOR_CONFIGURATION[index].id)) continue;
    sensors[index].sampledAt = now;
    float raw;
    if (!readSensorValue(SENSOR_CONFIGURATION[index].id, raw) || !isfinite(raw) || raw < 0 || raw > 99999.99) {
      xSemaphoreTake(cacheMutex, portMAX_DELAY);
      sensors[index].hasValue = false;
      xSemaphoreGive(cacheMutex);
      continue;
    }
    const double value = round(static_cast<double>(raw) * 100.0) / 100.0;
    const String readingId = bootId + ":" + String(++readingCounter);
    const String measuredAt = utcNow();
    xSemaphoreTake(cacheMutex, portMAX_DELAY);
    sensors[index].value = value;
    sensors[index].readingId = readingId;
    sensors[index].measuredAt = measuredAt;
    sensors[index].hasValue = true;
    xSemaphoreGive(cacheMutex);
    // Se vuelve a comprobar el plazo después de leer el driver, que podría haber tardado.
    if (!sensors[index].live || reached(millis(), sensors[index].leaseUntil) || !channel.isConnected()) continue;
    JsonDocument live;
    live["type"] = "monitoring.reading";
    live["sensorId"] = SENSOR_CONFIGURATION[index].id;
    live["requestId"] = sensors[index].requestId;
    live["sequence"] = sensors[index].sequence++;
    live["readingId"] = readingId;
    live["value"] = value;
    if (measuredAt.length()) live["measuredAt"] = measuredAt;
    String packet;
    serializeJson(live, packet);
    channel.sendTXT(packet);
  }
}
