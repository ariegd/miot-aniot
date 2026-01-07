#include "i2c_comp.h"
#include "esp_log.h"

static const char *TAG = "i2c_comp";

esp_err_t i2c_bus_init(i2c_master_bus_handle_t *bus_handle)
{
    ESP_LOGI(TAG, "Inicializando I2C Master Bus...");

    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&i2c_bus_config, bus_handle);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C Bus inicializado correctamente");
    } else {
        ESP_LOGE(TAG, "Fallo al inicializar I2C Bus: %s", esp_err_to_name(ret));
    }

    return ret;
}
