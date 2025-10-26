#pragma once
#include "esp_event.h"
void console_init(esp_event_loop_handle_t loop, esp_event_base_t base, int32_t evt_monitor_cmd);
void console_start(esp_event_loop_handle_t loop);
void console_stop(void);