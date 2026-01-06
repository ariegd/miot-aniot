idf_component_register(SRCS "hello_world_main.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ""
                       REQUIRES blink_comp 
                                            sht3c_comp 
                                            icm_comp)
