#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/i2c_master.h>
#include <led_strip.h>
#include "icm42670.h"
#include "icm_comp.h"

static const char *TAG = "icm_comp";

#define LED_GPIO    2
#define LED_COUNT   1

static led_strip_handle_t led_strip;

static void led_init(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = LED_COUNT,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}

static void led_set_color(uint8_t r, uint8_t g, uint8_t b) {
    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

void icm42670_test(void *pvParameters)
{
    // Recuperamos el handle del bus que pasamos al crear la tarea
    i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t) pvParameters;

    led_init();
    ESP_LOGI(TAG, "LED inicializado. Creando instancia ICM42670...");

    icm42670_handle_t icm_handle = NULL;
    
    // Aquí usamos el bus compartido para crear el dispositivo
    // Nota: El driver espressif/icm42670 usa internamente i2c_master_bus_add_device
    ESP_ERROR_CHECK(icm42670_create(bus_handle, ICM42670_I2C_ADDRESS, &icm_handle));

    icm42670_value_t acce_val;
    ESP_LOGI(TAG, "Sensor ICM42670 listo. Iniciando lecturas.");

    while (1)
    {
        esp_err_t ret = icm42670_get_acce_value(icm_handle, &acce_val);
        
        if (ret == ESP_OK) {
            float fax = acce_val.x;
            float fay = acce_val.y;
            float faz = acce_val.z;

            ESP_LOGI(TAG, "Accel: X=%.2f Y=%.2f Z=%.2f", fax, fay, faz);

            if (faz > 0.5) {
                led_set_color(0, 255, 0);  // Verde
            } else if (faz < -0.5) {
                led_set_color(255, 0, 0);  // Rojo
            } else {
                led_set_color(0, 0, 255);  // Azul
            }
        } else {
            ESP_LOGE(TAG, "Error al leer datos ICM");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void icm_start(i2c_master_bus_handle_t bus_handle)
{
    // Pasamos el bus_handle como parámetro a la tarea
    xTaskCreate(icm42670_test, "icm42670_test", 4096, (void *)bus_handle, 5, NULL);
}
