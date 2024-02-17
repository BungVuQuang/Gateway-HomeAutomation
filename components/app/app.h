/**
 ******************************************************************************
 * @file		app.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
#ifndef _APP_H_
#define _APP_H_
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "common.h"
#include "esp_timer.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "esp_wifi.h"
#include "mdns.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "driver/gpio.h"
#include "esp_task_wdt.h"
#include "app_http_server.h"
#include "wifi_connecting.h"
#include "mqtt_handle.h"
#include "peripherals.h"
#include "utilities.h"
#include "ble_mesh_handle.h"
#include "ConfigType.h"
#include "clock_rtc.h"
#include "DS1307.h"
#include "OLEDDisplay.h"
#include "IrHandle.h"
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

nvs_handle_t my_handler;
extern int internet_status;
extern int zlife_node[6];
extern int connect_thingsboard;
/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/

/**
 *  @brief Hàm này được gọi lại mỗi khi nhận được dữ liệu wifi local
 *
 */
void Wifi_init(void);

/**
 *  @brief Hàm này được gọi lại mỗi khi nhận được dữ liệu wifi local
 *
 *  @param[in] data dữ liệu
 *  @param[in] len chiều dài dữ liệu
 *  @return None
 */
void RTC_Init(void);
void wifi_data_callback(char *data, int len);
void System_Init(void);
#endif /* _APP_H_ */
       /********************************* END OF FILE ********************************/
       /******************************************************************************/