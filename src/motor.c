/* brushed dc motor control example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * This example will show you how to use MCPWM module to control brushed dc motor.
 * This code is tested with L298 motor driver.
 * User may need to make changes according to the motor driver they use.
*/

#include "../include/motor.h"

extern SemaphoreHandle_t xJoystickSemaphore;
extern SemaphoreHandle_t yJoystickSemaphore;
extern SemaphoreHandle_t scrollbarSemaphore;
extern SemaphoreHandle_t batteryPercentageSemaphore;

extern uint8_t joystick_y;
extern uint8_t joystick_x;
extern uint8_t scrollbar;
extern uint8_t battery_percentage;

/**
 * Control engines task
 * 
 * This task is responsible for controlling the engines. This task determines which way the submarine will move and how fast.
 * 
 * //TODO: move to motor.c
 *
 * @return void
 */
void control_engines_task(void* pvParameter)
{
    static const char* TASK_TAG = "control_engines_task";
    ESP_LOGI(TASK_TAG, "task started");

    while (1) {
        if (xJoystickSemaphore != NULL) {
            if (xSemaphoreTake(xJoystickSemaphore, (TickType_t)10) == pdTRUE) {
                ESP_LOGI(TASK_TAG, "joystick_x: %d", joystick_x);
                xSemaphoreGive(xJoystickSemaphore);
            }
        }

        if (yJoystickSemaphore != NULL) {
            if (xSemaphoreTake(yJoystickSemaphore, (TickType_t)10) == pdTRUE) {
                ESP_LOGI(TASK_TAG, "joystick_y: %d", joystick_y);
                xSemaphoreGive(yJoystickSemaphore);
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * controls the speed motors
 * 
 * @ param void *
 * @ return void 
 */
void motor_task(void* arg)
{
    printf("Testing brushed motor...\n");
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000; //frequency = 500Hz,
    pwm_config.cmpr_a = 0; //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0; //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config); //Configure PWM0A & PWM0B with above settings
    while (1) {
        brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 100.0);
        vTaskDelay(2000 / portTICK_RATE_MS);
        brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 30.0);
        vTaskDelay(2000 / portTICK_RATE_MS);
        brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

/**
 * initialize the gpio pins for pwm signal
 * 
 * @ param void *
 * @ return void 
 */
void mcpwm_example_gpio_initialize()
{
    printf("initializing mcpwm gpio...\n");
    //right motor
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
}

/**
 * brief motor moves in forward direction, with duty cycle = duty %
 * 
 * @ param void *
 * @ return void 
 */
void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * brief motor moves in backward direction, with duty cycle = duty %
 * 
 * @ param void *
 * @ return void 
*/
void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * brief motor stop
 * 
 * @ param void *
 * @ return void 
 */
void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}