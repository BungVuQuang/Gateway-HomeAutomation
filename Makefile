#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := Gateway_Mesh

EXTRA_COMPONENT_DIRS := $(IDF_PATH)/examples/bluetooth/esp_ble_mesh/common_components/button \
                        $(IDF_PATH)/examples/bluetooth/esp_ble_mesh/common_components/example_init \
                        $(IDF_PATH)/examples/bluetooth/esp_ble_mesh/common_components/example_nvs

include $(IDF_PATH)/make/project.mk
