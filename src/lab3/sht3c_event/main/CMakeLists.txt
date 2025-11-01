idf_component_register(SRCS "sht3c_event_main.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ""
                       REQUIRES shtc3
                       REQUIRES esp_event)
