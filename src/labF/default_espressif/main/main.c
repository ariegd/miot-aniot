#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <led_strip.h>
#include "icm42670.h"

static const char *TAG = "icm42670_app";

// --- CONFIGURACIÓN DIRECTA DE PINES (Ajusta según tu placa) ---
#define I2C_SCL_IO           8   // Cambia al pin que uses
#define I2C_SDA_IO           10    // Cambia al pin que uses
#define LED_GPIO             2     // Pin del LED RGB
#define LED_COUNT            1

static led_strip_handle_t led_strip;

static void led_init(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = LED_COUNT,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
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
    // 1. Inicializar el Bus I2C
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_SCL_IO,
        .sda_io_num = I2C_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    
    led_init();
    ESP_LOGI(TAG, "I2C Bus y LED inicializados");

    // 2. Crear instancia del Sensor
    icm42670_handle_t icm_handle = NULL;
    // El driver espera: bus, dirección I2C (0x68 por defecto), y puntero al handle
    ESP_ERROR_CHECK(icm42670_create(bus_handle, ICM42670_I2C_ADDRESS, &icm_handle));
    
    // 3. CONFIGURACIÓN DE PARÁMETROS
    //------------------- Se ha agregado, ya que en modo light sleep se desactiva ------------------------------
    icm42670_cfg_t sensor_cfg = {
        .acce_fs = ACCE_FS_16G,    // No lleva el prefijo ICM42670_
        .acce_odr = ACCE_ODR_100HZ, 
        .gyro_fs = GYRO_FS_2000DPS, 
        .gyro_odr = GYRO_ODR_100HZ,
    };

    ESP_ERROR_CHECK(icm42670_config(icm_handle, &sensor_cfg));

    // 4. ENCENDIDO DEL SENSOR (Solución al Modo Sleep)
    // Usamos la función icm42670_acce_set_pwr definida en tu .h
    // Pasamos el estado ACCE_PWR_LOWNOISE para máxima precisión
    ESP_ERROR_CHECK(icm42670_acce_set_pwr(icm_handle, ACCE_PWR_LOWNOISE));
    
    ESP_LOGI(TAG, "Sensor configurado y despertado con éxito");
    //------------------- FIN 3 ------------------------------

    icm42670_value_t acce_val;

    while (1)
    {
        // Leer valores del acelerómetro
        esp_err_t ret = icm42670_get_acce_value(icm_handle, &acce_val);
        
        if (ret == ESP_OK) {
            float fax = acce_val.x;
            float fay = acce_val.y;
            float faz = acce_val.z;

            ESP_LOGI(TAG, "Accel: X=%.2f Y=%.2f Z=%.2f", fax, fay, faz);

            // Lógica de color según inclinación
            if (faz > 0.5) {
                led_set_color(0, 255, 0);  // Verde: Boca arriba
            } else if (faz < -0.5) {
                led_set_color(255, 0, 0);  // Rojo: Boca abajo
            } else {
                led_set_color(0, 0, 255);  // Azul: Inclinado
            }
        } else {
            ESP_LOGE(TAG, "Error al leer datos");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void app_main()
{
    xTaskCreate(icm42670_test, "icm42670_test", 4096, NULL, 5, NULL);
}
