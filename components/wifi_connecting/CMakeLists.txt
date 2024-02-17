set(pri_req nvs_flash app_http_server ConfigType ble_mesh_handle thingsboard_handle)
idf_component_register(SRCS "wifi_connecting.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)
