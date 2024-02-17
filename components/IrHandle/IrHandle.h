#ifndef _IRHANDLE_H_
#define _IRHANDLE_H_

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "ir_tools.h"
#include "common.h"
#include "freertos/event_groups.h"
#include "OLEDDisplay.h"

#define BUTTON_1 0xba45
#define BUTTON_2 0xb946
#define BUTTON_3 0xb847
#define BUTTON_4 0xbb44
#define BUTTON_5 0xbf40
#define BUTTON_6 0xbc43
#define BUTTON_7 0xf807
#define BUTTON_8 0xea15
#define BUTTON_9 0xf609
#define BUTTON_0 0xe619
#define BUTTON_STAR 0xe916
#define BUTTON_THANG 0xf20d
#define BUTTON_UP 0xe718
#define BUTTON_DOWN 0xad52
#define BUTTON_OK 0xe31c
#define BUTTON_LEFT 0xf708
#define BUTTON_RIGHT 0xa55a

#define NEC_BITS 32
#define NEC_HDR_MARK 9000
#define NEC_HDR_SPACE 4500
#define NEC_BIT_MARK 560
#define NEC_ONE_SPACE 1690
#define NEC_ZERO_SPACE 560
#define NEC_RPT_SPACE 2250
#define rrmt_item32_tIMEOUT_US 9500 /*!< RMT receiver timeout value */

uint32_t cmd;
typedef enum
{
    NOT_LEARNED = 0U,
    LEARNED,
} State_Command_Rmt;

typedef struct
{
    uint16_t commandUP[440];
    uint16_t commanDown[440];
    uint16_t commandMode[440];
    uint16_t commandONOFF[440];
} AC_Properties_t;

typedef struct
{
    State_Command_Rmt commandUPLearned;
    State_Command_Rmt commandDownLearned;
    State_Command_Rmt commandModeLearned;
    State_Command_Rmt commandONOFFLearned;
} State_Command_Rx_t;

AC_Properties_t AC_Properties;
State_Command_Rx_t State_Command_Rx;
// State_LCD_t State_LCD;
EventGroupHandle_t g_control_event_group;
RingbufHandle_t rb;
ir_parser_t *ir_parser;
rmt_isr_handle_t xHandler;
void rmt_rx_config(void);

void rmt_tx_config(void);

void rmt_tx_sendData(int *rtmData, int lenght);

void IRAM_ATTR rmt_isr_handler(void *arg);
#endif /* _IRHANDLE_H_ */
