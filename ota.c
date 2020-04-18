#include "ota.h"

const int CONNECTED_BIT_OTA = BIT0;
static const char *TAG_OTA = "ota";
static EventGroupHandle_t wifi_event_group;
static char *ssid;
static char *pass;
static char *ota_address;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG_OTA, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG_OTA, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG_OTA, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG_OTA, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG_OTA, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG_OTA, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG_OTA, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

void ota_task(void *pvParameter)
{
    ESP_LOGI(TAG_OTA, "Starting OTA");

    esp_http_client_config_t config = {
        .url = ota_address,
        .cert_pem = (char *)"unused",
        .event_handler = _http_event_handler,
    };


    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE(TAG_OTA, "Firmware upgrade failed");
    }
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void IRAM_ATTR isr_handler(void *arg){
    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
}

void wifi_connect(){
    wifi_config_t cfg = {};
    strcpy((char *)cfg.sta.ssid, ssid);
    strcpy((char *)cfg.sta.password, pass);

    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

static esp_err_t event_handler(void *ctx, system_event_t *event){
    switch(event->event_id){
        case SYSTEM_EVENT_STA_START:
            wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT_OTA);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT_OTA);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void initialise_wifi(){
    esp_log_level_set("wifi", ESP_LOG_NONE);
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void init_gpio(){
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_0, isr_handler, (void*) GPIO_NUM_0);
}

void init_wifi(int use_example){
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_event_group = xEventGroupCreate();
    if(use_example)
        ESP_ERROR_CHECK(example_connect());
    else
        initialise_wifi();
}

void init_nvs_flash(){

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

}

void init_ota(int init_flash, int use_example_wifi, char *ssid_i, char *pass_i, char *ota_address_i){
    ssid = ssid_i;
    pass = pass_i;
    ota_address = ota_address_i;

    if(init_flash)
        init_nvs_flash();
    init_wifi(use_example_wifi);
    init_gpio();

}

void init_ota_default(char *addr){
    init_ota(1, 1, "unused", "unused", addr);
}
