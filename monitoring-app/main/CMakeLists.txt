idf_component_register(SRCS "main.c"
                       PRIV_REQUIRES nvs_flash spi_flash shtc3 mock-flash mock-wifi button_monitor console_shell
                       INCLUDE_DIRS "")
