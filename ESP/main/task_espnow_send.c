#include "task_espnow_send.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_now.h"
#include "esp_log.h"
#include "task_process_data.h"
#include "task_oled.h"
#include <string.h>
#include "struct_common.h"
static const char *TAG = "TASK_ESPNOW";

// MAC address của gateway
extern uint8_t peer_mac[6];

// Task handle của task_espnow (được set trong app_task_espnow_init)
TaskHandle_t task_espnow_handle = NULL;
QueueHandle_t espnow_data_queue = NULL; 
// ============== Callback Handler cho dữ liệu nhận từ Gateway ==============
/**
 * Hàm xử lý dữ liệu nhận được từ ESP-NOW (từ gateway)
 * @param info Thông tin về packet nhận được
 * @param data Dữ liệu nhận được
 * @param len Độ dài dữ liệu
 */
// void espnow_recv_handler(const esp_now_recv_info_t *info, const uint8_t *data, int len)
// {
//     if (info == NULL || data == NULL || len == 0) {
//         ESP_LOGE(TAG, "Invalid received data");
//         return;
//     }
    
//     // Log thông tin nhận được
//     ESP_LOGI(TAG, "Received data from gateway, length: %d bytes", len);
    
//     // Notify task_espnow để xử lý dữ liệu nhận được
//     if (task_espnow_handle != NULL) {
//         xTaskNotify(task_espnow_handle, ESPNOW_RECV_BIT, eSetBits);
//     }
    
//     // Có thể xử lý dữ liệu từ gateway ở đây (ví dụ: lệnh điều khiển, cấu hình)
//     // uint8_t* mac = info->src_addr;
//     // ESP_LOGI(TAG, "Gateway MAC: %02x:%02x:%02x:%02x:%02x:%02x",
//     //         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
//     // Ví dụ: nếu dữ liệu là lệnh, có thể phân tích và xử lý
//     // switch (data[0]) {
//     //     case CMD_START_CHARGE:
//     //         // Xử lý lệnh bắt đầu sạc
//     //         break;
//     //     case CMD_STOP_CHARGE:
//     //         // Xử lý lệnh dừng sạc
//     //         break;
//     // }
// }



// ============== Hàm gửi dữ liệu sạc qua ESP-NOW ==============
/**
 * Gửi dữ liệu sạc (power_data_t) qua ESP-NOW bằng queue peek
 * Dùng xQueuePeek để không mất dữ liệu, vẫn giữ cho OLED sử dụng
 * 
 * @return pdTRUE nếu gửi thành công, pdFALSE nếu không có dữ liệu hoặc gửi thất bại
 */
// static BaseType_t espnow_send_charging_data(void)
// {
    
    
//     // Sử dụng xQueuePeek để lấy dữ liệu mà không xóa khỏi queue
//     // Điều này đảm bảo OLED vẫn có dữ liệu để vẽ
//     BaseType_t result = xQueuePeek(oled_queue, &data, pdMS_TO_TICKS(100));
    
//     if (result != pdTRUE) {
//         ESP_LOGD(TAG, "No data available in oled_queue");
//         return pdFALSE;
//     }
    
//     // Gửi dữ liệu qua ESP-NOW
//     esp_err_t ret = esp_now_send(peer_mac, (uint8_t *)&data, sizeof(power_data_t));
    
//     if (ret == ESP_OK) {
//         ESP_LOGD(TAG, "Sent charging data - V:%.2f I:%.2f P:%.2f PF:%.2f WH:%.2f",
//                  data.voltage, data.current, data.power, data.power_fac, data.energy);
//         return pdTRUE;
//     } else {
//         ESP_LOGW(TAG, "Failed to send charging data (err: %d)", ret);
//         return pdFALSE;
//     }
// }

// ============== Main Task ESP-NOW ==============
/**
 * Task chính để quản lý gửi dữ liệu
 * Chờ notification từ:
 *   - Callback recv_cb: khi nhận dữ liệu từ gateway (ESPNOW_RECV_BIT)
 *   - Task OLED: khi đủ 2 lần lặp (ESPNOW_SEND_BIT)
 */
void task_espnow(void *pvParameters)
{
    ESP_LOGI(TAG, "ESP-NOW task started");
    uint32_t notify_value = 0;
    power_data_t data;
    while(1)
    {
        // Chờ notification từ recv_cb hoặc task_oled
        // Sử dụng RTOS notification thay vì delay
        // xTaskNotifyWait(
        //     0,                          // ulBitsToClearOnEntry (không clear trước)
        //     ESPNOW_RECV_BIT | ESPNOW_SEND_BIT,  // ulBitsToClearOnExit (clear sau khi xử lý)
        //     &notify_value,              // pulNotificationValue (lưu giá trị notification)
        //     portMAX_DELAY               // xTicksToWait (chờ vô hạn)
        // );
        
        
        // if (notify_value & ESPNOW_SEND_BIT) {
        //     // Gửi dữ liệu sạc (được notify sau 2 lần lặp task_oled)
        //     ESP_LOGD(TAG, "Send notification from task_oled");
        //     espnow_send_charging_data();
        // }
        if(xQueueReceive(espnow_data_queue, &data, portMAX_DELAY) == pdTRUE)
        {
            // Gửi dữ liệu qua ESP-NOW
            data.node_id = MY_NODE_ID; // node 1
            esp_now_send(peer_mac, (uint8_t *)&data, sizeof(power_data_t));

        }
    }
}

// ============== Khởi tạo ESP-NOW Task ==============
void app_task_espnow_send_init(void)
{
    
    espnow_data_queue = xQueueCreate(1, sizeof(power_data_t));
    xTaskCreatePinnedToCore(
        &task_espnow,           // Hàm task
        "task_espnow",          // Tên task
        4096,                   // Stack size
        NULL,                   // Parameters
        5,                      // Priority
        &task_espnow_handle,    // Handle (lưu vào biến global)
        1                       // Core 1
    );
    
    ESP_LOGI(TAG, "ESP-NOW task initialized with handle: %p", task_espnow_handle);
}