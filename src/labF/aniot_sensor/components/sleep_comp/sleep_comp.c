/*
 * main.c - Gestión de energía cíclica (10m ON / 10m OFF) con Auto Light Sleep
 */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "driver/gpio.h"

// --- CONFIGURACIÓN DE TIEMPOS ---
#define ACTIVE_DURATION_MIN     CONFIG_ACTIVE_DURATION_MIN
#define DEEP_SLEEP_DURATION_MIN CONFIG_DEEP_SLEEP_DURATION_MIN

// Conversión a microsegundos para los timers
#define DEEP_SLEEP_US  (DEEP_SLEEP_DURATION_MIN * 60 * 1000000ULL)
#define ACTIVE_DURATION_US (ACTIVE_DURATION_MIN * 60 * 1000000ULL)

static const char *TAG = "POWER_CYCLE";

// Callback que se ejecuta cuando pasan los 10 minutos de actividad
void cycle_timer_callback(void* arg)
{
    ESP_LOGW(TAG, "Tiempo de actividad (%d min) finalizado.", ACTIVE_DURATION_MIN);
    ESP_LOGW(TAG, "Entrando en DEEP SLEEP por %d minutos...", DEEP_SLEEP_DURATION_MIN);
    
    // Es importante vaciar el buffer del puerto serie antes de dormir
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(100));

    // 1. Configurar el disparador para despertar (Timer)
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_US);

    // 2. Entrar en Deep Sleep (el sistema se reiniciará al volver)
    esp_deep_sleep_start();
}

void power_manager_init(void)
{
#if CONFIG_PM_ENABLE
    // Configuración para permitir Light Sleep automático (Dynamic Frequency Scaling)
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,    // Frecuencia máxima (normalmente 160MHz)
        .min_freq_mhz = 40,     // Frecuencia mínima en inactividad (ajustar según XTAL)
        .light_sleep_enable = true // ¡CLAVE! Permite apagar periféricos en IDLE
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
    ESP_LOGI(TAG, "Power Management configurado: Auto Light Sleep HABILITADO");
#else
    ESP_LOGE(TAG, "CONFIG_PM_ENABLE no está activo en menuconfig");
#endif
}

void sleep_task(void *arg)
{
    // --- SEGURIDAD ANTI-BRICK ---
    // Esperamos 3 segundos antes de activar nada para dar tiempo al USB a conectarse
    // y permitir flashear si hubo un error previo.
    printf("Arrancando... Esperando 3s de seguridad...\n");
    vTaskDelay(pdMS_TO_TICKS(3000));

    // 1. Verificar razón del despertar (Boot normal o vuelta de Deep Sleep)
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause == ESP_SLEEP_WAKEUP_TIMER) {
        ESP_LOGI(TAG, "Despertado por Timer tras Deep Sleep");
    } else {
        ESP_LOGI(TAG, "Arranque normal (Power On / Reset)");
    }

    // 2. Iniciar gestión de energía (Auto Light Sleep)
    power_manager_init();

    // 3. Configurar el timer de software para apagar el sistema en 10 minutos
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &cycle_timer_callback,
            .name = "deep_sleep_timer"
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    
    // Iniciar el timer (se dispara una vez pasados los microsegundos definidos)
    ESP_ERROR_CHECK(esp_timer_start_once(periodic_timer, ACTIVE_DURATION_US));

    ESP_LOGI(TAG, "Sistema activo. Se apagará en %d minutos.", ACTIVE_DURATION_MIN);

    // 4. Bucle principal
    // Como hemos activado 'light_sleep_enable = true', cuando FreeRTOS
    // entra en este bucle y hace vTaskDelay, el chip entra en Light Sleep automáticamente.
    while (1) {
        // Simular una pequeña tarea cada cierto tiempo
        ESP_LOGI(TAG, "Sistema vivo... (En medio de Light Sleep intervals)");
        
        // Dormir 5 segundos (Durante este tiempo, la CPU entra en Light Sleep real)
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void sleep_start(void)
{
    xTaskCreate(&sleep_task, "sleep_task", 2048, NULL, 5, NULL);
}
