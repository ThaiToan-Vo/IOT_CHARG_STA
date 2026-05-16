#ifndef ESPNOW_INIT_H
#define ESPNOW_INIT_H

#include "esp_wifi.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_now.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "string.h"
#include <stdio.h>
#include "freertos/queue.h"
void wifi_init(void);
void espnow_init(void);
void add_peer(void);

void send_cb(const esp_now_send_info_t *info, esp_now_send_status_t status);
void recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len);

#endif // ESPNOW_INIT_H 