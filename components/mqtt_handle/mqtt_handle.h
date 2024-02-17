/**
 ******************************************************************************
 * @file		mqtt_handle.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
#ifndef _MQTT_HANDLE_H_
#define _MQTT_HANDLE_H_
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "common.h"
#include "app.h"
#include "mqtt_client.h"
#include "ble_mesh_handle.h"
#include "wifi_connecting.h"
#include "ConfigType.h"

/*-----------------------------------------------------------------------------*/
/* Constant definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Macro definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables  */
/*-----------------------------------------------------------------------------*/
extern esp_mqtt_client_handle_t clientMqtt[4];
extern char dataMqttMesh[200];
extern char dataMqttAlarm[100];
// extern char accessToken[7][30];
/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/
/**
 *  @brief Kết nối đến Broker
 *  @return None
 */
/// void mqtt_activeGW_connect(void);
void mqtt_connect(int index);
void mqtt_Gateway_connect(void);
#endif /* _MQTT_HANDLE_H_ */
       //********************************* END OF FILE ********************************/
       //******************************************************************************/