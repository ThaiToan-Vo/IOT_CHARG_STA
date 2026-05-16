#ifndef TASK_ESPNOW_H
#define TASK_ESPNOW_H

#include "espnow_init.h"
#include "esp_now.h"
#include "freertos/task.h"

// ============== Task Notification Bits ==============
#define ESPNOW_RECV_BIT  (1 << 0)   // Bit 0: Thông báo nhận dữ liệu từ gateway
#define ESPNOW_SEND_BIT  (1 << 1)   // Bit 1: Thông báo gửi dữ liệu (sau 2 lần lặp OLED)

// Task handle của task_espnow (dùng để notify từ callback và task khác)
extern TaskHandle_t task_espnow_handle;

// Hàm callback xử lý dữ liệu nhận từ ESP-NOW (gateway)
void espnow_recv_handler(const esp_now_recv_info_t *info, const uint8_t *data, int len);

// Hàm khởi tạo task ESP-NOW
void app_task_espnow_send_init(void);

#endif // TASK_ESPNOW_H