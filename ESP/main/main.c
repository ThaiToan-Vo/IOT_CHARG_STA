
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "config.h"
#include "process.h"
#include "task_read_data.h"
#include "task_process_data.h"
#include "task_oled.h"
#include "driver/gpio.h"
#include "task_espnow_send.h"
#include "espnow_init.h"
#include "struct_common.h"
#include "task_control.h"
#include "task_handshake.h"
static const char *TAG = "SPI_MAIN";



void app_main(void)
{
    spi_mutex = xSemaphoreCreateMutex();
    wifi_init();
    espnow_init();
    add_peer();
    app_task_handshake_init();
    app_task_control_init();
    app_task_espnow_send_init();
    spi_bus_init();
    app_task_read_data_init();
    app_task_process_data_init();    
    app_task_oled_init();
    
    
    
    
    
}
     













