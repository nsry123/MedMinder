#include <string.h>
#include <stdio.h>
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "ds_screen.h"
#include "ds_paint.h"
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

typedef enum{
    VIEW_LIST,
    VIEW_DETAIL
}LIST_STATUS_T;

typedef struct 
{
    uint8_t init_status;
    uint8_t hour;
    uint8_t minute;
    uint8_t hour_last;
    uint8_t minute_last;
    uint8_t page;
    uint8_t updateing;
    uint8_t intake_num;
    LIST_STATUS_T status;
    char* medicine_name;
    //局部刷新次数
    uint8_t partial_update_time;
}LIST_PAGE_T;

typedef struct{
    char* name;
    char* time;
    char* unit;
    int dose_per_time;
}MEDICINE_SORT_T;

//200*200 像素屏幕适配，偏移值
static int offset_v = 29; 
static int offset_h = 24; 

LIST_PAGE_T g_list_page;
MEDICINE_SORT_T *medicine_sort_list;// = malloc(50 * sizeof(MEDICINE_SORT_T));

void ds_ui_medbox_listpage_init(){
    memset(&g_list_page,0,sizeof(MEDICINE_SORT_T));
    g_list_page.updateing = 0;
    g_list_page.hour = 1;
    g_list_page.minute = 37;
    g_list_page.page = 0;
    g_list_page.intake_num = 0;
}

void ds_ui_medbox_listpage_display_update(){
    g_list_page.updateing = 1;

    int max_pages = (g_list_page.intake_num + 1) / 2;
    int progress_line_length = (int) (200.0/max_pages);

    int pagenum = (int) g_list_page.page;
    printf("------------------Page Number: %d----------------\n",pagenum);
    char *med1_name = medicine_sort_list[pagenum*2].name;
    char *med1_time = medicine_sort_list[pagenum*2].time;
    char *med2_name = NULL;
    char *med2_time = NULL;

    if (g_list_page.intake_num == (pagenum*2 + 1)) {
        med2_name = NULL;
        med2_time = NULL;
    } else {
        med2_name = medicine_sort_list[pagenum*2+1].name;
        med2_time = medicine_sort_list[pagenum*2+1].time;
    }
    printf("------------------Name1: %s----------------\n",med1_name);
    printf("------------------Name2: %s----------------\n",med1_time);
    printf("------------------Time1: %s----------------\n",med2_name);
    printf("------------------Time2: %s----------------\n",med2_time);

    g_list_page.hour = get_system_data().hour;
    g_list_page.minute = get_system_data().minute;
    g_list_page.updateing = 1;
    int num_size = 32;
    int num_size_y = 48;
	int vertical = 0; //垂直位置
    int horizontal = 0;  //水平位置
    int now_index;
    uint8_t *m_custom_image;
    m_custom_image = (uint8_t *)malloc(IMAGE_SIZE);

    
    Paint_NewImage(m_custom_image, EPD_2IN9BC_WIDTH, EPD_2IN9BC_HEIGHT, 0, WHITE);
    Paint_SelectImage(m_custom_image);
    Paint_Clear(WHITE);
    char* text = "阿";
    Paint_DrawLine(0,100,200,100,BLACK,DOT_PIXEL_2X2,LINE_STYLE_SOLID);

    int progress_bar_start = (int) (g_list_page.page*progress_line_length);
    int progress_bar_end = (int) ((g_list_page.page+1)*progress_line_length);

    Paint_DrawLine(195,progress_bar_start,195,progress_bar_end,BLACK,DOT_PIXEL_4X4,LINE_STYLE_SOLID);
    // Paint_DrawString_CN_scaled(30, 50, med1_time, WHITE, BLACK,1.5);
    Paint_DrawString_CN_scaled(5, 95, med1_name, WHITE, BLACK,1.8);
    Paint_DrawString_CN_scaled(5, 45, med1_time, WHITE, BLACK,1.8);
    
    // ds_screen_full_display(ds_paint_image_new);
    // ds_paint_image_copy();
    // free(m_custom_image);

    // ds_screen_partial_data_init();

    // ds_screen_partial_data_add(0,200,0,200,gImage_medbox_list_page_rev);
    
    //小时 十位
    now_index = med1_time[0]-'0';
    // ds_screen_partial_data_add(horizontal+0,horizontal+num_size,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
    //小时 个位
    now_index = med1_time[1]-'0';
    // ds_screen_partial_data_add(horizontal+num_size,horizontal+num_size*2,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
    //首次刷新才显示 :号
    // ds_screen_partial_data_add(horizontal+num_size*2,horizontal+num_size*2+8,vertical,vertical+num_size_y,gImage_time_symbol);
    
    // free(m_custom_image);
    //分钟 十位
    now_index = med1_time[3]-'0';
    // ds_screen_partial_data_add(horizontal+num_size*2+8,horizontal+num_size*3+8,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
    // //分钟 个位
    now_index = med1_time[4]-'0';
    // ds_screen_partial_data_add(horizontal+num_size*3+8,horizontal+num_size*4+8,vertical,vertical+num_size_y,gImage_num_size48[now_index]);    
    
    if(med2_time!=NULL){
        int vertical = 105;
        Paint_DrawString_CN_scaled(5, 195, med2_name, WHITE, BLACK,1.8);
        Paint_DrawString_CN_scaled(5, 145, med2_time, WHITE, BLACK,1.8);
        //小时 十位
        now_index = med2_time[0]-'0';
        // ds_screen_partial_data_add(horizontal+0,horizontal+num_size,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
        //小时 个位
        now_index = med2_time[1]-'0';
        // ds_screen_partial_data_add(horizontal+num_size,horizontal+num_size*2,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
        //首次刷新才显示 :号
        // ds_screen_partial_data_add(horizontal+num_size*2,horizontal+num_size*2+8,vertical,vertical+num_size_y,gImage_time_symbol);

        //分钟 十位
        now_index = med2_time[3]-'0';
        // ds_screen_partial_data_add(horizontal+num_size*2+8,horizontal+num_size*3+8,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
        // //分钟 个位
        now_index = med2_time[4]-'0';
        // ds_screen_partial_data_add(horizontal+num_size*3+8,horizontal+num_size*4+8,vertical,vertical+num_size_y,gImage_num_size48[now_index]);
    }else{
        Paint_DrawString_CN_scaled(5, 140, "无数据", WHITE, BLACK,1.8);
    }

    // ds_screen_partial_data_copy();
    ds_screen_full_display(ds_paint_image_new);

    g_list_page.hour_last = g_list_page.hour;
    g_list_page.minute_last = g_list_page.minute;
    g_list_page.init_status = 1;
    g_list_page.updateing = 0;
}

