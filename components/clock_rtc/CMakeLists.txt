set(pri_req nvs_flash main thingsboard_handle ble_mesh_handle ConfigType wifi_connecting mqtt mqtt_handle)
idf_component_register(SRCS "clock_rtc.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)
