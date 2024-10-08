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
#include "ds_qrcodegen.h"
#define TAG  "qrpage_debug"

#define TARGET_SIZE 200
#define OUTPUT_SIZE 5000

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
}QR_PAGE_T;

//200*200 像素屏幕适配，偏移值
static int offset_v = 29; 
static int offset_h = 24; 

QR_PAGE_T g_qr_page;

char* final_text = "hello_world";

static unsigned char hexOutput[OUTPUT_SIZE] = {0xff};

void ds_ui_medbox_qrcodepage_init(){
    memset(&final_text,0,sizeof(final_text));
    final_text = "hello_world";
}

void enlargeAndPadArray(uint8_t **inputArray, int qr_size, uint8_t outputArray[TARGET_SIZE][TARGET_SIZE]) {
	// Calculate the scaling factor to make the QR code as large as possible
	int scaleFactor = TARGET_SIZE / qr_size;
	int enlargedSize = qr_size * scaleFactor;

	// Initialize the entire output array to 0 (for padding)
	for (int i = 0; i < TARGET_SIZE; i++) {
		for (int j = 0; j < TARGET_SIZE; j++) {
			outputArray[i][j] = 0;
		}
	}

	// Enlarge the input array
	for (int i = 0; i < qr_size; i++) {
		for (int j = 0; j < qr_size; j++) {
			// Replicate the pixel (scaleFactor * scaleFactor times)
			for (int si = 0; si < scaleFactor; si++) {
				for (int sj = 0; sj < scaleFactor; sj++) {
					int enlargedRow = i * scaleFactor + si;
					int enlargedCol = j * scaleFactor + sj;
					if (enlargedRow < TARGET_SIZE && enlargedCol < TARGET_SIZE) {
						outputArray[enlargedRow][enlargedCol] = inputArray[i][j];
					}
				}
			}
		}
	}

	// If the enlarged size is smaller than TARGET_SIZE, pad the edges symmetrically
	int padding = (TARGET_SIZE - enlargedSize) / 2;

	// Create a temporary padded array
	uint8_t paddedArray[TARGET_SIZE][TARGET_SIZE] = {0};

	for (int i = 0; i < enlargedSize; i++) {
		for (int j = 0; j < enlargedSize; j++) {
			paddedArray[i + padding][j + padding] = outputArray[i][j];
		}
	}

	// Copy the padded array back into the output array
	for (int i = 0; i < TARGET_SIZE; i++) {
		for (int j = 0; j < TARGET_SIZE; j++) {
			outputArray[i][j] = paddedArray[i][j];
		}
	}
}

void convertToHex(uint8_t outputArray[TARGET_SIZE][TARGET_SIZE], const unsigned char hexOutput[OUTPUT_SIZE]) {
    int outputIndex = 0;
    int bitIndex = 0;
    unsigned char byte = 0;

    // Loop through the 200x200 array and pack 8 pixels into each byte
    for (int i = 0; i < TARGET_SIZE; i++) {
        for (int j = 0; j < TARGET_SIZE; j++) {
            uint8_t pixel = outputArray[i][j] & 1;  // Ensure pixel is 1 or 0

            // Shift pixel into the correct bit position in the byte
            byte |= pixel << (7 - bitIndex);
            bitIndex++;

            // Once we have 8 bits, store the byte in the hex output array
            if (bitIndex == 8) {
                ((unsigned char*)hexOutput)[outputIndex++] = byte;
                byte = 0;
                bitIndex = 0;

                // Stop if we've filled the hex output array
                if (outputIndex >= OUTPUT_SIZE) return;
            }
        }
    }

    // Handle the case where the last byte isn't fully filled
    if (bitIndex > 0) {
        ((unsigned char*)hexOutput)[outputIndex++] = byte;
    }

    return;
}

// void processQRCode(uint8_t **inputArray, int A, const unsigned char hexOutput[OUTPUT_SIZE]) {
//     unsigned char byte = 0;
//     int bitIndex = 0, outputIndex = 0;
// 	int max_scale_factor = (int) TARGET_SIZE / A;
//     int scaleFactor = (200 / A) > max_scale_factor ? max_scale_factor : (200 / A); // Determine scale factor

//     // Scale and convert to hex output on-the-fly
//     for (int i = 0; i < A; i++) {
//         for (int j = 0; j < A; j++) {
//             uint8_t pixel = inputArray[i][j] & 1; // Ensure pixel is 0 or 1