void listpage_move_up(){
    int max_pages = (g_list_page.intake_num + 1) / 2;
    printf("----------------MaxPage:%d----------------",max_pages);
    if(g_list_page.page >= max_pages-1){
        return;
    }
    g_list_page.page++;
    ds_ui_medbox_listpage_display_update();
}

void listpage_move_down(){
    if(g_list_page.page <= 0){
        return;
    }
    g_list_page.page--;
    ds_ui_medbox_listpage_display_update();
}

int cmp(const void *a, const void *b) {
  MEDICINE_SORT_T *med_a = (MEDICINE_SORT_T *)a;
  MEDICINE_SORT_T *med_b = (MEDICINE_SORT_T *)b;
  return strcmp(med_a->time, med_b->time);
}

LIST_STATUS_T get_list_status(){
    return g_list_page.status;
}


void handle_click(int type){
    int pagenum = (int) g_list_page.page;
    if(type == 1){//如果是上面的药
        g_list_page.status = VIEW_DETAIL;
        
        char *med1_name = medicine_sort_list[pagenum*2].name;
        char *med1_time = medicine_sort_list[pagenum*2].time;
        char *unit = medicine_sort_list[pagenum*2].unit;
        char *dose_per_time = malloc(sizeof(char)*30);
        sprintf(dose_per_time,"%d%s",medicine_sort_list[pagenum*2].dose_per_time,unit);
        
        uint8_t *m_detail_image;
        m_detail_image = (uint8_t *)malloc(IMAGE_SIZE);
        Paint_NewImage(m_detail_image, EPD_2IN9BC_WIDTH, EPD_2IN9BC_HEIGHT, 0, WHITE);
        Paint_SelectImage(m_detail_image);
        Paint_Clear(WHITE);
        Paint_DrawLine(0,100,200,100,BLACK,DOT_PIXEL_2X2,LINE_STYLE_SOLID);
        Paint_DrawString_CN_scaled(5, 95, med1_name, WHITE, BLACK,1.8);
        Paint_DrawString_CN_scaled(5, 45, med1_time, WHITE, BLACK,1.8);
        Paint_DrawString_CN_scaled(5, 145, dose_per_time, WHITE, BLACK,1.8);
        ds_screen_full_display(ds_paint_image_new);
    }
    if(type == 2){//如果是下面的药
        if(g_list_page.intake_num == (pagenum*2 + 1)){
            return;
        }
        g_list_page.status = VIEW_DETAIL;
        
        char *med1_name = medicine_sort_list[pagenum*2+1].name;
        char *med1_time = medicine_sort_list[pagenum*2+1].time;
        char *unit = medicine_sort_list[pagenum*2+1].unit;
        char *dose_per_time = malloc(sizeof(char)*30);
        sprintf(dose_per_time,"%d%s",medicine_sort_list[pagenum*2+1].dose_per_time,unit);
        
        uint8_t *m_detail_image;
        m_detail_image = (uint8_t *)malloc(IMAGE_SIZE);
        Paint_NewImage(m_detail_image, EPD_2IN9BC_WIDTH, EPD_2IN9BC_HEIGHT, 0, WHITE);
        Paint_SelectImage(m_detail_image);
        Paint_Clear(WHITE);
        Paint_DrawLine(0,100,200,100,BLACK,DOT_PIXEL_2X2,LINE_STYLE_SOLID);
        Paint_DrawString_CN_scaled(5, 95, med1_name, WHITE, BLACK,1.8);
        Paint_DrawString_CN_scaled(5, 45, med1_time, WHITE, BLACK,1.8);
        Paint_DrawString_CN_scaled(5, 145, dose_per_time, WHITE, BLACK,1.8);
        ds_screen_full_display(ds_paint_image_new);
    }
}

