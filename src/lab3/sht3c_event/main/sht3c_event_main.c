/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "sensor_events.h"
ESP_EVENT_DEFINE_BASE(SENSOR_EVENT);

#include "shtc3.h"  // <-- componente para temperatura
#include "esp_log.h"  // <-- LOGGING

static const char *TAG = "EventSht3c";

static void hello_world(void) 
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

/* A ver si funciona el sensor*/
shtc3_t tempSensor;
  i2c_master_bus_handle_t bus_handle;

void init_i2c(void) {
i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = 8, // señal
        .sda_io_num = 10,  // dato
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
   shtc3_init(&tempSensor, bus_handle, 0x70);
}

void p3MeasureHumidityTask(void)
{
    ESP_LOGI(TAG,"Inicializando I2C y sensor SHTC3..." );
    init_i2c();
    ESP_LOGI(TAG, "Inicialización completa");

    float temperature_c;
    float humidity_rh;

    while (1) {
        ESP_LOGI(TAG, "Realizando lectura del sensor SHTC3.");
        
        esp_err_t ret = shtc3_get_temp_and_hum_lpm(&tempSensor, &temperature_c, &humidity_rh);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Temperatura: %.2f °C, Humedad: %.2f %%RH",  temperature_c, humidity_rh);
        } else {
            // Manejo de errores si la lectura falla
            ESP_LOGE("SHTC3", "Error al leer el sensor: %s", esp_err_to_name(ret));
        }
        
        // Obtener la prioridad de las tareas
        UBaseType_t xPriorityTask = uxTaskPriorityGet(NULL);           // Esta tarea
         ESP_LOGI(TAG, "Priority of Task 2 is %d", xPriorityTask);
        
        vTaskDelay(pdMS_TO_TICKS(3000)); 
    }
}

esp_event_loop_handle_t hum_event_loop;

void hum_event_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    hum_sample_t* sample = (hum_sample_t*)event_data;
    ESP_LOGI("HANDLER", "Nueva muestra: %.2f °C, %.2f %%RH", sample->temperature, sample->humidity);
}

void p4MeasureHumidityTask(void *pvParameters)
{
    uint32_t periodo_ms = *(uint32_t*)pvParameters;

    ESP_LOGI(TAG,"Inicializando I2C y sensor SHTC3..." );
    init_i2c();
    ESP_LOGI(TAG, "Inicialización completa");

    float temperature_c;
    float humidity_rh;

    while (1) {
        ESP_LOGI(TAG, "Realizando lectura del sensor SHTC3.");
        esp_err_t ret = shtc3_get_temp_and_hum_lpm(&tempSensor, &temperature_c, &humidity_rh);

        if (ret == ESP_OK) {
            hum_sample_t sample = {
                .temperature = temperature_c,
                .humidity = humidity_rh
            };
            // Postear el evento al event loop
            esp_event_post_to(hum_event_loop, HUM_EVENT_BASE, HUM_EVENT_NEW_SAMPLE, &sample, sizeof(sample), portMAX_DELAY);
        } else {
            ESP_LOGE("SHTC3", "Error al leer el sensor: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(periodo_ms));
    }
}

void app_main(void) {
    // Crear el event loop
    esp_event_loop_args_t loop_args = {
        .queue_size = 5,
        .task_name = "hum_event_task",
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 4096, // <-- By increasing the stack size
        .task_core_id = tskNO_AFFINITY
    };
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &hum_event_loop));

    // Registrar el handler
    ESP_ERROR_CHECK(esp_event_handler_register_with(hum_event_loop, HUM_EVENT_BASE, HUM_EVENT_NEW_SAMPLE, hum_event_handler, NULL));

    // Crear la tarea muestreadora, pasando el periodo como argumento
    static uint32_t periodo_ms = 3000;
    xTaskCreate(p4MeasureHumidityTask, "p3MeasureHumidityTask", 4096, &periodo_ms, 5, NULL);
}

