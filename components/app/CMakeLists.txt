set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/bluetooth/esp_ble_mesh/common_components/example_init
    $ENV{IDF_PATH}/examples/bluetooth/esp_ble_mesh/common_components/example_nvs
    $ENV{IDF_PATH}/components/bt)

set(pri_req nvs_flash mdns main clock_rtc DS1307 Oled IrHandle app_http_server ble_mesh_handle ConfigType wifi_connecting mqtt mqtt_handle peripherals)
idf_component_register(SRCS "app.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)

# target_include_directories(ble_mesh_handle PUBLIC D:/esp32-idf/Espressif/frameworks/esp-idf-v4.4.3/examples/bluetooth/esp_ble_mesh/common_components/example_init)
# target_include_directories(ble_mesh_handle PUBLIC D:/esp32-idf/Espressif/frameworks/esp-idf-v4.4.3/examples/bluetooth/esp_ble_mesh/common_components/example_nvs)

# set(COMPONENT_SRCS app.c)
# set(COMPONENT_ADD_INCLUDEDIRS ".")

# register_component()
