/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "shtc3.h" // <---

  // Variables para almacenar las lecturas
  float temperature_c;
  float humidity_rh;
  esp_err_t ret;


/* A ver si funciona el sensor*/
shtc3_t tempSensor;
  i2c_master_bus_handle_t bus_handle;

void init_i2c(void) {
 uint16_t id;
i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = 8,
        .sda_io_num = 10,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

   shtc3_init(&tempSensor, bus_handle, 0x70);
}

void tareaInicial(void *pvParameters) {
    while (1) {
        printf("Tarea Inicial\n");


        if (ret == ESP_OK) {
            printf("Temperatura: %.2f °C, Humedad: %.2f %%RH\n", temperature_c, humidity_rh);
        } else {
            // Manejo de errores si la lectura falla
            ESP_LOGE("SHTC3", "Error al leer el sensor: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

void muestreadora(void *pvParameters)
{
    // 1. Inicialización de Periféricos
    printf("Inicializando I2C y sensor SHTC3...\n");
    init_i2c();
    printf("Inicialización completa.\n");

    // 2. Bucle Principal de Lectura
    while (1) {
        printf("Realizando lectura del sensor SHTC3...\n");

        ret = shtc3_get_temp_and_hum_lpm(&tempSensor, &temperature_c, &humidity_rh);


        // 3. Esperar antes de la siguiente lectura
        vTaskDelay(pdMS_TO_TICKS(7000)); 
    }
}

void app_main(void)
{
    xTaskCreatePinnedToCore(tareaInicial, "tareaInicial", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(muestreadora, "muestreadora", 2048, NULL, 2, NULL, 0);
}
