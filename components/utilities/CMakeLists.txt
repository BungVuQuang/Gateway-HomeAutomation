set(pri_req ConfigType ble_mesh_handle)
idf_component_register(SRCS "utilities.c"
    INCLUDE_DIRS "."
    REQUIRES ${pri_req}
)
