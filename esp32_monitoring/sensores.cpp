#include "sensores.h"

/** Inicializar aquí los drivers y pines reales cuando se definan los modelos de sensores. */
void initializeSensors() {
  // No se asignan pines de ejemplo que puedan activar hardware por error.
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

