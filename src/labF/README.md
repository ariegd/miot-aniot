| Supported Targets | ESP32-C3  | Linux |
| ----------------- | -------- | -------- | ----- |
```
Máster IoT, curso 25-26
 	└── Autor
 		    └── Ariel Gámez <arielg01@ucm.es>
```
# Práctica Final ANIOT- Objetivos
```
▪ Sistema que monitorice temperatura, humedad y actividad sísmica (acelerómetro)
    - Monitoriza cada magnitud cada n segundos, configurable en menuconfig
▪ Cada nodo tendrá un identificador (número entero) configurable por menuconfig
▪ Uso de Wifi real (basado en el ejemplo de OTA)
▪ Gestión de bajo consumo
    - Debe pasar automáticamente a light sleep cuando detecte inactividad
    - El sistemas funcionará durante 10 minutos y pasará los siguientes 10 minutos en deep sleep
▪ OTA
    - Al pulsar el botón se conectará al servidor designado en menuconfig para bajar un binario con nombre dado (también en menuconfig)
▪ Escribe en el terminal periódicamente la temperatura y humedad
    - Cada m segundos. Se imprimirá la media de las últimas muestras
    - Si el acelerómetro detecta una perturbación, se escribirá un mensaje en pantalla
```

# aniot_sensor Práctica Final

1. Tarea FreeRTOS para imprimir "Temperatura: 31.19 °C, Humedad: 27.06 %RH".
2. Tarea FreeRTOS para imprimir "Sistema vivo... (En medio de Light Sleep intervals)".
3. Tarea FreeRTOS para imprimir ">>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.41) <<<" y "Accel: X=-0.73 Y=0.19 Z=0.40".
4. Tarea FreeRTOS para imprimir "Iniciando proceso de actualización OTA...".

## Como utilizar el aniot_sensor

Siga las instrucciones detalladas proporcionadas específicamente para este ejemplo:

1. `python server_ota.py`
Ejecutar el servidor OTA, con los pasos previos para crear el certificado.
        
2.  `I2C Configuration`
Conecte los pines `SCL` y `SDA` a los siguientes pines con resistencias pull-up adecuadas.

| Descripción | Defectos |
|-------------|----------|
 | GPIO number for `SCL` | "8" (`esp32c3`-based ESP-RS board)  |
 | GPIO number for `SDA` | "10" (`esp32c3`-based ESP-RS board) |
         
