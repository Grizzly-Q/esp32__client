#include <Arduino.h>
#include "sensores.h"
#include "comunicacion.h"

/** Inicia el diagnóstico serie, los sensores y la comunicación del controlador. */
void setup() {
  Serial.begin(115200);
  initializeSensors();
  initializeCommunication();
}

/** Mantiene el monitoreo y cede brevemente tiempo a las tareas de la placa. */
void loop() {
  processCommunication();
  delay(2);
}
