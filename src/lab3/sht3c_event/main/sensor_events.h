#include "esp_event.h"

ESP_EVENT_DEFINE_BASE(HUM_EVENT_BASE);

typedef enum {
    HUM_EVENT_NEW_SAMPLE,
} hum_event_id_t;

typedef struct {
    float temperature;
    float humidity;
} hum_sample_t;
