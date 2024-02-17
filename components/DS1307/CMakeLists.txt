set(pri_req)
idf_component_register(SRCS "DS1307.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})
