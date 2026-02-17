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
#include "mqtt_gateway.h"
#include <strings.h>

static const char *TAG = "APP";

static void mqtt_write_callback(uint8_t device_id, uint16_t address, uint16_t value)
{
    modbus_device_t *device = modbus_get_device(device_id);
    if (device == NULL) {
        ESP_LOGE(TAG, "Device %d not found", device_id);
        return;
    }

    modbus_register_t *reg = modbus_get_register(device_id, address);
    if (reg == NULL) {
        ESP_LOGE(TAG, "Register %d not found in device %d", address, device_id);
        return;
    }

    modbus_result_t result = MODBUS_RESULT_OK;

    if (reg->type == REGISTER_TYPE_COIL) {
        bool coil_value = (value != 0);
        ESP_LOGI(TAG, "Writing to coil: device=%d, address=%d, value=%s",
                  device_id, address, coil_value ? "ON" : "OFF");
        result = modbus_write_single_coil(device_id, address, coil_value);
    } else if (reg->type == REGISTER_TYPE_HOLDING && reg->writable) {
        ESP_LOGI(TAG, "Writing to holding register: device=%d, address=%d, value=%d",
                  device_id, address, value);
        result = modbus_write_single_register(device_id, address, value);
    } else {
        ESP_LOGW(TAG, "Register type %d at address %d is not writable", reg->type, address);
        return;
    }

    if (result == MODBUS_RESULT_OK) {
        modbus_update_register_value(device_id, address, value);
        mqtt_client_publish_register(device_id, device->name, reg);
        ESP_LOGI(TAG, "Successfully wrote to register %d on device %d", address, device_id);
    } else {
        ESP_LOGE(TAG, "Failed to write to register %d on device %d: %s",
                  address, device_id, modbus_result_to_string(result));
    }
}

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
            mqtt_client_set_register_write_callback(mqtt_write_callback);
            ESP_LOGI(TAG, "MQTT client started");
        }
    }

    ESP_LOGI(TAG, "ESP32 WiFi Manager with Modbus is running");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "System running normally");
    }
}
