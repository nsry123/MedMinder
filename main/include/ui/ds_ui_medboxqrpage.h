#ifndef _DS_UI_MEDBOX_QRPAGE_H_
#define _DS_UI_MEDBOX_QRPAGE_H_

#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "esp_types.h"
#include "driver/periph_ctrl.h"
#include "esp_log.h"




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

// void ds_ui_medbox_mainpage_updatetime(void);
// void ds_ui_medbox_mainpage_show_time(uint8_t type);
// void ds_ui_medbox_mainpage_show_time_init();
// void ds_ui_medbox_mainpage_init(void);

void ds_ui_medbox_qrcodepage_init(void);
void ds_ui_medbox_qrcodepage_display_init(void);


#endif