3.  `OTA Configuration`
- Colocar el identificación del Nodo `(2) Identificador único del Nodo (ID)`
- (https://192.168.1.39:8070/simple_ota.bin) firmware upgrade url endpoint

## Contenido de carpeta

Los proyectos ESP-IDF se compilan con CMake. La configuración de compilación del proyecto se encuentra en los archivos `CMakeLists.txt`, que contienen un conjunto de directivas e instrucciones que describen los archivos fuente y los destinos del proyecto (ejecutable, biblioteca o ambos).

A continuación se muestra una breve explicación de los archivos restantes en la carpeta del proyecto.

```
├── CMakeLists.txt
├── pytest_simple_ota.py      
├── main
│   ├── CMakeLists.txt
│   ├── idf_component.yml
│   ├── Kconfig.projbuild
│   └── aniot_sensor.c
├── server_certs
│   └── ca_cert.pem
├── components
│   ├── blink_comp
│   ├── i2c_comp
│   ├── icm_comp
│   ├── sht3c_comp
│   ├── shtc3
│   └── sleep_comp
└── README.md                  
```

## Build y Flash

Construya el proyecto y fórmelo en la placa, luego ejecute la herramienta de monitorización para ver la salida en serie::

* Run `python server_ota.py` 
* Run `idf.py set-target esp32c` to set target chip
* Run `idf.py -p PORT flash monitor` to build, flash and monitor the project

(To exit the serial monitor, type `Ctrl-]`.)

## Ejemplo de Salida

```
I (4851) button: IoT Button Version: 4.1.5
I (4851) aniot_sensor: Tarea OTA iniciada. Esperando pulsación del botón...
I (4861) i2c_comp: Inicializando I2C Master Bus...
I (4861) i2c_comp: I2C Bus inicializado correctamente
I (4871) aniot_sensor: Iniciando SHTC3...
I (4871) shtc3_comp: Configurando dispositivo SHTC3 en el bus compartido...
I (4881) shtc3_comp: Iniciando bucle de lectura SHTC3 (Periodo: 5 s, Promedio: 10)
I (4881) aniot_sensor: Iniciando ICM42670...
I (4891) shtc3_comp: Temperatura: 31.19 °C, Humedad: 27.06 %RH
I (4891) main_task: Returned from app_main()
I (4891) icm_comp: LED inicializado. Creando instancia ICM42670...
I (4901) icm_comp: Sensor configurado y despertado con éxito
I (4911) icm_comp: Sensor ICM42670 listo. Iniciando lecturas.
I (8401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (9891) shtc3_comp: Temperatura: 30.88 °C, Humedad: 26.64 %RH
I (13401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (14891) shtc3_comp: Temperatura: 31.14 °C, Humedad: 26.84 %RH
I (18401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (19891) shtc3_comp: Temperatura: 31.08 °C, Humedad: 26.65 %RH
I (23401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (24891) shtc3_comp: Temperatura: 31.14 °C, Humedad: 27.23 %RH
I (28401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (29891) shtc3_comp: Temperatura: 30.91 °C, Humedad: 26.87 %RH
I (33401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (34891) shtc3_comp: Temperatura: 31.08 °C, Humedad: 26.94 %RH
I (38401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (39891) shtc3_comp: Temperatura: 30.75 °C, Humedad: 26.98 %RH
I (43401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
W (44311) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.41) <<<
I (44311) icm_comp: Accel: X=-0.73 Y=0.19 Z=0.40
W (44711) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.92) <<<
I (44711) icm_comp: Accel: X=-0.60 Y=0.08 Z=0.06
I (44891) shtc3_comp: Temperatura: 30.72 °C, Humedad: 26.89 %RH
W (44911) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.63) <<<
I (44911) icm_comp: Accel: X=-1.05 Y=0.09 Z=-0.10
W (45111) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 1.06) <<<
I (45111) icm_comp: Accel: X=-0.21 Y=0.01 Z=0.03
W (45311) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 1.17) <<<
I (45311) icm_comp: Accel: X=-0.97 Y=-0.21 Z=-0.15
I (48401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (49891) shtc3_comp: [MEDIA de 10 muestras] Temp: 30.99 °C, Hum: 27.05 %RH
I (49891) shtc3_comp: Temperatura: 31.01 °C, Humedad: 28.41 %RH
W (51511) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.66) <<<
I (51511) icm_comp: Accel: X=-0.90 Y=-0.41 Z=0.35
W (51711) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 1.08) <<<
I (51711) icm_comp: Accel: X=-0.86 Y=0.07 Z=0.91
W (51911) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 1.00) <<<
I (51911) icm_comp: Accel: X=-0.17 Y=0.23 Z=1.06
W (52111) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 2.06) <<<
I (52111) icm_comp: Accel: X=-1.34 Y=0.03 Z=0.37
W (52311) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 2.20) <<<
I (52311) icm_comp: Accel: X=-0.07 Y=0.28 Z=1.05
W (53111) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.57) <<<
I (53111) icm_comp: Accel: X=-0.12 Y=0.21 Z=0.84
W (53311) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.40) <<<
I (53311) icm_comp: Accel: X=-0.27 Y=0.25 Z=1.06
I (53401) POWER_CYCLE: Sistema vivo... (En medio de Light Sleep intervals)
I (54091) aniot_sensor: Botón presionado: Solicitando OTA...
I (54091) aniot_sensor: Iniciando proceso de actualización OTA...
I (54101) aniot_sensor: Versión actual ejecutándose: 1.0.0
I (54101) aniot_sensor: Descargando desde: https://192.168.1.39:8070/simple_ota.bin
W (54111) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.68) <<<
I (54111) icm_comp: Accel: X=-0.75 Y=-0.04 Z=0.75
W (54321) icm_comp: >>> ¡PERTURBACIÓN DETECTADA! (Diff: 0.64) <<<
I (54321) icm_comp: Accel: X=-0.87 Y=-0.14 Z=0.32
I (54421) esp-x509-crt-bundle: Certificate validated
I (54781) esp_https_ota: Starting OTA...
```

## Problemas encontrados
### Corrige la indentación en Kconfig.projbuild
* El lenguaje Kconfig es muy estricto con los espacios. El texto de ayuda (help) debe estar indentado (tener espacios a la izquierda) respecto a la palabra help.

###  Si el acelerómetro detecta una perturbación, se escribirá un mensaje en pantalla
*   Para detectar una "perturbación" (un movimiento brusco o agitación), lo más efectivo es comparar las lecturas actuales con las lecturas anteriores. Si la diferencia (la variación) supera un cierto umbral, consideramos que ha ocurrido una perturbación.

###  Por qué falló el "float"
* Aunque algunos sistemas de Kconfig extendidos permiten float, el núcleo que utiliza ESP-IDF es más restrictivo para asegurar la compatibilidad entre diferentes versiones de Python y herramientas de compilación. Usar un string y convertirlo en tiempo de ejecución es la práctica recomendada por Espressif para estos casos.

###  Stack Overflow	`sht3c_comp.c`
* Solución: Aumenta el tamaño de la pila de la tarea en sht3c_comp.c. En tu archivo sht3c_comp.c, busca la función shtc3_start y cambia el valor de 2048 (o el que tengas) a 4096:
```
// Aumentamos a 4096 para dar espacio a los arrays de promedios y logs de float
    xTaskCreate(&leer_temperatura, "leer_temperatura", 4096, NULL, 2, NULL);
```

###  El mensaje waiting for download 
1.  que ves ahora indica que el ESP32 ha entrado en modo de arranque (bootloader) en lugar de ejecutar tu código. Esto sucede porque el hardware ha detectado que el pin de configuración de arranque (GPIO9 en el ESP32-C3) estaba en nivel bajo durante el reinicio.

2.  Aquí te explico por qué te está pasando ahora y cómo solucionarlo: ¿Por qué aparece ahora?
Existen tres razones comunes basadas en los cambios que has realizado:
* Conflicto de GPIO: En tu archivo main.c, el botón está configurado en el BOOT_BUTTON_NUM (GPIO9 para el C3). Si mantienes pulsado el botón físicamente mientras el chip se reinicia (o durante el proceso de Deep Sleep), el ESP32 entrará en modo descarga en lugar de arrancar normalmente.

* Estado del pin tras Deep Sleep: Como has implementado ciclos de sueño en sleep_comp.c, el chip se reinicia constantemente. Si hay ruido eléctrico o el circuito del botón mantiene el GPIO9 bajo durante ese microsegundo de arranque, verás ese mensaje.

* Configuración del monitor: El comando idf.py monitor a veces envía señales de reset a través del chip USB-UART que fuerzan este modo si la placa no tiene el circuito de auto-reset correctamente ajustado.

###  Procedimiento de actualización
* Para detener el bucle, debes asegurarte de que el binario que el ESP32 descarga e instala también tenga integrada la lógica del botón.

* En el ecosistema de ESP-IDF, la arquitectura recomendada es mantener el proyecto OTA como el "Proyecto Padre" (Project) y añadir las funcionalidades adicionales como componentes.

### ¿Por qué el proyecto OTA debe ser el Padre?
ESP-IDF está diseñado para que el archivo CMakeLists.txt de la raíz gestione las particiones, el cargador de arranque (bootloader) y la configuración de actualización. Si intentas meter un proyecto entero de OTA dentro de otro como "componente", tendrás muchos problemas de dependencias con esp_https_ota.

###  El chip sigue en modo Sleep.
* Parece que estás enfrentando el problema común donde el sensor está conectado y el bus I2C responde, pero los datos no cambian (valores fijos o en cero) porque el chip sigue en modo Sleep.

* En la serie ICM-42670 de Invensense, tras el encendido, el dispositivo inicia en modo de bajo consumo y es necesario configurar el registro de gestión de energía para activar el acelerómetro.


###  Gestión de bajo consumo
## Fase 1. Configuración Necesaria (menuconfig)
Para que el código funcione y realmente ahorre energía, debes activar estas opciones en idf.py menuconfig. Si no lo haces, esp_pm_configure fallará o no hará nada.

1. Habilitar Power Management:
* Component config -> Power Management -> [*] Support for power management

2. Habilitar Tickless Idle (Crucial para Auto Light Sleep):
* Component config -> FreeRTOS -> Kernel -> [*] Tickless Idle (Suele estar activado por defecto).

3. Configurar Frecuencia:
* Asegúrate de que Max CPU Frequency coincide con lo que pones en el código (160MHz es estándar para C3).

## Fase 2. Explicación del comportamiento
1. Los 10 minutos "Activos":
* Durante este tiempo, el sistema NO está consumiendo energía al máximo todo el rato.
* Gracias a esp_pm_configure y el bucle vTaskDelay(5000), el ESP32 pasará el 99% de esos 10 minutos en Light Sleep (ahorrando batería), despertándose solo brevemente para imprimir el log "Sistema vivo..." y volver a dormir.
* Nota sobre USB: Durante los micro-cortes de Light Sleep, el USB nativo del C3 se desconectará. Es normal. Si necesitas depurar continuamente, tendrás que desactivar light_sleep_enable temporalmente.

2. Los 10 minutos "Deep Sleep":
* Cuando salta el timer cycle_timer_callback, el sistema se apaga casi totalmente (consumo mínimo absoluto).
* Al despertar, el código empieza desde app_main de nuevo (reset), no continúa donde se quedó.


###  ESP-IDF 5.x usa el sistema de componentes gestionados.
## Error de CMake aparece cuando dos componentes distintos con el mismo nombre lógico (icm42670) 
* esp-idf-lib__icm42670 (el de UncleRus / esp-idf-lib)
* espressif__icm42670 (el oficial de Espressif)

ESP-IDF no sabe cuál usar, así que falla.

## Solución: Alternativa: renombrar el componente del registro que quieras usar localmente
Si no quieres copiar todo, clona el repo `esp-idf-lib/icm42670` dentro de `components/` y cambia su name en `idf_component.yml`. Esto lo convierte en componente local y evita la gestión por registro.

## Crear un componente i2c_comp el Circuito integrado (I2C) compartiendo
* , incluir únicamente el driver nuevo. Adactar el hello_world_main.c para utilizar el i2c_comp que llame a sht3c_comp.c y icm_comp.c. Tener en cuenta que el icm_comp.c tiene la dependencia espressif/icm42670 que internamente utiliza el driver nuevo (driver/i2c_master.h)
* El objetivo es crear un "Singleton" o un recurso compartido para el bus I2C, inicializarlo una sola vez en el main, y pasar ese handle (manejador) a los componentes sht3c y icm.
