idf_component_register(SRCS "imu_comp.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_timer icm42670_local led_strip)
