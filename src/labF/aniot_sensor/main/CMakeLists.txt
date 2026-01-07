# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "aniot_sensor.c"
                    INCLUDE_DIRS "."
                    REQUIRES blink_comp sht3c_comp icm_comp i2c_comp sleep_comp
                    PRIV_REQUIRES esp_http_client app_update esp_https_ota
                                  esp_netif mbedtls nvs_flash esp_wifi esp_psram
                    EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem)
