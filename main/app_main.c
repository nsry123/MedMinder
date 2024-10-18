/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_flash.h"
#include "nvs_flash.h"
#include "esp_event.h"


#include "driver/gpio.h"
#include "esp_sleep.h"
#include <time.h>
#include <sys/time.h>
#include "esp_timer.h"

#include "esp_task_wdt.h"
#include "esp_chip_info.h"
#include "soc/rtc.h"
#include "driver/rmt.h"

#include "ds_test.h"
#include "ds_wifi_scan.h"
#include "ds_ft6336.h"
#include "ds_screen.h"
#include "ds_spi.h"
#include "ds_wifi_ap.h"
#include "ds_wifi_sta.h"
#include "ds_system_data.h"
#include "ds_nvs.h"
#include "ds_https_request.h"
#include "ds_http_request.h"
#include "ds_timer.h"
#include "ds_spiffs.h"
#include "ds_ui_timepage.h"
#include "ds_ui_page_manage.h"
#include "ds_ui_weatherpage.h"
#include "ds_ui_wordpage.h"
#include "ds_ui_tomatopage.h"
#include "ds_ui_medboxqrpage.h"
#include "ds_ui_medboxlistpage.h"
#include "ds_ui_medboxmainpage.h"
#include "ds_ui_fans.h"
#include "ds_font.h"
#include "ds_paint.h"
#include "ds_wifi_ap_sta.h"
#include "ds_pwm.h"
#include "ds_gpio.h"
#include "ds_http_server.h"
#include "ds_sntp.h"
#define TAG  "app_main_debug"

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

#ifdef CONFIG_IDF_TARGET_ESP32S2BETA
#define CHIP_NAME "ESP32-S2 Beta"
#endif

#define BUTTON_GPIO_NUM_DEFAULT  4
#define TWDT_TIMEOUT_S          3
#define SLEEP_TIME_RESET 600

static void sleep_mode_init(){
    set_tp_wackup_timeleft(SLEEP_TIME_RESET);
    while (true) {
        
        //ap&sta关闭、且当前在主页、且超过5min未触摸时才进入低功耗
        do {
            esp_task_wdt_reset(); 
            vTaskDelay(pdMS_TO_TICKS(1000));
            count_tp_wackup_timeleft();
            printf("wait enter sleep mode run... %ld\n",get_tp_wackup_timeleft());
            if(get_is_ap_sta_open() == false && ds_ui_get_now_show_page() == PAGE_TYPE_MEMU && get_tp_wackup_timeleft() == 0){
                break;
            }
        } while (1);

        ds_touch_gpio_isr_remove();
        gpio_wakeup_enable(BUTTON_GPIO_NUM_DEFAULT,GPIO_INTR_LOW_LEVEL);
        /* Wake up in 60 seconds, or when button is pressed */
        esp_sleep_enable_timer_wakeup(60000000);
        esp_sleep_enable_gpio_wakeup();

        printf("Entering light sleep\n");

        /* Get timestamp before entering sleep */
        int64_t t_before_us = esp_timer_get_time();

        /* Enter sleep mode */
        esp_light_sleep_start();
        /* Execution continues here after wakeup */

        /* Get timestamp after waking up from sleep */
        int64_t t_after_us = esp_timer_get_time();
        /* Determine wake up reason */
        const char* wakeup_reason;
        uint32_t wackup_timeleft = 60;
        switch (esp_sleep_get_wakeup_cause()) {
            case ESP_SLEEP_WAKEUP_TIMER:
                wakeup_reason = "timer";
                wackup_timeleft = 1;
                update_system_time_minute();
                break;
            case ESP_SLEEP_WAKEUP_GPIO:
                wakeup_reason = "pin";
                gpio_wakeup_disable(BUTTON_GPIO_NUM_DEFAULT);
                esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
                ds_touch_gpio_init();
                ds_touch_gpio_isr_add();
                reset_tp_action_manage();
                break;
            default:
                wakeup_reason = "other";
                gpio_wakeup_disable(BUTTON_GPIO_NUM_DEFAULT);
                esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
                ds_touch_gpio_init();
                ds_touch_gpio_isr_add();
                reset_tp_action_manage();
                break;
        }
        printf("Returned from light sleep, reason: %s, t=%lld ms, slept for %lld ms  wackup_timeleft=%ld\n",
                wakeup_reason, t_after_us / 1000, (t_after_us - t_before_us) / 1000,wackup_timeleft);

        set_tp_wackup_timeleft(wackup_timeleft);
    }
}

