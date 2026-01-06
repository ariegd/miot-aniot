#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

// Definición de pines (Centralizada aquí)
#define I2C_MASTER_SCL_IO           8
#define I2C_MASTER_SDA_IO           10
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000

/**
 * @brief Inicializa el bus I2C maestro y devuelve el handle
 * * @param[out] bus_handle Puntero donde se guardará el handle del bus
 * @return esp_err_t ESP_OK si todo fue bien
 */
esp_err_t i2c_bus_init(i2c_master_bus_handle_t *bus_handle);
