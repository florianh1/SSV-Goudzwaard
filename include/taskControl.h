/******************************************************************************
 * File           : taskControl.h
******************************************************************************/
#ifndef _TASKCONTROL_H_
#define _TASKCONTROL_H_

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

/******************************************************************************
  Function prototypes
******************************************************************************/

void task_control(void* pvParameter);

#endif // _TASKCONTROL_H_