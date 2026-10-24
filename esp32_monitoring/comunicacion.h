#pragma once

/**
 * Configura Wi-Fi, hora UTC, TLS, WebSocket y la tarea de envío histórico.
 * Debe invocarse una sola vez después de iniciar Serial y los sensores.
 * Si falta configuración o memoria, informa por Serial y detiene el arranque.
 */
void initializeCommunication();

/**
 * Procesa órdenes, vencimientos y muestreo físico sin esperar conexiones de red.
 * Debe invocarse continuamente desde loop(); publica las lecturas disponibles.
 */
void processCommunication();
