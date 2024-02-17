/**
 ******************************************************************************
 * @file		app.c
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "app.h"
#include "ping/ping_sock.h"

/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/
static const char *TAG = "APP";
const char *NVS_KEY_DEVICE = "DEVICE";
/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/
#define TWDT_TIMEOUT_S 40
#define TASK_WS_STACK_SIZE 3796
#define TASK_NETWORK_STACK_SIZE 2300
#define TASK_INTERUPT_STACK_SIZE 1024
#define TASK_RTC_STACK_SIZE 1800
#define CHECK_ERROR_CODE(returned, expected) ({ \
    if (returned != expected)                   \
    {                                           \
        printf("TWDT ERROR\n");                 \
        abort();                                \
    }                                           \
})

/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/
int meshSizeInfo = 2;
typedef enum
{
    TIME_DATE_OLED_STATE = 0U,
    MESH_INFO_OLED_STATE,
    NODE1_CONTROL_OLED_STATE,
    NODE2_CONTROL_OLED_STATE,
    NODE3_CONTROL_OLED_STATE
} State_OLED_t;

typedef enum
{
    ROW_1 = 0U,
    ROW_2,
    ROW_3,
    ROW_4,
} Row_Oled_t;
Row_Oled_t Row_Oled = ROW_1;

typedef struct
{
    char macNode1[30];
    int macNode2[30];
    int macNode3[30];
    int sizeMesh;
} Mesh_Properties_t;

/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/
EventGroupHandle_t s_wifi_event_group;
EventGroupHandle_t s_thingsboard_event_group;
EventGroupHandle_t s_mesh_network_event_group;
extern nvs_handle_t NVS_HANDLE;
extern xQueueHandle interputQueue;
extern uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN];
extern struct wifi_info_t wifi_info;
static TaskHandle_t reset_wdt_task_handles;
int zlife_node[6] = {1, 1, 1, 1, 1, 1};
int connect_thingsboard = 0;
TaskHandle_t xTaskRtcHandle;
StaticTask_t xTaskWsHandle;
StaticTask_t xTaskNetworkHandle;
TaskHandle_t xTaskInterrupt;
// static StackType_t xStackInterrupt[TASK_INTERUPT_STACK_SIZE];
static StackType_t xStackWs[TASK_WS_STACK_SIZE];
// static StackType_t xStackRTC[TASK_RTC_STACK_SIZE];
//  StaticTask_t xTaskNetworkHandle;
static StackType_t xStackNetwork[TASK_NETWORK_STACK_SIZE];

DeviceStateNoInternet_t deviceStateNoInternet[6];
i2c_dev_t dev;
int internet_status = LOST_INTERNET_STATE;
/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/

// void esp_task_wdt_isr_user_handler()
// {
//     // Using this will cause reset when TWDT is triggered. Comment out this line to see the TWDT tigger message.
//     printf("******** user-defined  task_wdt_isr *********");
// }

//========================================IR ====================================
OLEDDisplay_t *oled = NULL;
void xTaskHandelInfrated(void *arg)
{
    char oled_data[50];
    uint8_t match[2] = {0x32, 0x10}; // 2 byte đầu của UUID
    while (1)
    {
        xEventGroupWaitBits(g_control_event_group, // đợi đến khi có tin nhắn từ node gửi đến
                            INFRATE_RECIVE_MESSAGE,
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);
        if (cmd == BUTTON_0)
        {
            ESP_LOGI(TAG, " Đã ấn button 0");
        }
        else if (cmd == BUTTON_1)
        {
            ESP_LOGI(TAG, " Đã ấn button 1");
        }
        else if (cmd == BUTTON_2)
        {
            ESP_LOGI(TAG, " Đã ấn button 2");
        }
        else if (cmd == BUTTON_3)
        {
            ESP_LOGI(TAG, " Đã ấn button 3");
        }
        else if (cmd == BUTTON_4)
        {
            ESP_LOGI(TAG, " Đã ấn button 4");
        }
        else if (cmd == BUTTON_5)
        {
            ESP_LOGI(TAG, " Đã ấn button 5");
        }
        else if (cmd == BUTTON_6)
        {
            ESP_LOGI(TAG, " Đã ấn button 6");
        }
        else if (cmd == BUTTON_7)
        {
            ESP_LOGI(TAG, " Đã ấn button 7");
        }
        else if (cmd == BUTTON_8)
        {
            ESP_LOGI(TAG, " Đã ấn button 8");
        }
        else if (cmd == BUTTON_9)
        {
            ESP_LOGI(TAG, " Đã ấn button 9");
            esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);
        }
        else if (cmd == BUTTON_STAR)
        {
            ESP_LOGI(TAG, " Đã ấn button STAR");
            xEventGroupClearBits(g_control_event_group, OLED_SWITCH_DISPLAY); // xoá Eventbit
            OLEDDisplay_clear(oled);
            sprintf(oled_data, "LIST NODE");
            OLEDDisplay_drawString(oled, 01, 0, oled_data);
            if (meshSizeInfo > 0)
            {
                MeshNodeInfo *currentNode = meshDataPrevious.head;
                int index = 1;
                int indexRow = 0;
                while (currentNode != NULL)
                {
                    if (indexRow == 0)
                    {
                        sprintf(oled_data, "=> %s:%s ", currentNode->role, currentNode->uuid);
                        OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                        currentNode = currentNode->next;
                    }
                    else
                    {
                        sprintf(oled_data, "%s:%s ", currentNode->role, currentNode->uuid);
                        OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                        currentNode = currentNode->next;
                    }
                    index++;
                    indexRow++;
                }
                OLEDDisplay_display(oled);
                Row_Oled = ROW_1;
            }
            else if (meshSizeInfo == 0)
            {
                sprintf(oled_data, "NetWork Empty!");
                OLEDDisplay_drawString(oled, 01, 25, oled_data);
                OLEDDisplay_display(oled);
            }
        }
        else if (cmd == BUTTON_THANG)
        {
            ESP_LOGI(TAG, " Đã ấn button THANG");
            xEventGroupSetBits(g_control_event_group, OLED_SWITCH_DISPLAY); // xoá Eventbit
        }
        else if (cmd == BUTTON_UP)
        {
            OLEDDisplay_clear(oled);
            sprintf(oled_data, "LIST NODE");
            OLEDDisplay_drawString(oled, 01, 0, oled_data);
            if (Row_Oled == ROW_2)
            {
                Row_Oled = ROW_1;
                if (meshSizeInfo > 0)
                {
                    MeshNodeInfo *currentNode = meshDataPrevious.head;
                    int index = 1;
                    int indexRow = 0;
                    while (currentNode != NULL)
                    {
                        if (indexRow == (int)ROW_1)
                        {
                            sprintf(oled_data, "=> %s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        else
                        {
                            sprintf(oled_data, "%s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        index++;
                        indexRow++;
                    }
                    OLEDDisplay_display(oled);
                }
                else if (meshSizeInfo == 0)
                {

                    sprintf(oled_data, "NetWork Empty!");
                    OLEDDisplay_drawString(oled, 01, 0, oled_data);
                    OLEDDisplay_display(oled);
                }
            }
            else if (Row_Oled == ROW_3)
            {
                Row_Oled = ROW_2;
                if (meshSizeInfo > 0)
                {
                    MeshNodeInfo *currentNode = meshDataPrevious.head;
                    int index = 1;
                    int indexRow = 0;
                    while (currentNode != NULL)
                    {
                        if (indexRow == (int)ROW_2)
                        {
                            sprintf(oled_data, "=> %s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        else
                        {
                            sprintf(oled_data, "%s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        index++;
                        indexRow++;
                    }
                    OLEDDisplay_display(oled);
                }
                else if (meshSizeInfo == 0)
                {
                    sprintf(oled_data, "NetWork Empty!");
                    OLEDDisplay_drawString(oled, 01, 0, oled_data);
                    OLEDDisplay_display(oled);
                }
            }
        }
        else if (cmd == BUTTON_DOWN)
        {
            ESP_LOGI(TAG, " Đã ấn button DOWN");
            OLEDDisplay_clear(oled);
            sprintf(oled_data, "LIST NODE");
            OLEDDisplay_drawString(oled, 01, 0, oled_data);
            if (Row_Oled == ROW_1)
            {
                Row_Oled = ROW_2;
                if (meshSizeInfo > 0)
                {
                    MeshNodeInfo *currentNode = meshDataPrevious.head;
                    int index = 1;
                    int indexRow = 0;
                    while (currentNode != NULL)
                    {
                        if (indexRow == (int)ROW_2)
                        {
                            sprintf(oled_data, "=> %s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        else
                        {
                            sprintf(oled_data, "%s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        index++;
                        indexRow++;
                    }
                    OLEDDisplay_display(oled);
                }
                else if (meshSizeInfo == 0)
                {
                    sprintf(oled_data, "NetWork Empty!");
                    OLEDDisplay_drawString(oled, 01, 0, oled_data);
                    OLEDDisplay_display(oled);
                }
            }
            else if (Row_Oled == ROW_2)
            {
                Row_Oled = ROW_3;
                if (meshSizeInfo > 0)
                {
                    MeshNodeInfo *currentNode = meshDataPrevious.head;
                    int index = 1;
                    int indexRow = 0;
                    while (indexRow != NULL)
                    {
                        if (index == (int)ROW_3)
                        {
                            sprintf(oled_data, "=> %s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        else
                        {
                            sprintf(oled_data, "%s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        index++;
                        indexRow++;
                    }
                    OLEDDisplay_display(oled);
                }
                else if (meshSizeInfo == 0)
                {

                    sprintf(oled_data, "NetWork Empty!");
                    OLEDDisplay_drawString(oled, 01, 0, oled_data);
                    OLEDDisplay_display(oled);
                }
            }
            else if (Row_Oled == ROW_3)
            {
                Row_Oled = ROW_4;
                if (meshSizeInfo > 0)
                {
                    MeshNodeInfo *currentNode = meshDataPrevious.head;
                    int index = 1;
                    int indexRow = 0;

                    while (currentNode != NULL)
                    {
                        if (indexRow == (int)ROW_4)
                        {
                            sprintf(oled_data, "=> %s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        else
                        {
                            sprintf(oled_data, "%s:%s ", currentNode->role, currentNode->uuid);
                            OLEDDisplay_drawString(oled, 01, 0 + index * 15, oled_data);
                            currentNode = currentNode->next;
                        }
                        index++;
                        indexRow++;
                    }
                    OLEDDisplay_display(oled);
                }
                else if (meshSizeInfo == 0)
                {
                    sprintf(oled_data, "NetWork Empty!");
                    OLEDDisplay_drawString(oled, 01, 0, oled_data);
                    OLEDDisplay_display(oled);
                }
            }
        }
        else if (cmd == BUTTON_OK)
        {
            ESP_LOGI(TAG, " Đã ấn button OK");
        }
        else if (cmd == BUTTON_LEFT)
        {
            ESP_LOGI(TAG, " Đã ấn button LEFT");
        }
        else if (cmd == BUTTON_RIGHT)
        {
            ESP_LOGI(TAG, " Đã ấn button RIGHT");
        }
        xEventGroupClearBits(g_control_event_group, INFRATE_RECIVE_MESSAGE);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

//========================================IR ====================================
void Get_Role(uint8_t rx_node, char *role)
{
    if (rx_node == 1)
    {
        sprintf(role, "Node1");
    }
    else if (rx_node == 2)
    {
        sprintf(role, "Node2");
    }
    else if (rx_node == 3)
    {
        sprintf(role, "Node3");
    }
    else if (rx_node == 4)
    {
        sprintf(role, "Node4");
    }
    else if (rx_node == 5)
    {
        sprintf(role, "Node5");
    }
    else if (rx_node == 6)
    {
        sprintf(role, "Node6");
    }
    else if (rx_node == 7)
    {
        sprintf(role, "Node7");
    }
    else if (rx_node == 8)
    {
        sprintf(role, "Node8");
    }
}

/**
 *  @brief Task này thực hiện Publish bản tin khi có tin nhắn gửi đến
 *
 */
