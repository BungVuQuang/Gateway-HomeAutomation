set(pri_req infrared_tools Oled main)
idf_component_register(SRCS "IrHandle.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})
