set(pri_req nvs_flash ble_mesh_handle ConfigType)
idf_component_register(SRCS "peripherals.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)
