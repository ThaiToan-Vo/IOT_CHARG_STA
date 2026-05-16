#include "espnow_init.h"

#include "task_espnow_send.h"
#include "struct_common.h"

// Biến trạng thái để dò kênh
volatile bool is_gateway_found = false;
volatile esp_now_send_status_t last_status = ESP_NOW_SEND_FAIL;
uint8_t current_ch = 1;

static const char *TAG = "ESP_NOW_SENDER";

void wifi_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            nvs_flash_erase();
            ret = nvs_flash_init();
        }

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

}

// declare functions
void send_cb(const esp_now_send_info_t *info, esp_now_send_status_t status);
void recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len);

void espnow_init(void)
{
    esp_now_init();
    esp_now_register_send_cb(send_cb);
    esp_now_register_recv_cb(recv_cb);
}

// MAC RX
uint8_t peer_mac[6] = {0xFC, 0xB4, 0x67, 0x74, 0x61, 0x00};

// add peer
void add_peer(void)
{
    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, peer_mac, 6);
    peer.channel = 0;
    peer.encrypt = false;

    esp_now_add_peer(&peer);
}

// send callback function
void send_cb(const esp_now_send_info_t *info, esp_now_send_status_t status)
{
    last_status = status;
    if (status == ESP_NOW_SEND_SUCCESS) {
        is_gateway_found = true;
        printf("Send OK\n");
    } else {
        printf("Send FAIL\n");
        is_gateway_found = false;
    }
}

// receive callback function
void recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    node_ctrl_t *incoming = (node_ctrl_t *)data;
    xQueueSend(control_node_queue, incoming, 0);
    // Gọi handler trong task_espnow để xử lý dữ liệu nhận được
    //espnow_recv_handler(info, data, len);
}