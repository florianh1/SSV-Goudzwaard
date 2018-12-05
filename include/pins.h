/******************************************************************************
 * File           : pins.h
******************************************************************************/
#ifndef _PINS_H_
#define _PINS_H_

// Pins for reading battery level
#define BATTERY_CELL_1 ADC1_CHANNEL_4
#define BATTERY_CELL_2 ADC1_CHANNEL_6
#define BATTERY_CELL_3 ADC1_CHANNEL_7

// Pins for camera
#define CAMERA_D0 36
#define CAMERA_D1 39
#define CAMERA_D2 19
#define CAMERA_D3 18
#define CAMERA_D4 5
#define CAMERA_D5 33
#define CAMERA_D6 25
#define CAMERA_D7 26
#define CAMERA_XCLK 27
#define CAMERA_PCLK 14
#define CAMERA_VSYNC 13
#define CAMERA_SDA 21
#define CAMERA_SCL 22

// Pins for syringe
#define SYRINGE_IN1 12
#define SYRINGE_IN2 15
#define SYRINGE_IN3 16
#define SYRINGE_IN4 17

// Pins for controlling engines
// Motor right
#define MOTOR_PWM0A_OUT 2 //Set GPIO 2 as PWM0A
#define MOTOR_PWM0B_OUT 4 //Set GPIO 4 as PWM0B
// Motor motor left
#define MOTOR_PWM1A_OUT 3 //Set GPIO 3 as PWM1A
#define MOTOR_PWM1B_OUT 23 //Set GPIO 23 as PWM1B

// Blink pin for testing
#define BLINK_GPIO 2

#endif //_PINS_H_