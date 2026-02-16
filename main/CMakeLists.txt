idf_component_register(SRCS "main.c" "wifi_manager.c" "web_server.c" "nvs_storage.c"
                       "modbus_protocol.c" "modbus_devices.c" "modbus_manager.c"
                       "mqtt_client.c"
                     INCLUDE_DIRS "."
                     EMBED_FILES "html/index.html" "html/style.css" "html/script.js"
                     "html/modbus.html" "html/dashboard.html" "html/modbus.js"
                     "html/mqtt.html"
                     REQUIRES esp_mqtt)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-error=format" "-Wformat-security")
