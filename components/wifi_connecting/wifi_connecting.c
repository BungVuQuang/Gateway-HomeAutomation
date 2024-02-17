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
#include "wifi_connecting.h"

/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/
const char *TAG_WIFI = "wifi_connect";
const char *TAG = "wifi_connect";
const char *NVS_KEY_WIFI = "WIFI";
char *TARGET_HOST = "www.espressif.com";
/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/
extern nvs_handle_t my_handler;
extern struct wifi_info_t wifi_info;
TaskHandle_t loopHandle = NULL;
int s_retry_num = 0;
uint8_t internet_check = 0;
/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/
/**
 *  @brief Event Handler xử lý các sự kiện về kết nối đến Router
 *
 */
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    int s_retry_num = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {

        if (s_retry_num < 20)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_WIFI, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }

    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG_WIFI, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG_WIFI, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

/**
 *  @brief Event Handler xử lý các sự kiện về kết nối đến Router
 *
 *  @param[in] arg argument
 *  @param[in] event_base Tên Event
 *  @param[in] event_id Mã Event
 *  @param[in] event_data IP được trả về
 *  @return None
 */
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) // bắt đầu kết nối
    {
        printf("WIFI_EVENT EVENT WIFI_EVENT_STA_START : the event id is %d \n", event_id);
        esp_wifi_connect();
        // start task after the wifi is connected
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) // event báo mất kết nối đến AP
    {
        internet_status = LOST_INTERNET_STATE;
        internet_check = 0;
        connect_thingsboard = 0;
        gpio_set_level(LED_NETWORK_PIN, 1);
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        printf("WIFI_EVENT EVENT WIFI_EVENT_STA_DISCONNECTED : the event id is %d \n", event_id);
        if (loopHandle != NULL)
        {
            printf("WIFI_EVENT task !NULL *** %d \n", (int)loopHandle);
            vTaskDelete(loopHandle);
        }
        else
        {
            printf("WIFI_EVENT task  NULL *** %d \n", (int)loopHandle);
        }

        if (s_retry_num < 10) // cố gắng kết nối dưới 10 lần
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
        }
        else // quá 10 thì coi như mất mạng
        {
            esp_restart(); // reset esp32
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_WIFI, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) // Event đã được cấp IP
    {
        internet_status = CONNECTED_STATE;
        internet_check = 1;
        gpio_set_level(LED_NETWORK_PIN, 0);
        printf("IP EVENT IP_EVENT_STA_GOT_IP : the event id is %d \n", event_id);
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        // wifi connected ip assigned now start mqtt.
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT); // set bit WIFI_CONNECTED_BIT
        // GetMeshFireBase();
        GetTokenThingsBoard();
        xEventGroupWaitBits(s_thingsboard_event_group, // đợi đến khi có tin nhắn từ node gửi đến
                            GET_TOKEN_COMPLETE,
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);
        websocket_app_start();
        // mqtt_activeGW_connect();
        mqtt_Gateway_connect();
        printMeshList(&meshDataPrevious);
        // if (initialize_ping(10000, 7, TARGET_HOST) == ESP_OK)
        // {
        //     ESP_LOGI(TAG, "initialize_ping success");
        // }
        // else
        // {
        //     ESP_LOGE(TAG, "initialize_ping fail");
        // }

        // GetMeshThingsBoard();
        //  mqttApp_app_start();
        //  char role[10];
        //  char unicast[10];
        //  char mac[30];
        //  sprintf(role, "Node1");
        //  sprintf(unicast, "0x0005");
        //  sprintf(mac, "34:B4:A9:D3:A4:C5");
        //  insertMeshNode(&meshDataPrevious, role, unicast, "No", mac);
        //  firebase_mesh_info(role, unicast, "No", mac);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) // Event đã kết nối thành công
    {
        printf("WIFI_EVENT EVENT WIFI_EVENT_STA_CONNECTED : the event id is %d \n", event_id);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) // K được cấp phát IP
    {
        printf("IP_EVENT EVENT IP_EVENT_STA_LOST_IP : the event id is %d \n", event_id);
    }
}

