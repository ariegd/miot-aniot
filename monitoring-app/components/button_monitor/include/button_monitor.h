#pragma once
#include "esp_event.h"

void button_init(esp_event_loop_handle_t loop, esp_event_base_t base, int32_t evt_pressed,
                 int gpio, int period_sec);