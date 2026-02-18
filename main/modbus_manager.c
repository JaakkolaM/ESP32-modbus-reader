#include "modbus_manager.h"
#include "modbus_protocol.h"
#include "modbus_devices.h"
#include "nvs_storage.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

static const char *TAG = "MODBUS_MANAGER";

#define UART_NUM UART_NUM_1
#define BUF_SIZE 256

static modbus_config_t modbus_config;
static TaskHandle_t polling_task_handle = NULL;
static volatile bool polling_active = false;
static volatile uint32_t last_error = 0;
static bool modbus_logging_enabled = false;
static SemaphoreHandle_t modbus_mutex = NULL;

static void log_hex_dump(const uint8_t *data, uint16_t len)
{
    if (!modbus_logging_enabled || data == NULL || len == 0) {
        return;
    }

    char hex_str[256] = {0};
    int pos = 0;
    int max_bytes = (len > 64) ? 64 : len;

    for (int i = 0; i < max_bytes && pos < sizeof(hex_str) - 3; i++) {
        pos += snprintf(hex_str + pos, sizeof(hex_str) - pos, "%02X ", data[i]);
    }

    if (len > 64) {
        snprintf(hex_str + pos, sizeof(hex_str) - pos, "...(+%d)", len - 64);
    }

    ESP_LOGI(TAG, "FRAME: %s", hex_str);
}

static void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = modbus_config.baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, (int)modbus_config.tx_pin, (int)modbus_config.rx_pin,
                                UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0));

    ESP_LOGI(TAG, "UART initialized: TX=%d, RX=%d, Baud=%d",
              modbus_config.tx_pin, modbus_config.rx_pin, modbus_config.baudrate);
}

static void gpio_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << modbus_config.de_pin) | (1ULL << modbus_config.re_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(modbus_config.de_pin, 0);
    gpio_set_level(modbus_config.re_pin, 0);

    ESP_LOGI(TAG, "GPIO initialized: DE=%d, RE=%d", modbus_config.de_pin, modbus_config.re_pin);
}

static void set_transmit_mode(void)
{
    gpio_set_level(modbus_config.de_pin, 1);
    gpio_set_level(modbus_config.re_pin, 1);
}

static void set_receive_mode(void)
{
    gpio_set_level(modbus_config.de_pin, 0);
    gpio_set_level(modbus_config.re_pin, 0);
}

static modbus_result_t send_request(uint8_t *frame, uint16_t frame_len)
{
    int64_t start_time = esp_timer_get_time();

    set_transmit_mode();
    uart_flush_input(UART_NUM);

    ESP_LOGI(TAG, "SENDING: DevID=%d, FC=0x%02X, Addr=%d, Qty=%d, Bytes=%d",
              frame[0], frame[1], (frame[2] << 8) | frame[3], (frame[4] << 8) | frame[5], frame_len);

    log_hex_dump(frame, frame_len);

    int written = uart_write_bytes(UART_NUM, (const char *)frame, frame_len);
    if (written != frame_len) {
        ESP_LOGE(TAG, "Failed to write all bytes to UART: %d/%d", written, frame_len);
        set_receive_mode();
        return MODBUS_RESULT_UART_ERROR;
    }

    uart_wait_tx_done(UART_NUM, pdMS_TO_TICKS(100));
    set_receive_mode();

    int64_t tx_time = (esp_timer_get_time() - start_time) / 1000;
    ESP_LOGI(TAG, "TX completed in %lld ms", tx_time);

    return MODBUS_RESULT_OK;
}

static modbus_result_t receive_response(uint8_t *frame, uint16_t *frame_len)
{
    int64_t start_time = esp_timer_get_time();

    uint8_t buf[BUF_SIZE];
    int len = uart_read_bytes(UART_NUM, buf, sizeof(buf), pdMS_TO_TICKS(modbus_config.timeout_ms));

    if (len < 3) {
        ESP_LOGW(TAG, "Timeout waiting for response: %d bytes", len);
        return MODBUS_RESULT_TIMEOUT;
    }

    if (!modbus_validate_crc(buf, len)) {
        ESP_LOGE(TAG, "CRC validation failed");
        return MODBUS_RESULT_CRC_ERROR;
    }

    ESP_LOGI(TAG, "RECEIVED: %d bytes, DevID=%d, FC=0x%02X",
              len, buf[0], buf[1]);

    log_hex_dump(buf, len);

    int64_t rx_time = (esp_timer_get_time() - start_time) / 1000;
    ESP_LOGI(TAG, "RX completed in %lld ms", rx_time);

    memcpy(frame, buf, len);
    *frame_len = len;

    return MODBUS_RESULT_OK;
}

