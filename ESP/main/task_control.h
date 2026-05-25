#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_wifi.h"

extern uint8_t gain;
void app_task_control_init(void);

#endif /* TASK_CONTROL_H */