# set(srcs "main.c"
# "board.c")

# set(pri_req json_generator json_parser nvs_flash)
# idf_component_register(SRCS "${srcs}"
# INCLUDE_DIRS "."
# REQUIRES ${pri_req})
set(COMPONENT_SRCS main.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
