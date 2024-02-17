/**
 ******************************************************************************
 * @file		ble_mesh.h
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
#ifndef _BLE_MESH_HANDLE_H_
#define _BLE_MESH_HANDLE_H_
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_sensor_model_api.h"
#include "esp_bt.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "common.h"
#include "app.h"
#include "utilities.h"
#include "wifi_connecting.h"
#include "thingsboard_handle.h"
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
#define CID_ESP 0x02E5
#define ESP_BLE_MESH_VND_MODEL_OP_SEND ESP_BLE_MESH_MODEL_OP_3(0x00, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_STATUS ESP_BLE_MESH_MODEL_OP_3(0x01, CID_ESP)
/*-----------------------------------------------------------------------------*/
/* Global variables  */
/*-----------------------------------------------------------------------------*/
extern uint8_t rx_mess_check;
extern uint8_t rx_node;
extern int unicast_delete;
extern char num_delete[5];
extern const char *NVS_KEY;
extern char data_mesh_rx[80];
int meshSize;
/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/

/**
 *  @brief Hàm này thực hiện xoá 1 node ra khỏi mạng
 *
 *  @param[in] addr Unicast address
 *  @return none
 */
uint8_t esp_ble_mesh_delete_node(uint16_t unicast);

/**
 *  @brief Khởi tạo BLE Mesh module và đăng ký các hàm callback xử lý event
 *
 *  @return None
 */
esp_err_t ble_mesh_init(void);

/**
 *  @brief Gửi thông tin gateway về sensor model lần đầu tiên đến các node
 *
 *  @param[in] opcode opcode message muốn nhận
 *  @return none
 */
void example_ble_mesh_send_sensor_message(uint32_t opcode);
void ble_mesh_send_vendor_publish_message(void);
/**
 *  @brief Hàm gửi tin nhắn đến node cụ thể theo unicast address
 *
 *  @param[in] addr địa chỉ unicast address
 *  @param[in] data data cần gửi
 *  @return none
 */
void example_ble_mesh_send_vendor_message(uint16_t addr, void *data);
uint8_t esp_ble_mesh_delete_node(uint16_t unicast);
#endif /* _BLE_MESH_HANDLE_H_ */
       /********************************* END OF FILE ********************************/
       /******************************************************************************/