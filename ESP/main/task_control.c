#include "task_control.h"
#include "struct_common.h"
#include "config.h"

QueueHandle_t control_node_queue = NULL;
static const char *TAG = "TASK_CONTROL";
void spi_send_cmd(spi_device_handle_t dev, uint8_t cmd) 
{
    if (xSemaphoreTake(spi_mutex, portMAX_DELAY) == pdTRUE) 
    {
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));       // Xóa sạch bộ nhớ để tránh dữ liệu rác
        t.length = 16;                   // 8 bits = 1 byte
        t.flags = SPI_TRANS_USE_TXDATA;
        t.tx_data[0] = cmd;             // Ghi lệnh vào byte đầu tiên
        t.tx_data[1] = 0;               // Đảm bảo byte thứ hai là 0 (không sử dụng)
        esp_err_t ret = spi_device_transmit(dev, &t);
        if (ret != ESP_OK) 
        {
            ESP_LOGE("SPI", "Send command failed!");
        }
        xSemaphoreGive(spi_mutex);
    }
}

float Wh = 0.0f;

void task_control(void *pvParameters)
{
    static int count = 0;  // Static: chỉ initialize một lần, persist qua các loop
    node_ctrl_t ctrl_data;
    
    while(1)
    {
        if(xQueueReceive(control_node_queue, &ctrl_data, portMAX_DELAY) == pdTRUE)
        {
            gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
            gpio_set_level(GPIO_NUM_33, 0);

            Wh = ctrl_data.energy_cmd; // Cập nhật giá trị Wh từ lệnh nhận được

            spi_send_cmd(spi_i, ctrl_data.gain); // Gửi lệnh khởi động cho Slave 
            vTaskDelay(pdMS_TO_TICKS(1));

            // Lần thứ nhất: Init ISR (chỉ gọi một lần)
            if (count == 0)
            {
                Ex_ISR_Init();
                count++;
            }
            // Lần thứ 2+: Trigger ISR
            else
            {
                Ex_ISR_trigger();
            }
            
            ESP_LOGI(TAG, "Received control data for Node %d: gain=%d, energy_cmd=%d",
                     ctrl_data.node_id, ctrl_data.gain, ctrl_data.energy_cmd);
            
        }
    
        // Xử lý dữ liệu điều khiển nhận được từ gateway
        // ESP_LOGI("CONTROL", "Received control data for Node %d: gain=%d, energy_cmd=%d",
        //          ctrl_data.node_id, ctrl_data.gain, ctrl_data.energy_cmd);

            
    }    
}


void app_task_control_init(void)
{
    control_node_queue = xQueueCreate(10, sizeof(node_ctrl_t));
    xTaskCreatePinnedToCore(&task_control, "task_control", 4096, NULL, 3, NULL, 1);
}