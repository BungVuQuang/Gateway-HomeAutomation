/**
 ******************************************************************************
 * @file		utilities.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
#ifndef __UTILITIES_H
#define __UTILITIES_H
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "app.h"
#include "freertos/event_groups.h"
#include "ConfigType.h"
#include "ping/ping_sock.h"
/*-----------------------------------------------------------------------------*/
/* Constant definitions  */
/*-----------------------------------------------------------------------------*/
#define INPUT_PIN 0

/*-----------------------------------------------------------------------------*/
/* Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Macro definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables  */
/*-----------------------------------------------------------------------------*/
// extern int state;

/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/

void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args);
void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args);
void cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *args);
esp_err_t initialize_ping(uint32_t interval_ms, uint32_t task_prio, char *target_host);

#endif
//********************************* END OF FILE ********************************/
//******************************************************************************/