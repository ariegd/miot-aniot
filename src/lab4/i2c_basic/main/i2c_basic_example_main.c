/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* i2c - Simple Example

   Simple I2C example that shows how to initialize I2C
   as well as reading and writing from and to registers for a sensor connected over I2C.

   The sensor used in this example is a MPU9250 inertial measurement unit.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_check.h" /*GhatGPT*/


static const char *TAG = "example";

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL       /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA       /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          CONFIG_I2C_MASTER_FREQUENCY /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define MPU9250_SENSOR_ADDR         0x68        /*!< Address of the MPU9250 sensor */
#define MPU9250_WHO_AM_I_REG_ADDR   0x75        /*!< Register addresses of the "who am I" register */
#define MPU9250_PWR_MGMT_1_REG_ADDR 0x6B        /*!< Register addresses of the power management register */
#define MPU9250_RESET_BIT           7

/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
static esp_err_t mpu9250_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t mpu9250_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief i2c master initialization
 */
static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
       // .device_address = MPU9250_SENSOR_ADDR,
        .device_address = 0x40,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

/*ChatGPT si7021*/
static esp_err_t si7021_measure_humidity(i2c_master_dev_handle_t dev, float *humidity)
{
    uint8_t cmd = 0xF5;  // Measure Humidity (No Hold Master)
    uint8_t data[2];

    // Enviar comando
    ESP_RETURN_ON_ERROR(i2c_master_transmit(dev, &cmd, 1, 1000 / portTICK_PERIOD_MS),
                        TAG, "Error enviando comando humedad");

    // El sensor tarda aprox 20-30ms en convertir
    vTaskDelay(pdMS_TO_TICKS(30));

    // Leer 2 bytes
    ESP_RETURN_ON_ERROR(i2c_master_receive(dev, data, 2, 1000 / portTICK_PERIOD_MS),
                        TAG, "Error recibiendo datos humedad");

    uint16_t raw = (data[0] << 8) | data[1];
    *humidity = ((125.0 * raw) / 65536.0) - 6.0;

    return ESP_OK;
}

/*ChatGPT si7021*/
static esp_err_t si7021_measure_temperature(i2c_master_dev_handle_t dev, float *temp)
{
    uint8_t cmd = 0xF3;  // Measure Temperature (No Hold Master)
    uint8_t data[2];

    ESP_RETURN_ON_ERROR(i2c_master_transmit(dev, &cmd, 1, 1000 / portTICK_PERIOD_MS),
                        TAG, "Error enviando comando temperatura");

    vTaskDelay(pdMS_TO_TICKS(30));

    ESP_RETURN_ON_ERROR(i2c_master_receive(dev, data, 2, 1000 / portTICK_PERIOD_MS),
                        TAG, "Error recibiendo datos temperatura");

    uint16_t raw = (data[0] << 8) | data[1];
    *temp = ((175.72 * raw) / 65536.0) - 46.85;

    return ESP_OK;
}

void prueba_si7021(void)
{
  float humidity, temperature;
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;

    // Inicializar bus I2C (igual que tu código)
    i2c_master_init(&bus_handle, &dev_handle);
    ESP_LOGI(TAG, "I2C inicializado");

    // Leer humedad
    ESP_ERROR_CHECK(si7021_measure_humidity(dev_handle, &humidity));
    ESP_LOGI(TAG, "Humedad: %.2f %%", humidity);

    // Leer temperatura
    ESP_ERROR_CHECK(si7021_measure_temperature(dev_handle, &temperature));
    ESP_LOGI(TAG, "Temperatura: %.2f C", temperature);

    // Desinicializar
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
    ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
    ESP_LOGI(TAG, "I2C desinicializado");
}

void app_main(void)
{
  /*  uint8_t data[2];
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    i2c_master_init(&bus_handle, &dev_handle);
    ESP_LOGI(TAG, "I2C initialized successfully");

    ESP_ERROR_CHECK(mpu9250_register_read(dev_handle, MPU9250_WHO_AM_I_REG_ADDR, data, 1));
    ESP_LOGI(TAG, "WHO_AM_I = %X", data[0]);

    ESP_ERROR_CHECK(mpu9250_register_write_byte(dev_handle, MPU9250_PWR_MGMT_1_REG_ADDR, 1 << MPU9250_RESET_BIT));

    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
    ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
    ESP_LOGI(TAG, "I2C de-initialized successfully");*/
     prueba_si7021();
}
