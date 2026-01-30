#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "moisture_sensor.h"
#include "webserver.h"

static const char *TAG = "main";

static void on_wifi_connected(void)
{
    webserver_start();
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    ESP_LOGI(TAG, "Starting moisture sensor...");

    moisture_sensor_init();
    wifi_init_sta(on_wifi_connected);
}
