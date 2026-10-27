# Ejemplo para Arduino IDE

Abrir `esp32_monitoring.ino` en Arduino IDE. Es un ejemplo de integración del protocolo; aún no es un firmware completo para tus sensores porque faltan sus modelos, pines y calibración.

## Preparación

1. Instalar el soporte ESP32 de Espressif, versión 3.x, y seleccionar la placa correcta.
2. Instalar **WebSockets**, de Markus Sattler/Links2004, y **ArduinoJson**, versión 7.x.
3. Completar Wi-Fi y credencial en `config.h`; si falta, copiar la plantilla `config.example.h`. Dominio y puerto están preparados para la API desplegada. La raíz de confianza está en `certificado_raiz.h`. `config.h` está excluido de Git.
4. Configurar los identificadores reales de sensores y sus frecuencias mínimas. Para un sensor cuyo modelo exige dos segundos, usar `2000`, aunque Android pida `1000`.
5. Completar `initializeSensors()`, `sensorAvailable()` y `readSensorValue()` en `sensores.cpp` con los drivers y pines reales. Calibrar porcentajes; no enviar directamente el ADC como humedad.
6. Compilar para la placa seleccionada, cargar y probar primero HTTPS normal y después el modo en vivo.

Por defecto `sensorAvailable()` y `readSensorValue()` devuelven false. Esto impide publicar valores inventados y responde con rechazo explícito al solicitar un sensor sin driver. No hay pines, sensores simulados ni claves reales preconfiguradas.

## Comportamiento

- Conecta por WSS usando la cabecera Bearer y verifica el servidor con su certificado CA.
- Sincroniza hora mediante NTP; usa UTC para las lecturas y `millis()` para las concesiones.
- Confirma las órdenes con la frecuencia física admitida, y cancela el modo rápido al vencer el plazo local o al desconectarse.
- Incluye un identificador aleatorio de arranque y contador para no reutilizar identificadores tras reiniciar.
- Envía HTTPS histórico aproximadamente cada 30 segundos en otra tarea, por lo que ese POST no bloquea el procesamiento de órdenes.
- Reintenta el mismo JSON ante errores temporales. El ejemplo conserva un único lote pendiente en RAM; no es una cola durable y puede perder muestras durante cortes prolongados o reinicios.

Los drivers físicos deben respetar sus tiempos mínimos y tener tiempos de espera acotados: un driver que bloquea indefinidamente también bloquea el bucle de órdenes. La comprobación del plazo se repite después de leer el sensor para no transmitir una lectura rápida con concesión vencida.

No usar `setInsecure()` ni enviar la clave por Serial. Si el servidor cambia de certificado raíz, actualizar la configuración de confianza según corresponda. El registro de errores del ejemplo no imprime la cabecera Authorization ni la clave.

## Estado de verificación

El servidor y el protocolo se verifican con un ESP32 emulado mediante WebSocket real. La compilación y carga se realizarán en tu laptop mediante Arduino IDE después de completar los drivers. La separación de archivos no ha sido compilada para una placa en este equipo.

[Contrato de comunicación](../docs/protocolo.md) · [Biblioteca WebSockets](https://github.com/Links2004/arduinoWebSockets) · [ArduinoJson](https://arduinojson.org/).

## Distribución del programa

`esp32_monitoring.ino` inicializa Serial y llama a los módulos. `sensores.h/.cpp` expone la lectura física. `comunicacion.h/.cpp` mantiene Wi-Fi, la tarea histórica y el monitoreo temporal. `configuracion_tipos.h` define cada sensor configurado; `config.h` y su plantilla incluyen el certificado público de `certificado_raiz.h`. Mantener todos los archivos juntos al copiar el proyecto a la laptop.

Al arrancar con marcadores `CONFIGURAR` en Wi-Fi, credencial o IDs de sensores, el programa informa por Serial y detiene el inicio para que se complete la configuración. No hay credenciales reales incluidas.


## Lectura local de sensores, sin servidor

El sketch de `esp32_monitoring.ino` inicia `initializeSensors()` y llama a `processSensors()`. Esta última lee el ADC del FC-28, los dos contactos de nivel y la humedad/temperatura del DHT11; imprime el estado por Serial cada dos segundos. No inicia Wi-Fi, no crea tareas de telemetría y no controla relés.

Asignación según el cableado indicado: FC-28 AO en GPIO34 (D34), flotador superior provisionalmente en GPIO18 (D18), inferior en GPIO19 (D19) y datos del DHT11 en GPIO4 (D4). Confirmar físicamente cuál flotador está arriba y cuál abajo; si están invertidos, intercambiar los pines en `sensores.cpp`. El relé está conectado a GPIO23 (D23), pero este sketch todavía no lo controla. GPIO34 es solo entrada y pertenece a ADC1. GPIO4 es un pin de arranque: comprobar que el sensor no impida el arranque de la placa. La lectura de flotadores asume contactos pasivos conectados entre GPIO y GND, cerrados cuando indican activo; verificar con multímetro el estado y orientar según el fabricante. Confirmar que la salida AO del FC-28 no exceda 3.3 V.

La lectura del FC-28 se muestra como valor ADC crudo, no como porcentaje calibrado. `sensorAvailable()` y `readSensorValue()` siguen reservadas para la integración con la API y no participan en este modo local. Mantener la bomba y el circuito de potencia del relé desconectados durante estas lecturas.
