#ifndef STRUCT_COMMON_H
#define STRUCT_COMMON_H

#include <stdint.h>
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
extern QueueHandle_t control_node_queue;
extern QueueHandle_t espnow_data_queue;
extern float Wh;

extern volatile bool is_gateway_found;
extern volatile esp_now_send_status_t last_status;
extern uint8_t current_ch;
extern uint8_t peer_mac[6];
extern uint8_t dummy_data;
extern SemaphoreHandle_t spi_mutex;

extern int64_t start_time;
#define MY_NODE_ID 1
// dữ liệu đo được gửi từ Node lên Gateway qua ESP-NOW
typedef struct {
    char header[2];    // "N1" - Mật khẩu nhận diện
    uint8_t node_id;   // ID của Node (1, 2, 3...)
    float voltage;     // V
    float current;     // I
    float power;       // P
    float energy;      // Wh
    float power_fac;
} __attribute__((packed)) power_data_t;

// dữ liệu nhận được từ MQTT để gửi xuống node
typedef struct {
    uint8_t node_id;
    uint8_t gain;
    uint8_t energy_cmd; 
} __attribute__((packed)) node_ctrl_t;


#endif // STRUCT_COMMON_H