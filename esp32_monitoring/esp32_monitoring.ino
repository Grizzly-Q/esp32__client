#include <Arduino.h>
#include "sensores.h"

/** Inicia Serial y prepara únicamente los sensores locales. */
void setup() {
  Serial.begin(115200);
  initializeSensors();
  Serial.println("Lectura local iniciada; comunicación con el servidor desactivada.");
}

/** Lee los sensores cada dos segundos y muestra sus estados en el Monitor serie. */
void loop() {
  processSensors();
  delay(10);
}
