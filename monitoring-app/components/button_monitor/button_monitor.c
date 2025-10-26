#include "button_monitor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

static esp_event_loop_handle_t s_loop; static esp_event_base_t s_base; static int32_t s_evt;
static TaskHandle_t s_task=NULL; static int s_gpio; static int s_period;

static void task(void *arg){
    gpio_config_t io = {.pin_bit_mask = 1ULL<<s_gpio, .mode=GPIO_MODE_INPUT, .pull_up_en=1};
    gpio_config(&io);
    int last = 1, stable_cnt = 0;
    while(1){
        int v = gpio_get_level(s_gpio);
        if (v != last) { stable_cnt = 0; last = v; }
        else if (stable_cnt++ > 2 && v==0) { // pulsación activa a 0
            esp_event_post_to(s_loop, s_base, s_evt, NULL, 0, portMAX_DELAY);
            // evitar rebotes
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(s_period*1000/5)); // varias lecturas por periodo
    }
}

void button_init(esp_event_loop_handle_t loop, esp_event_base_t base, int32_t evt_pressed,
                 int gpio, int period_sec) {
    s_loop = loop; s_base = base; s_evt = evt_pressed; s_gpio = gpio; s_period=period_sec;
    if (!s_task) xTaskCreate(task, "button_task", 2048, NULL, 5, &s_task);
}