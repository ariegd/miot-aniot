#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <led_strip.h>
#include <icm42670.h>

static const char *TAG = "imu_comp";

#define PORT 0
#if defined(CONFIG_EXAMPLE_I2C_ADDRESS_GND)
#define I2C_ADDR ICM42670_I2C_ADDR_GND
#endif
#if defined(CONFIG_EXAMPLE_I2C_ADDRESS_VCC)
#define I2C_ADDR ICM42670_I2C_ADDR_VCC
#endif

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

#define LED_GPIO 2   // tu pin del LED
#define LED_COUNT 1

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
    icm42670_t dev = {0};
    ESP_ERROR_CHECK(icm42670_init_desc(&dev, I2C_ADDR, PORT, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));
    ESP_ERROR_CHECK(icm42670_init(&dev));

    ESP_ERROR_CHECK(icm42670_set_accel_pwr_mode(&dev, ICM42670_ACCEL_ENABLE_LN_MODE));
    ESP_ERROR_CHECK(icm42670_set_accel_fsr(&dev, ICM42670_ACCEL_RANGE_2G));
    ESP_ERROR_CHECK(icm42670_set_accel_odr(&dev, ICM42670_ACCEL_ODR_200HZ));

    led_init();

    int16_t ax, ay, az;
    while (1)
    {
        ESP_ERROR_CHECK(icm42670_read_raw_data(&dev, ICM42670_REG_ACCEL_DATA_X1, &ax));
        ESP_ERROR_CHECK(icm42670_read_raw_data(&dev, ICM42670_REG_ACCEL_DATA_Y1, &ay));
        ESP_ERROR_CHECK(icm42670_read_raw_data(&dev, ICM42670_REG_ACCEL_DATA_Z1, &az));

        float fax = ax / 16384.0f;
        float fay = ay / 16384.0f;
        float faz = az / 16384.0f;

        ESP_LOGI(TAG, "Accel X=%.2f Y=%.2f Z=%.2f g", fax, fay, faz);

        if (faz > 0.5) {
            led_set_color(0, 255, 0);  // verde
        } else if (faz < -0.5) {
            led_set_color(255, 0, 0);  // rojo
        } else {
            led_set_color(0, 0, 255);  // azul
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void imu_start(void)
{
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(&icm42670_test,  "icm42670_test", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);
}
