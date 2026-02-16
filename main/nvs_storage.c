#include "nvs_storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "NVS_STORAGE";

esp_err_t nvs_storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition needs to be erased, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS storage initialized");
    return ESP_OK;
}

esp_err_t nvs_save_wifi_credentials(const char *ssid, const char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_WIFI_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS namespace: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_str(nvs_handle, NVS_SSID_KEY, ssid);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving SSID: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_set_str(nvs_handle, NVS_PASSWORD_KEY, password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving password: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "WiFi credentials saved successfully");
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t nvs_load_wifi_credentials(char *ssid, char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_WIFI_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS namespace: %s", esp_err_to_name(err));
        return err;
    }

    size_t required_size;

    required_size = WIFI_SSID_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_SSID_KEY, ssid, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading SSID: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    required_size = WIFI_PASSWORD_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_PASSWORD_KEY, password, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error reading password: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    ESP_LOGI(TAG, "WiFi credentials loaded: %s", ssid);
    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t nvs_clear_wifi_credentials(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_WIFI_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS namespace: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_erase_all(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error erasing NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "WiFi credentials cleared successfully");
    }

    nvs_close(nvs_handle);
    return err;
}

bool nvs_has_credentials(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_WIFI_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return false;
    }

    size_t required_size;
    err = nvs_get_str(nvs_handle, NVS_SSID_KEY, NULL, &required_size);
    nvs_close(nvs_handle);

    return (err == ESP_OK);
}

esp_err_t nvs_save_modbus_logging(bool enabled)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_MODBUS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS namespace: %s", esp_err_to_name(err));
        return err;
    }

    uint8_t enabled_u8 = enabled ? 1 : 0;
    err = nvs_set_u8(nvs_handle, NVS_LOGGING_KEY, enabled_u8);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving logging config: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Modbus logging config saved: %s", enabled ? "enabled" : "disabled");
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t nvs_load_modbus_logging(bool *enabled)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_MODBUS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Modbus logging config not found, using default (disabled)");
        return ESP_ERR_NOT_FOUND;
    }

    uint8_t enabled_u8;
    err = nvs_get_u8(nvs_handle, NVS_LOGGING_KEY, &enabled_u8);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error reading logging config: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    *enabled = enabled_u8 ? true : false;
    ESP_LOGI(TAG, "Modbus logging config loaded: %s", *enabled ? "enabled" : "disabled");
    nvs_close(nvs_handle);
    return ESP_OK;
}

esp_err_t nvs_save_mqtt_config(const mqtt_config_t *config)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_MQTT_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening MQTT NVS namespace: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_u8(nvs_handle, NVS_MQTT_ENABLED_KEY, config->enabled ? 1 : 0);
    if (err != ESP_OK) goto save_error;

    err = nvs_set_str(nvs_handle, NVS_MQTT_BROKER_KEY, config->broker);
    if (err != ESP_OK) goto save_error;

    err = nvs_set_u16(nvs_handle, NVS_MQTT_PORT_KEY, config->port);
    if (err != ESP_OK) goto save_error;

    err = nvs_set_str(nvs_handle, NVS_MQTT_USERNAME_KEY, config->username);
    if (err != ESP_OK) goto save_error;

    err = nvs_set_str(nvs_handle, NVS_MQTT_PASSWORD_KEY, config->password);
    if (err != ESP_OK) goto save_error;

    err = nvs_set_str(nvs_handle, NVS_MQTT_PREFIX_KEY, config->prefix);
    if (err != ESP_OK) goto save_error;

    err = nvs_set_u16(nvs_handle, NVS_MQTT_INTERVAL_KEY, config->publish_interval);
    if (err != ESP_OK) goto save_error;

save_error:
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving MQTT config: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "MQTT config saved successfully");
    }

    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return err;
}

esp_err_t nvs_load_mqtt_config(mqtt_config_t *config)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_MQTT_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "MQTT config not found in NVS, using defaults");
        config->enabled = false;
        config->broker[0] = '\0';
        config->port = MQTT_DEFAULT_PORT;
        config->username[0] = '\0';
        config->password[0] = '\0';
        strcpy(config->prefix, MQTT_DEFAULT_PREFIX);
        config->publish_interval = MQTT_DEFAULT_INTERVAL;
        return ESP_ERR_NOT_FOUND;
    }

    uint8_t enabled;
    err = nvs_get_u8(nvs_handle, NVS_MQTT_ENABLED_KEY, &enabled);
    config->enabled = (err == ESP_OK) ? (enabled != 0) : false;

    size_t len = MQTT_BROKER_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_MQTT_BROKER_KEY, config->broker, &len);
    if (err != ESP_OK || len == 0) {
        config->broker[0] = '\0';
    }

    uint16_t port;
    err = nvs_get_u16(nvs_handle, NVS_MQTT_PORT_KEY, &port);
    config->port = (err == ESP_OK) ? port : MQTT_DEFAULT_PORT;

    len = MQTT_USERNAME_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_MQTT_USERNAME_KEY, config->username, &len);
    if (err != ESP_OK || len == 0) {
        config->username[0] = '\0';
    }

    len = MQTT_PASSWORD_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_MQTT_PASSWORD_KEY, config->password, &len);
    if (err != ESP_OK || len == 0) {
        config->password[0] = '\0';
    }

    len = MQTT_PREFIX_MAX_LEN;
    err = nvs_get_str(nvs_handle, NVS_MQTT_PREFIX_KEY, config->prefix, &len);
    if (err != ESP_OK || len == 0) {
        strcpy(config->prefix, MQTT_DEFAULT_PREFIX);
    }

    uint16_t interval;
    err = nvs_get_u16(nvs_handle, NVS_MQTT_INTERVAL_KEY, &interval);
    config->publish_interval = (err == ESP_OK) ? interval : MQTT_DEFAULT_INTERVAL;

    ESP_LOGI(TAG, "MQTT config loaded: enabled=%d, broker=%s, port=%d, prefix=%s",
              config->enabled, config->broker, config->port, config->prefix);

    nvs_close(nvs_handle);
    return ESP_OK;
}
