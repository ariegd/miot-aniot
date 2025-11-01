idf_component_register(SRCS "sht3c_event_main.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ""
                       REQUIRES shtc3 esp_event sensor_shtc3 logger wifi_sim button_console flash_sim
)
