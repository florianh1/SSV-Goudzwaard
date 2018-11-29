/******************************************************************************
 * File           : syringe.h
******************************************************************************/
#ifndef _WIFI_H_
#define _WIFI_H_

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

EventGroupHandle_t wifi_event_group;

/******************************************************************************
  Function prototypes
******************************************************************************/
esp_err_t event_handler(void* ctx, system_event_t* event);
void wifi_init();
void printStationList();
void start_dhcp_server();
void print_sta_info(void* pvParam);

#endif // _WIFI_H_