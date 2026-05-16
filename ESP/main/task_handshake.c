#include "task_handshake.h"
#include "struct_common.h"

static const char *TAG = "TASK_HANDSHAKE";
void task_handshake(void *pvParameters)
{
    power_data_t data;
    uint8_t dummy_data = 0;
    while(1)
    {
        if (!is_gateway_found) 
        {
            ESP_LOGI(TAG, "Dang do tim Gateway tai Channel: %d", current_ch);
            esp_wifi_set_channel(current_ch, WIFI_SECOND_CHAN_NONE);
            
            // Gửi thử một gói tin dummy để check kênh
            esp_now_send(peer_mac, &dummy_data, 1);
            
            vTaskDelay(pdMS_TO_TICKS(200)); // Đợi callback trả về kết quả
            if (is_gateway_found) 
            {
                ESP_LOGI(TAG, ">>> DA TIM THAY GATEWAY TAI KENH %d <<<", current_ch);
                data.header[0] = 'N';
                data.header[1] = '1';
                data.node_id = MY_NODE_ID; // node 1
                esp_now_send(peer_mac, (uint8_t *)&data, sizeof(power_data_t));
            } 
            else 
            {
                current_ch = (current_ch % 11) + 1; // Quét từ 1-11
            }
            
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay giữa các lần quét
    }
}

void app_task_handshake_init()
{
    xTaskCreatePinnedToCore(&task_handshake, "task_handshake", 4096, NULL, 3, NULL, 1);
}

