
#include <stdio.h>
#include <esp_log.h>
#include <ssd1306.h>
#include "task_oled.h"
#include "task_process_data.h"
#include "task_espnow_send.h"
#include "struct_common.h"
QueueHandle_t oled_queue;

void task_oled( void *pvParameters)
{
    init_ssd1306();
    power_data_t d;
    char buf[32];
    int loop_count = 0;  // Đếm số lần lặp để notify sau 2 lần
    
    while(1)
    {   
        if (xQueueReceive(oled_queue, &d, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI("OLED", "V: %.2f I: %.2f P: %.2f", d.voltage, d.current, d.power);
            ssd1306_clear_buffer();
            snprintf(buf, sizeof(buf), "V:%.2f", d.voltage);
            ssd1306_print_str(0, 3, buf, false);

            snprintf(buf, sizeof(buf), "I:%.2f", d.current);
            ssd1306_print_str(75, 3, buf, false);

            snprintf(buf, sizeof(buf), "P:%.2f", d.power);
            ssd1306_print_str(0, 16, buf, false);

            snprintf(buf, sizeof(buf), "PF:%.2f", d.power_fac);
            ssd1306_print_str(65, 16, buf, false);

            snprintf(buf, sizeof(buf), "Wh:%.2f", d.energy);
            ssd1306_print_str(0, 29, buf, false);

            ssd1306_display();
            vTaskDelay(pdMS_TO_TICKS(500));
            
            // Đếm số lần lặp, mỗi 2 lần thì notify task_espnow để gửi dữ liệu
            loop_count++;
            if (loop_count >= 2) {
                loop_count = 0;
                xQueueSend(espnow_data_queue, &d, portMAX_DELAY); // gửi dữ liệu cho espnow
                // Notify task_espnow để gửi dữ liệu (với bit ESPNOW_SEND_BIT)
                // if (task_espnow_handle != NULL) {
                //     xTaskNotify(task_espnow_handle, ESPNOW_SEND_BIT, eSetBits);
                // }
            }
        }
    }
}
void app_task_oled_init()
{
    xTaskCreatePinnedToCore(&task_oled, "task_oled", 4096, NULL, 3, NULL, 1);
}