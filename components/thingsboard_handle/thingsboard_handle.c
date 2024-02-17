/**
 ******************************************************************************
 * @file		thingsboard_handle.c
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "thingsboard_handle.h"
/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/
static const char *TAG = "THINGSBOARD";
char *CONFIG_WEBSOCKET_URI = "wss://thingsboard.cloud:443/api/ws/plugins/telemetry?token=";
/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/
#define MAX_DATA_SIZE 1350
int countReconnect = 0;
/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/
esp_websocket_client_handle_t Wsclient;
char jwtToken[900];
char dataRxWs[300];
int isJustSendThingsBoard = 0;
char deviceID[7][50] = {"1c2f2e10-bcc4-11ee-a441-01b9c6b9277c", "22781110-bcc4-11ee-862d-c1e0a53112a0", "a75fcf30-bcc4-11ee-a1be-053f95f9b401", "ab700040-bcc4-11ee-a124-0d00bef77fcc",
                        "b4776700-bcc4-11ee-a441-01b9c6b9277c", "870daf90-4a0f-11ee-ab62-bf6c2845e826", "8b01f430-4a0f-11ee-adc5-170e82239536"};
/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/
static void freeMeshList(MeshData *meshData);
static void parseMeshData(const char *jsonData, MeshData *meshData);
/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        countReconnect++;
        if (countReconnect == 2)
        {
            esp_restart();
        }
        break;
    case WEBSOCKET_EVENT_DATA:
        if (data->data_len != 0)
        {
            if (isJustSendThingsBoard == 1)
            {
                isJustSendThingsBoard = 0;
            }
            else
            {
                ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
                sprintf(dataRxWs, "%.*s", data->data_len, (char *)data->data_ptr);
                ESP_LOGI(TAG, "Data ThingsBoard: %s", dataRxWs);
                // removeBackslashes(dataRxWs);
                cJSON *root = cJSON_Parse(dataRxWs);
                if (root == NULL)
                {
                    printf("Lỗi trong việc phân tích cú pháp chuỗi JSON.\n");
                    return 1;
                }
                cJSON *subscriptionId = cJSON_GetObjectItem(root, "subscriptionId");
                if (cJSON_IsNumber(subscriptionId))
                {
                    int subscriptionIdValue = subscriptionId->valueint;
                    if (isFirstRxWs[subscriptionIdValue] == 0)
                    {
                        isFirstRxWs[subscriptionIdValue] = 1;
                    }
                    else
                    {
                        xEventGroupSetBits(s_thingsboard_event_group, WEBSOCKET_RECIVE_MESSAGE); // set bit GET_TOKEN_COMPLETE
                    }
                }
                connect_thingsboard = 1;
                cJSON_Delete(root);
            }
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}
static void register_listen(esp_websocket_client_handle_t client, const char *entityId, uint8_t u8cmdId)
{
    const char cmdId[5];
    const char cmdSubId[5];
    sprintf(cmdId, "%d", u8cmdId);
    sprintf(cmdSubId, "%d", u8cmdId + 10);

    cJSON *tsSubCmds = cJSON_CreateArray();
    cJSON *historyCmds = cJSON_CreateArray();
    cJSON *attrSubCmds = cJSON_CreateArray();
    cJSON *entity = cJSON_CreateObject();

    cJSON *tsSubCmd = cJSON_CreateObject();
    cJSON_AddStringToObject(tsSubCmd, "entityType", "DEVICE");
    cJSON_AddStringToObject(tsSubCmd, "entityId", entityId);
    cJSON_AddStringToObject(tsSubCmd, "scope", "LASTEST_TELEMETRY");
    cJSON_AddStringToObject(tsSubCmd, "cmdId", cmdId);
    cJSON_AddStringToObject(tsSubCmd, "keys", "");

    cJSON_AddItemToArray(tsSubCmds, tsSubCmd);

    cJSON_AddItemToObject(entity, "tsSubCmds", tsSubCmds);
    cJSON_AddItemToObject(entity, "historyCmds", historyCmds);
    cJSON_AddItemToObject(entity, "attrSubCmds", attrSubCmds);

    // Chuyển cấu trúc JSON sang chuỗi
    char *data = cJSON_Print(entity);

    esp_websocket_client_send_text(client, data, strlen(data), portMAX_DELAY);

    cJSON_free(data);
    cJSON_Delete(entity);
}

void websocket_lister_resigter(uint16_t index)
{
    int i = 0;
    while (i < 1)
    {
        if (esp_websocket_client_is_connected(Wsclient))
        {
            register_listen(Wsclient, deviceID[index], index);
            i++;
        }
    }
}

void websocket_app_start(void)
{
    esp_websocket_client_config_t websocket_cfg = {};
    int websocketUrl_len = strlen(CONFIG_WEBSOCKET_URI) + strlen(jwtToken);
    char *websocketUrl = (char *)malloc(websocketUrl_len + 1);
    if (websocketUrl != NULL)
    {
        strcpy(websocketUrl, CONFIG_WEBSOCKET_URI);
        strcat(websocketUrl, jwtToken);
    }

    memset(isFirstRxWs, 0, sizeof(isFirstRxWs));
    websocket_cfg.uri = websocketUrl;
    websocket_cfg.port = 443;
    Wsclient = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(Wsclient, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)Wsclient);
    esp_websocket_client_start(Wsclient);
    free(websocketUrl);
    vTaskDelay(2000 / portTICK_RATE_MS);
    // }
}

static esp_err_t _http_event_handler_token(esp_http_client_event_t *evt)
{
    static int data_offset = 0;
    static char data_buffer[MAX_DATA_SIZE];
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "Connected to server");
        break;
    case HTTP_EVENT_ON_DATA:
        // ESP_LOGI(TAG, "Received data:   %.*s", evt->data_len, (char *)evt->data);
        if (data_offset + evt->data_len < MAX_DATA_SIZE)
        {
            memcpy(data_buffer + data_offset, evt->data, evt->data_len);
            data_offset += evt->data_len;
        }
        else
        {
            ESP_LOGE(TAG, "Data buffer is full, cannot store more data.");
        }

        if (strstr(data_buffer, "}") != NULL)
        {
            ESP_LOGI(TAG, "Data Receive Complete !!!");
            cJSON *root = cJSON_Parse(data_buffer);
            cJSON *tokenItem = cJSON_GetObjectItem(root, "token");
            const char *token = tokenItem->valuestring;
            sprintf(jwtToken, "%s", token);
            data_offset = 0;
            memset(data_buffer, 0, sizeof(data_buffer));
            cJSON_Delete(root);
            xEventGroupSetBits(s_thingsboard_event_group, GET_TOKEN_COMPLETE); // set bit GET_TOKEN_COMPLETE
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

void GetTokenThingsBoard(void)
{
    char *post_data = "{\"username\":\"bungvu50@gmail.com\", \"password\":\"123456\"}";
    esp_http_client_config_t config = {
        .url = "https://thingsboard.cloud/api/auth/login",
        .event_handler = _http_event_handler_token,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP request  complete");
    }
    else
    {
        esp_restart();
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

/*=================================Mesh Info=======================*/
int findUnicastNodeByRole(MeshData *meshData, const char *role)
{
    MeshNodeInfo *currentNode = meshData->head;
    int currentCount = 0;
    int unicastAddr = 0;
    while (currentNode != NULL)
    {
        if (strcmp(currentNode->role, role) == 0)
        {
            sscanf(currentNode->unicast, "%x", &unicastAddr);
            return unicastAddr;
        }
        currentNode = currentNode->next;
    }
    return 0;
}

