/******************************************************************************
 * File           : camera.h
******************************************************************************/
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <I2Scamera.h>
#include <camera.h>
#include <ov7670.h>

#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "driver/ledc.h"

/******************************************************************************
  Function prototypes
******************************************************************************/
void camera_task(void* pvParameter);

#endif // _CAMERA_H_