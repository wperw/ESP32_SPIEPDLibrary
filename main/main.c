#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "EPD_driver.h"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

EPDDriverParams_t EPDDriver;

void app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "myAP",
            .password = "mypersonalAP",
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    EPDDriver = initEPDDriver(GPIO_NUM_4, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_5, GPIO_NUM_23, GPIO_NUM_18);
    int result = initEPDDisplay(&EPDDriver);
    printf("\r\n=================================\r\n");
    printf("Init finished with %d\r\n", result);
	printf("=================================\r\n\r\n");
    while (true) {
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

