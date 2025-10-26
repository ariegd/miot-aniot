#include "console_shell.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "mock-flash.h"
#include "esp_log.h"
#include <stdatomic.h>

static const char* TAG="CONSOLE";
static esp_event_loop_handle_t s_loop; static esp_event_base_t s_base; static int32_t s_evt;
static _Atomic bool s_running = false;
static TaskHandle_t s_task = NULL;

static int cmd_help(int argc, char **argv) {
    printf("Comandos disponibles:\n  help\n  monitor\n  quota\n");
    return 0;
}
static int cmd_monitor(int argc, char **argv) {
    esp_event_post_to(s_loop, s_base, s_evt, NULL, 0, portMAX_DELAY);
    return 0;
}
static int cmd_quota(int argc, char **argv) {
    printf("Flash ocupada: %u bytes\n", (unsigned)getDataLeft());
    return 0;
}

static void register_cmds(void){
    const esp_console_cmd_t cmds[] = {
        {.command="help", .help="Muestra ayuda", .hint=NULL, .func=&cmd_help},
        {.command="monitor", .help="Volver a modo monitor", .hint=NULL, .func=&cmd_monitor},
        {.command="quota", .help="Bytes pendientes en flash", .hint=NULL, .func=&cmd_quota},
    };
    for (size_t i=0;i<sizeof(cmds)/sizeof(cmds[0]);++i) esp_console_cmd_register(&cmds[i]);
}

static void console_task(void *arg){
    esp_console_config_t console_cfg = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
        .hint_color = 0
#endif
    };
    esp_console_init(&console_cfg);
    register_cmds();
    linenoiseSetMultiLine(1);
    linenoiseHistorySetMaxLen(32);
    printf("\n[CONSOLE] Escribe 'help'. 'monitor' para salir.\n");
    s_running = true;
    while (s_running) {
        char* line = linenoise("> ");
        if (line == NULL) continue;
        if (strlen(line) > 0) {
            int ret;
            esp_console_run(line, &ret);
            linenoiseHistoryAdd(line);
        }
        linenoiseFree(line);
    }
    esp_console_deinit();
    vTaskDelete(NULL);
}

void console_init(esp_event_loop_handle_t loop, esp_event_base_t base, int32_t evt_monitor_cmd){
    s_loop = loop; s_base = base; s_evt = evt_monitor_cmd;
}
void console_start(esp_event_loop_handle_t loop){
    if (!s_task) xTaskCreate(console_task, "console_task", 4096, NULL, 5, &s_task);
}
void console_stop(void){
    s_running = false;
    s_task = NULL;
}
