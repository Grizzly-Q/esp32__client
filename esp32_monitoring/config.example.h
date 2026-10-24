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

// Sustituir por los IDs reales de GET /controllers/<id>/sensors.
// Agregar una entrada por sensor; ajustar el intervalo según su modelo físico.
constexpr SensorConfiguration SENSOR_CONFIGURATION[] = {{"CONFIGURAR_ID_SENSOR", 1000}};
