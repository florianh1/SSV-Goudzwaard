/******************************************************************************
 * File           : settings.h
******************************************************************************/
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/******************************************************************************
  WiFi Settings
******************************************************************************/

/* Wi-Fi SSID, note a maximum character length of 32 */
#define ESP_WIFI_SSID "SSV Goudzwaard"

/* Wi-Fi password, note a minimum length of 8 and a maximum length of 64 characters */
#define ESP_WIFI_PASS "12345678"

/* Wi-Fi channel, note a range from 0 to 13 */
#define ESP_WIFI_CHANNEL 0

/* Amount of Access Point connections, note a maximum amount of 4 */
#define AP_MAX_CONNECTIONS 1

/******************************************************************************
  Pins
******************************************************************************/

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

/******************************************************************************
  Camera Settings
******************************************************************************/

// Camera image settings
//#define CAM_VGA
//#define CAM_CIF
//#define CAM_QVGA
#define CAM_QQVGA
//#define CAM_QCIF

// Camera color settings
//#define CAM_COLOR YUV422
#define CAM_COLOR RGB565
//#define CAM_COLOR BAYER_RAW
//#define CAM_COLOR PBAYER_RAW

#define CONVERT_RGB565_TO_RGB332
//#define USE_BMP_HEADER

/******************************************************************************
  Port numbers
******************************************************************************/

/* Port number of UDP-servers */
#define RECEIVE_CONTROL_UDP_PORT 3333
#define TRANSMIT_BATTERY_PERCENTAGE_UDP_PORT 3334

#endif //_SETTINGS_H_