/******************************************************************************
 * File           : L298N dual motor driving
******************************************************************************/
#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

/******************************************************************************
  Function prototypes
******************************************************************************/
void motor_task(void *arg);
void mcpwm_example_gpio_initialize();
void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle);
void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle);
void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num);

#endif // _MOTOR_H_