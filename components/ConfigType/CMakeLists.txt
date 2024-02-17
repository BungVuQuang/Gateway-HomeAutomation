set(pri_req nvs_flash app_http_server ble_mesh_handle ConfigType wifi_connecting mqtt mqtt_handle utilities peripherals)
idf_component_register(SRCS "ConfigType.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)
