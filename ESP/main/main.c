
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
const char *TAG = "SPI_MAIN";

void app_main(void)
{
    spi_bus_init();
    app_task_read_data_init();
    app_task_process_data_init();    
    app_task_oled_init();
    Ex_ISR_Init();
    gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 0);
    
}
     













