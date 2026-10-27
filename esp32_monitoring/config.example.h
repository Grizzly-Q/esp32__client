#pragma once
#include "configuracion_tipos.h"
#include "certificado_raiz.h"

// Red Wi-Fi existente a la que se conectará la placa; el ESP32 no crea una red.
constexpr char WIFI_SSID[] = "CONFIGURAR_WIFI";
constexpr char WIFI_PASSWORD[] = "CONFIGURAR_PASSWORD";

// Dominio público sin protocolo, barra final ni rutas; HTTPS y WSS usan el puerto 443.
constexpr char API_HOST[] = "invernadero-api.jair77253.workers.dev";
constexpr uint16_t API_PORT = 443;

// Emitir esta credencial en el servidor y activar el controlador antes de conectarlo.
// No usar el JWT de una persona ni publicar este archivo cuando tenga secretos.
constexpr char ESP32_CREDENTIAL[] = "esp32_CONFIGURAR_CREDENCIAL";

// IDs del controlador 3: humedad ambiental, nivel de agua y humedad del suelo.
// El DHT11 requiere al menos dos segundos entre lecturas.
constexpr SensorConfiguration SENSOR_CONFIGURATION[] = {{"7", 2000}, {"8", 2000}, {"9", 2000}};
