#include <Arduino.h>
#include "sensores.h"
#include "comunicacion.h"

/** Inicia Serial, los sensores y la comunicación con el servidor. */
void setup() {
  Serial.begin(115200);
  initializeSensors();
  Serial.println("Lectura local iniciada; preparando conexion con el servidor.");
  initializeCommunication();
}

/** Muestra las lecturas y atiende continuamente la comunicación. */
void loop() {
  processSensors();
  processCommunication();
  delay(10);
}
