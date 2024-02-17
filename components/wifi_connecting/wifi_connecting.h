/**
 ******************************************************************************
 * @file		wifi_connecting.c
 * @author	Vu Quang Bung
 * @date		26 June 2023
 * Copyright (C) 2023 - Bung.VQ
 ******************************************************************************
 **/
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#ifndef _WIFI_CONNECTING_H_
#define _WIFI_CONNECTING_H_

#include "common.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "app_http_server.h"
#include "app.h"
#include "mqtt_handle.h"
#include "peripherals.h"
#include "ConfigType.h"
#include "thingsboard_handle.h"
#include "utilities.h"
/*-----------------------------------------------------------------------------*/
/* Constant definitions  */
/*-----------------------------------------------------------------------------*/
extern const char *NVS_KEY_WIFI;

/*-----------------------------------------------------------------------------*/
/* Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Macro definitions  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables  */
/*-----------------------------------------------------------------------------*/

extern uint8_t internet_check;
extern EventGroupHandle_t s_wifi_event_group;
extern EventGroupHandle_t s_thingsboard_event_group;
extern EventGroupHandle_t s_mesh_network_event_group;
extern char ssid_remote[15];
extern char password_remote[15];
/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/
void wifi_data_callback(char *data, int len);
/**
 *  @brief Kết nối đến router ở chế độ Station
 *
 *  @return None
 */
void wifi_init_sta(void);

/**
 *  @brief Khởi tạo AP để cấu hình wifi từ User
 *
 *  @return None
 */
void wifi_init_softap(void);

/**
 *  @brief Event Handler xử lý các sự kiện về kết nối đến Router
 *
 *  @param[in] arg argument
 *  @param[in] event_base Tên Event
 *  @param[in] event_id Mã Event
 *  @param[in] event_data IP được trả về
 *  @return None
 */
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);

/**
 *  @brief lưu lại thông tin wifi từ nvs
 *
 *  @param[in] c_handle nvs_handle_t
 *  @param[in] key Key để lấy dữ liệu
 *  @param[in] value Data output
 *  @param[in] length chiều dài dữ liệu
 *  @return None
 */
esp_err_t nvs_save_Info(nvs_handle_t c_handle, const char *key, const void *value, size_t length);

/**
 *  @brief Lấy lại trong tin wifi từ nvs
 *
 *  @param[in] c_handle nvs_handle_t
 *  @param[in] key Key để lấy dữ liệu
 *  @param[in] out_value Data output
 *  @return None
 */
esp_err_t nvs_get_Info(nvs_handle_t c_handle, const char *key, void *out_value);

#endif /* _WIFI_CONNECTING_H_ */
//********************************* END OF FILE ********************************/
//******************************************************************************/
