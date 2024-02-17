/**
 ******************************************************************************
 * @file		peripherals.c
 * @author	Vu Quang Bung
 * @date		26 June 2023
 * Copyright (C) 2023 - Bung.VQ
 ******************************************************************************
 **/
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "peripherals.h"
/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/
xQueueHandle interputQueue;

/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/
/**
 *  @brief Config các chân đầu vào
 *
 */
void input_create(int pin)
{
    gpio_config_t io_conf;
    // disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << pin);
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
}

/**
 *  @brief Config các chân đầu vào
 *
 */
void output_create(int pin)
{
    gpio_config_t io_conf;
    // disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << pin);
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
}

/**
 *  @brief Config các chân đầu vào
 *
 */
void IRAM_ATTR gpio_interrupt_handler(void *args) // hàm ISR ngắt ngoài
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interputQueue, &pinNumber, NULL);
}

/**
 *  @brief Config các chân đầu vào
 *
 */
void IRAM_ATTR gpio_interrupt_handler2(void *args) // hàm ISR ngắt ngoài
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interputQueue, &pinNumber, NULL);
}

/**
 *  @brief Config các chân đầu vào
 *
 */
void IRAM_ATTR gpio_interrupt_handler3(void *args) // hàm ISR ngắt ngoài
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interputQueue, &pinNumber, NULL);
}
/**
 *  @brief Config các chân đầu vào
 *
 */
void Show_Backup_Task(void *params)
{
    int pinNumber = 0;
    uint8_t match[2] = {0x32, 0x10}; // 2 byte đầu của UUID
    while (true)
    {
        if (xQueueReceive(interputQueue, &pinNumber, portMAX_DELAY)) // chờ có items
        {
            if (pinNumber == 35)
            {
                printf("Da an Button 35\n");
                esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);
                // if (internet_check == 1)
                //{
                // esp_ble_mesh_delete_node(0x05);
                // esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);
                //}
            }
            else if (pinNumber == 36)
            {
                printf("Da an Button 36\n");

                // esp_ble_mesh_delete_node(0x05);
                // wifi_info.state = INITIAL_STATE;
                // nvs_save_Info(my_handler, NVS_KEY_WIFI, &wifi_info, sizeof(wifi_info)); // luu lai
                // esp_restart();
            }
            else if (pinNumber == 39)
            {
                printf("Da an Button 39\n");

                // esp_ble_mesh_delete_node(0x05);
                // wifi_info.state = INITIAL_STATE;
                // nvs_save_Info(my_handler, NVS_KEY_WIFI, &wifi_info, sizeof(wifi_info)); // luu lai
                // esp_restart();
            }
            else if (pinNumber == 0)
            {
                printf("Da an Button 0\n");
                esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);
                // esp_ble_mesh_delete_node(0x05);
                // wifi_info.state = INITIAL_STATE;
                // nvs_save_Info(my_handler, NVS_KEY_WIFI, &wifi_info, sizeof(wifi_info)); // luu lai
                // esp_restart();
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}