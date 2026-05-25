
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"   
#include "freertos/queue.h"
#include "process.h"
#include "task_process_data.h"
#include "task_read_data.h"
#include "task_oled.h"
#include "esp_log.h"
#include <string.h>
#include "esp_timer.h"
#include "struct_common.h"
#include <ssd1306.h>

TaskHandle_t task_process_handle;
static const char *TAG = "DATA_PROCESS";

// Buffer để lưu dữ liệu từ queue
uint16_t v_buf[FRAME_SAMPLES];      // Dùng cho RMS
uint16_t vp_buf[FRAME_SAMPLES];     // Dùng cho Power (instant)
uint16_t i_buf[FRAME_SAMPLES];      // Dùng cho RMS
uint16_t ip_buf[FRAME_SAMPLES];     // Dùng cho Power (instant)

// Variables for average voltage
float v_acc = 0.0f;
int   v_cnt = 0;

// Variables for average current
float i_acc = 0.0f;
int   i_cnt = 0;

// Variables for average power
float p_acc = 0.0f;
int   p_cnt = 0 ;

float pf_acc = 0.0f;
// Biến lưu giá trị mới nhất
float v_latest;
float i_latest;
float p_latest;
float pf_latest;


void task_process_data(void *pvParameters)
{
    frame_data_t frame;
    static double total_energy_wh = 0.0; // Biến tĩnh lưu năng lượng tích lũy
    while(1)
    {
        // Nhận frame từ queue
        if(xQueueReceive(frame_queue, &frame, portMAX_DELAY) == pdTRUE)
        {
            // Copy dữ liệu vào 2 buffer: 1 cho RMS, 1 cho Power
            
            memcpy(v_buf, frame.v_buf, sizeof(v_buf));
            //memcpy(vp_buf, frame.v_buf, sizeof(vp_buf));
            memcpy(i_buf, frame.i_buf, sizeof(i_buf));
            //memcpy(ip_buf, frame.i_buf, sizeof(ip_buf));


        // ===== Process voltage =====
        // frame_v_t v = process_v_frame(v_buf);
        // v_acc += v.vrms;

        // // ===== Process current =====
        // frame_i_t i = process_i_frame(i_buf);
        // i_acc += i.irms;
        // ESP_LOGI(TAG, "Current: %.2f A, Offset: %.2f", i.irms, i.mean);

        // ===== Process power =====
        frame_p_t p = process_p_frame(v_buf, i_buf);
        // duration_us nhận từ task_read (khoảng thời gian thực tế của 50 mẫu)
        double delta_t_h = (double)frame.duration_us / 3600000000.0; // Đổi us sang giờ
        p_acc += p.p;
        v_acc += p.vrms;
        i_acc += p.irms;
        pf_acc += p.pf;
        v_cnt++;

        total_energy_wh += (double)p.p * delta_t_h;
        if (total_energy_wh >= Wh)
        {
            total_energy_wh = 0.0; // Reset năng lượng tích lũy khi đạt ngưỡng
            Wh = 0.0; // Reset biến Wh trong struct_common để đồng bộ với OLED
            // reset các buffer tính toán của read data
            memset(frame.v_buf, 0, sizeof(frame.v_buf));
            memset(frame.i_buf, 0, sizeof(frame.i_buf));
            reset_read_buffers();  // Reset v_buf_raw, i_buf_raw, v_idx, i_idx
            // relay off
            gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
            gpio_set_level(GPIO_NUM_33, 1);
            // gửi ISR_external để reset slave
            Ex_ISR_trigger();

            ssd1306_clear_buffer();
            ssd1306_print_str(20, 24, "SESSION ENDED", false);
            ssd1306_display();
        }    
        // ===== Khi đủ AVG_FRAMES =====
        if (v_cnt >= AVG_FRAMES)
        {
            v_latest = v_acc / AVG_FRAMES;
            i_latest = i_acc / AVG_FRAMES;
            p_latest = p_acc / AVG_FRAMES;
            pf_latest = pf_acc / AVG_FRAMES;
            //ESP_LOGI(TAG, "AVG over %d frames - V: %.2f V, I: %.2f A, P: %.2f W", AVG_FRAMES, v_latest, i_latest, p_latest);
            v_acc = 0.0f;
            i_acc = 0.0f;
            p_acc = 0.0f;
            pf_acc = 0.0f;
            v_cnt = 0;
            // ===== Gửi snapshot ổn định cho OLED =====
            power_data_t d = {
                .voltage = v_latest,
                .current = i_latest,
                .power = p_latest,
                .power_fac = pf_latest,
                .energy = total_energy_wh // Gửi giá trị Wh mới nhất
            };
                xQueueOverwrite(oled_queue, &d);
            }
        
        //UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
        //ESP_LOGI(TAG, "Process Data Task Watermark: %u bytes", watermark);  
        } 
    }
}     

void app_task_process_data_init()
{
    oled_queue = xQueueCreate(1, sizeof(power_data_t));
    xTaskCreatePinnedToCore(&task_process_data, "task_process_data", 4096, NULL, 4, &task_process_handle, 1);
}
