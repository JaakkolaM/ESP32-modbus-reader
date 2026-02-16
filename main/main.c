#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_storage.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "modbus_devices.h"
#include "modbus_manager.h"
#include "mqtt_client.h"

static const char *TAG = "APP";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 WiFi Manager with Modbus");

    ESP_ERROR_CHECK(nvs_storage_init());
    ESP_LOGI(TAG, "NVS storage initialized");

    ESP_ERROR_CHECK(modbus_devices_init());
    ESP_LOGI(TAG, "Modbus devices manager initialized");

    ESP_ERROR_CHECK(modbus_devices_load());
    ESP_LOGI(TAG, "Modbus devices loaded from NVS");

    ESP_ERROR_CHECK(modbus_manager_init(NULL));
    ESP_LOGI(TAG, "Modbus manager initialized");

    ESP_ERROR_CHECK(modbus_manager_start_polling());
    ESP_LOGI(TAG, "Modbus polling started");

    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_LOGI(TAG, "WiFi manager initialized");

    ESP_ERROR_CHECK(wifi_manager_start());
    ESP_LOGI(TAG, "WiFi operations started");

    ESP_ERROR_CHECK(web_server_start());
    ESP_LOGI(TAG, "Web server started");

    ESP_ERROR_CHECK(mqtt_client_init());
    ESP_LOGI(TAG, "MQTT client initialized");
    
    mqtt_config_t mqtt_cfg;
    if (nvs_load_mqtt_config(&mqtt_cfg) == ESP_OK && mqtt_cfg.enabled) {
        if (strlen(mqtt_cfg.broker) > 0) {
            ESP_ERROR_CHECK(mqtt_client_start(&mqtt_cfg));
            ESP_LOGI(TAG, "MQTT client started");
        }
    }

    ESP_LOGI(TAG, "ESP32 WiFi Manager with Modbus is running");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "System running normally");
    }
}
