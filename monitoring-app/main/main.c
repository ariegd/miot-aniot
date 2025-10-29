/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include <math.h>

#include "shtc3.h"
#include "mock-flash.h"
#include "mock_wifi.h"
#include "button_monitor.h"
#include "console_shell.h"

static esp_timer_handle_t s_sensor_timer;

static const char *TAG = "APP";

/*** BASE de eventos propia para coordinar app ***/
ESP_EVENT_DECLARE_BASE(APP_EVENTS);
ESP_EVENT_DEFINE_BASE(APP_EVENTS);

typedef enum {
    EVT_SENSOR_TICK = 1,     // “tick” para simular lectura periódica de sensor
    EVT_BUTTON_PRESSED,
    EVT_CONSOLE_CMD_MONITOR
} app_event_id_t;

typedef enum { MODE_MONITOR = 0, MODE_CONSOLE } app_mode_t;
static app_mode_t g_mode = MODE_MONITOR;
static bool g_has_ip = false;
static esp_event_loop_handle_t app_loop = NULL;

/*** Parámetros por menuconfig (o defines) ***/
#ifndef CONFIG_MONITOR_PERIOD_SEC
#define CONFIG_MONITOR_PERIOD_SEC 5
#endif
#ifndef CONFIG_BUTTON_PERIOD_SEC
#define CONFIG_BUTTON_PERIOD_SEC 1
#endif
#ifndef CONFIG_BUTTON_GPIO
#define CONFIG_BUTTON_GPIO 2
#endif
#ifndef CONFIG_FLASH_CAPACITY_BYTES
#define CONFIG_FLASH_CAPACITY_BYTES (16*1024)   // capacidad de la mock-flash en bytes
#endif

static esp_timer_handle_t s_sensor_timer; //Temporidzados

static void do_fake_measure(float *t, float *h) {
    static float base_t = 24.0f, base_h = 45.0f;
    uint64_t us = esp_timer_get_time();
    *t = base_t + 0.3f * sinf((float)us / 1e6f);
    *h = base_h + 0.4f * cosf((float)us / 1e6f);
}

static void sensor_tick_cb(void *arg) {
    esp_event_post_to(app_loop, APP_EVENTS, EVT_SENSOR_TICK, NULL, 0, portMAX_DELAY);
}

static void drain_flash_if_possible(void) { //lee floats y los envía por WiFi
    if (!g_has_ip) return;
    while (getDataLeft() >= sizeof(float)) {
        float *pf = (float*)readFromFlash(sizeof(float));
        if (!pf) break;
        if (send_data_wifi(pf, sizeof(float)) != ESP_OK) { //si falla volvemos a insertar
            writeToFlash(pf, sizeof(float));
            free(pf);
            break;
        }
        free(pf);
    }
}

static void reconnect_cb(void *arg) {
    wifi_connect();
}

static void on_event(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) { //manejador de eventos
    if (base == WIFI_MOCK) {
        switch (id) {
            case WIFI_MOCK_EVENT_WIFI_CONNECTED:
                ESP_LOGI(TAG, "WiFi conectado (mock)");
                break;
            case WIFI_MOCK_EVENT_WIFI_GOT_IP:
                g_has_ip = true;
                ESP_LOGI(TAG, "IP conseguida (mock) ⇒ drenando mock-flash");
                drain_flash_if_possible();
                break;
            case WIFI_MOCK_EVENT_WIFI_DISCONNECTED:
                g_has_ip = false;
                ESP_LOGW(TAG, "WiFi desconectado (mock)");
                if (g_mode == MODE_MONITOR) {
                static esp_timer_handle_t s_reconnect_timer = NULL;
                if (!s_reconnect_timer) {
                    const esp_timer_create_args_t args = {
                        .callback = &reconnect_cb,
                        .name = "reconn"
                    };
                    ESP_ERROR_CHECK(esp_timer_create(&args, &s_reconnect_timer));
                    }
                    ESP_ERROR_CHECK(esp_timer_start_once(s_reconnect_timer, 3 * 1000000ULL));
                }
                break;
        }
        return;
    }

    if (base == APP_EVENTS) {
        switch (id) {
            case EVT_SENSOR_TICK: { //Enviamos temp y hum para el mockeo
                if (g_mode != MODE_MONITOR) break;
                float t, h;
                do_fake_measure(&t, &h);

                if (g_has_ip) { 
                    if (send_data_wifi(&t, sizeof(float)) != ESP_OK) {
                        writeToFlash(&t, sizeof(float));
                    }
                    if (send_data_wifi(&h, sizeof(float)) != ESP_OK) {
                        writeToFlash(&h, sizeof(float));
                    }
                } else {
                    writeToFlash(&t, sizeof(float));
                    writeToFlash(&h, sizeof(float));
                }
                break;
            }
            case EVT_BUTTON_PRESSED: //Desactivamos
                ESP_LOGI(TAG, "Botón pulsado ⇒ MODO CONSOLA");
                if (g_mode != MODE_CONSOLE) {
                    esp_timer_stop(s_sensor_timer);
                    wifi_disconnect();
                    g_has_ip = false;
                    g_mode = MODE_CONSOLE;
                    console_start(app_loop);
                }
                break;
            case EVT_CONSOLE_CMD_MONITOR: //Reconectamos
                ESP_LOGI(TAG, "Consola: volver a monitorización");
                console_stop();
                g_mode = MODE_MONITOR;
                esp_timer_start_periodic(s_sensor_timer, (uint64_t)CONFIG_MONITOR_PERIOD_SEC * 1000000ULL);
                wifi_connect(); 
                break;
        }
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    esp_event_loop_args_t loop_args = {
        .queue_size = 16,
        .task_name = "app_event_loop",
        .task_priority = 5,
        .task_stack_size = 4096,
        .task_core_id = tskNO_AFFINITY
    };
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &app_loop));

    ESP_ERROR_CHECK(esp_event_handler_instance_register_with( //Registrar handler para eventos de la app
        app_loop, APP_EVENTS, ESP_EVENT_ANY_ID, on_event, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with( //Registrar handler para eventos de la app
        app_loop, WIFI_MOCK, ESP_EVENT_ANY_ID, on_event, NULL, NULL));

    ESP_ERROR_CHECK(mock_flash_init(CONFIG_FLASH_CAPACITY_BYTES)); // Inicializar mock-flash (capacidad en BYTES)

    wifi_mock_init(app_loop);

    button_init(app_loop, APP_EVENTS, EVT_BUTTON_PRESSED,
                CONFIG_BUTTON_GPIO, CONFIG_BUTTON_PERIOD_SEC);      // Botón y consola, componentes
    console_init(app_loop, APP_EVENTS, EVT_CONSOLE_CMD_MONITOR);

    const esp_timer_create_args_t sens_args = {
        .callback = &sensor_tick_cb, .name = "sensor_tick"
    };
    esp_timer_create(&sens_args, &s_sensor_timer);

    g_mode = MODE_MONITOR;//MODO CONSOLA o MONITOR
    esp_timer_start_periodic(s_sensor_timer, (uint64_t)CONFIG_MONITOR_PERIOD_SEC * 1000000ULL);
    wifi_connect();

    ESP_LOGI(TAG, "Sistema iniciado en modo MONITOR (mock_wifi + mock_flash)");
    //esp_event_post_to(app_loop, APP_EVENTS, EVT_BUTTON_PRESSED, NULL, 0, portMAX_DELAY);
}
