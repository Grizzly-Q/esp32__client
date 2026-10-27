# ESP32: lectura y comunicación

Abrir `esp32_monitoring.ino` en Arduino IDE. Seleccionar ESP32 Dev Module e instalar el paquete ESP32 de Espressif 3.x y las bibliotecas DHT sensor library, Adafruit Unified Sensor, WebSockets de Markus Sattler y ArduinoJson 7.x.

Copiar `config.example.h` a `config.h` si no existe y completar allí el nombre y contraseña de Wi-Fi y la credencial privada del ESP32. No publicar `config.h` ni compartir credenciales. La placa necesita acceso a Internet y sincronización NTP. El certificado público está en `certificado_raiz.h`.

El firmware está configurado para el controlador 3: ID 7 para humedad ambiental, 8 para nivel de agua y 9 para humedad del suelo. El DHT11 también muestra temperatura por Serial, pero no se envía porque no hay un sensor de temperatura registrado para este controlador.

Los pines son GPIO34 para la salida AO del sensor de suelo, GPIO18 para el flotador superior, GPIO19 para el inferior y GPIO4 para los datos del DHT11. Los flotadores se interpretan como contactos entre GPIO y GND: LOW significa activo. Verificar que cada uno esté activo cuando el agua alcanza su altura. GPIO4 interviene en el arranque; la salida AO no debe exceder 3,3 V. El relé en GPIO23 no se controla.

El porcentaje de suelo es una referencia de demostración interpolada entre 4095 al aire (0 %) y 1104 en agua (100 %); no es una calibración de humedad real de suelo. El nivel usa valores representativos, no una medición exacta: 10 % si ninguno está activo, 45 % si solo está activo el inferior y 80 % si ambos están activos. Si solo está activo el superior, se rechaza esa lectura. Confirmar la orientación de los contactos antes de enviar datos. Estos valores quedan guardados como porcentajes en el servidor.

El Monitor serie usa 115200 baudios y muestra lecturas cada dos segundos. La comunicación envía histórico aproximadamente cada 30 segundos y atiende solicitudes temporales por WebSocket. La conexión en vivo debe ser solicitada por un cliente autorizado; la conexión abierta por sí sola no produce lecturas rápidas. Las funciones de comunicación y su certificado deben probarse en la placa y contra el despliegue real; no se ha verificado aquí la conexión de extremo a extremo.

[Contrato de comunicación](../docs/protocolo.md).
