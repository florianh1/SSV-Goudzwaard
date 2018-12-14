/******************************************************************************
 * File           : I2Scamera.h
******************************************************************************/
#ifndef _I2SCAMERA_H_
#define _I2SCAMERA_H_

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/periph_ctrl.h"
#include "esp_event_loop.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "rom/lldesc.h"
#include "soc/gpio_sig_map.h"
#include "soc/i2s_reg.h"
#include "soc/i2s_struct.h"
#include "soc/io_mux_reg.h"
#include "soc/soc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <settings.h>

typedef struct {
    int D0; /*!< GPIO pin for camera D0 line */
    int D1; /*!< GPIO pin for camera D1 line */
    int D2; /*!< GPIO pin for camera D2 line */
    int D3; /*!< GPIO pin for camera D3 line */
    int D4; /*!< GPIO pin for camera D4 line */
    int D5; /*!< GPIO pin for camera D5 line */
    int D6; /*!< GPIO pin for camera D6 line */
    int D7; /*!< GPIO pin for camera D7 line */
    int XCLK; /*!< GPIO pin for camera XCLK line */
    int PCLK; /*!< GPIO pin for camera PCLK line */
    int VSYNC; /*!< GPIO pin for camera VSYNC line */
    int xclk_freq_hz; /*!< Frequency of XCLK signal, in Hz */
    ledc_timer_t ledc_timer; /*!< LEDC timer to be used for generating XCLK  */
    ledc_channel_t ledc_channel; /*!< LEDC channel to be used for generating XCLK  */
    int frame_width;
    int frame_height;
    uint8_t pixel_byte_num;

} camera_config_t;

esp_err_t I2S_camera_init(camera_config_t* config);
#ifdef CONVERT_RGB565_TO_RGB332
uint8_t* camera_getLine(uint16_t lineno);
#else
uint16_t* camera_getLine(uint16_t lineno);
#endif // CONVERT_RGB565_TO_RGB332

#endif //_I2SCAMERA_H_