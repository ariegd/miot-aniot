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

#include "esp_log.h"
//#include "esp_pm.h"
#include "iot_button.h"
#include "esp_sleep.h"
#include "esp_idf_version.h"
#include "button_gpio.h"

// Definición del pin GPIO (mantener el 0 para el botón BOOT del DevKitC)
#define BUTTON_GPIO_PIN  0 
#define BUTTON_ACTIVE_LEVEL 0

static const char *TAG = "OTA_TASK_APP";

static void button_short_press_cb(void *arg, void *data) 
{
    ESP_LOGI(TAG, "¡Botón Presionado! Enviando mensaje a la consola.");
    ESP_LOGI(TAG, "--- MENSAJE: Se ha detectado una pulsación corta en el botón (GPIO %d) ---\n", BUTTON_GPIO_PIN);
}

void button_short_press(void *pvParameter) {
// 1. Configuración de Tiempos
    button_config_t btn_cfg = {
        .long_press_time = 1000, 
        .short_press_time = 300,  
    };

    // 2. Configuración Específica de GPIO
    button_gpio_config_t gpio_cfg = {
        .gpio_num = BUTTON_GPIO_PIN,
        .active_level = BUTTON_ACTIVE_LEVEL, // O 0
    };
    
    button_handle_t btn_handle = NULL;
    esp_err_t ret;

    ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn_handle);
    
    if (ret == ESP_OK && btn_handle) {
        ESP_LOGI(TAG, "Botón en GPIO %d inicializado con éxito.", BUTTON_GPIO_PIN);

        // 3. REGISTRO CORREGIDO CON 5 ARGUMENTOS
        ret = iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, NULL, button_short_press_cb, NULL);
        //                                                      ↑ Este era el argumento faltante
        ESP_ERROR_CHECK(ret);
        
    } else {
        ESP_LOGE(TAG, "Error al inicializar el botón. Código de error: %d", ret);
    }
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void hello_task(void *pvParameter) {
    ESP_LOGI(TAG, "Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
   ESP_LOGI(TAG,"This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
   ESP_LOGI(TAG, "silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
       ESP_LOGE(TAG, "Get flash size failed");
        return;
    }

    ESP_LOGI(TAG, "%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

   ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        ESP_LOGI(TAG, "Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

void app_main(void)
{
    xTaskCreate( &hello_task, "hello_task", 2048, NULL, 4, NULL );
    xTaskCreate( &button_short_press, "button_short_press", 2048, NULL, 5, NULL );
}
