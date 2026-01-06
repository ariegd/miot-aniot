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
