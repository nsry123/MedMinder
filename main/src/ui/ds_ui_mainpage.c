
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "ds_screen.h"
#include "ds_ui_mainpage.h"
#include "ds_system_data.h"
#include "ds_spi.h"

#include "ds_data_page.h"
#include "ds_wifi_ap_sta.h"
#include "ds_ui_medboxmainpage.h"


//暂时无用
void ds_ui_back_main_page(void){
    unsigned int i;
	for(i=0;i<5000;i++){
		spi_send_data(gImage_main_page[i]);  
	} 
}

//展示主界面
void ds_screen_display_main(){
	ds_screen_full_display_bydata(ds_screen_full_display_data,gImage_medbox_main_page);
}  

//展示设置页面
void ds_screen_setting(){
	ds_screen_full_display_bydata(ds_screen_full_display_data,gImage_setting_page);
}

//展示medbox主页
void ds_screen_display_medbox_mainpage(){
	// ds_screen_full_display_bydata(ds_screen_full_display_data,gImage_medbox_main_page);
	ds_ui_medbox_mainpage_show_time_init();
}  



