/******************************************************************************
 * File           : battery.h
******************************************************************************/
#ifndef _BATTERY_H_
#define _BATTERY_H_

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

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "driver/adc.h"

#include <syringe.h>

/******************************************************************************
  Function prototypes
******************************************************************************/
void battery_percentage_transmit_task(void* pvParameter);

#endif // _BATTERY_H_