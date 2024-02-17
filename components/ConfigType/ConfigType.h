/**
 ******************************************************************************
 * @file		ConfigType.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/

#ifndef __CONFIG_TYPE_H
#define __CONFIG_TYPE_H
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_sensor_model_api.h"
#include "esp_bt.h"
/*-----------------------------------------------------------------------------*/
/* Constant definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Data type definitions */
/*-----------------------------------------------------------------------------*/
/*-------------------------------WIFI----------------------------------------------*/
typedef enum
{
    INITIAL_STATE,
    NORMAL_STATE,
    LOST_WIFI_STATE,
    CHANGE_PASSWORD_STATE,

} wifi_state_t;

struct wifi_info_t // struct gồm các thông tin của wifi
{
    char SSID[20];
    char PASSWORD[10];
    wifi_state_t state;
} __attribute__((packed)) wifi_info;

/*-------------------------------ThingsBoard Mesh----------------------------------------------*/

typedef struct MeshNodeInfo
{
    char role[20];
    char unicast[20];
    char parent[20];
    char uuid[20];
    struct MeshNodeInfo *next;
} MeshNodeInfo;

typedef struct
{
    MeshNodeInfo *head;
    MeshNodeInfo *tail;
} MeshData;

/*-------------------------------INTERNET STATE---------------------------------------------*/

typedef struct
{
    uint8_t led;
    uint8_t fan;
    uint8_t device1;
    uint8_t device2;
    uint8_t device3;
} DeviceStateNoInternet_t;

// typedef enum
// {
//     CONNECTED_STATE,
//     LOST_INTERNET_STATE,
// } internet_state_t;
/*-----------------------------------------------------------------------------*/
/* Macro definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables  */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/

#endif
/********************************* END OF FILE ********************************/
/******************************************************************************/