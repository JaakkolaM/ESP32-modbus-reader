#ifndef BOARD_H
#define BOARD_H

#ifdef BOARD_ESP32C3
#include "esp32c3_board.h"
#elif defined(BOARD_WEACT_ESP32)
#include "weact_esp32_board.h"
#else
#error "No board selected! Define BOARD_ESP32C3 or BOARD_WEACT_ESP32"
#endif

#endif
