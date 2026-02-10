#ifndef NVS_STORAGE_H
#define NVS_STORAGE_H

#include <stdbool.h>
#include "esp_err.h"

#define NVS_WIFI_NAMESPACE "wifi_config"
#define NVS_SSID_KEY "ssid"
#define NVS_PASSWORD_KEY "password"
#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASSWORD_MAX_LEN 64

#define NVS_MODBUS_NAMESPACE "modbus_config"
#define NVS_LOGGING_KEY "logging_enabled"

esp_err_t nvs_storage_init(void);
esp_err_t nvs_save_wifi_credentials(const char *ssid, const char *password);
esp_err_t nvs_load_wifi_credentials(char *ssid, char *password);
esp_err_t nvs_clear_wifi_credentials(void);
bool nvs_has_credentials(void);

esp_err_t nvs_save_modbus_logging(bool enabled);
esp_err_t nvs_load_modbus_logging(bool *enabled);

#endif
