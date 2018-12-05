/******************************************************************************
 * File           : blink.h
******************************************************************************/
#ifndef _BLINK_H_
#define _BLINK_H_

#include <settings.h>

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

/******************************************************************************
  Function prototypes
******************************************************************************/
void blink_task(void* pvParameter);

#endif // _BLINK_H_