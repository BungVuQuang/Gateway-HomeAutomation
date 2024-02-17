set(pri_req nvs_flash mqtt ble_mesh_handle ConfigType wifi_connecting app)
idf_component_register(SRCS "mqtt_handle.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)
