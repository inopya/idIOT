# idIOT
## implementing dummy iot


Añadiendo un poco de IoT (no invasivo) a electrodomesticos "torpes".

Muchos de nuestros electrodomesticos "menos modernos" nos indican su funcionamiento (o termino de proceso) mediante una luz que se enciende,se apaga, o parpadea...
Podemos obtener notificaciones de ellos mediante la lectura de dichas señales luminosas.
Para ello podemos usar una simple LDR como sensor de luz y un bot de telegram implementado en un ESP8266 o un ESP32.
(Ampliable a otro tipo de informaciones que deseamos medir como 'beeps', cambios de color de una luz...)


## Esquema de montaje
![](./imagenes/wemos-idIOT_mini.png)


## Firmware
La carpeta **SRC** contiene el firmware y el fichero **librerias.rar** las librerias necesarias, por si otras versiones (pasadas o futuras) pudiesen no ser compatibles.

En el fichero **constantes.h** debe configurarse el **TOKEN** del bot de telegram y el **chad_id** del usuario 'administrador' (para restringir el acceso a usuarios no deseados)
Con la variable **FLAG_public_access** del fichero **variables.h** se puede permitir el acceso publico al boot.
Esta 'bandera' puede cambiase mediante el comando **/public** (no mostrado en el menu de ayuda) solo accesible para el usuario administrador

## Freecad
En la carpeta **Frecad** se encuentra el modelo 3D de un soporte para la LDR

![](./imagenes/todo.png)

![](./imagenes/soporte-LDR.png)




