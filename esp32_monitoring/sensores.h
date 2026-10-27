#pragma once

/** Prepara el ADC, los pines de entrada y el driver del DHT11. */
void initializeSensors();

/**
 * Indica si el sensor tiene un driver disponible.
 * @param sensorId Identificador del sensor registrado en el servidor.
 * @returns true para los ID implementados del controlador 3; false en otro caso.
 */
bool sensorAvailable(const char* sensorId);

/**
 * Obtiene una lectura física; suelo y nivel utilizan referencias de demostración.
 * @param sensorId Identificador del sensor registrado en el servidor.
 * @param value Salida en la unidad registrada; solo válida si devuelve true.
 * @returns true si la lectura es válida; false ante falta de driver o error de lectura.
 */
bool readSensorValue(const char* sensorId, float& value);


/**
 * Lee los sensores locales y escribe un resumen por Serial.
 * No conecta con la red, no transmite datos y no acciona la bomba.
 */
void processSensors();
