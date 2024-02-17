/**
 ******************************************************************************
 * @file		mqtt_handle.c
 * @author	Vu Quang Bung
 * @date		26 June 2023
 * Copyright (C) 2023 - Bung.VQ
 ******************************************************************************
 **/
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "mqtt_handle.h"
#include "wifi_connecting.h"
/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/
const char *TAG_MQTT = "MQTT:";
// const char *MESH_TEMPLATE = "{
// \"Node3\":
// {
//     \"role\": \"Node3\",
//   \"parent\": \"No\",
//   \"uuid\": \"34:B6:D9:E8:C3:69\",
//   \"unicast\": \"0x0007\"
// }
// }
// ";
/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/
#define PUBLISH_TELEMETRY "v1/devices/me/telemetry"
#define PUBLISH_ATTIBUTES "v1/devices/me/attributes"

/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/
char data_tx_ble[20];
// char topicName[40];
nvs_handle_t my_handler;
esp_mqtt_client_handle_t clientMqtt[4];
char dataMqttMesh[200];
char dataMqttAlarm[100];
char accessToken[7][30] = {"7ZvQBCNeQICxPQkSxlfh", "wRYkjd8aLCnBejYhxPcz", "DpX5Fc6ip4azNoyBhnnh", "l4odG8CgyhSLW0CzsmsS",
                           "eIpdTlv1I4ivvljJIrKd", "pvyVFSHWOn3pouqF8CIq", "4YF3LKxRoIKj8f3x9Lfs"};
// ];
// sprintf(data, "{\"%s\":{\"role\": \"%s\",\"parent\": \"No\",\"uuid\": \"34:B6:D9:E8:C3:69\",\"unicast\": \"%s\"}}", );
// esp_mqtt_client_publish(client, PUBLISH_ATTIBUTES, "{\"clientKeys\":\"\"", 0, 1, 0);
/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/
/**
 *  @brief Xử lý các sự kiện từ Mqtt
 *
 *  @param[in] event Data về event được gửi đến
 *  @return ESP_OK
 */
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    char *ptr;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:

        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "v1/devices/me/attributes", 1); // shared attribute

        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
        sprintf(dataMqttAlarm, "%.*s", event->data_len, event->data);
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        xEventGroupSetBits(s_thingsboard_event_group, MQTT_MESSAGE_ALARM);
        break;
    default:
        ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
        break;
    }

    return ESP_OK;
}

/**
 *  @brief Xử lý các sự kiện từ Mqtt
 *
 *  @param[in] event Data về event được gửi đến
 *  @return ESP_OK
 */
static esp_err_t mqtt_gateway_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    char *ptr;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:

        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
        // msg_id = esp_mqtt_client_subscribe(client, "v1/devices/me/attributes/response/+", 1); // sub topic
        msg_id = esp_mqtt_client_subscribe(client, "v1/devices/me/attributes", 1); // shared attribute
                                                                                   // esp_mqtt_client_publish(client, "v1/devices/me/attributes/request/1", "{\"clientKeys\":\"Node1\"}", 0, 1, 0);
        esp_mqtt_client_publish(client, "v1/devices/me/attributes/request/1", "{\"sharedKeys\":\"Node1,Node2,Node3,Node4,Node5,Node6\"}", 0, 1, 0);
        // sprintf(data, "{\"Node3\":{\"role\": \"Node3\",\"parent\": \"No\",\"uuid\": \"34:B6:D9:E8:C3:69\",\"unicast\": \"0x0007\"}}", );
        // esp_mqtt_client_publish(client, PUBLISH_ATTIBUTES, "{\"Node3\":{\"role\": \"Node3\",\"parent\": \"No\",\"uuid\": \"34:B6:D9:E8:C3:69\",\"unicast\": \"0x0007\"}}", 0, 1, 0); // shared attribute
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
        sprintf(dataMqttMesh, "%.*s", event->data_len, event->data);
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if (strstr(dataMqttMesh, "shared") != NULL)
        {
            parseMeshDataThingsBoard(dataMqttMesh, &meshDataPrevious);
            printMeshList(&meshDataPrevious);
        }
        else if (strstr(dataMqttMesh, "deleted") != NULL)
        {
            xEventGroupSetBits(s_thingsboard_event_group, MQTT_MESSAGE_MESH);
        }
        break;
    default:
        ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
        break;
    }

    return ESP_OK;
}

