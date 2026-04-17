
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

void spi_send_cmd(spi_device_handle_t dev, uint8_t cmd) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       // Xóa sạch bộ nhớ để tránh dữ liệu rác
    t.length = 16;                   // 8 bits = 1 byte
    t.flags = SPI_TRANS_USE_TXDATA;
    t.tx_data[0] = cmd;             // Ghi lệnh vào byte đầu tiên
    t.tx_data[1] = 0;               // Đảm bảo byte thứ hai là 0 (không sử dụng)
    esp_err_t ret = spi_device_transmit(dev, &t);
    if (ret != ESP_OK) {
        ESP_LOGE("SPI", "Send command failed!");
    }
}

void app_main(void)
{
    spi_bus_init();
    app_task_read_data_init();
    app_task_process_data_init();    
    app_task_oled_init();
    spi_send_cmd(spi_i, 2); // Gửi lệnh khởi động cho Slave 
    vTaskDelay(pdMS_TO_TICKS(1));
    
    gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_33, 0);
    Ex_ISR_Init();
    
}
     













