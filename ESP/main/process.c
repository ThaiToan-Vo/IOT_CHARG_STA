#include "process.h"
#include "struct_common.h"
#include "task_control.h"
static const char *TAG = "SPI_PROCESS";
SemaphoreHandle_t spi_mutex = NULL;


// convert ADC value
float adc_to_vin(uint16_t adc)
{
    return (adc * ADC_TO_VOLT_SCALE);
}

/*=== Master read sample from Slave ===*/ 

bool spi_read_sample(spi_device_handle_t dev, uint16_t *out)
{
    if (!dev) 
    {
        ESP_LOGI(TAG, "ERROR: dev is NULL\n");
        return false;
    }

    if (!out) 
    {
        ESP_LOGI(TAG, "ERROR: out is NULL\n");
        return false;
    }
    if (xSemaphoreTake(spi_mutex, portMAX_DELAY) == pdTRUE) 
    {
        // Use static transaction to avoid stack corruption
        static spi_transaction_t t;
        memset(&t, 0, sizeof(spi_transaction_t));
    
        t.length = 16;
        t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
        t.tx_data[0] = 0;
        t.tx_data[1] = 0;


        esp_err_t ret = spi_device_transmit(dev, &t);
        if (ret != ESP_OK) 
        {
            ESP_LOGI(TAG, "SPI transmit failed: %d\n", ret);
            return false;
        }


        *out = t.rx_data[0]  | (t.rx_data[1] << 8 );
        
        xSemaphoreGive(spi_mutex);
        return true;
    }
    return false;
}


/*=== Process average power ===*/

frame_p_t process_p_frame(uint16_t *v_buf, uint16_t *i_buf)
{
    frame_p_t r;
    static float offsetI = 2045.0f; // giá trị ADC với vref=3.36, và adc 12 bit
    float sumI  = 0.0f;
    float filter_I = 0.0f;
    float sqI =0.0f; // square current (bình phương dòng điện)
    
    static float offsetV = 2045.0f; // giá trị ADC với vref=3.36, và adc 12 bit
    float sumV  = 0.0f;
    float filter_V = 0.0f;
    float sqV =0.0f; // square voltage (bình phương điện áp)

    float inst_p = 0.0f;
    float sum_p  = 0.0f;

    float Vcal = 238.0f; // giá trị điện áp scale 220V / 1.018V(sau chia áp) / 1.01(gain)
    float Ical_10 = 2.3f;  // giá trị dòng điện scale 1000 / 51 * 1 / 10.1
    float Ical_1 = 21.95f;  // giá trị dòng điện scale 1000 / 51 * 1 / 1.01
    float Ical = 0.0f;
    uint8_t k = 1;
    if (gain == 2)
    {
        Ical = Ical_10;
    }
    else
    {
        Ical = Ical_1;
    }   
    /* 1. Loại bỏ offset */
    for (int n = 0; n < FRAME_SAMPLES; n++) 
    {
        offsetI = (offsetI +(i_buf[n] - offsetI)/1024.0f);
        filter_I = i_buf[n] - offsetI;
        sqI = filter_I * filter_I;
        sumI += sqI;

        offsetV = (offsetV +(v_buf[n] - offsetV)/1024.0f);
        filter_V = v_buf[n] - offsetV;

        sqV = filter_V * filter_V;
        sumV += sqV;

        inst_p = filter_I * filter_V;
        // Thử thay n bằng (n + k) % 50 để dịch chuyển I so với V
        // Thử k lần lượt là 1, 2, 3, 4
    
    }

    for (int n = 0; n < (FRAME_SAMPLES - k); n++) // Chạy đến 45 để n+5 không quá 50
    {
        // 1. Lấy giá trị I đã trừ offset tại vị trí n
        float i_now = (float)i_buf[n] - offsetI;

        // 2. Lấy giá trị V đã trừ offset tại vị trí n + 4 (Bù pha)
        // float v_sample_1 = (float)v_buf[n ] - offsetV;
        // float v_sample_2 = (float)v_buf[n + 1] - offsetV;
        // float v_future = (v_sample_1 + v_sample_2) / 2.0f;
        float v_future = (float)v_buf[n + k] - offsetV;

        // 3. Nhân công suất tức thời
        sum_p += i_now * v_future;
        
    }
    float I_ratio = Ical * 3.3f / 4095.0f;
    r.irms    = sqrtf(sumI / FRAME_SAMPLES) * I_ratio;
    if(r.irms < 0.021f)
    {
        r.irms = 0.0f; // loại bỏ nhiễu khi không có tải
    }
       
    
    float V_ratio = Vcal * 3.3f / 4095.0f;
    r.vrms    = sqrtf(sumV / FRAME_SAMPLES) * V_ratio;

    r.p = ( (sum_p / (FRAME_SAMPLES - k)) * I_ratio * V_ratio);
    // if(r.p < 2.0f)
    // {
    //     r.p = 0.01f; // loại bỏ nhiễu khi không có tải
    // }
    r.apparent_power = r.irms * r.vrms;
    
    r.pf = r.p / r.apparent_power;
    if(r.pf < 0.07f)
    {
        r.pf = 0.0f; // loại bỏ nhiễu khi không có tải
    }


    return r;
}