static modbus_result_t execute_modbus_transaction(uint8_t device_id, uint8_t function,
                                                uint16_t address, uint16_t quantity,
                                                const uint8_t *data, uint16_t data_len,
                                                uint8_t *response_frame, uint16_t *response_len)
{
    int64_t transaction_start = esp_timer_get_time();

    if (modbus_mutex == NULL) {
        ESP_LOGE(TAG, "Modbus mutex not initialized");
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    if (xSemaphoreTake(modbus_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take Modbus mutex");
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t request_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t request_len = 0;
    modbus_response_t response;
    modbus_result_t result = MODBUS_RESULT_OK;

    ESP_LOGI(TAG, "TRANSACTION START: DevID=%d, FC=0x%02X (%s), Addr=%d, Qty=%d",
              device_id, function, modbus_function_to_string(function), address, quantity);

    esp_err_t err = modbus_build_request(device_id, function, address, quantity,
                                        data, data_len, request_frame, &request_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to build request frame");
        xSemaphoreGive(modbus_mutex);
        return MODBUS_RESULT_INVALID_RESPONSE;
    }

    for (uint8_t retry = 0; retry < modbus_config.retry_attempts; retry++) {
        result = send_request(request_frame, request_len);
        if (result != MODBUS_RESULT_OK) {
            ESP_LOGW(TAG, "ATTEMPT %d/%d: DevID=%d, FC=0x%02X, Addr=%d, Result=%s",
                      retry + 1, modbus_config.retry_attempts, device_id, function, address,
                      modbus_result_to_string(result));
            continue;
        }

        result = receive_response(response_frame, response_len);
        if (result != MODBUS_RESULT_OK) {
            ESP_LOGW(TAG, "ATTEMPT %d/%d: DevID=%d, FC=0x%02X, Addr=%d, Result=%s",
                      retry + 1, modbus_config.retry_attempts, device_id, function, address,
                      modbus_result_to_string(result));
            continue;
        }

        err = modbus_parse_response(response_frame, *response_len, &response);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to parse response: %s", esp_err_to_name(err));
            result = MODBUS_RESULT_INVALID_RESPONSE;
            ESP_LOGW(TAG, "ATTEMPT %d/%d: DevID=%d, FC=0x%02X, Addr=%d, Result=%s",
                      retry + 1, modbus_config.retry_attempts, device_id, function, address,
                      modbus_result_to_string(result));
            continue;
        }

        if (response.is_exception) {
            ESP_LOGE(TAG, "Modbus exception: %s", modbus_exception_to_string(response.exception_code));
            last_error = response.exception_code;
            result = MODBUS_RESULT_EXCEPTION;
            ESP_LOGW(TAG, "ATTEMPT %d/%d: DevID=%d, FC=0x%02X, Addr=%d, Result=Exception",
                      retry + 1, modbus_config.retry_attempts, device_id, function, address);
            break;
        }

        ESP_LOGI(TAG, "ATTEMPT %d/%d: DevID=%d, FC=0x%02X, Addr=%d, Result=OK",
                  retry + 1, modbus_config.retry_attempts, device_id, function, address);

        int64_t total_time = (esp_timer_get_time() - transaction_start) / 1000;
        ESP_LOGI(TAG, "TRANSACTION SUCCESS: DevID=%d, FC=0x%02X, Attempts=%d, Total Time=%lld ms",
                  device_id, function, retry + 1, total_time);

        last_error = 0;
        xSemaphoreGive(modbus_mutex);
        return MODBUS_RESULT_OK;
    }

    int64_t total_time = (esp_timer_get_time() - transaction_start) / 1000;
    ESP_LOGE(TAG, "TRANSACTION FAILED: DevID=%d, FC=0x%02X, Attempts=%d, Total Time=%lld ms",
              device_id, function, modbus_config.retry_attempts, total_time);

    xSemaphoreGive(modbus_mutex);
    return result;
}

esp_err_t modbus_manager_init(modbus_config_t *config)
{
    if (modbus_config.initialized) {
        ESP_LOGW(TAG, "Modbus manager already initialized");
        return ESP_OK;
    }

    if (config == NULL) {
        modbus_config.tx_pin = MODBUS_DEFAULT_TX_PIN;
        modbus_config.rx_pin = MODBUS_DEFAULT_RX_PIN;
        modbus_config.de_pin = MODBUS_DEFAULT_DE_PIN;
        modbus_config.re_pin = MODBUS_DEFAULT_RE_PIN;
        modbus_config.baudrate = MODBUS_DEFAULT_BAUDRATE;
        modbus_config.timeout_ms = MODBUS_DEFAULT_TIMEOUT_MS;
        modbus_config.retry_attempts = MODBUS_MAX_RETRY_ATTEMPTS;
    } else {
        memcpy(&modbus_config, config, sizeof(modbus_config_t));
    }

    modbus_mutex = xSemaphoreCreateMutex();
    if (modbus_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create Modbus mutex");
        return ESP_ERR_NO_MEM;
    }

    gpio_init();
    uart_init();

    bool logging_enabled;
    if (nvs_load_modbus_logging(&logging_enabled) == ESP_OK) {
        modbus_logging_enabled = logging_enabled;
    } else {
        modbus_logging_enabled = false;
    }
    ESP_LOGI(TAG, "Modbus logging %s", modbus_logging_enabled ? "enabled" : "disabled");

    modbus_config.initialized = true;
    ESP_LOGI(TAG, "Modbus manager initialized successfully");
    return ESP_OK;
}

esp_err_t modbus_manager_deinit(void)
{
    if (!modbus_config.initialized) {
        return ESP_OK;
    }

    if (polling_active) {
        modbus_manager_stop_polling();
    }

    if (modbus_mutex != NULL) {
        vSemaphoreDelete(modbus_mutex);
        modbus_mutex = NULL;
    }

    uart_driver_delete(UART_NUM);
    gpio_reset_pin(modbus_config.de_pin);
    gpio_reset_pin(modbus_config.re_pin);

    modbus_config.initialized = false;
    ESP_LOGI(TAG, "Modbus manager deinitialized");
    return ESP_OK;
}

bool modbus_manager_is_initialized(void)
{
    return modbus_config.initialized;
}

modbus_result_t modbus_read_holding_registers(uint8_t device_id, uint16_t address,
                                           uint16_t count, uint16_t *values)
{
    if (!modbus_config.initialized) {
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t response_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t response_len = 0;
    modbus_response_t response;

    modbus_result_t result = execute_modbus_transaction(device_id, MODBUS_FC_READ_HOLDING_REGISTERS,
                                                     address, count, NULL, 0,
                                                     response_frame, &response_len);
    if (result != MODBUS_RESULT_OK) {
        return result;
    }

    if (modbus_parse_response(response_frame, response_len, &response) != ESP_OK) {
        return MODBUS_RESULT_INVALID_RESPONSE;
    }

    if (response.byte_count != count * 2) {
        ESP_LOGE(TAG, "Unexpected byte count: %d (expected %d)", response.byte_count, count * 2);
        return MODBUS_RESULT_INVALID_RESPONSE;
    }

    for (uint16_t i = 0; i < count; i++) {
        values[i] = (response.data[i * 2] << 8) | response.data[i * 2 + 1];
    }

    return MODBUS_RESULT_OK;
}

modbus_result_t modbus_read_input_registers(uint8_t device_id, uint16_t address,
                                          uint16_t count, uint16_t *values)
{
    if (!modbus_config.initialized) {
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t response_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t response_len = 0;

    modbus_result_t result = execute_modbus_transaction(device_id, MODBUS_FC_READ_INPUT_REGISTERS,
                                                     address, count, NULL, 0,
                                                     response_frame, &response_len);
    if (result != MODBUS_RESULT_OK) {
        return result;
    }

    modbus_response_t response;
    if (modbus_parse_response(response_frame, response_len, &response) != ESP_OK) {
        return MODBUS_RESULT_INVALID_RESPONSE;
    }

    if (response.byte_count != count * 2) {
        return MODBUS_RESULT_INVALID_RESPONSE;
    }

    for (uint16_t i = 0; i < count; i++) {
        values[i] = (response.data[i * 2] << 8) | response.data[i * 2 + 1];
    }

    return MODBUS_RESULT_OK;
}

modbus_result_t modbus_read_coils(uint8_t device_id, uint16_t address,
                                  uint16_t count, uint8_t *values)
{
    if (!modbus_config.initialized) {
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t response_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t response_len = 0;

    modbus_result_t result = execute_modbus_transaction(device_id, MODBUS_FC_READ_COILS,
                                                     address, count, NULL, 0,
                                                     response_frame, &response_len);
    if (result != MODBUS_RESULT_OK) {
        return result;
    }

    modbus_response_t response;
    if (modbus_parse_response(response_frame, response_len, &response) != ESP_OK) {
        return MODBUS_RESULT_INVALID_RESPONSE;
    }

    memcpy(values, response.data, response.byte_count);
    return MODBUS_RESULT_OK;
}

modbus_result_t modbus_read_discrete_inputs(uint8_t device_id, uint16_t address,
                                          uint16_t count, uint8_t *values)
{
    if (!modbus_config.initialized) {
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t response_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t response_len = 0;

    modbus_result_t result = execute_modbus_transaction(device_id, MODBUS_FC_READ_DISCRETE_INPUTS,
                                                     address, count, NULL, 0,
                                                     response_frame, &response_len);
    if (result != MODBUS_RESULT_OK) {
        return result;
    }

    modbus_response_t response;
    if (modbus_parse_response(response_frame, response_len, &response) != ESP_OK) {
        return MODBUS_RESULT_INVALID_RESPONSE;
    }

    memcpy(values, response.data, response.byte_count);
    return MODBUS_RESULT_OK;
}

modbus_result_t modbus_write_single_register(uint8_t device_id, uint16_t address,
                                           uint16_t value)
{
    if (!modbus_config.initialized) {
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t response_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t response_len = 0;

    modbus_result_t result = execute_modbus_transaction(device_id, MODBUS_FC_WRITE_SINGLE_REGISTER,
                                                     address, 1, (uint8_t*)&value, 2,
                                                     response_frame, &response_len);
    return result;
}

modbus_result_t modbus_write_multiple_registers(uint8_t device_id, uint16_t address,
                                             uint16_t *values, uint16_t count)
{
    if (!modbus_config.initialized) {
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t response_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t response_len = 0;

    modbus_result_t result = execute_modbus_transaction(device_id, MODBUS_FC_WRITE_MULTIPLE_REGISTERS,
                                                     address, count, (uint8_t*)values, count * 2,
                                                     response_frame, &response_len);
    return result;
}

modbus_result_t modbus_write_single_coil(uint8_t device_id, uint16_t address,
                                        bool value)
{
    if (!modbus_config.initialized) {
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t coil_value = value ? 0xFF : 0x00;
    uint8_t response_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t response_len = 0;

    modbus_result_t result = execute_modbus_transaction(device_id, MODBUS_FC_WRITE_SINGLE_COIL,
                                                     address, 1, &coil_value, 1,
                                                     response_frame, &response_len);
    return result;
}

modbus_result_t modbus_write_multiple_coils(uint8_t device_id, uint16_t address,
                                          uint8_t *values, uint16_t count)
{
    if (!modbus_config.initialized) {
        return MODBUS_RESULT_NOT_INITIALIZED;
    }

    uint8_t response_frame[MODBUS_MAX_FRAME_LEN];
    uint16_t response_len = 0;

    modbus_result_t result = execute_modbus_transaction(device_id, MODBUS_FC_WRITE_MULTIPLE_COILS,
                                                     address, count, values, count,
                                                     response_frame, &response_len);
    return result;
}

static void polling_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Modbus polling task started");

    while (polling_active) {
        uint8_t device_count = modbus_get_device_count();
        if (device_count == 0) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
        uint8_t count;
        modbus_device_t *devices = modbus_list_devices(&count);
        if (devices != NULL) {
            for (uint8_t i = 0; i < count && polling_active; i++) {
                if (!devices[i].enabled) {
                    continue;
                }

                for (uint8_t j = 0; j < devices[i].register_count && polling_active; j++) {
                    uint16_t value;
                    modbus_result_t result = MODBUS_RESULT_OK;

                    switch (devices[i].registers[j].type) {
                        case REGISTER_TYPE_HOLDING:
                            result = modbus_read_holding_registers(devices[i].device_id,
                                                                devices[i].registers[j].address, 1, &value);
                            break;
                        case REGISTER_TYPE_INPUT:
                            result = modbus_read_input_registers(devices[i].device_id,
                                                               devices[i].registers[j].address, 1, &value);
                            break;
                        case REGISTER_TYPE_COIL:
                            {
                                uint8_t coil_val;
                                result = modbus_read_coils(devices[i].device_id,
                                                          devices[i].registers[j].address, 1, &coil_val);
                                value = coil_val;
                            }
                            break;
                        case REGISTER_TYPE_DISCRETE:
                            {
                                uint8_t discrete_val;
                                result = modbus_read_discrete_inputs(devices[i].device_id,
                                                                devices[i].registers[j].address, 1, &discrete_val);
                                value = discrete_val;
                            }
                            break;
                    }

                    devices[i].poll_count++;
                    if (result == MODBUS_RESULT_OK) {
                        modbus_update_register_value(devices[i].device_id,
                                                   devices[i].registers[j].address, value);
                        devices[i].last_seen = xTaskGetTickCount() * portTICK_PERIOD_MS;
                        devices[i].status = DEVICE_STATUS_ONLINE;
                    } else {
                        devices[i].error_count++;
                        devices[i].last_error = last_error;
                        devices[i].status = DEVICE_STATUS_ERROR;
                        ESP_LOGW(TAG, "Failed to read register %d from device %d: %s",
                                  devices[i].registers[j].address, devices[i].device_id,
                                  modbus_result_to_string(result));
                    }

                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                if (devices[i].register_count > 0) {
                    vTaskDelay(pdMS_TO_TICKS(devices[i].poll_interval_ms));
                }
            }
        }
    }

    ESP_LOGI(TAG, "Modbus polling task stopped");
    polling_task_handle = NULL;
    vTaskDelete(NULL);
}

esp_err_t modbus_manager_start_polling(void)
{
    if (polling_active) {
        ESP_LOGW(TAG, "Polling already active");
        return ESP_OK;
    }

    polling_active = true;
    xTaskCreate(polling_task, "modbus_poll", 12288, NULL, 5, &polling_task_handle);
    ESP_LOGI(TAG, "Modbus polling started");
    return ESP_OK;
}

esp_err_t modbus_manager_stop_polling(void)
{
    if (!polling_active) {
        return ESP_OK;
    }

    polling_active = false;
    if (polling_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    ESP_LOGI(TAG, "Modbus polling stopped");
    return ESP_OK;
}

bool modbus_manager_is_polling(void)
{
    return polling_active;
}

uint32_t modbus_manager_get_last_error(void)
{
    return last_error;
}

const char* modbus_result_to_string(modbus_result_t result)
{
    switch (result) {
        case MODBUS_RESULT_OK: return "OK";
        case MODBUS_RESULT_TIMEOUT: return "Timeout";
        case MODBUS_RESULT_CRC_ERROR: return "CRC Error";
        case MODBUS_RESULT_EXCEPTION: return "Exception";
        case MODBUS_RESULT_INVALID_RESPONSE: return "Invalid Response";
        case MODBUS_RESULT_UART_ERROR: return "UART Error";
        case MODBUS_RESULT_NOT_INITIALIZED: return "Not Initialized";
        default: return "Unknown";
    }
}

void modbus_manager_set_logging(bool enabled)
{
    modbus_logging_enabled = enabled;
    esp_err_t err = nvs_save_modbus_logging(enabled);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Modbus logging %s and saved to NVS", enabled ? "enabled" : "disabled");
    } else {
        ESP_LOGW(TAG, "Modbus logging %s but failed to save to NVS", enabled ? "enabled" : "disabled");
    }
}

bool modbus_manager_get_logging(void)
{
    return modbus_logging_enabled;
}