# Comunicación con el servidor

La placa usa su credencial propia, una cadena aleatoria con prefijo `esp32_`, en la cabecera `Authorization: Bearer <credencial>`. No utiliza el JWT de una persona. El servidor almacena el hash de esa credencial; el secreto se configura únicamente en la placa.

## Histórico cada 30 segundos

Enviar `POST https://<dominio>/telemetry`, con `Content-Type: application/json`:

```json
{
  "readings": [
    {
      "sensorId": "1",
      "readingId": "arranque-unico:125",
      "value": 48.5,
      "measuredAt": "2026-10-23T18:00:00Z"
    }
  ]
}
```

El lote admite de una a cien lecturas. Los IDs de sensores son cadenas y deben pertenecer al controlador autenticado. `readingId` identifica una lectura por sensor: un reintento conserva el mismo identificador y contenido. `measuredAt` es la hora UTC obtenida mediante NTP; puede omitirse o ser null si se desconoce. El servidor registra por separado la hora de recepción. No sustituir una hora desconocida por una inventada.

## Monitoreo temporal

Abrir un WebSocket estándar en `wss://<dominio>/controllers/socket`, usando la misma cabecera de autenticación. No es Socket.IO.

El servidor envía `monitoring.ready` y puede solicitar:

```json
{"type":"monitoring.start","requestId":"ea928fd4-9a20-4538-8c76-7c71d248b78a","sensorId":"1","intervalMs":1000,"durationSeconds":60}
```

Confirmar en cinco segundos, declarando el intervalo real que permite el sensor:

```json
{"type":"monitoring.ack","requestId":"ea928fd4-9a20-4538-8c76-7c71d248b78a","sensorId":"1","accepted":true,"intervalMs":1000}
```

Si no existe driver, responder `accepted: false` y omitir `intervalMs`. Una lectura rápida tiene esta forma:

```json
{"type":"monitoring.reading","requestId":"ea928fd4-9a20-4538-8c76-7c71d248b78a","sensorId":"1","sequence":0,"readingId":"arranque-unico:126","value":48.5,"measuredAt":"2026-10-23T18:00:01Z"}
```

Incrementar `sequence` para cada orden; reiniciarla cuando cambie `requestId`. Las renovaciones llegan como nuevas órdenes de inicio. `monitoring.stop` incluye `sensorId` y `requestId` y cancela el modo rápido correspondiente. Cancelar también al desconectarse o al vencer el plazo local de 60 segundos medido con `millis()`. El envío histórico continúa durante el monitoreo; las lecturas rápidas no se guardan en el historial del servidor.

Verificar siempre el certificado TLS. No imprimir la credencial ni incluirla en la URL o en Git.
