/**
 ******************************************************************************
 * @file		common.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 * Copyright (C) 2023 - Bung.VQ
 ******************************************************************************
 **/
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#ifndef __COMMON_CONFIG_H
#define __COMMON_CONFIG_H

#include "ble_mesh_example_init.h"
#include "ble_mesh_example_nvs.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"
/*-----------------------------------------------------------------------------*/
/* Constant definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Macro definitions  */
/*-----------------------------------------------------------------------------*/
#define CONNECTED_STATE 0
#define LOST_INTERNET_STATE 1
//================s_thingsboard_event_group
#define GET_TOKEN_COMPLETE BIT3
#define WEBSOCKET_RECIVE_MESSAGE BIT0
#define MQTT_MESSAGE_ALARM BIT1
#define MQTT_MESSAGE_MESH BIT2

//================s_mesh_network_event_group
#define MESH_MESSAGE_ARRIVE_BIT BIT0

//================s_wifi_event_group
#define WIFI_RECV_INFO BIT0
#define WIFI_CONNECTED_BIT BIT1
#define WIFI_FAIL_BIT BIT2

//================s_control_event_group
#define OLED_SWITCH_DISPLAY BIT3
#define INFRATE_RECIVE_MESSAGE BIT0
#define SET_COMMAND_RECIVE_MESSAGE BIT1
#define SETUP_RECIVE_MESSAGE BIT2
/*-----------------------------------------------------------------------------*/
/* Global variables  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/

#endif
//********************************* END OF FILE ********************************/
//******************************************************************************/
