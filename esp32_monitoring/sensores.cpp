#include <Arduino.h>
#include <DHT.h>
#include <math.h>
#include <string.h>
#include "sensores.h"

namespace {
  constexpr int PIN_HUMEDAD_SUELO = 34;
  constexpr int PIN_NIVEL_AGUA_SUPERIOR = 18;
  constexpr int PIN_NIVEL_AGUA_INFERIOR = 19;
  constexpr int PIN_DHT = 4;
  constexpr int TIPO_DHT = DHT11;
  // Referencias de demostración: sonda al aire y sumergida en agua.
  constexpr int SUELO_SECO_ADC = 4095;
  constexpr int SUELO_EN_AGUA_ADC = 1104;
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

/** Indica si el ID pertenece a un sensor del controlador 3 con lectura implementada. */
bool sensorAvailable(const char* sensorId) {
  return sensorId && (strcmp(sensorId, "7") == 0 || strcmp(sensorId, "8") == 0 || strcmp(sensorId, "9") == 0);
}

/**
 * Lee el sensor del controlador 3; suelo y nivel son porcentajes de demostración.
 * @param sensorId Identificador configurado en la API.
 * @param value Salida en la unidad registrada, por ejemplo porcentaje calibrado.
 * @returns true si la lectura es válida; false ante error o flotadores inconsistentes.
 */
bool readSensorValue(const char* sensorId, float& value) {
  if (!sensorId) return false;
  if (strcmp(sensorId, "7") == 0) {
    value = dht.readHumidity();
    return isfinite(value) && value >= 0 && value <= 100;
  }
  if (strcmp(sensorId, "9") == 0) {
    const int raw = analogRead(PIN_HUMEDAD_SUELO);
    value = constrain(100.0f * (SUELO_SECO_ADC - raw) / (SUELO_SECO_ADC - SUELO_EN_AGUA_ADC), 0.0f, 100.0f);
    return true;
  }
  if (strcmp(sensorId, "8") == 0) {
    const bool superior = digitalRead(PIN_NIVEL_AGUA_SUPERIOR) == LOW;
    const bool inferior = digitalRead(PIN_NIVEL_AGUA_INFERIOR) == LOW;
    if (superior && !inferior) return false;
    // Valores representativos, no mediciones exactas del nivel.
    value = superior ? 80.0f : (inferior ? 45.0f : 10.0f);
    return true;
  }
  return false;
}



/**
 * Muestra las lecturas locales por Serial; no controla relés ni actuadores.
 * Debe llamarse periódicamente desde loop().
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

  Serial.println("\n--- Lectura de sensores ---");
  const float humedadDemostracion = constrain(
      100.0f * (SUELO_SECO_ADC - humedadSueloRaw) / (SUELO_SECO_ADC - SUELO_EN_AGUA_ADC),
      0.0f, 100.0f);
  Serial.printf("Suelo: %.1f %% (demostracion; ADC: %d)\n", humedadDemostracion, humedadSueloRaw);
  Serial.printf("Flotador superior (D%d): %s\n", PIN_NIVEL_AGUA_SUPERIOR,
                nivelSuperiorActivo ? "ACTIVO" : "INACTIVO");
  Serial.printf("Flotador inferior (D%d): %s\n", PIN_NIVEL_AGUA_INFERIOR,
                nivelInferiorActivo ? "ACTIVO" : "INACTIVO");

  if (isfinite(humedadAmbiente) && isfinite(temperatura)) {
    Serial.printf("Humedad del aire: %.1f %%\nTemperatura: %.1f C\n", humedadAmbiente, temperatura);
  } else {
    Serial.println("DHT11: sin lectura valida; comprobar sensor y conexiones.");
  }
}
