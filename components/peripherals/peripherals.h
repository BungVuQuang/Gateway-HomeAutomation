/**
 ******************************************************************************
 * @file		peripherals.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_
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
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "ConfigType.h"
#include "app.h"
#include "ble_mesh_handle.h"
#include "wifi_connecting.h"
/*-----------------------------------------------------------------------------*/
/* Constant definitions  */
/*-----------------------------------------------------------------------------*/
#define BUTTON_UP_PIN 36
#define BUTTON_OK_PIN 35
#define BUTTON_DOWN_PIN 39

#define LED_NETWORK_PIN 19
#define INFRATED_PIN 23
/*-----------------------------------------------------------------------------*/
/* Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Macro definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/
/**
 *  @brief Config các chân đầu vào
 *
 *  @return None
 */
void input_create(int pin);
void output_create(int pin);
/**
 *  @brief Config các chân đầu vào
 *
 */
void IRAM_ATTR gpio_interrupt_handler(void *args);
void IRAM_ATTR gpio_interrupt_handler2(void *args);
void IRAM_ATTR gpio_interrupt_handler3(void *args);

/**
 *  @brief Config các chân đầu vào
 *
 *  @return None
 */
void Show_Backup_Task(void *params);
#endif /* _PERIPHERALS_H_ */
       //********************************* END OF FILE ********************************/
       //******************************************************************************/