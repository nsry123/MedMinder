
#include "ds_ui_page_manage.h"
#include "ds_ui_mainpage.h"
#include "ds_ui_timepage.h"
#include "ds_ui_wordpage.h"
#include "ds_screen.h"
#include "ds_ui_weatherpage.h"
#include "ds_ui_tomatopage.h"
#include "ds_http_request.h"
#include "ds_ui_fans.h"
#include "ds_ui_medboxqrpage.h"
#include "ds_ui_medboxlistpage.h"
#include "ds_pwm.h"

static const char *TAG = "DS_UI_PAGE_MANAGE";

PAGE_MANAGE_T g_page_manage;

typedef struct
{
	TP_ACTION_E key;
	uint8_t touch_x;
	uint8_t touch_y;
	PAGE_TYPE_E action;
}UI_EVENT_T;

QueueHandle_t ui_event_queue;

//TP点击事件
void ds_ui_page_manage_send_event(TP_ACTION_E key,uint8_t touch_x,uint8_t touch_y){
	UI_EVENT_T evt;
	evt.key = key;
	evt.touch_x = touch_x;
	evt.touch_y = touch_y;
	if(evt.key == TP_ACTION_SHORT){
		if(evt.touch_x < 75){
			if(evt.touch_y < 75){
				//时钟
				evt.action = PAGE_TYPE_TIME;
			}else{
				//二维码
				evt.action = PAGE_TYPE_MEDBOX_QRCODE;
			}
		}else{
			if(evt.touch_y < 75){
				//天气
				evt.action = PAGE_TYPE_WEATHER;
			}else{
				//番茄时钟
				evt.action = PAGE_TYPE_MEDBOX_LIST;
			}
		}
	}
	send_beep_event_from_isr(BEEP_SHORT_100MS);
	xQueueSendFromISR(ui_event_queue, &evt, NULL);
}

//页面切换事件
void ds_ui_page_manage_send_action(PAGE_TYPE_E action){
	UI_EVENT_T evt;
	evt.action = action;
	xQueueSendFromISR(ui_event_queue, &evt, NULL);
}


//获取点击事件类型，并执行对应操作——————————————————————————————————————————————————————————
static void ui_page_evt_task(void *arg)
{
    while (1) {
        UI_EVENT_T evt;
        xQueueReceive(ui_event_queue, &evt, portMAX_DELAY);
        ESP_LOGI(TAG, "now_show_page %d ui_page_evt_task %d evt.action %d",g_page_manage.now_show_page,evt.key,evt.action);
		//页面切换事件
		if(evt.action == PAGE_TYPE_MEDBOX_MAIN){
			g_page_manage.now_show_page = PAGE_TYPE_MEDBOX_MAIN;
			ds_screen_display_medbox_mainpage();
		}
		// }else if(evt.action == PAGE_TYPE_SETTING){
		// 	g_page_manage.now_show_page = PAGE_TYPE_SETTING;
		// 	ds_screen_setting();
		// }
		//TP点击事件
		if(g_page_manage.now_show_page == PAGE_TYPE_MEDBOX_MAIN){
			if(evt.key == TP_ACTION_SHORT){
				if(evt.action == PAGE_TYPE_TIME){
					// printf("-----------------jumping_to_time----------------");
					g_page_manage.now_show_page = PAGE_TYPE_TIME;
					ds_ui_timepage_show_time_init();
				}else if(evt.action == PAGE_TYPE_MEDBOX_QRCODE){
					printf("-----------------jumping_to_qrcode----------------");
					g_page_manage.now_show_page = PAGE_TYPE_MEDBOX_QRCODE;
					ds_ui_medbox_qrcodepage_display_init();
				}else if(evt.action == PAGE_TYPE_WEATHER){
					g_page_manage.now_show_page = PAGE_TYPE_WEATHER;
					ds_ui_weather_show(0);
				}else if(evt.action == PAGE_TYPE_MEDBOX_LIST){
					g_page_manage.now_show_page = PAGE_TYPE_MEDBOX_LIST;
					ds_ui_medbox_listpage_display_init();
				}
			}
			// else if(evt.key == TP_ACTION_LONG){
			// 	g_page_manage.now_show_page = PAGE_TYPE_SETTING;
			// 	ds_screen_setting();
			// }
			// else if(evt.key == TP_ACTION_MOVE_LEFT){
			// 	g_page_manage.now_show_page = PAGE_TYPE_FANS;
			// 	ds_ui_fans_show_init(0);
			// }
			else if(evt.key == TP_ACTION_MOVE_DOWN){
				set_update_httpdata_request(true);
			}
		}else if(g_page_manage.now_show_page != PAGE_TYPE_MEDBOX_MAIN){
			if(g_page_manage.now_show_page == PAGE_TYPE_FANS){
				if(evt.key == TP_ACTION_MOVE_LEFT){
					g_page_manage.now_show_page = PAGE_TYPE_FANS;
					ds_ui_fans_show_init(1);
				}
			}else if(g_page_manage.now_show_page == PAGE_TYPE_MEDBOX_QRCODE){
				if(evt.key == TP_ACTION_MOVE_LEFT){
					g_page_manage.now_show_page = PAGE_TYPE_MEDBOX_QRCODE;
					ds_ui_medbox_qrcodepage_display_init();
				}
			}else if(g_page_manage.now_show_page == PAGE_TYPE_MEDBOX_LIST){
				if(evt.key == TP_ACTION_MOVE_UP){
					listpage_move_up();
				}
				if(evt.key == TP_ACTION_MOVE_DOWN){
					listpage_move_down();
				}
				if(evt.key == TP_ACTION_SHORT){
					if(get_list_status()==VIEW_LIST){
						if(evt.touch_y<80){
							handle_click(1);
						}else{
							handle_click(2);
						}
					}
				}
				// if(evt.key == TP_ACTION_SHORT){
				// 	ds_ui_tomatopage_start_toggle();
				// }
			}
			if(evt.key == TP_ACTION_MOVE_RIGHT){
				
				if(g_page_manage.now_show_page == PAGE_TYPE_MEDBOX_LIST && get_list_status() == VIEW_DETAIL){
					ds_ui_medbox_listpage_display_init();
				}else{
					if(g_page_manage.now_show_page == PAGE_TYPE_MEDBOX_QRCODE){
						// ds_wifi_send_event(AP_STA_STOP);
					}
					g_page_manage.now_show_page = PAGE_TYPE_MEDBOX_MAIN;
					// ds_screen_display_main();
					// printf("------------------------------------------------returnning to home---------------------------------------");
					ds_screen_display_medbox_mainpage();
				}
			}
		} 
    }
}

PAGE_TYPE_E ds_ui_get_now_show_page(){
	return g_page_manage.now_show_page;
}

void ds_ui_page_manage_init(){
	g_page_manage.now_show_page = PAGE_TYPE_MEDBOX_MAIN;
	ui_event_queue = xQueueCreate(10, sizeof(UI_EVENT_T));
    xTaskCreate(ui_page_evt_task, "ui_page_evt_task", 4096, NULL, 5, NULL);
}


