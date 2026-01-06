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

// Incluye tus componentes
#include "blink_comp.h"
#include "sht3c_comp.h"
//#include "imu_comp.h"
#include "icm_comp.h"
#include "i2c_comp.h"
#include "sleep_comp.h"

static const char *TAG = "hello_world";

void app_main(void)
{
   // ESP_LOGI(TAG,  "Iniciando sistema...");
    //blink_start();
    //shtc3_start();
    // imu_start();
    //icm_start();
    
    ESP_LOGI(TAG, "Iniciando sistema...");

    // 1. Iniciar Blink (Independiente)
    blink_start();
    // 2. Iniciar Light Sleep (Independiente)
    sleep_start();

    // 2. Inicializar el Bus I2C Compartido
    i2c_master_bus_handle_t i2c_bus_handle = NULL;
    ESP_ERROR_CHECK(i2c_bus_init(&i2c_bus_handle));

    // 3. Iniciar sensores pasando el bus handle
    // Nota: El orden no es crítico, pero el bus debe existir antes
    if (i2c_bus_handle != NULL) {
        ESP_LOGI(TAG, "Iniciando SHTC3...");
        shtc3_start(i2c_bus_handle);

        ESP_LOGI(TAG, "Iniciando ICM42670...");
        icm_start(i2c_bus_handle);
    } else {
        ESP_LOGE(TAG, "No se pudo iniciar los sensores por fallo en I2C Bus");
    }

    // El main puede terminar aquí, las tareas (FreeRTOS tasks) seguirán corriendo
}