void xTaskHandleMessageNetwork(void *pvParameters)
{
    char data[90] = {0};
    char value[20] = {0};
    char tem[10] = {0};
    char hum[10] = {0};
    char led1[5] = {0};
    char led2[5] = {0};
    char led3[5] = {0};
    char led4[5] = {0};
    char air[5] = {0};
    while (1)
    {
        xEventGroupWaitBits(s_mesh_network_event_group, // đợi đến khi có tin nhắn từ node gửi đến
                            MESH_MESSAGE_ARRIVE_BIT,
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);
        memset(value, 0, strlen(value));
        memset(data, 0, strlen(data));
        char *pElement = strtok((char *)(data_mesh_rx), " ");
        if (strstr(pElement, "tem") != NULL)
        {
            pElement = strtok(NULL, " ");
            sprintf(tem, "%s", pElement);
            pElement = strtok(NULL, " ");
            pElement = strtok(NULL, " ");
            sprintf(hum, "%s", pElement);
            if (strstr(tem, "-1.-1") == NULL)
            {
                sprintf(data, "{\"%s\":%s,\"%s\":%s,\"zlife\":1}", "temperature", tem, "humidity", hum);
                esp_mqtt_client_publish(clientMqtt[rx_node], "v1/devices/me/telemetry", data, 0, 1, 0);
            }
            else
            {
                printf("DHT11 loi roi1\n");
            }
        }
        if (strstr(pElement, "sync") != NULL)
        {
            pElement = strtok(NULL, " ");
            sprintf(led1, "%s", pElement);
            pElement = strtok(NULL, " ");
            sprintf(led2, "%s", pElement);
            pElement = strtok(NULL, " ");
            sprintf(led3, "%s", pElement);
            pElement = strtok(NULL, " ");
            sprintf(led4, "%s", pElement);
            pElement = strtok(NULL, " ");
            sprintf(air, "%s", pElement);
            sprintf(data, "{\"%s\":%s,\"%s\":%s,\"%s\":%s,\"%s\":%s,\"%s\":%s,\"zlife\":1}", "led1", led1, "led2", led2, "led3", led3, "led4", led4, "aOnOff", air);
            esp_mqtt_client_publish(clientMqtt[rx_node], "v1/devices/me/telemetry", data, 0, 1, 0);
        }
        else if (strstr(pElement, "led1") != NULL || strstr(pElement, "led2") != NULL || strstr(pElement, "led3") != NULL || strstr(pElement, "aOnOff") != NULL || strstr(pElement, "light") != NULL || strstr(pElement, "aChange") != NULL)
        {
            sprintf(value, "%s", data_mesh_rx + strlen(pElement) + 1);
            sprintf(data, "{\"%s\":%s,\"zlife\":1}", pElement, value);
            esp_mqtt_client_publish(clientMqtt[rx_node], "v1/devices/me/telemetry", data, 0, 1, 0);
        }
        isJustSendThingsBoard = 1;
        xEventGroupClearBits(s_mesh_network_event_group, MESH_MESSAGE_ARRIVE_BIT); // xoá Eventbit
        // vTaskDelay(100 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}
int isWsMess = 0;

void xTaskHandleWebsocket(void *pvParameters)
{
    int subscriptionIdValue = 0;
    int unicastAddr = 0;
    EventBits_t uxBits;
    char dataAlarm[100];
    char dataControlNode[30];
    while (1)
    {
        uxBits = xEventGroupWaitBits(s_thingsboard_event_group, // đợi đến khi có tin nhắn từ node gửi đến
                                     WEBSOCKET_RECIVE_MESSAGE | MQTT_MESSAGE_ALARM | MQTT_MESSAGE_MESH,
                                     pdFALSE,
                                     pdFALSE,
                                     portMAX_DELAY);
        if ((uxBits & WEBSOCKET_RECIVE_MESSAGE) != 0)
        {
            printf("Da vao xTaskHandleWebsocket\r\n");
            cJSON *root = cJSON_Parse(dataRxWs);
            if (root == NULL)
            {
                printf("Lỗi trong việc phân tích cú pháp chuỗi JSON.\n");
                goto errorWS;
            }
            cJSON *subscriptionId = cJSON_GetObjectItem(root, "subscriptionId");
            if (cJSON_IsNumber(subscriptionId))
            {
                subscriptionIdValue = subscriptionId->valueint;
                printf("subscriptionId: %d\n", subscriptionIdValue);
                if (subscriptionIdValue < 10 && subscriptionIdValue > 0)
                {
                    isWsMess = 1;
                    zlife_node[subscriptionIdValue] = 0;
                }
            }
            cJSON *dataJson = cJSON_GetObjectItem(root, "data");
            if (cJSON_IsObject(dataJson))
            {
                if (subscriptionIdValue < 10)
                {
                    cJSON *data_item;
                    cJSON_ArrayForEach(data_item, dataJson)
                    {
                        const char *key = data_item->string;
                        cJSON *value_array = cJSON_GetArrayItem(data_item, 0);
                        if (cJSON_IsArray(value_array) && cJSON_GetArraySize(value_array) >= 2)
                        {
                            cJSON *value_item = cJSON_GetArrayItem(value_array, 1);
                            const char *value = cJSON_GetStringValue(value_item);
                            cJSON *ts_item = cJSON_GetArrayItem(value_array, 0);
                            double ts = cJSON_GetNumberValue(ts_item);
                            if (ts != 0.00)
                            {
                                if (strstr(key, "tPeriod") != NULL)
                                {
                                    sprintf(dataControlNode, "Period temperature %s", value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else if (strstr(key, "hPeriod") != NULL)
                                {
                                    sprintf(dataControlNode, "Period humidity %s", value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else if (strstr(key, "lPeriod") != NULL)
                                {
                                    sprintf(dataControlNode, "Period light %s", value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else if (strstr(key, "aOnOff") != NULL)
                                {
                                    sprintf(dataControlNode, "Air onoff %s", value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else if (strstr(key, "aChange") != NULL)
                                {
                                    sprintf(dataControlNode, "Air change %s", value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else if (strstr(key, "aMode") != NULL)
                                {
                                    sprintf(dataControlNode, "Air mode %s", value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else if (strstr(key, "aFan") != NULL)
                                {
                                    sprintf(dataControlNode, "Air fan %s", value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else if (strstr(key, "aSwing") != NULL)
                                {
                                    sprintf(dataControlNode, "Air swing %s", value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else if (strstr(key, "sync") != NULL)
                                {
                                    sprintf(dataControlNode, "Sync 1");
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                                else
                                {
                                    sprintf(dataControlNode, "State %s %s", key, value);
                                    char role[9];
                                    Get_Role(subscriptionIdValue, &role);
                                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                                    example_ble_mesh_send_vendor_message(unicastAddr, dataControlNode);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                printf("Không tìm thấy data hoặc không phải kiểu đối tượng JSON.\n");
            }
        errorWS:
            // Giải phóng bộ nhớ
            cJSON_Delete(root);
            xEventGroupClearBits(s_thingsboard_event_group, WEBSOCKET_RECIVE_MESSAGE); // xoá Eventbit
        }
        else if ((uxBits & MQTT_MESSAGE_ALARM) != 0)
        {
            printf("Da vao xTaskTransmitDelete\r\n");
            if (strstr(dataMqttAlarm, "deleted") != NULL)
            {
                char *start = strstr(dataMqttAlarm, "[\"") + 2;
                char *endId = strstr(dataMqttAlarm, "\"]");
                char *endName = strstr(dataMqttAlarm, "_");
                if (start != NULL && endId != NULL && start < endId)
                {
                    size_t length = endId - start;
                    size_t lengthRole = endName - start;
                    char deleted_Id_value[length + 1];
                    char deleted_role_value[lengthRole + 1];
                    strncpy(deleted_Id_value, start, length);
                    strncpy(deleted_role_value, start, lengthRole);
                    deleted_Id_value[length] = '\0';
                    deleted_role_value[lengthRole] = '\0';
                    unicastAddr = findUnicastNodeByRole(&meshDataPrevious, deleted_role_value);
                    sprintf(dataAlarm, "Delete %s", deleted_Id_value);
                    example_ble_mesh_send_vendor_message(unicastAddr, dataAlarm);
                }
            }
            else
            {
                cJSON *root = cJSON_Parse(dataMqttAlarm);
                if (root == NULL)
                {
                    printf("Lỗi trong việc phân tích cú pháp chuỗi JSON.\n");
                }
                else
                {
                    cJSON *current_element = root->child;
                    char *id = current_element->string; // Lấy tên khóa
                    char role[strlen(id) + 1];
                    strcpy(role, id);
                    // Tìm vị trí của ký tự `_` trong chuỗi tên khóa
                    char *underscore_pos = strchr(role, '_');
                    if (underscore_pos != NULL)
                    {
                        *underscore_pos = '\0';
                    }
                    cJSON *node_obj = NULL;
                    cJSON_ArrayForEach(node_obj, root)
                    {
                        // cJSON *role = cJSON_GetObjectItem(node_obj, "role");
                        cJSON *date = cJSON_GetObjectItem(node_obj, "date");
                        cJSON *state = cJSON_GetObjectItem(node_obj, "state");
                        cJSON *device = cJSON_GetObjectItem(node_obj, "device");
                        // cJSON *id = cJSON_GetObjectItem(node_obj, "id");
                        cJSON *time = cJSON_GetObjectItem(node_obj, "time");

                        if (cJSON_IsString(date) && cJSON_IsString(device) && cJSON_IsString(time))
                        {
                            sprintf(dataAlarm, "Alarm %s %s %s %s %s", id, device->valuestring, state->valuestring, date->valuestring, time->valuestring);
                            unicastAddr = findUnicastNodeByRole(&meshDataPrevious, role);
                            example_ble_mesh_send_vendor_message(unicastAddr, dataAlarm);
                        }
                    }
                }
                cJSON_Delete(root);
            }
            xEventGroupClearBits(s_thingsboard_event_group, MQTT_MESSAGE_ALARM); // xoá Eventbit
        }
        else if ((uxBits & MQTT_MESSAGE_MESH) != 0)
        {
            printf("Da vao xTaskTransmitDelete\r\n");
            char *start = strstr(dataMqttMesh, "[\"") + 2;
            char *end = strstr(dataMqttMesh, "\"]");
            if (start != NULL && end != NULL && start < end)
            {
                size_t length = end - start;
                char deleted_value[length + 1];
                strncpy(deleted_value, start, length);
                deleted_value[length] = '\0';
                unicastAddr = findUnicastNodeByRole(&meshDataPrevious, deleted_value);
                esp_ble_mesh_delete_node(unicastAddr);
                deleteMeshNodeByName(&meshDataPrevious, deleted_value);
            }
            xEventGroupClearBits(s_thingsboard_event_group, MQTT_MESSAGE_MESH); // xoá Eventbit
        }
        memset(dataRxWs, 0, strlen(dataRxWs));
        // vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

/*=================================RTC============================*/

void xTaskRtcGetTime(void *pvParameter)
{
    // CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
    // CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);
    // char *TARGET_HOST = "www.espressif.com";
    char dataSync[60];
    while (1)
    {
        time_t now;
        time(&now);
        localtime_r(&now, &timeinfo);

        // In this example, we simply print the current time
        printf("Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
               timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
               timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        if (Hour_Incre == timeinfo.tm_hour && internet_status == CONNECTED_STATE)
        {
            xEventGroupClearBits(g_control_event_group, OLED_SWITCH_DISPLAY);
            vTaskDelay(pdMS_TO_TICKS(1000));
            Set_SystemTime_SNTP();
            Get_current_date_time();
            timeinfo.tm_year = timeinfo_ntp.tm_year;
            timeinfo.tm_mon = timeinfo_ntp.tm_mon;
            timeinfo.tm_mday = timeinfo_ntp.tm_mday;
            timeinfo.tm_hour = timeinfo_ntp.tm_hour;
            timeinfo.tm_min = timeinfo_ntp.tm_min;
            timeinfo.tm_sec = timeinfo_ntp.tm_sec;
            Hour_Incre = timeinfo.tm_hour + 1;
            time_t default_time_secs = mktime(&timeinfo);
            struct timeval tv = {
                .tv_sec = default_time_secs,
                .tv_usec = 0};

            ds1307_init_desc(&dev, I2C_NUM_0, 21, 22);
            struct tm time_set = {
                .tm_year = timeinfo_ntp.tm_year + 1900,
                .tm_mon = timeinfo_ntp.tm_mon, // 0-based
                .tm_mday = timeinfo_ntp.tm_mday,
                .tm_hour = timeinfo_ntp.tm_hour,
                .tm_min = timeinfo_ntp.tm_min,
                .tm_sec = timeinfo_ntp.tm_sec};
            ds1307_set_time(&dev, &time_set);
            sprintf(dataSync, "RTC %02d/%02d/%04d %02d:%02d:%02d", timeinfo_ntp.tm_mday, timeinfo_ntp.tm_mon, timeinfo_ntp.tm_year + 1900, timeinfo_ntp.tm_hour, timeinfo_ntp.tm_min, timeinfo_ntp.tm_sec);
            printf("%s\n", dataSync);
            // store.param_po.model->pub->publish_addr = 0xFFFF;
            // esp_ble_mesh_model_publish(store.param_po.model, ESP_BLE_MESH_MODEL_OP_SENSOR_STATUS, sizeof(dataSync), (uint8_t *)dataSync, ROLE_NODE);
            example_ble_mesh_send_vendor_message(0xFFFF, dataSync);
            xEventGroupSetBits(g_control_event_group, OLED_SWITCH_DISPLAY);
        }
        if (timeinfo.tm_hour % 4 == 0 && timeinfo.tm_min == 58)
        {
            esp_restart();
        }
        // CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
        vTaskDelay(pdMS_TO_TICKS(30000)); // Delay for 30 second
                                          // time_t now;
                                          // struct tm timeinfo;
                                          // time(&now);
                                          // localtime_r(&now, &timeinfo);

        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 *  @brief Hàm này được gọi lại mỗi khi nhận được dữ liệu wifi local
 *
 */
void wifi_data_callback(char *data, int len)
{
    char data_wifi[30];
    sprintf(data_wifi, "%.*s", len, data);
    printf("%.*s", len, data);
    char *pt = strtok(data_wifi, "/");
    strcpy(wifi_info.SSID, pt);
    pt = strtok(NULL, "/");
    strcpy(wifi_info.PASSWORD, pt);
    printf("\nssid: %s \n pass: %s\n", wifi_info.SSID, wifi_info.PASSWORD);
    nvs_save_Info(my_handler, NVS_KEY_WIFI, &wifi_info, sizeof(wifi_info)); // luu lai
    xEventGroupSetBits(s_wifi_event_group, WIFI_RECV_INFO);                 // set cờ
}

void RTC_Init(void)
{
    esp_err_t ret;
    Set_SystemTime_SNTP();
    Get_current_date_time();
    timeinfo.tm_year = timeinfo_ntp.tm_year;
    timeinfo.tm_mon = timeinfo_ntp.tm_mon;
    timeinfo.tm_mday = timeinfo_ntp.tm_mday;
    timeinfo.tm_hour = timeinfo_ntp.tm_hour;
    timeinfo.tm_min = timeinfo_ntp.tm_min;
    timeinfo.tm_sec = timeinfo_ntp.tm_sec;
    Hour_Incre = timeinfo.tm_hour + 1;
    if (Hour_Incre == 24)
    {
        Hour_Incre = 0;
    }
    time_t default_time_secs = mktime(&timeinfo);
    xTaskCreate(xTaskRtcGetTime, "xTaskRtcGetTime", 3000, NULL, 12, NULL);
    printf("Initializing RTC GPIO\n");
}

void getClock(void *pvParameters)
{
    // Initialize RTC
    i2c_dev_t dev;
    if (ds1307_init_desc(&dev, I2C_NUM_0, 21, 22) != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "Could not init device descriptor.");
        while (1)
        {
            vTaskDelay(1);
        }
    }
    xEventGroupSetBits(g_control_event_group, OLED_SWITCH_DISPLAY); // xoá Eventbit
    // Initialise the xLastWakeTime variable with the current time.
    TickType_t xLastWakeTime = xTaskGetTickCount();
    oled = OLEDDisplay_init(0, 0x78, 21, 22);
    OLEDDisplay_setFont(oled, ArialMT_Plain_10);
    char oled_data[50];
    int count = 0;
    char data[30] = {0};
    while (1)
    {
        xEventGroupWaitBits(g_control_event_group, // đợi đến khi có tin nhắn từ node gửi đến
                            OLED_SWITCH_DISPLAY,
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);
        struct tm time;
        if (count == 3 && connect_thingsboard == 1)
        {
            count = 0;
            for (int i = 0; i < 5; i++)
            {
                if (zlife_node[i] == 0)
                {
                    sprintf(data, "{\"zlife\":0}");
                    esp_mqtt_client_publish(clientMqtt[i], "v1/devices/me/telemetry", data, 0, 1, 0);
                    isJustSendThingsBoard = 1;
                }
            }
            isWsMess = 0;
        }
        if (ds1307_get_time(&dev, &time) != ESP_OK)
        {
            ESP_LOGE(pcTaskGetName(0), "Could not get time.");
            while (1)
            {
                vTaskDelay(1);
            }
        }
        OLEDDisplay_clear(oled);
        sprintf(oled_data, "CLOCK");
        OLEDDisplay_drawString(oled, 01, 00, oled_data);
        // OLEDDisplay_drawRect(oled, 01, 00, 120, 20);
        sprintf(oled_data, "Time:%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
        OLEDDisplay_drawString(oled, 01, 25, oled_data);
        sprintf(oled_data, "Date:%04d-%02d-%02d", time.tm_year, time.tm_mon + 1,
                time.tm_mday);
        OLEDDisplay_drawString(oled, 01, 45, oled_data);
        // OLEDDisplay_invertDisplay(oled);
        OLEDDisplay_display(oled);
        if (connect_thingsboard == 1 && isWsMess == 1)
        {
            count++;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Wifi_init(void)
{
    nvs_get_Info(my_handler, NVS_KEY_WIFI, &wifi_info); // lấy thông tin wifi lưu trữ từ nvs

    wifi_info.state = NORMAL_STATE;
    nvs_save_Info(my_handler, NVS_KEY_WIFI, &wifi_info, sizeof(wifi_info)); // luu lai
    if (wifi_info.state == INITIAL_STATE)                                   // Xem state có phải ban đầu hay k
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
        assert(sta_netif);
        wifi_init_softap(); // chạy AP để user config wifi
    }
    else if (wifi_info.state == NORMAL_STATE) // Trạng thái bình thường
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
        assert(sta_netif);

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        wifi_init_sta();
    }
    xTaskCreateStatic(xTaskHandleWebsocket, "xTaskHandleWebsocket", TASK_WS_STACK_SIZE, NULL, 2, xStackWs, &xTaskWsHandle);
    xTaskCreateStatic(xTaskHandleMessageNetwork, "xTaskHandleMessageNetwork", TASK_NETWORK_STACK_SIZE, NULL, 1, xStackNetwork, &xTaskNetworkHandle);
}

void System_Init(void)
{
    //================System Initalize===============================
    esp_err_t err;
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    s_thingsboard_event_group = xEventGroupCreate();
    s_wifi_event_group = xEventGroupCreate();
    s_mesh_network_event_group = xEventGroupCreate();
    g_control_event_group = xEventGroupCreate();
    bluetooth_init();
    ble_mesh_nvs_open(&NVS_HANDLE);
    ble_mesh_get_dev_uuid(dev_uuid);
    ble_mesh_init();
    //========================= GPIO INIT ===============================
    // gpio_set_direction(BUTTON_UP_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_OK_PIN, GPIO_MODE_INPUT);
    // gpio_set_direction(BUTTON_DOWN_PIN, GPIO_MODE_INPUT);

    // gpio_set_pull_mode(BUTTON_UP_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(BUTTON_OK_PIN, GPIO_PULLUP_ONLY);
    // gpio_set_pull_mode(BUTTON_DOWN_PIN, GPIO_PULLUP_ONLY);
    output_create(LED_NETWORK_PIN);
    gpio_set_level(LED_NETWORK_PIN, 1);
    // gpio_set_intr_type(BUTTON_UP_PIN, GPIO_INTR_POSEDGE);                                // Cấu hình ngắt ngoài cho nút IO0
    // gpio_set_intr_type(BUTTON_OK_PIN, GPIO_INTR_POSEDGE);                                // Cấu hình ngắt ngoài cho nút IO0
    // gpio_set_intr_type(BUTTON_DOWN_PIN, GPIO_INTR_POSEDGE);                              // Cấu hình ngắt ngoài cho nút IO0
    // interputQueue = xQueueCreate(1, sizeof(int));                                        // sử dụng queue cho ngắt
    // gpio_install_isr_service(0);                                                         // khởi tạo ISR Service cho ngắt
    //                                                                                      // gpio_isr_handler_add(BUTTON_UP_PIN, gpio_interrupt_handler, (void *)BUTTON_UP_PIN);  // đăng ký hàm handler cho ngắt ngoài
    // gpio_isr_handler_add(BUTTON_OK_PIN, gpio_interrupt_handler2, (void *)BUTTON_OK_PIN); // đăng ký hàm handler cho ngắt ngoài
    // gpio_isr_handler_add(BUTTON_DOWN_PIN, gpio_interrupt_handler3, (void *)BUTTON_DOWN_PIN);
    //================BLE Initalize===============================
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    //================RMT Initalize===============================
    rmt_rx_config();
    //================DS1307 Initalize===============================
    // ds1307_init_desc(&dev, I2C_NUM_0, 21, 22);
    // struct tm time_set = {
    //     .tm_year = 24 + 2000,
    //     .tm_mon = 0, // 0-based
    //     .tm_mday = 16,
    //     .tm_hour = 20,
    //     .tm_min = 40,
    //     .tm_sec = 10};
    // ds1307_set_time(&dev, &time_set);

    //================WIFI Initalize===============================
    Wifi_init();
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    //================TASK Create===============================
    xTaskCreate(getClock, "getClock", 2048, NULL, 10, NULL);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    xTaskCreate(xTaskHandelInfrated, "xTaskHandelInfrated", 2048 * 2, NULL, 5, NULL);
}
