/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "app_main.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"

#include "esp_smartconfig.h"
#include "esp_spi_flash.h"

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "sdkconfig.h"
#include "app_main.h"
#include "Button_Task.h"
#include "Ssd1309_Test_Task.h"
#include "music.h"

static const char *TAG = "APP_MAIN";

// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

// Mount path for the partition
const char *c_cpBasePath = "/spiflash";

const char *gc_sMEX_10_FIRM_VERSION="V1.0";

static esp_err_t EventHandler(void *ctx, system_event_t *event)
{
    wifi_config_t wifi_config;
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);
        ESP_LOGI(TAG, "WIFI Connected to AP");
        ESP_LOGI(TAG, "WIFI STA SSID %s",wifi_config.sta.ssid);
        ESP_LOGI(TAG, "WIFI STA PASSWD %s",wifi_config.sta.password);
        esp_wifi_connect();
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s\n",ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently auto-reassociate. */
    		ESP_LOGI(TAG, "DIS CONNECTED");
        esp_wifi_connect();
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void InitialiseWifi(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(EventHandler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA));
//    wifi_config_t sta_config = {
//          .sta = {
//             .ssid = "ss-ipc1",
//             .password = "123456789",
//             .bssid_set = 0
//          }
//    };
//    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    	ESP_ERROR_CHECK( esp_wifi_set_auto_connect(true));
    ESP_ERROR_CHECK( esp_wifi_start() );
    esp_wifi_set_ps(DEFAULT_PS_MODE);
}

void app_main()
{
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s,",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    //heap_caps_print_heap_info(MALLOC_CAP_8BIT);

    ESP_LOGI(TAG, "Mounting FAT filesystem");
    // To mount device we need name of device partition, define c_cpBasePath
    // and allow format partition in case if it is new one and was not formated before
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true
    };

    esp_err_t err = esp_vfs_fat_spiflash_mount(c_cpBasePath, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (0x%x)", err);
        return;
    }

    //nvs启动初始化
    nvs_flash_init();

    //初始化启动wifi
//    InitialiseWifi();

    //启动任务
//    xTaskCreate(&PMSENSOR_DRIVER_TEST_TASK, "PMSEN_DRI_TEST", 5*1024, NULL, 8, NULL);
//    xTaskCreate(&MUSIC_TEST_TASK, "RGB_LED_TEST", 5*1024, NULL, 8, NULL);
    xTaskCreate(&SSD1309_TEST_TASK, "SSD1309_TEST", 10*1024, NULL, 8, NULL);
    xTaskCreate(&ButtonTask, "Button", 5*1024, NULL, 8, NULL);
}