void insertMeshNode(MeshData *meshData, const char *role, const char *unicast, const char *parent, const char *uuid)
{
    MeshNodeInfo *newNode = (MeshNodeInfo *)malloc(sizeof(MeshNodeInfo));
    if (newNode == NULL)
    {
        printf("Failed to allocate memory for new alarm node\n");
        return;
    }

    strcpy(newNode->role, role);
    strcpy(newNode->unicast, unicast);
    strcpy(newNode->parent, parent);
    strcpy(newNode->uuid, uuid);
    newNode->next = NULL;

    if (meshData->head == NULL)
    {
        meshData->head = newNode;
        meshData->tail = newNode;
    }
    else
    {
        meshData->tail->next = newNode;
        meshData->tail = newNode;
    }
}

static void freeMeshList(MeshData *meshData)
{
    MeshNodeInfo *currentNode = meshData->head;
    while (currentNode != NULL)
    {
        MeshNodeInfo *nextNode = currentNode->next;
        free(currentNode);
        currentNode = nextNode;
    }
    meshData->head = NULL;
    meshData->tail = NULL;
}

void parseMeshDataThingsBoard(const char *jsonData, MeshData *meshData)
{
    // Xóa danh sách liên kết hiện tại (nếu có)
    freeMeshList(meshData);

    cJSON *root = cJSON_Parse(jsonData);
    if (root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
        cJSON_Delete(root);
        return;
    }

    // Duyệt qua tất cả các cặp key-value trong JSON
    cJSON *childroot = root->child;
    cJSON *child = childroot->child;
    while (child != NULL)
    {
        cJSON *roleItem = cJSON_GetObjectItem(child, "role");
        cJSON *unicastItem = cJSON_GetObjectItem(child, "unicast");
        cJSON *parentItem = cJSON_GetObjectItem(child, "parent");
        cJSON *uuidItem = cJSON_GetObjectItem(child, "uuid");
        if (roleItem != NULL && unicastItem != NULL && parentItem != NULL && uuidItem != NULL)
        {
            const char *role = roleItem->valuestring;
            const char *unicast = unicastItem->valuestring;
            const char *parent = parentItem->valuestring;
            const char *uuid = uuidItem->valuestring;
            insertMeshNode(meshData, role, unicast, parent, uuid);
        }

        child = child->next;
    }

    cJSON_Delete(root);
}

