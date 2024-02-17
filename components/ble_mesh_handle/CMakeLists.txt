# //include_directories(D:\\esp32-idf\\Espressif\\frameworks\\esp-idf-v4.4.3\\examples\\bluetooth\\esp_ble_mesh\\common_components\\example_init)
set(pri_req nvs_flash main app_http_server ConfigType wifi_connecting thingsboard_handle bt)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/bluetooth/esp_ble_mesh/common_components/example_init
    $ENV{IDF_PATH}/examples/bluetooth/esp_ble_mesh/common_components/example_nvs)
idf_component_register(SRCS "ble_mesh_handle.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)

# target_include_directories(ble_mesh_handle PUBLIC D:/esp32-idf/Espressif/frameworks/esp-idf-v4.4.3/examples/bluetooth/esp_ble_mesh/common_components/example_init)
# target_include_directories(ble_mesh_handle PUBLIC D:/esp32-idf/Espressif/frameworks/esp-idf-v4.4.3/examples/bluetooth/esp_ble_mesh/common_components/example_nvs)

# target_include_directories(${COMPONENT_LIB} PUBLIC "D:/esp32-idf/Espressif/frameworks/esp-idf-v4.4.3/examples/bluetooth/esp_ble_mesh/common_components/example_init")
# target_include_directories(${COMPONENT_LIB} PUBLIC "D:/esp32-idf/Espressif/frameworks/esp-idf-v4.4.3/examples/bluetooth/esp_ble_mesh/common_components/example_nvs")
# set(COMPONENT_ADD_INCLUDEDIRS ".")

# set(COMPONENT_SRCS ble_mesh.c)
# set(COMPONENT_ADD_INCLUDEDIRS ".")