/**
 *  @brief Kết nối đến router ở chế độ Station
 *
 *  @return None
 */
void wifi_init_sta(void) // khoi tao wifi o che do station
{
    printf("wifi_init_sta\n");
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL)); // đăng ký function call back được gọi bất cứ có sự kiện nào
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    // strcpy((char *)wifi_config.ap.ssid, wifi_info.SSID);
    // strcpy((char *)wifi_config.ap.password, wifi_info.PASSWORD);
    strcpy((char *)wifi_config.ap.ssid, "Vu Quang Thang");
    strcpy((char *)wifi_config.ap.password, "66668888");
    printf("%s\n", wifi_info.SSID);
    printf("%s\n", wifi_info.PASSWORD);
    esp_wifi_stop();
    // esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // set mode station
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start()); // bắt đầu kết nối đến router

    ESP_LOGI(TAG_WIFI, "wifi_init_sta finished.");

    while (1)
    {
        EventBits_t uxBits = xEventGroupWaitBits(s_wifi_event_group,
                                                 WIFI_CONNECTED_BIT,
                                                 pdTRUE,
                                                 pdFALSE,
                                                 portMAX_DELAY);
        if ((uxBits & WIFI_CONNECTED_BIT) == WIFI_CONNECTED_BIT)
        {
            wifi_info.state = NORMAL_STATE;
            nvs_save_Info(my_handler, NVS_KEY_WIFI, &wifi_info, sizeof(wifi_info)); // lưu lại trạng thái của wifi                              // bật led báo mạng
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);             // bật tín hiệu báo mạng đã  kết nối
            ESP_LOGI(TAG_WIFI, "WIFI_CONNECTED_BIT");
            break;
        }
    }
    ESP_LOGI(TAG_WIFI, "Got IP Address.");
}

/**
 *  @brief Khởi tạo AP để cấu hình wifi từ User
 *
 */
void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init()); // khởi tạo network interface
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "setup",
            .ssid_len = 5,
            .channel = 1,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen((char *)wifi_config.ap.password) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    start_webserver();
    printf("start_webserver\n");
    http_post_set_callback(wifi_data_callback);
    xEventGroupWaitBits(s_wifi_event_group, WIFI_RECV_INFO, true, false, portMAX_DELAY);
    stop_webserver();
    printf("stop_webserver\n");
    wifi_init_sta();
}

/**
 *  @brief lưu lại thông tin wifi từ nvs
 *
 */
esp_err_t nvs_save_Info(nvs_handle_t c_handle, const char *key, const void *value, size_t length)
{
    esp_err_t err;
    nvs_open("storage0", NVS_READWRITE, &c_handle);
    // strcpy(wifi_info.SSID, "anhbung");
    nvs_set_blob(c_handle, key, value, length);
    err = nvs_commit(c_handle);
    if (err != ESP_OK)
        return err;

    // Close
    nvs_close(c_handle);
    return err;
}
/**
 *  @brief Lấy lại trong tin wifi từ nvs
 *
 */
esp_err_t nvs_get_Info(nvs_handle_t c_handle, const char *key, void *out_value)
{
    esp_err_t err;
    err = nvs_open("storage0", NVS_READWRITE, &c_handle);
    if (err != ESP_OK)
        return err;
    size_t required_size = 0; // value will default to 0, if not set yet in NVS
    err = nvs_get_blob(c_handle, NVS_KEY_WIFI, NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        return err;
    if (required_size == 0)
    {
        printf("Nothing saved yet!\n");
    }
    else
    {
        nvs_get_blob(c_handle, NVS_KEY_WIFI, out_value, &required_size);

        err = nvs_commit(c_handle);
        if (err != ESP_OK)
            return err;

        // Close
        nvs_close(c_handle);
    }
    return err;
}
