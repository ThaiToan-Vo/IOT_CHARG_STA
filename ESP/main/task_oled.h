
#ifndef TASK_OLED_H
#define TASK_OLED_H

#include "freertos/queue.h"




extern QueueHandle_t oled_queue;

void app_task_oled_init();
#endif /* TASK_OLED_H */