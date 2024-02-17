set(pri_req nvs_flash mdns main app_http_server thingsboard_handle ble_mesh_handle ConfigType wifi_connecting mqtt mqtt_handle)
idf_component_register(SRCS "thingsboard_handle.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)
