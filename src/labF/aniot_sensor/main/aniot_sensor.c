/*
 * Ejemplo combinado: Simple OTA + IoT Button
 * Al pulsar el botón (BOOT), se descarga la actualización.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // Necesario para el semáforo
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "protocol_examples_common.h"
#include "string.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <sys/socket.h>

// Headers del botón
#include "iot_button.h"
#include "button_gpio.h"

// Incluye los componentes
#include "blink_comp.h"
#include "sht3c_comp.h"
#include "icm_comp.h"
#include "i2c_comp.h"
#include "sleep_comp.h"

#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
#include "esp_crt_bundle.h"
#endif

#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

// --- CONFIGURACIÓN DEL BOTÓN (Tomada de main.c) ---
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32H2 || CONFIG_IDF_TARGET_ESP32C6
#define BOOT_BUTTON_NUM         9
#else
#define BOOT_BUTTON_NUM         0
#endif
#define BUTTON_ACTIVE_LEVEL     0

// --- CONFIGURACIÓN OTA ---
#define HASH_LEN 32
#define OTA_URL_SIZE 256

static const char *TAG = "aniot_sensor";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

// Semáforo para señalar que el botón fue pulsado
static SemaphoreHandle_t ota_sem = NULL;

/* Handler de eventos HTTP (Mantenido del original) */
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

/* Callback del Botón: Se ejecuta cuando pulsas el botón */
static void button_single_click_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Botón presionado: Solicitando OTA...");
    // Liberamos el semáforo para desbloquear la tarea OTA
    xSemaphoreGive(ota_sem);
}

/* Inicialización del Botón (Adaptado de main.c) */
void button_init(uint32_t button_num)
{
    button_config_t btn_cfg = {0};
    button_gpio_config_t gpio_cfg = {
        .gpio_num = button_num,
        .active_level = BUTTON_ACTIVE_LEVEL,
        // Desactivamos power save aquí porque necesitamos WiFi activo para OTA
        .enable_power_save = false, 
    };

    button_handle_t btn;
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn);
    assert(ret == ESP_OK);

    // Solo registramos SINGLE_CLICK para evitar múltiples disparos
    ret = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NULL, button_single_click_cb, NULL);
    ESP_ERROR_CHECK(ret);
}

/* Tarea Principal de OTA */
void simple_ota_example_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Tarea OTA iniciada. Esperando pulsación del botón...");

    while (1) {
        // Esperar al botón
        if (xSemaphoreTake(ota_sem, portMAX_DELAY) == pdTRUE) {
            
            ESP_LOGI(TAG, "Iniciando proceso de actualización OTA...");

            // 1. Obtener información de la versión ACTUAL antes de empezar
            const esp_app_desc_t *running_app_info = esp_app_get_description();
            ESP_LOGI(TAG, "Versión actual ejecutándose: %s", running_app_info->version);

            esp_http_client_config_t config = {
                .url = CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL,
#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
                .crt_bundle_attach = esp_crt_bundle_attach,
#else
                .cert_pem = (char *)server_cert_pem_start,
#endif
                .event_handler = _http_event_handler,
                .keep_alive_enable = true,
            };

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
            config.skip_cert_common_name_check = true;
#endif

            esp_https_ota_config_t ota_config = {
                .http_config = &config,
            };

            ESP_LOGI(TAG, "Descargando desde: %s", config.url);

            // Ejecuta la OTA (Bloqueante)
            esp_err_t ret = esp_https_ota(&ota_config);
            
            if (ret == ESP_OK) {
                
                /* --- NUEVO: LOGICA PARA MOSTRAR VERSIONES --- */
                // Buscamos la partición que "será" la siguiente activa (donde acabamos de escribir)
                const esp_partition_t *next_partition = esp_ota_get_next_update_partition(NULL);
                
                if (next_partition != NULL) {
                    esp_app_desc_t new_app_info;
                    // Leemos la cabecera del binario recién descargado en esa partición
                    esp_err_t err = esp_ota_get_partition_description(next_partition, &new_app_info);
                    
                    if (err == ESP_OK) {
                        ESP_LOGW(TAG, "============================================");
                        ESP_LOGW(TAG, "      ACTUALIZACIÓN COMPLETADA              ");
                        ESP_LOGW(TAG, "============================================");
                        ESP_LOGI(TAG, "Versión Anterior : %s", running_app_info->version);
                        ESP_LOGI(TAG, "Versión Nueva    : %s", new_app_info.version);
                        ESP_LOGI(TAG, "Fecha Compilación: %s %s", new_app_info.date, new_app_info.time);
                        ESP_LOGW(TAG, "============================================");
                    } else {
                        ESP_LOGE(TAG, "No se pudo leer la descripción de la nueva versión.");
                    }
                }
                /* -------------------------------------------- */

                ESP_LOGI(TAG, "Reiniciando sistema en 3 segundos...");
                vTaskDelay(pdMS_TO_TICKS(3000)); // Espera un poco para que puedas leer el log
                esp_restart();

            } else {
                ESP_LOGE(TAG, "Fallo en la actualización (Error: %s). Esperando botón...", esp_err_to_name(ret));
            }
        }
    }
}

static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s %s", label, hash_print);
}

static void get_sha256_of_partitions(void)
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");
}

void ota_start(void)
{
    ESP_LOGI(TAG, "OTA Example con Botón - Inicio");
    
    // Crear semáforo binario
    ota_sem = xSemaphoreCreateBinary();

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    get_sha256_of_partitions();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Conexión a WiFi/Ethernet */
    ESP_ERROR_CHECK(example_connect());

#if CONFIG_EXAMPLE_CONNECT_WIFI
    /* Desactivar WiFi Power Save para máxima velocidad en OTA */
    esp_wifi_set_ps(WIFI_PS_NONE);
#endif 

    // Inicializar el botón (GPIO 0 o 9 según placa)
    button_init(BOOT_BUTTON_NUM);

    // Crear la tarea OTA (se quedará esperando al semáforo)
    xTaskCreate(&simple_ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando sistema...");

    // 1. Iniciar Blink (Independiente)
    blink_start();
    // 2. Iniciar Light Sleep (Independiente)
    sleep_start();
    // 3. Iniciar OTA (Independiente)
    ota_start();

    // 4. Inicializar el Bus I2C Compartido
    i2c_master_bus_handle_t i2c_bus_handle = NULL;
    ESP_ERROR_CHECK(i2c_bus_init(&i2c_bus_handle));

    // 5. Iniciar sensores pasando el bus handle
    // Nota: El orden no es crítico, pero el bus debe existir antes
    if (i2c_bus_handle != NULL) {
        ESP_LOGI(TAG, "Iniciando SHTC3...");
        shtc3_start(i2c_bus_handle);

        ESP_LOGI(TAG, "Iniciando ICM42670...");
        icm_start(i2c_bus_handle);
    } else {
        ESP_LOGE(TAG, "No se pudo iniciar los sensores por fallo en I2C Bus");
    }
}
