idf_component_register(SRCS "hello_world_main.c"
                       INCLUDE_DIRS ""
                       REQUIRES espressif__button
                       EMBED_TXTFILES "server_cert.pem"
                      PRIV_REQUIRES spi_flash 
                                                     freertos
                                                     nvs_flash
                                                     esp_event
                                                     esp_netif
                                                     esp_http_client
                                                     wifi_provisioning
                                                    esp_https_ota
                                                     )
