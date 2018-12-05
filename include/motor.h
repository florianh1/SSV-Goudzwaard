/******************************************************************************
 * File           : L298N dual motor driving
******************************************************************************/
#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <pins.h>

#include "esp_attr.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

/******************************************************************************
  Function prototypes
******************************************************************************/
void motor_task(void* arg);
void MCPWMinit();
void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle);
void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle);
void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num);
void control_engines_task(void* pvParameter);

#endif // _MOTOR_H_