void ds_ui_medbox_listpage_display_init(){
    
    g_list_page.updateing = 1;
    g_list_page.page = 0;
    g_list_page.status = VIEW_LIST;
    // MEDICINE_SORT_T medicine_sort_list[500] = {};
    medicine_sort_list = malloc(50 * sizeof(MEDICINE_SORT_T));
    
    int cou = 0;
    cJSON *medicine_list_json = get_system_data().medicine_json;
    MEDICINE_SORT_T single_sort_medicine;

    int medicine_list_size = cJSON_GetArraySize(medicine_list_json);
    for (int i = 0; i < medicine_list_size; i++) {
        cJSON *single_medicine_json = cJSON_GetArrayItem(medicine_list_json, i);
        cJSON *times_list_json = cJSON_GetObjectItem(single_medicine_json, "times_list");
        char *medicine_name = cJSON_GetStringValue(cJSON_GetObjectItem(single_medicine_json, "name"));
        char *unit = cJSON_GetStringValue(cJSON_GetObjectItem(single_medicine_json, "unit"));
        int dose_per_time = cJSON_GetNumberValue(cJSON_GetObjectItem(single_medicine_json, "dose_per_time"));
        int times_list_size = cJSON_GetArraySize(times_list_json);
        for (int j = 0; j < times_list_size; j++) {
            char *single_time = cJSON_GetStringValue(cJSON_GetArrayItem(times_list_json, j));
            size_t name_len = strlen(medicine_name) + 1;
            size_t time_len = strlen(single_time) + 1;
            size_t unit_len = strlen(unit) + 1;
            single_sort_medicine.name = malloc(name_len);
            single_sort_medicine.time = malloc(time_len);
            single_sort_medicine.dose_per_time = malloc(sizeof(int));
            single_sort_medicine.unit = malloc(unit_len);

            strcpy(single_sort_medicine.name, medicine_name);
            strcpy(single_sort_medicine.time, single_time);
            strcpy(single_sort_medicine.unit, unit);
            single_sort_medicine.dose_per_time = dose_per_time;
            medicine_sort_list[cou++] = single_sort_medicine;
        }
    }
  // int sort_len = sizeof(medicine_sort_list)/sizeof(medicine_sort_list[0]);
    qsort(medicine_sort_list, cou, sizeof(single_sort_medicine), cmp);
    g_list_page.intake_num = cou;
    ds_ui_medbox_listpage_display_update();
    g_list_page.updateing = 0;
}

