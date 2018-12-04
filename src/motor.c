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
#define GPIO_PWM0A_OUT 1 //Set GPIO 15 as PWM0A
#define GPIO_PWM0B_OUT 3 //Set GPIO 16 as PWM0B

//motor motor left
#define GPIO_PWM1A_OUT 5 //Set GPIO 25 as PWM0A
#define GPIO_PWM1B_OUT 23 //Set GPIO 26 as PWM0B

//settings for the motor speed
#define MAX_SPEED 100
#define ACCElERATION_TIMES 3
#define ACCELERATION (MAX_SPEED / ACCElERATION_TIMES)
#define JOYSTICK_MIDDLE_LOW 3
#define JOYSTICK_MIDDLE_HIGH 4

extern SemaphoreHandle_t xJoystickSemaphore;
extern SemaphoreHandle_t yJoystickSemaphore;

extern uint8_t joystick_y;
extern uint8_t joystick_x;

int8_t positionTabel[3][3][2] = {
    { { 99, 81 }, { 99, 63 }, { 99, 45 } },
    { { 66, 45 }, { 66, 36 }, { 99, 27 } },
    { { 33, 9 }, { 99, 9 }, { 99, 9 } }
};

/**
 * controls the speed of both motors
 * 
 * @ param void *
 * @ return void 
 */
void motor_task(void* arg)
{
    static const char* TASK_TAG = "motor_task";

    esp_log_level_set(TASK_TAG, ESP_LOG_VERBOSE);

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
    float right = 0;
    float left = 0;

    while (1) {
        // ESP_LOGE(TASK_TAG, "Joystick X = %d || Y = %d \n", joystick_x, joystick_y);

        if (yJoystickSemaphore != NULL && xJoystickSemaphore != NULL) {

            bool ahead = (joystick_y < 3);
            bool rightSide = (joystick_x > 4);

            int8_t xValue = (joystick_x <= 3) ? abs(joystick_x - 3) : abs(joystick_x - 4);
            int8_t yValue = (joystick_y <= 3) ? abs(joystick_y - 3) : abs(joystick_y - 4);

            if (yValue == 0 && xValue == 0) { // Middle
                right = 0;
                left = 0;
            } else if (xValue == 0) { // Y in the middle
                right = yValue * 33;
                left = yValue * 33;
            } else if (yValue == 0) { // X in the middle
                right = (rightSide) ? 0 : (xValue * 33);
                left = (!rightSide) ? 0 : (xValue * 33);
            } else {
                xValue--;
                yValue--;

                right = (rightSide) ? positionTabel[xValue][yValue][1] : positionTabel[xValue][yValue][0];
                left = (rightSide) ? positionTabel[xValue][yValue][0] : positionTabel[xValue][yValue][1];
            }

            ESP_LOGI(TASK_TAG, "Joystick(x: %d - y: %d) Power(L: %f - R: %f)", xValue, yValue, left, right);

            if (ahead) {
                brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, right); // right motor
                brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, left); // left motor
            } else {
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