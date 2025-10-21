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

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;

#ifdef CONFIG_BLINK_LED_STRIP

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#error "unsupported LED type"
#endif


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

void vTaskFunction(void *pvParameters)
{
    // 1. Inicialización de Periféricos
    printf("Inicializando I2C y sensor SHTC3...\n");
    init_i2c();
    printf("Inicialización completa.\n");

    // Variables para almacenar las lecturas
    float temperature_c;
    float humidity_rh;

    // 2. Bucle Principal de Lectura
    while (1) {
        printf("Realizando lectura del sensor SHTC3...\n");

        // *******************************************
        // Lógica de Lectura del Sensor (Asumiendo que tienes esta función)
        // *******************************************
        
        // La función shtc3_measure_temp_and_humidity() (o similar)
        // debe tomar las mediciones y almacenarlas en las variables.
        esp_err_t ret = shtc3_get_temp_and_hum_lpm(&tempSensor, &temperature_c, &humidity_rh);

        if (ret == ESP_OK) {
            printf("Temperatura: %.2f °C, Humedad: %.2f %%RH\n", temperature_c, humidity_rh);
        } else {
            // Manejo de errores si la lectura falla
            ESP_LOGE("SHTC3", "Error al leer el sensor: %s", esp_err_to_name(ret));
        }

        // 3. Esperar antes de la siguiente lectura
        // Usamos vTaskDelay para liberar la CPU y esperar 5 segundos (5000ms)
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}

void app_main(void)
{
    xTaskCreatePinnedToCore(vTaskFunction, "TaskName", 2048, NULL, 1, NULL, 0);
}
