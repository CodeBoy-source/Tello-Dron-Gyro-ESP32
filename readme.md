# Prototipo de guante para el control de Tello.

Esta es la versión que utiliza datos del acelerómetro, más limpio que
el uso del giroscopio cuyo código se encuentra en otra rama, para
el control del drone Tello por medio de ESP32.

La esencia está en el uso del SDK-3 del dron, que se basa en paquetes
UDP con un cuerpo que lleva una cadena que representa un commando.

Para replicar el uso del guante es necesario:
- ESP-32 de 38 pins.
- GY521, accelerómetro de kit-arduino.
- Muchos cables.

Además, sería necesario cambiar los datos del código como lo sería el
nombre del wifi que genera el drone a probar en particular.

Para ejecutar es necesario mínimo 2 cables del guante, ya que para
enviar el comando principal `takeoff/land` es necesario el uso de
un cable, que sería el pulgar, que está a `5V` y cuyo ground 
sería el `pin 25`.

Sin embargo, se recomienda conectar todos los pines:
```
  // subir
  pinMode(4, INPUT_PULLUP);
  // Bajar
  pinMode(15, INPUT_PULLUP);
  // Despegar / Aterrizar
  pinMode(25, INPUT_PULLUP);
  // Flip
  pinMode(33, INPUT_PULLUP);
```
Have fun!