//             // Replicate the pixel scaleFactor times
//             for (int si = 0; si < scaleFactor; si++) {
//                 for (int sj = 0; sj < scaleFactor; sj++) {
//                     // Add pixel to the byte
//                     byte |= pixel << (7 - bitIndex);
//                     if (++bitIndex == 8) {
//                         ((unsigned char*)hexOutput)[outputIndex++] = byte;
//                         byte = 0;
//                         bitIndex = 0;
//                         if (outputIndex >= OUTPUT_SIZE) return; // Prevent overflow
//                     }
//                 }
//             }
//         }
//     }
//     // Handle any remaining bits
//     if (bitIndex > 0) ((unsigned char*)hexOutput)[outputIndex++] = byte;
// }

void convert_qr_to_image(const uint8_t qr_code[], int A, unsigned char image[]) {
    int scale = 200 / A; // Scaling factor to fit the QR code into 200x200
    int x, y, i, j, byte_index, bit_position;
    
    // Initialize the image array with zeros (all white)
    for (i = 0; i < 5000; i++) {
        image[i] = 0x00;
    }

    // Loop over the image 200x200
    for (y = 0; y < 200; y++) {
        for (x = 0; x < 200; x++) {
            // Find the corresponding pixel in the original QR code
            int qr_x = x / scale;
            int qr_y = y / scale;
			// printf("-----x%d------\n",qr_x);
			// printf("-----y%d------\n",qr_y);
			// ESP_LOGI("123","----------------%d------------------", qr_x);
			// ESP_LOGI("123","-------------------%d------------------",qr_y);

            // Determine if the QR module is black (1) or white (0)
            if (qrcodegen_getModule(qr_code, qr_x, qr_y) == 1) {
				
                // Calculate the byte and bit position in the image array
                byte_index = (y * 25) + (x / 8);
                bit_position = 7 - (x % 8);

                // Set the corresponding bit in the image array to 1 (black pixel)
                image[byte_index] |= (1 << bit_position);
            }
        }
    }
}

void generate_qrcode_by_text(char* text){

    uint8_t qr0[qrcodegen_BUFFER_LEN_FOR_VERSION(5)];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(5)];
	bool ok = qrcodegen_encodeText(text,
	                               tempBuffer, qr0, qrcodegen_Ecc_LOW,
	                               qrcodegen_VERSION_MIN, 5,
	                               qrcodegen_Mask_AUTO, true);
    
    int qr_size = qrcodegen_getSize(qr0);  // Example size of the input array (21x21)
	printf("-------------------qrcode obtained---------------------");

	// Dynamically allocate a 2D array for the input QR code
	// uint8_t **qrArray = (uint8_t **)malloc(qr_size * sizeof(uint8_t *));
	// for (int i = 0; i < qr_size; i++) {
	// 	qrArray[i] = (uint8_t *)malloc(qr_size * sizeof(uint8_t));
	// 	for (int j = 0; j < qr_size; j++) {
	// 		qrArray[i][j] = qrcodegen_getModule(qr0, i, j);  // Random 0 or 1 (just an example)
	// 	}
	// }
	convert_qr_to_image(qr0, qr_size, hexOutput);

	// processQRCode(qrArray,qr_size,hexOutput);

    // static uint8_t outputArray[TARGET_SIZE][TARGET_SIZE];

	// // // Enlarge and pad the input array to fit 200x200
	// enlargeAndPadArray(qrArray, qr_size, outputArray);
	// printf("-------------enlarged_obtained---------------");
	// // Convert the 200x200 array to 1-bit-per-pixel hex output
	// convertToHex(outputArray, hexOutput);

	// printf("-----------output obtained------------");

    // for (int i = 0; i < qr_size; i++) {
	// 	free(qrArray[i]);
	// }
	// free(qrArray);

    // for (int i = 0; i < TARGET_SIZE; i++) {
    //     for (int j=0; j< TARGET_SIZE; j++){
    //         free(outputArray[i]);
    //     }
	// }
	// free(outputArray);
    
    // return;
}

void ds_ui_medbox_qrcodepage_display_init(){
    g_qr_page.updateing = 1;
    
    char* qrcode_wifi_message = malloc(100*sizeof(char)+1);
    char* ssid = CONFIG_ESP_AP_WIFI_SSID;
    char* pwd = CONFIG_ESP_AP_WIFI_PASSWORD;
    sprintf(qrcode_wifi_message,"WIFI:T:WPA;S:%s;P:%s;;",ssid,pwd);
    ds_screen_partial_data_init();
    generate_qrcode_by_text(qrcode_wifi_message);
    ds_screen_partial_data_add(0,200,0,200,hexOutput);
    ds_screen_partial_data_copy();
    // refresh_part();
    g_qr_page.updateing = 0;
    // print_system_data_wifi_info();
}





