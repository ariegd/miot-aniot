idf_component_register(SRCS "icm_comp.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_timer icm42670 led_strip)
