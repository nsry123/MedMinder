#include <string.h>
#include <stdio.h>
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "ds_screen.h"
#include "ds_ui_mainpage.h"
#include "ds_system_data.h"
#include "ds_spi.h"

#include "ds_data_page.h"
#include "ds_wifi_ap_sta.h"

#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "ds_screen.h"
#include "ds_spi.h"
#include "ds_ui_page_manage.h"
#include "ds_ui_timepage.h"
#include "ds_system_data.h"
#include "ds_data_num.h"
#include "ds_data_icon.h"

typedef struct 
{
    uint8_t init_status;
    uint8_t hour;
    uint8_t minute;
    uint8_t hour_last;
    uint8_t minute_last;
    uint8_t updateing;

    char* medicine_name;
    //局部刷新次数
    uint8_t partial_update_time;
}MAIN_PAGE_T;

//200*200 像素屏幕适配，偏移值
static int offset_v = 29; 
static int offset_h = 24; 

MAIN_PAGE_T g_main_page;

void ds_ui_medbox_mainpage_init(){
    memset(&g_main_page,0,sizeof(MAIN_PAGE_T));
    g_main_page.updateing = 0;
    g_main_page.hour = 1;
    g_main_page.minute = 37;
}

static void ds_ui_medbox_mainpage_num_set(const uint8_t *data){
	unsigned int i;
	for(i=0;i<192;i++){	
		spi_send_data(~data[i]);
	}
}

static void ds_ui_medbox_mainpage_num_clear(const uint8_t *data)
{
	unsigned int i;
    if(g_main_page.init_status == 0){
        for(i=0;i<192;i++){
            spi_send_data(0xff);  
        }  
    }else{
        for(i=0;i<192;i++)	     {
		    spi_send_data(~data[i]);  
	    }  
    }
}


void ds_ui_medbox_mainpage_show_time_init(){
    g_main_page.hour = get_system_data().hour;
    g_main_page.minute = get_system_data().minute;
    g_main_page.updateing = 1;
    int num_size = 32;
    int num_size_y = 48;
	int vertical = 7 + offset_v; //垂直位置
    int horizontal = 8 + offset_h;  //水平位置
    int now_index;

    ds_screen_partial_data_init();

    ds_screen_partial_data_add(0,200,0,200,gImage_medbox_main_page_rev);
    // ds_screen_partial_data_add(0,200,0,200,gImage_qrcode_test);
    

    //小时 十位
    now_index = g_main_page.hour / 10;
    ds_screen_partial_data_add(horizontal+0,horizontal+num_size,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
    //小时 个位
    now_index = g_main_page.hour % 10;
    ds_screen_partial_data_add(horizontal+num_size,horizontal+num_size*2,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
    //首次刷新才显示 :号
    ds_screen_partial_data_add(horizontal+num_size*2,horizontal+num_size*2+8,vertical,vertical+num_size_y,gImage_time_symbol);

    //分钟 十位
    now_index = g_main_page.minute / 10;
    ds_screen_partial_data_add(horizontal+num_size*2+8,horizontal+num_size*3+8,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
    // //分钟 个位
    now_index = g_main_page.minute % 10;
    ds_screen_partial_data_add(horizontal+num_size*3+8,horizontal+num_size*4+8,vertical,vertical+num_size_y,gImage_num_size48[now_index]);

    ds_screen_partial_data_copy();

    g_main_page.hour_last = g_main_page.hour;
    g_main_page.minute_last = g_main_page.minute;
    g_main_page.init_status = 1;
    g_main_page.updateing = 0;
}

//type = 0-全部刷新 1-部分刷新
void ds_ui_medbox_mainpage_show_time(uint8_t type){
    g_main_page.updateing = 1;
    g_main_page.init_status = type;
    int num_size = 32;
    int num_size_y = 48;
	int vertical = 7 + offset_v; //垂直位置
    int horizontal = 8 + offset_h;  //水平位置
    int last_index,now_index;

    ds_screen_partial_data_add(0,200,0,200,gImage_medbox_main_page_rev);
    // ds_screen_partial_data_add(0,200,0,200,gImage_qrcode_test);

    //小时 十位
    now_index = g_main_page.hour / 10;
    ds_screen_partial_display_bydata(horizontal+0,vertical,ds_ui_medbox_mainpage_num_set,gImage_num_size48[now_index],num_size_y,num_size);
    // //小时 个位
    now_index = g_main_page.hour % 10;
    ds_screen_partial_display_bydata(horizontal+num_size,vertical,ds_ui_medbox_mainpage_num_set,gImage_num_size48[now_index],num_size_y,num_size);
    //分钟 十位 10 *8 = 80  14*8 112
    now_index = g_main_page.minute / 10;
    ds_screen_partial_display_bydata(horizontal+num_size*2+8,vertical,ds_ui_medbox_mainpage_num_set,gImage_num_size48[now_index],num_size_y,num_size);
    // //分钟 个位
    now_index = g_main_page.minute % 10;
    ds_screen_partial_display_bydata(horizontal+num_size*3+8,vertical,ds_ui_medbox_mainpage_num_set,gImage_num_size48[now_index],num_size_y,num_size);
    refresh_part();
	deep_sleep();
    g_main_page.hour_last = g_main_page.hour;
    g_main_page.minute_last = g_main_page.minute;
    g_main_page.init_status = 1;
    g_main_page.updateing = 0;
}

void ds_ui_medbox_mainpage_updatetime(){
    g_main_page.hour = get_system_data().hour;
    g_main_page.minute = get_system_data().minute;
    if(g_main_page.updateing == 0){
        if(ds_ui_get_now_show_page() == PAGE_TYPE_MEDBOX_MAIN){
            if(g_main_page.partial_update_time > 5){
                g_main_page.partial_update_time = 0;
                ds_ui_medbox_mainpage_show_time_init();
            }else{
                g_main_page.partial_update_time ++;
                ds_ui_medbox_mainpage_show_time(1);
            }
        }
    }
}