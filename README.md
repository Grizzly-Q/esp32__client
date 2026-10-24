# Cliente ESP32 del invernadero

Código para cargar en una placa ESP32 con Arduino IDE, independiente del repositorio del servidor. Un firmware es el programa que se ejecuta en la placa; el archivo `.ino` es el sketch que abre Arduino IDE.

## Archivos

- `esp32_monitoring/esp32_monitoring.ino`: punto de entrada con `setup()` y `loop()`.
- `esp32_monitoring/sensores.h` y `sensores.cpp`: interfaz y drivers físicos pendientes.
- `esp32_monitoring/comunicacion.h` y `comunicacion.cpp`: Wi-Fi, HTTPS, WSS y muestreo.
- `esp32_monitoring/configuracion_tipos.h`: tipo compartido de configuración de sensores.
- `esp32_monitoring/certificado_raiz.h`: certificado público ISRG Root X1 verificado con el dominio.
- `esp32_monitoring/config.h`: configuración privada local, excluida de Git.
- `esp32_monitoring/config.example.h`: plantilla de configuración sin credenciales reales.
- `esp32_monitoring/README.md`: detalles técnicos y limitaciones del firmware.
- `docs/protocolo.md`: mensajes que intercambia la placa con el servidor.

## Primeros pasos en Arduino IDE

1. Instalar Arduino IDE y conectar el ESP32 mediante un cable USB que permita transferir datos.
2. Instalar el paquete de placas **esp32 de Espressif Systems**, versión 3.x, desde el Gestor de placas. La guía oficial está en https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html.
3. Seleccionar el modelo exacto de placa y su puerto USB en Arduino IDE. El modelo todavía está pendiente de confirmar.
4. Desde el Gestor de bibliotecas instalar **WebSockets** de Markus Sattler y **ArduinoJson** versión 7.x.
5. Abrir `esp32_monitoring/esp32_monitoring.ino`. Conservar el nombre de esa carpeta: Arduino exige que coincida con el archivo principal.
6. Usar el `config.h` local ya creado; si no está presente en la copia, copiar `config.example.h` como `config.h`. El dominio `invernadero-api.jair77253.workers.dev`, el puerto 443 y el certificado raíz ya están configurados. Completar el nombre y contraseña de tu red Wi-Fi existente y la credencial propia del controlador. La placa se conecta como cliente; no crea una red Wi-Fi. No publicar `config.h`.
7. Registrar los sensores en el servidor y usar sus identificadores en `SENSOR_CONFIGURATION`. Cada sensor físico tiene su propio ID, incluso cuando dos miden humedad del suelo.
8. Completar los drivers de los sensores cuando se definan sus modelos, pines y calibración. Las funciones pendientes están en `sensores.cpp`: `initializeSensors()`, `sensorAvailable()` y `readSensorValue()`.
9. Pulsar **Verificar** para compilar y después **Subir** para grabar la placa. Abrir el Monitor serie a 115200 baudios, la velocidad indicada por `Serial.begin()` en el sketch para revisar su ejecución.

Por ahora se conocen dos sensores de humedad del suelo y uno de nivel de agua; queda por confirmar si habrá un cuarto. No se asignaron pines ni drivers porque todavía no se conocen los modelos. El programa no generará mediciones hasta completar esa parte. Además, el servidor debe permitir registrar dos sensores del mismo tipo; ese ajuste de base de datos sigue pendiente.

El envío histórico usa HTTPS aproximadamente cada 30 segundos. Una conexión WSS recibe solicitudes temporales para enviar un sensor con mayor frecuencia, según su capacidad física. Ese modo caduca localmente a los 60 segundos si no se renueva.

## Guardarlo en un repositorio

Desde PowerShell, estando en esta carpeta, puedes iniciar el repositorio:

```powershell
git init -b main
git add .
git status
git commit -m "Agregar cliente ESP32 para Arduino IDE"
```

Antes del commit, comprobar que `config.h` no aparece entre los archivos preparados. Después puedes crear un repositorio vacío en tu proveedor Git y conectar su URL mediante `git remote add origin <URL>`. No se ha creado ni publicado ningún repositorio remoto.

## Verificación pendiente

Este sketch todavía requiere compilación en Arduino IDE y pruebas en la placa real. Los modelos de sensores, conexiones eléctricas y calibración deben definirse antes de usar mediciones reales.

## Llevar el proyecto a tu laptop

Copiar la carpeta completa `esp32_monitoring`, incluidos todos los `.h`, `.cpp` y el `config.h` privado si ya lo completaste. Git no copia `config.h` porque está excluido; en ese caso crearlo desde la plantilla en la laptop. Instalar allí las placas y bibliotecas y abrir únicamente `esp32_monitoring.ino`: Arduino compila también los `.cpp` de la carpeta y carga un solo programa en la placa.

## HTTPS y WSS del servidor desplegado

HTTPS protege los envíos históricos; WSS protege el canal de órdenes WebSocket. Cloudflare presenta el certificado público del dominio y el ESP32 comprueba su cadena con `certificado_raiz.h`; no se necesita instalar un certificado privado en la placa ni usar `setInsecure()`.

El 24/10/2026 se verificó el nombre del dominio, la cadena hasta ISRG Root X1 y la compatibilidad TLS 1.2 desde este equipo. Una petición con identificador `ESP32HTTPClient` a `/controllers/me` llegó a la API y devolvió 401 al no incluir credencial, como corresponde. Esto no sustituye la prueba autenticada con el ESP32 real. Si Cloudflare cambia la autoridad certificadora, revisar y actualizar la raíz de confianza. La clave del controlador es independiente del certificado y todavía debe emitirse en el servidor.
