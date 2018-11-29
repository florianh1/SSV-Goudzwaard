/******************************************************************************
 * File           : L298N dual motor driving
******************************************************************************/
#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <stdio.h>

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "nvs_flash.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

/******************************************************************************
  pin definitions of the pwm pins
******************************************************************************/
//motor right
#define GPIO_PWM0AR_OUT 15   //Set GPIO 15 as PWM0A
#define GPIO_PWM0BR_OUT 16   //Set GPIO 16 as PWM0B

//motor motor left
#define GPIO_PWM0AL_OUT 25   //Set GPIO 25 as PWM0A
#define GPIO_PWM0BL_OUT 26   //Set GPIO 26 as PWM0B

/******************************************************************************
  Function prototypes
******************************************************************************/
void motor_task(void *arg);
void mcpwm_example_gpio_initialize();
void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle);
void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle);
void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num);
void control_engines_task(void *pvParameter);

#endif // _MOTOR_H_