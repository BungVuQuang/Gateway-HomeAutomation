/**
 ******************************************************************************
 * @file		clock_rtc.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
#ifndef _CLOCK_RTC_H_
#define _CLOCK_RTC_H_
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "common.h"
#include "esp_timer.h"
#include "esp_http_client.h"
#include "esp_flash.h"
#include "wifi_connecting.h"
#include "esp_system.h"
#include "esp32/rom/rtc.h"
#include "esp32/rom/crc.h"
#include "driver/rtc_io.h"
#include "esp_sntp.h"
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
char strftime_buf[64];
nvs_handle_t nvs_handle_1;
struct tm timeinfo;
uint8_t Hour_Incre;
struct tm timeinfo_ntp;
time_t now_ntp;
/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/
void xTaskRtcGetTime(void *pvParameter);
void read_rtc_time(struct tm *timeinfo);
void write_rtc_time(const struct tm *timeinfo);
void Get_current_date_time(void);
void Set_SystemTime_SNTP(void);
#endif /* _CLOCK_RTC_H_ */
       /********************************* END OF FILE ********************************/
       /******************************************************************************/