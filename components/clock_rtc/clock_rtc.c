/**
 ******************************************************************************
 * @file		clock_rtc.c
 * @author	Vu Quang Bung
 * @date		26 June 2023
 ******************************************************************************
 **/
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include "clock_rtc.h"

/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/
static const char *TAG = "RTC";
/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/
void write_rtc_time(const struct tm *timeinfo);
void Get_current_date_time(void);
/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/
void read_rtc_time(struct tm *timeinfo)
{
    esp_err_t ret = nvs_open("rtc_data", NVS_READONLY, &nvs_handle_1);
    if (ret == ESP_OK)
    {
        size_t required_size;
        ret = nvs_get_blob(nvs_handle_1, "timeinfo", NULL, &required_size);
        if (ret == ESP_OK && required_size == sizeof(struct tm))
        {
            ret = nvs_get_blob(nvs_handle_1, "timeinfo", timeinfo, &required_size);
        }
        nvs_close(nvs_handle_1);
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read RTC time: %s", esp_err_to_name(ret));
    }
}

void write_rtc_time(const struct tm *timeinfo)
{
    esp_err_t ret = nvs_open("rtc_data", NVS_READWRITE, &nvs_handle_1);
    if (ret == ESP_OK)
    {
        ret = nvs_set_blob(nvs_handle_1, "timeinfo", timeinfo, sizeof(struct tm));
        if (ret == ESP_OK)
        {
            ret = nvs_commit(nvs_handle_1);
        }
        nvs_close(nvs_handle_1);
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write RTC time: %s", esp_err_to_name(ret));
    }
}

/*=================================NTP============================*/

void Get_current_date_time(void)
{
    setenv("TZ", "GMT-7", 1);
    tzset();
    localtime_r(&now_ntp, &timeinfo_ntp);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo_ntp);
    // strcpy(date_time, strftime_buf);
    ESP_LOGI(TAG, "The current date/time in VietNam is: %s", strftime_buf);
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    // sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}

void sntp_sync_time(struct timeval *tv)
{
    settimeofday(tv, NULL);
    ESP_LOGI(TAG, "Time is synchronized from custom code");
    sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}

static void obtain_time(void)
{

    initialize_sntp();
    // wait for time to be set
    time_t now_ntp = 0;
    struct tm timeinfo_ntp = {0};
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if (retry == 9)
    {
        esp_restart();
    }
    time(&now_ntp);
    localtime_r(&now_ntp, &timeinfo_ntp);
}

void Set_SystemTime_SNTP(void)
{
    time(&now_ntp);
    localtime_r(&now_ntp, &timeinfo_ntp);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo_ntp.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now_ntp);
    }
}