void deleteMeshNodeByName(MeshData *meshData, char *nameNode)
{
    MeshNodeInfo *currentNode = meshData->head;
    MeshNodeInfo *prevNode = NULL;
    while (currentNode != NULL)
    {
        if (strstr(currentNode->role, nameNode) != NULL)
        {
            // Nếu tìm thấy nút có số thứ tự trùng với count, thì xoá nút đó khỏi danh sách
            if (prevNode == NULL)
            {
                // Nếu nút cần xoá là nút đầu tiên, cập nhật head
                meshData->head = currentNode->next;
            }
            else
            {
                // Nếu nút cần xoá không phải là nút đầu tiên, cập nhật liên kết giữa các nút
                prevNode->next = currentNode->next;
            }

            // Giải phóng bộ nhớ của nút bị xoá
            free(currentNode);
            return;
        }

        prevNode = currentNode;
        currentNode = currentNode->next;
    }
}

// Hàm xoá nút trong danh sách liên kết dựa vào số thứ tự (count)
void deleteMeshNodeByUnicast(MeshData *meshData, char *unicast)
{
    MeshNodeInfo *currentNode = meshData->head;
    MeshNodeInfo *prevNode = NULL;
    while (currentNode != NULL)
    {
        if (strstr(currentNode->unicast, unicast) != NULL)
        {
            // Nếu tìm thấy nút có số thứ tự trùng với count, thì xoá nút đó khỏi danh sách
            if (prevNode == NULL)
            {
                // Nếu nút cần xoá là nút đầu tiên, cập nhật head
                meshData->head = currentNode->next;
            }
            else
            {
                // Nếu nút cần xoá không phải là nút đầu tiên, cập nhật liên kết giữa các nút
                prevNode->next = currentNode->next;
            }

            // Giải phóng bộ nhớ của nút bị xoá
            free(currentNode);
            return;
        }

        prevNode = currentNode;
        currentNode = currentNode->next;
    }
}

void printMeshList(const MeshData *meshData)
{
    MeshNodeInfo *currentNode = meshData->head;
    int index = 0;
    while (currentNode != NULL)
    {
        if (sscanf(currentNode->role, "Node%d", &index) == 1)
        {
            websocket_lister_resigter(index);
            mqtt_connect(index);
        }
        currentNode = currentNode->next;
    }
}

void syncDateTime(const MeshData *meshData, char *dataSync)
{
    MeshNodeInfo *currentNode = meshData->head;
    int unicastAddr;
    while (currentNode != NULL)
    {
        sscanf(currentNode->unicast, "%x", &unicastAddr);
        example_ble_mesh_send_vendor_message(unicastAddr, dataSync);
        vTaskDelay(1000 / portTICK_RATE_MS);
        currentNode = currentNode->next;
    }
}
