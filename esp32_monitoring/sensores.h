#pragma once

/** Inicializa los pines y drivers físicos; todavía pendiente de implementar. */
void initializeSensors();

/**
 * Indica si el sensor tiene un driver disponible.
 * @param sensorId Identificador del sensor registrado en el servidor.
 * @returns false mientras no se implemente el driver físico correspondiente.
 */
bool sensorAvailable(const char* sensorId);

/**
 * Obtiene una lectura física nueva y calibrada.
 * @param sensorId Identificador del sensor registrado en el servidor.
 * @param value Salida en la unidad registrada; solo válida si devuelve true.
 * @returns true si la lectura es válida; false ante falta de driver o error de lectura.
 */
bool readSensorValue(const char* sensorId, float& value);
