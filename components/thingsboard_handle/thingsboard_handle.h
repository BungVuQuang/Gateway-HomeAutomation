/**
 ******************************************************************************
 * @file		thingsboard_handle.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
#ifndef _THINGSBOARD_HANDLE_H_
#define _THINGSBOARD_HANDLE_H_
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "common.h"
#include "esp_timer.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_http_client.h"
#include "esp_flash.h"
#include <stdlib.h>
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_vfs.h"
#include "nvs.h"
#include "math.h"
#include "driver/gpio.h"
#include <time.h>
#include <sys/time.h>
#include "esp_attr.h"
#include "esp_sntp.h"
#include "esp_websocket_client.h"
#include "app_http_server.h"
#include "wifi_connecting.h"
#include "ble_mesh_handle.h"
#include <cJSON.h>
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
MeshData meshDataPrevious;
extern char dataRxWs[300];
int isJustSendThingsBoard;
uint8_t isFirstRxWs[21];
//===============================  MQTT HTTP VARIABLE=================

/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/
/**
 *  @brief Task này thực hiện Publish bản tin khi có tin nhắn gửi đến
 *
 *  @param[in] pvParameters tham số truyền vào khi tạo task
 *  @return ESP_OK
 */
void websocket_app_start(void);
int findUnicastNodeByRole(MeshData *meshData, const char *role);
void parseMeshDataThingsBoard(const char *jsonData, MeshData *meshData);
void GetTokenThingsBoard(void);
void websocket_lister_resigter(uint16_t index);
void printMeshList(const MeshData *meshData);
void deleteMeshNodeByName(MeshData *meshData, char *nameNode);
void deleteMeshNodeByUnicast(MeshData *meshData, char *unicast);
void insertMeshNode(MeshData *meshData, const char *role, const char *unicast, const char *parent, const char *uuid);
void syncDateTime(const MeshData *meshData, char *dataSync);
#endif /* _THINGSBOARD_HANDLE_H_ */
       /********************************* END OF FILE ********************************/
       /******************************************************************************/