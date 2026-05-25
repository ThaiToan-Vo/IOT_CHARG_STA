#ifndef TASK_READ_DATA_H
#define TASK_READ_DATA_H


#include "process.h"



extern TaskHandle_t read_data_handle;




extern uint16_t v_buf_raw[FRAME_SAMPLES];
extern uint16_t i_buf_raw[FRAME_SAMPLES];

extern uint8_t  v_idx;
extern uint8_t  i_idx;

// Struct để gửi frame data qua queue
typedef struct {
    uint16_t v_buf[FRAME_SAMPLES];
    uint16_t i_buf[FRAME_SAMPLES];
    uint32_t duration_us; // Thời gian thu thập frame (micro giây)
} frame_data_t;

// Queue để gửi frame data
extern QueueHandle_t frame_queue;

void app_task_read_data_init();
void reset_read_buffers(void);  // Function để reset buffer và index

#endif /* TASK_READ_DATA_H */