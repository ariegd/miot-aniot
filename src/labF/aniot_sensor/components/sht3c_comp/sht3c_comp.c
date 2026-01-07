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

    // Arrays para almacenar el historial de muestras
    
    float temp_history[CONFIG_SHT3C_AVG_SAMPLES] = {0};
    float hum_history[CONFIG_SHT3C_AVG_SAMPLES] = {0};
    int index = 0;
    int samples_count = 0;
    

    ESP_LOGI(TAG, "Iniciando bucle de lectura SHTC3 (Periodo: %d s, Promedio: %d)", 
                 CONFIG_SHT3C_READ_INTERVAL, CONFIG_SHT3C_AVG_SAMPLES);

    while (1) {
        // Usamos la función lpm (Low Power Mode) o normal según tu driver
        esp_err_t ret = shtc3_get_temp_and_hum_lpm(&tempSensor, &temperature_c, &humidity_rh);

        if (ret == ESP_OK) {
            // Guardar en el buffer circular
            
            temp_history[index] = temperature_c;
            hum_history[index] = humidity_rh;
            
            index = (index + 1) % CONFIG_SHT3C_AVG_SAMPLES;
            if (samples_count < CONFIG_SHT3C_AVG_SAMPLES) {
                samples_count++;
            }
            
            if (samples_count == 10){
                // Calcular media
                float temp_sum = 0, hum_sum = 0;
                for (int i = 0; i < samples_count; i++) {
                    temp_sum += temp_history[i];
                    hum_sum += hum_history[i];
                }

                float temp_avg = temp_sum / samples_count;
                float hum_avg = hum_sum / samples_count;
                
                ESP_LOGI(TAG, "[MEDIA de %d muestras] Temp: %.2f °C, Hum: %.2f %%RH", 
                         samples_count, temp_avg, hum_avg);
                         
                samples_count = 0;
            }
            
           ESP_LOGI(TAG, "Temperatura: %.2f °C, Humedad: %.2f %%RH", temperature_c, humidity_rh);
        } else {
            ESP_LOGE(TAG, "Error lectura SHTC3: %s", esp_err_to_name(ret));
        }

        // Multiplicamos por 1000 porque pdMS_TO_TICKS espera milisegundos
        vTaskDelay(pdMS_TO_TICKS(CONFIG_SHT3C_READ_INTERVAL * 1000));
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
    // 4096
    xTaskCreate(&leer_temperatura, "leer_temperatura", 4096, NULL, 2, NULL);
}
