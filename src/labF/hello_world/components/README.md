#  El chip sigue en modo Sleep.
* Parece que estás enfrentando el problema común donde el sensor está conectado y el bus I2C responde, pero los datos no cambian (valores fijos o en cero) porque el chip sigue en modo Sleep.

* En la serie ICM-42670 de Invensense, tras el encendido, el dispositivo inicia en modo de bajo consumo y es necesario configurar el registro de gestión de energía para activar el acelerómetro.


# Gestión de bajo consumo
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


# ESP-IDF 5.x usa el sistema de componentes gestionados.
## Error de CMake aparece cuando dos componentes distintos con el mismo nombre lógico (icm42670) 
* esp-idf-lib__icm42670 (el de UncleRus / esp-idf-lib)
* espressif__icm42670 (el oficial de Espressif)

ESP-IDF no sabe cuál usar, así que falla.

## Solución: Alternativa: renombrar el componente del registro que quieras usar localmente
Si no quieres copiar todo, clona el repo `esp-idf-lib/icm42670` dentro de `components/` y cambia su name en `idf_component.yml`. Esto lo convierte en componente local y evita la gestión por registro.

## Crear un componente i2c_comp el Circuito integrado (I2C) compartiendo
* , incluir únicamente el driver nuevo. Adactar el hello_world_main.c para utilizar el i2c_comp que llame a sht3c_comp.c y icm_comp.c. Tener en cuenta que el icm_comp.c tiene la dependencia espressif/icm42670 que internamente utiliza el driver nuevo (driver/i2c_master.h)
* El objetivo es crear un "Singleton" o un recurso compartido para el bus I2C, inicializarlo una sola vez en el main, y pasar ese handle (manejador) a los componentes sht3c y icm.
