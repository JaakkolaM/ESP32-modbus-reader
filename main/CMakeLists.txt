idf_component_register(SRCS "main.c" "wifi_manager.c" "web_server.c" "nvs_storage.c"
                       "modbus_protocol.c" "modbus_devices.c" "modbus_manager.c"
                       "mqtt_gateway.c"
                      INCLUDE_DIRS "." "../boards"
                      EMBED_FILES "html/index.html" "html/style.css" "html/script.js"
                      "html/modbus.html" "html/dashboard.html" "html/modbus.js"
                      "html/mqtt.html"
                      REQUIRES driver nvs_flash mqtt esp_netif esp_event esp_wifi esp_http_server json)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-error=format" "-Wformat-security")

if(NOT BOARD)
    set(BOARD "esp32c3" CACHE STRING "Board selection")
endif()

set_property(CACHE BOARD PROPERTY STRINGS "esp32c3" "weact_esp32")

message(STATUS "Building for board: ${BOARD}")

if(BOARD STREQUAL "esp32c3")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE BOARD_ESP32C3)
elseif(BOARD STREQUAL "weact_esp32")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE BOARD_WEACT_ESP32)
endif()
