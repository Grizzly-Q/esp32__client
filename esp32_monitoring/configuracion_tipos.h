#pragma once
#include <stdint.h>

/** Configuración de un sensor físico registrado en el servidor. */
struct SensorConfiguration {
  const char* id;  ///< Identificador del sensor en la API; no corresponde a un pin.
  uint32_t minimumIntervalMs;  ///< Tiempo mínimo entre lecturas admitido por el sensor.
};
