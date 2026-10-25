#include <Arduino.h>
#include <DHT.h>
#include <math.h>
#include "sensores.h"

namespace {
    constexpr int PIN_HUMEDAD_SUELO = 34;
  constexpr int PIN_NIVEL_AGUA_SUPERIOR = 32;
  constexpr int PIN_NIVEL_AGUA_INFERIOR = 33;
  constexpr int PIN_DHT = 25;
  constexpr int TIPO_DHT = DHT11;
  constexpr uint32_t INTERVALO_LECTURA_MS = 2000;
  uint32_t ultimaLectura = 0;
  DHT dht(PIN_DHT, TIPO_DHT);
}

/**
 * Inicializa el ADC, las entradas de los flotadores y el DHT11.
 * Los flotadores se consideran contactos entre GPIO y GND; verificar su orientación.
 */
void initializeSensors() {
  analogReadResolution(12);
  pinMode(PIN_HUMEDAD_SUELO, INPUT);
  pinMode(PIN_NIVEL_AGUA_SUPERIOR, INPUT_PULLUP);
  pinMode(PIN_NIVEL_AGUA_INFERIOR, INPUT_PULLUP);
  dht.begin();
}

/** Indicar qué sensores tienen un driver implementado; false rechaza el modo rápido explícitamente. */
bool sensorAvailable(const char* sensorId) {
  (void)sensorId;
  return false;
}

/**
 * Leer y calibrar el sensor físico indicado; nunca devolver valores simulados.
 * @param sensorId Identificador configurado en la API.
 * @param value Salida en la unidad registrada, por ejemplo porcentaje calibrado.
 * @returns true únicamente si se obtuvo una lectura física válida y nueva.
 */
bool readSensorValue(const char* sensorId, float& value) {
  (void)sensorId;
  (void)value;
  // Completar con el driver real. Para porcentajes validar también el rango 0..100.
  return false;
}



/**
 * Lee los sensores localmente y muestra sus valores por Serial, sin Wi-Fi ni servidor.
 * Debe llamarse periódicamente desde loop(); no controla relés ni actuadores.
 */
void processSensors() {
  if (millis() - ultimaLectura < INTERVALO_LECTURA_MS) return;
  ultimaLectura = millis();

  const int humedadSueloRaw = analogRead(PIN_HUMEDAD_SUELO);
  // Con INPUT_PULLUP y el contacto conectado a GND, LOW significa circuito cerrado.
  const bool nivelSuperiorActivo = digitalRead(PIN_NIVEL_AGUA_SUPERIOR) == LOW;
  const bool nivelInferiorActivo = digitalRead(PIN_NIVEL_AGUA_INFERIOR) == LOW;
  const float humedadAmbiente = dht.readHumidity();
  const float temperatura = dht.readTemperature();

  Serial.printf("FC-28 ADC: %d | Flotador 65%%: %s | Flotador 20%%: %s",
                humedadSueloRaw,
                nivelSuperiorActivo ? "ACTIVO" : "INACTIVO",
                nivelInferiorActivo ? "ACTIVO" : "INACTIVO");

  if (isfinite(humedadAmbiente) && isfinite(temperatura)) {
    Serial.printf(" | DHT11: %.1f %%HR, %.1f C\n", humedadAmbiente, temperatura);
  } else {
    Serial.println(" | DHT11: sin lectura válida");
  }
}
