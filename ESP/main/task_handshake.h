#ifndef TASK_HANDSHAKE_H
#define TASK_HANDSHAKE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"

void app_task_handshake_init(void);
#endif /* TASK_HANDSHAKE_H */