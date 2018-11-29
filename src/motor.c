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

/******************************************************************************
  pin definitions of the pwm pins
******************************************************************************/
//motor right
#define GPIO_PWM0A_OUT 15 //Set GPIO 15 as PWM0A
#define GPIO_PWM0B_OUT 16 //Set GPIO 16 as PWM0B

//motor motor left
#define GPIO_PWM1A_OUT 25 //Set GPIO 25 as PWM0A
#define GPIO_PWM1B_OUT 26 //Set GPIO 26 as PWM0B

extern SemaphoreHandle_t xJoystickSemaphore;
extern SemaphoreHandle_t yJoystickSemaphore;

extern uint8_t joystick_y;
extern uint8_t joystick_x;

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
    static const char* TASK_TAG = "control_engines_task";

    //1. mcpwm gpio initialization
    ESP_LOGE(TASK_TAG, "Initializing MCPWM pins...");
    MCPWMinit();

    //2. initial mcpwm configuration
    ESP_LOGE(TASK_TAG, "Configuring Initial Parameters of mcpwm...");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000; //frequency = 500Hz,
    pwm_config.cmpr_a = 0; //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0; //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config); //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &pwm_config);
    float right;
    float left;

    while (1) {

        if (yJoystickSemaphore != NULL && xJoystickSemaphore != NULL) {
            // printf("Semaforen zijn vrij!\n");
            // printf("X = %d  | Y = %d  \n", joystick_x, joystick_y);

            // Y in the middle
            if (joystick_y == 3 || joystick_y == 4) {
                if (joystick_x == 3 || joystick_x == 4) {
                    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
                    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);
                } else if (joystick_x < 3) {
                    left = (3 - joystick_x) * 33;
                    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 0); // right motor
                    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, left); // left motor
                } else if (joystick_x > 4) {
                    right = (joystick_x - 4) * 33;
                    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, right); // right motor
                    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, 0); // left motor
                }
            } else
                // Y on the upper side
                if (joystick_y < 3) {
                right = (3 - joystick_y) * 33;
                left = (3 - joystick_y) * 33;
                if (joystick_x > 4) {
                    //to the right
                    right = (joystick_x - 3) * 33;
                } else if (joystick_x < 3) {
                    //to the left
                    left = (3 - joystick_x) * 33;
                }

                brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, right); // right motor
                brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, left); // left motor
            } else
                // Y on the under side
                if (joystick_y > 4) {
                //forward
                right = (joystick_y - 4) * 33;
                left = (joystick_y - 4) * 33;
                if (joystick_x > 4) {
                    //to the right
                    right = (joystick_x - 3) * 33;
                } else if (joystick_x < 3) {
                    //to the left
                    left = (3 - joystick_x) * 33;
                }

                brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, right); // right motor
                brushed_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_1, left); // left motor
            }
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

/**
 * initialize the gpio pins for pwm signal
 * 
 * @ param void *
 * @ return void 
 */
void MCPWMinit()
{
    printf("initializing mcpwm gpio...\n");
    //right motor
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);

    //left motor
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, GPIO_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1B, GPIO_PWM1B_OUT);
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