static void background_task(void* arg)
{
    int apsta_close_count = 60;
    for(;;){
        printf("background_task run... \n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        //如果有首次更新的请求，且STA模式已经建立连接
        // if(has_first_time_httpdata_request() == true && get_wifi_sta_status() == WIFI_STA_MODE_CONNECT_SUCCESS){
        //     // printf("------------------condition1");
        //     ds_http_request_all();
        //     initialize_sntp();
        //     set_has_first_time_httpdata_request();//将首次请求标志位置为0
        // }

        //如果有下拉更新的请求
        // if(has_update_httpdata_request() == true){
        //     // printf("------------------condition2");
        //     //如果AP和STA开启
        //     if(get_is_ap_sta_open()){
        //         if(get_wifi_sta_status() == WIFI_STA_MODE_CONNECT_SUCCESS){
        //             ds_http_request_all();//发送所有http请求
        //             initialize_sntp();//从服务器同步时间
        //             set_update_httpdata_request(false);//设置请求更新标识位为否
        //         }
        //     }else{
        //         ds_wifi_send_event(AP_STA_START);//发送开启AP和STA的事件
        //         vTaskDelay(1000 / portTICK_PERIOD_MS);
        //     }
        // }

        //如果ap和sta开启，且倒计时为0
        // if(get_is_ap_sta_open() == true && apsta_close_count == 0){
        //     // printf("------------------condition3");
        //     //重置10min计时
        //     apsta_close_count = (SLEEP_TIME_RESET - 10);
        // }
        if(apsta_close_count > 0 && ds_ui_get_now_show_page()!=PAGE_TYPE_ALARM){
            // printf("------------------condition4");
            apsta_close_count --;
            if(apsta_close_count == 0){//如果倒计时为0
                // ds_wifi_send_event(AP_STA_STOP);//停止ap和sta
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                // set_update_httpdata_request(false);//设置请求标识位为false
                if(ds_ui_get_now_show_page() == PAGE_TYPE_MEDBOX_MAIN){
                    printf("----------------Refreshing main page-----------\n");
                    ds_ui_medbox_mainpage_show_time_init();
                }
                apsta_close_count = 60;
            }
        }
        //如果页面为设置页
        // if(ds_ui_get_now_show_page() == PAGE_TYPE_SETTING){
        //     // printf("------------------condition5");
        //     if(get_is_ap_sta_open() == false){//如果ap和sta未打开
        //         ds_wifi_send_event(AP_STA_START);
        //         vTaskDelay(1000 / portTICK_PERIOD_MS);
        //     }
        // }
    }
}

void app_main(void)
{
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    printf("----- app start! -----\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
            CHIP_NAME,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);
    uint32_t size_flash_chip;
+   esp_flash_get_size(NULL, &size_flash_chip);

    printf("%dMB %s flash\n", size_flash_chip / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("sw revision V4.0.3 \n");

    //看门狗初始化
    // esp_task_wdt_init(TWDT_TIMEOUT_S, false);
    // esp_task_wdt_add(NULL);

    //系统所有相关数据初始化
    ds_system_data_init();
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //http请求初始化
    ds_http_request_init();
    ds_http_server_init();
    //ap&sta模式初始化
    ds_wifi_ap_sta_init();  
    ds_wifi_send_event(AP_STA_START);


    //TP模块初始化
    init_ft6336();
    //屏幕接口初始化
    init_screen_interface();


    //屏幕显示数据初始化
    ds_ui_timepage_init();
    ds_ui_page_manage_init();


    ds_ui_weather_init();
    
    ds_ui_wordpage_init();
    ds_ui_wordpage_init();
    ds_ui_tomatopage_init();
    ds_ui_fans_init();
    ds_ui_medbox_qrcodepage_init();
    

    //定时器初始化
    //设定刷新任务 ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    ds_timer_init();
    //PWM蜂鸣器初始化
    ds_pwm_init();
    ds_ui_page_manage_send_action(PAGE_TYPE_MEDBOX_MAIN);
    
    ds_ui_medbox_listpage_init();

    //默认Wifi账号密码设置
    // char *ssid="notifier";
    // char *psw="notifier_123";
    // ds_nvs_save_wifi_info(ssid,psw);
    
    //启动AP&STA模式
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    // ds_wifi_send_event(AP_STA_START);
    //开启background_task
    xTaskCreate(background_task, "background_task", 4096, NULL, 10, NULL); 
    
    //睡眠模式判定初始化
    // sleep_mode_init();


    // printf("--------------------------------line after sleep mode------------------------------------");

    for(;;){
        printf("app run \n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //喂狗
        esp_task_wdt_reset();  //Comment this line to trigger a TWDT timeout

        //堆栈信息打印，需打开USE_TRACE_FACILITY、USE_STATS_FORMATTING_FUNCTIONS
        // vTaskList((char *)&pWriteBuffer);
        // printf("task_name   task_state  priority   stack  tasK_num\n");
        // printf("%s\n", pWriteBuffer);  
    }
    esp_task_wdt_delete(NULL);
}
