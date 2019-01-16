#include <motor.h>

extern SemaphoreHandle_t xJoystickSemaphore;
extern SemaphoreHandle_t yJoystickSemaphore;

extern uint8_t joystick_y;
extern uint8_t joystick_x;

int8_t positionTabel[3][3][2] = {
    { { 99, 81 }, { 99, 63 }, { 0, 0 } },
    { { 66, 45 }, { 99, 36 }, { 99, 27 } },
    { { 33, 9 }, { 99, 9 }, { 99, 9 } }
};

int pwm_left = 0;
int pwm_right = 0;
int direction_left = 0;
int direction_right = 0;

/**
 * @controls the speed of both motors
 * 
 * @param arg 
 */
void motor_task(void* arg)
{
    static const char* TASK_TAG = "motor_task";

    esp_log_level_set("motor_task", ESP_LOG_ERROR);

    //1. mcpwm gpio initialization
    ESP_LOGI(TASK_TAG, "Initializing MCPWM pins...");
    MCPWMinit();

    //2. initial mcpwm configuration
    // ESP_LOGE(TASK_TAG, "Configuring Initial Parameters of mcpwm...");
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
        if (yJoystickSemaphore != NULL && xJoystickSemaphore != NULL) {

            bool ahead = (joystick_y <= 4);
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
                right = (!rightSide) ? 0 : (xValue * 33);
                left = (rightSide) ? 0 : (xValue * 33);
            } else { // Joystick is slanted to any corner
                right = (rightSide) ? positionTabel[xValue - 1][(4 - yValue) - 1][0] : positionTabel[xValue - 1][(4 - yValue) - 1][1];
                left = (rightSide) ? positionTabel[xValue - 1][(4 - yValue) - 1][1] : positionTabel[xValue - 1][(4 - yValue) - 1][0];
            }

            // right *= 0.7;
            // left *= 0.7;

            if (ahead) {
                brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, right); // right motor
                brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, left); // left motor
            } else {
                brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, right * 0.8); // right motor
                brushed_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_1, left * 0.8); // left motor
            }
        }

        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

/**
 * @initialize the gpio pins for pwm signal
 * 
 */
void MCPWMinit()
{
    //right motor
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MOTOR_PWM0B_OUT);

    //left motor
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, MOTOR_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1B, MOTOR_PWM1B_OUT);
}

/**
 * @motor moves in forward direction, with duty cycle = duty %
 * 
 * @param mcpwm_num 
 * @param timer_num 
 * @param duty_cycle 
 */
void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle)
{
    if (mcpwm_num) {
        if (direction_left != 0) {
            pwm_left = 0;
        }
        for (size_t i = pwm_left; i < duty_cycle; i++) {
            mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
            mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, i);
            mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        pwm_left = duty_cycle;
    } else {
        if (direction_right != 0) {
            pwm_left = 0;
        }
        for (size_t i = pwm_right; i < duty_cycle; i++) {
            mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
            mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, i);
            mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        pwm_right = duty_cycle;
    }

    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @motor moves in backward direction, with duty cycle = duty %
 * 
 * @param mcpwm_num 
 * @param timer_num 
 * @param duty_cycle 
 */
void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, float duty_cycle)
{
    if (mcpwm_num) {
        if (direction_left != 1) {
            pwm_left = 1;
        }
        for (size_t i = pwm_left; i < duty_cycle; i++) {
            mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
            mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, i);
            mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        pwm_left = duty_cycle;
    } else {
        if (direction_right != 1) {
            pwm_left = 1;
        }
        for (size_t i = pwm_right; i < duty_cycle; i++) {
            mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
            mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, i);
            mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        pwm_right = duty_cycle;
    }

    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @stops the motors
 * 
 * @param mcpwm_num 
 * @param timer_num 
 */
void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}