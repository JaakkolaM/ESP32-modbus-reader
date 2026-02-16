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

#define NVS_MQTT_NAMESPACE "mqtt_config"
#define NVS_MQTT_ENABLED_KEY "enabled"
#define NVS_MQTT_BROKER_KEY "broker"
#define NVS_MQTT_PORT_KEY "port"
#define NVS_MQTT_USERNAME_KEY "username"
#define NVS_MQTT_PASSWORD_KEY "password"
#define NVS_MQTT_PREFIX_KEY "prefix"
#define NVS_MQTT_INTERVAL_KEY "interval"

#define MQTT_BROKER_MAX_LEN 64
#define MQTT_USERNAME_MAX_LEN 32
#define MQTT_PASSWORD_MAX_LEN 64
#define MQTT_PREFIX_MAX_LEN 32
#define MQTT_DEFAULT_PREFIX "esp32modbus"
#define MQTT_DEFAULT_PORT 1883
#define MQTT_DEFAULT_INTERVAL 30

typedef struct {
    bool enabled;
    char broker[MQTT_BROKER_MAX_LEN];
    uint16_t port;
    char username[MQTT_USERNAME_MAX_LEN];
    char password[MQTT_PASSWORD_MAX_LEN];
    char prefix[MQTT_PREFIX_MAX_LEN];
    uint16_t publish_interval;
} mqtt_config_t;

esp_err_t nvs_storage_init(void);
esp_err_t nvs_save_wifi_credentials(const char *ssid, const char *password);
esp_err_t nvs_load_wifi_credentials(char *ssid, char *password);
esp_err_t nvs_clear_wifi_credentials(void);
bool nvs_has_credentials(void);

esp_err_t nvs_save_modbus_logging(bool enabled);
esp_err_t nvs_load_modbus_logging(bool *enabled);

esp_err_t nvs_save_mqtt_config(const mqtt_config_t *config);
esp_err_t nvs_load_mqtt_config(mqtt_config_t *config);

#endif
