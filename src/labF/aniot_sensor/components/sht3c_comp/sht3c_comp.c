#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "shtc3.h"
#include "sht3c_comp.h" // Asegúrate de incluir tu header actualizado

static const char *TAG = "shtc3_comp";

// Objeto del sensor (global en este archivo)
shtc3_t tempSensor;

void leer_temperatura(void *pvParameters)
{
    // Variables para almacenar las lecturas
    float temperature_c;
    float humidity_rh;

    ESP_LOGI(TAG, "Iniciando bucle de lectura SHTC3...");

    while (1) {
        // Usamos la función lpm (Low Power Mode) o normal según tu driver
        esp_err_t ret = shtc3_get_temp_and_hum_lpm(&tempSensor, &temperature_c, &humidity_rh);

        if (ret == ESP_OK) {
           ESP_LOGI(TAG, "Temperatura: %.2f °C, Humedad: %.2f %%RH", temperature_c, humidity_rh);
        } else {
            ESP_LOGE(TAG, "Error lectura SHTC3: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}

// Modificamos start para recibir el handle del bus ya creado
void shtc3_start(i2c_master_bus_handle_t bus_handle)
{
    ESP_LOGI(TAG, "Configurando dispositivo SHTC3 en el bus compartido...");
    
    // Inicializamos el dispositivo SHTC3 en el bus existente.
    // 0x70 es la dirección I2C común para el SHTC3
    shtc3_init(&tempSensor, bus_handle, 0x70);

    // Creamos la tarea
    xTaskCreate(&leer_temperatura, "leer_temperatura", 2048, NULL, 2, NULL);
}