/**
 *  @brief Nhận các event được kich hoạt
 *  @param[in] handler_args argument
 *  @param[in] base Tên Event
 *  @param[in] event_id Mã Event
 *  @param[in] event_data dữ liệu từ event loop
 *  @return None
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

/**
 *  @brief Nhận các event được kich hoạt
 *  @param[in] handler_args argument
 *  @param[in] base Tên Event
 *  @param[in] event_id Mã Event
 *  @param[in] event_data dữ liệu từ event loop
 *  @return None
 */
static void mqtt_gateway_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_gateway_event_handler_cb(event_data);
}

// /**
//  *  @brief Nhận các event được kich hoạt
//  *  @param[in] handler_args argument
//  *  @param[in] base Tên Event
//  *  @param[in] event_id Mã Event
//  *  @param[in] event_data dữ liệu từ event loop
//  *  @return None
//  */
// static void mqtt_active_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
// {
//     ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
// }
/**
 *  @brief Kết nối đến Broker
 *  @return None
 */
void mqtt_connect(int index)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .host = "thingsboard.cloud",
        .username = accessToken[index],
        .disable_auto_reconnect = false,
        .task_stack = 4096,
        .buffer_size = 512,
        .reconnect_timeout_ms = 2000,
        //.network_timeout_ms = 2000,
        .port = 1883,
    }; // ESP_EVENT_ANY_ID
    clientMqtt[index] = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(clientMqtt[index], MQTT_EVENT_DATA, mqtt_event_handler, clientMqtt[index]);      // đăng kí hàm callback
    esp_mqtt_client_register_event(clientMqtt[index], MQTT_EVENT_CONNECTED, mqtt_event_handler, clientMqtt[index]); // đăng kí hàm callback
    esp_mqtt_client_start(clientMqtt[index]);                                                                       // bắt đầu kết nối
}

/**
 *  @brief Kết nối đến Broker
 *  @return None
 */
void mqtt_Gateway_connect(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .host = "thingsboard.cloud",
        .username = accessToken[0],
        .disable_auto_reconnect = false,
        .task_stack = 4096,
        .reconnect_timeout_ms = 2000,
        //.network_timeout_ms = 2000,
        .port = 1883,
    };

    clientMqtt[0] = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(clientMqtt[0], MQTT_EVENT_DATA, mqtt_gateway_event_handler, clientMqtt[0]);      // đăng kí hàm callback
    esp_mqtt_client_register_event(clientMqtt[0], MQTT_EVENT_CONNECTED, mqtt_gateway_event_handler, clientMqtt[0]); // đăng kí hàm callback
    esp_mqtt_client_start(clientMqtt[0]);                                                                           // bắt đầu kết nối
}

// /**
//  *  @brief Kết nối đến Broker
//  *  @return None
//  */
// void mqtt_activeGW_connect(void)
// {
//     esp_mqtt_client_config_t mqtt_cfg = {
//         .host = "thingsboard.cloud",
//         .username = "rbRz3Lqb0m1M0ZvyWHzn",
//         .disable_auto_reconnect = false,
//         .reconnect_timeout_ms = 2000,
//         .port = 1883,
//     };

//     clientActive = esp_mqtt_client_init(&mqtt_cfg);
//     esp_mqtt_client_register_event(clientActive, MQTT_EVENT_CONNECTED, mqtt_active_event_handler, clientActive); // đăng kí hàm callback
//     esp_mqtt_client_start(clientActive);                                                                         // bắt đầu kết nối
// }

// /**
//  *  @brief Nhận các event được kich hoạt
//  *  @param[in] handler_args argument
//  *  @param[in] base Tên Event
//  *  @param[in] event_id Mã Event
//  *  @param[in] event_data dữ liệu từ event loop
//  *  @return None
//  */
// static void mqtt_app_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
// {
//     ESP_LOGD(TAG_MQTT, "Event App dispatched from event loop base=%s, event_id=%d", base, event_id);
//     mqtt_App_event_handler_cb(event_data);
// }
// // /**
// //  *  @brief Kết nối đến Broker
// //  *  @return None
// //  */
// void mqttApp_app_start(void)
// {
//     esp_mqtt_client_config_t mqtt_cfg = {
//         .uri = "mqtt://broker.mqttdashboard.com:1883", // uri của broker hiveMQ
//         .disable_auto_reconnect = false,
//         .reconnect_timeout_ms = 2000,
//     };

//     clientApp = esp_mqtt_client_init(&mqtt_cfg);
//     esp_mqtt_client_register_event(clientApp, ESP_EVENT_ANY_ID, mqtt_app_event_handler, clientApp); // đăng kí hàm callback
//     esp_mqtt_client_start(clientApp);                                                               // bắt đầu kết nối
// }
