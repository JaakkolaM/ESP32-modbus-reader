#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#define MODBUS_DEFAULT_TX_PIN 21
#define MODBUS_DEFAULT_RX_PIN 20
#define MODBUS_DEFAULT_DE_PIN 7
#define MODBUS_DEFAULT_RE_PIN 6
#define MODBUS_DEFAULT_BAUDRATE 9600
#define MODBUS_DEFAULT_TIMEOUT_MS 1000
#define MODBUS_MAX_RETRY_ATTEMPTS 3

typedef enum {
    MODBUS_RESULT_OK = 0,
    MODBUS_RESULT_TIMEOUT,
    MODBUS_RESULT_CRC_ERROR,
    MODBUS_RESULT_EXCEPTION,
    MODBUS_RESULT_INVALID_RESPONSE,
    MODBUS_RESULT_UART_ERROR,
    MODBUS_RESULT_NOT_INITIALIZED
} modbus_result_t;

typedef struct {
    int tx_pin;
    int rx_pin;
    int de_pin;
    int re_pin;
    uint32_t baudrate;
    uint32_t timeout_ms;
    uint8_t retry_attempts;
    bool initialized;
} modbus_config_t;

esp_err_t modbus_manager_init(modbus_config_t *config);
esp_err_t modbus_manager_deinit(void);
bool modbus_manager_is_initialized(void);

modbus_result_t modbus_read_holding_registers(uint8_t device_id, uint16_t address, 
                                           uint16_t count, uint16_t *values);
modbus_result_t modbus_read_input_registers(uint8_t device_id, uint16_t address,
                                          uint16_t count, uint16_t *values);
modbus_result_t modbus_read_coils(uint8_t device_id, uint16_t address,
                                  uint16_t count, uint8_t *values);
modbus_result_t modbus_read_discrete_inputs(uint8_t device_id, uint16_t address,
                                          uint16_t count, uint8_t *values);

modbus_result_t modbus_write_single_register(uint8_t device_id, uint16_t address,
                                           uint16_t value);
modbus_result_t modbus_write_multiple_registers(uint8_t device_id, uint16_t address,
                                             uint16_t *values, uint16_t count);
modbus_result_t modbus_write_single_coil(uint8_t device_id, uint16_t address,
                                        bool value);
modbus_result_t modbus_write_multiple_coils(uint8_t device_id, uint16_t address,
                                          uint8_t *values, uint16_t count);

esp_err_t modbus_manager_start_polling(void);
esp_err_t modbus_manager_stop_polling(void);
bool modbus_manager_is_polling(void);

uint32_t modbus_manager_get_last_error(void);
const char* modbus_result_to_string(modbus_result_t result);

void modbus_manager_set_logging(bool enabled);
bool modbus_manager_get_logging(void);

#endif