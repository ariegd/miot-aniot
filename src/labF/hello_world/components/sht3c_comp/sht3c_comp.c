#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "shtc3.h" // <---

static const char *TAG = "shtc3_comp";

  // Variables para almacenar las lecturas
  float temperature_c;
  float humidity_rh;
  esp_err_t ret;

shtc3_t tempSensor;
i2c_master_bus_handle_t bus_handle;

void init_i2c(void) {
 // uint16_t id;
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

void leer_temperatura(void *pvParameters)
{
    // 1. Inicialización de Periféricos
    ESP_LOGI(TAG,  "Inicializando I2C y sensor SHTC3..");
    init_i2c();
   ESP_LOGI(TAG, "Inicialización completa.");

    // Variables para almacenar las lecturas
    float temperature_c;
    float humidity_rh;

    // 2. Bucle Principal de Lectura
    while (1) {
        esp_err_t ret = shtc3_get_temp_and_hum_lpm(&tempSensor, &temperature_c, &humidity_rh);

        if (ret == ESP_OK) {
           ESP_LOGI(TAG, "Temperatura: %.2f °C, Humedad: %.2f %%RH", temperature_c, humidity_rh);
        } else {
            // Manejo de errores si la lectura falla
            ESP_LOGE("SHTC3", "Error al leer el sensor: %s", esp_err_to_name(ret));
        }

        // 3. Esperar antes de la siguiente lectura
        // Usamos vTaskDelay para liberar la CPU y esperar 5 segundos (5000ms)
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}

void shtc3_start(void)
{
    xTaskCreate(&leer_temperatura, "leer_temperatura", 2048, NULL, 2, NULL);
}
