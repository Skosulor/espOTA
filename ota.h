#ifndef __OTA_H_
#define __OTA_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "protocol_examples_common.h"
#include "string.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"


#define OTA_URL_SIZE 256
#define GPIO_INPUT_SEL ((1ULL << GPIO_NUM_0))
#define USE_EXAMPLE_WIFI 1
#define USE_CUSTOM_WIFI 0
#define INIT_NVS_TRUE 1
#define INIT_NVS_FALSE 0

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void simple_ota_example_task(void *pvParameter);
void wifi_connect();
void init_gpio();
void init_nvs_flash();
void init_wifi(int use_example);
void init_ota(int init_flash, int use_example_wifi, char *ssid_i, char *pass_i, char *ota_address_i);
void init_ota_default(char *addr);

#endif // __OTA_H_
