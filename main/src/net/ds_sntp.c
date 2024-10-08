/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "ds_system_data.h"
#include "rom/gpio.h"

static const char *TAG = "ds_sntp";

bool sntp_1st_init = true;

void get_system_time()
{
    static struct tm timeinfo = {0}; // 时间寄存器
    time_t now = 0;

    time(&now);
    localtime_r(&now, &timeinfo);

    /* 打印获取到的时间 */
    char str[64];
    strftime(str, sizeof(str), "%c", &timeinfo);
    ESP_LOGI(TAG, "time updated: %s", str);

    ESP_LOGI(TAG, "%d%d:%d%d", timeinfo.tm_hour / 10, timeinfo.tm_hour % 10, timeinfo.tm_min / 10, timeinfo.tm_min % 10);
    ESP_LOGI(TAG, "%d-%d-%d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);

    update_system_time(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
    /* 设置时区 */
    setenv("TZ", "CST-8", 1);
    tzset();
    get_system_time();
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    if (sntp_1st_init) // doing this again?
    {
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_set_time_sync_notification_cb(time_sync_notification_cb);
        sntp_init();
        sntp_1st_init = false;
    }else{
        ESP_LOGI(TAG, "Syncing System Time");
        esp_sntp_stop();
        sntp_init();
    }
}