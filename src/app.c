#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "../include/motor.h"

/* Bling GPIO pin */
#define BLINK_GPIO 2

/* Wi-Fi SSID, note a maximum character length of 32 */
#define ESP_WIFI_SSID "SSV GoudZwaard"

/* Wi-Fi password, note a minimal length of 8 and a maximum length of 64 characters */
#define ESP_WIFI_PASS "12345678"

/* Wi-Fi channel, note a range from 0 to 13 */
#define ESP_WIFI_CHANNEL 0

/* Amount of Access Point connections, note a maximum amount of 4 */
#define AP_MAX_CONNECTIONS 4

/* Port number of TCP-server */
#define TCP_PORT 3000

/* Maximum queue length of pending connections */
#define LISTEN_QUEUE 2

/* Size of the recvieve buffer (amount of charcters) */
#define RECV_BUF_SIZE 64

/* Port number of UDP-servers */
#define RECEIVE_CONTROL_UDP_PORT 3333
#define TRANSMIT_BATTERY_PERCENTAGE_UDP_PORT 3334

/* Pre-defined delay times, in seconds */
#define TASK_DELAY_1 1000
#define TASK_DELAY_5 5000

const int CLIENT_CONNECTED_BIT = BIT0;
const int CLIENT_DISCONNECTED_BIT = BIT1;
const int AP_STARTED_BIT = BIT2;

static const char *TAG = "Wifi";
static EventGroupHandle_t wifi_event_group;

#include <battery.h>
#include <controls.h>
#include <syringe.h>
#include <blink.h>
#include <wifi.h>

SemaphoreHandle_t xJoystickSemaphore = NULL;
SemaphoreHandle_t yJoystickSemaphore = NULL;
SemaphoreHandle_t scrollbarSemaphore = NULL;
SemaphoreHandle_t batteryPercentageSemaphore = NULL;

uint8_t joystick_y = 4;
uint8_t joystick_x = 4;
uint8_t scrollbar = 0;
uint8_t battery_percentage = 94;

extern EventGroupHandle_t wifi_event_group;;

/**
 * Initialization of Non-Volitile Storage
 *
 * @return void
 */
void nvs_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("NVS init", "NVS setup finished...");
}

/**
 * Control engines task
 *
 * This task is responsible for controlling the engines. This task determines which way the submarine will move and how fast.
 *
 * //TODO: move to motor.c
 *
 * @return void
 */
void control_engines_task(void *pvParameter)
{
    static const char *TASK_TAG = "control_engines_task";
    ESP_LOGI(TASK_TAG, "task started");

    while (1)
    {
        if (yJoystickSemaphore != NULL)
        {
            if (xSemaphoreTake(yJoystickSemaphore, (TickType_t)10) == pdTRUE)
            {
                ESP_LOGI(TASK_TAG, "joystick_y: %d", joystick_y);
                xSemaphoreGive(yJoystickSemaphore);
            }
        }
        if (xJoystickSemaphore != NULL)
        {
            if (xSemaphoreTake(xJoystickSemaphore, (TickType_t)10) == pdTRUE)
            {
                ESP_LOGI(TASK_TAG, "joystick_x: %d", joystick_x);
                xSemaphoreGive(xJoystickSemaphore);
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * Main loop
 *
 * @return void
 */
void app_main()
{
    xTaskCreate(motor_task, "motor_task", 4096, NULL, 5, NULL);

    while(1);

    static const char *APP_MAIN_TAG = "app_main";

    xJoystickSemaphore = xSemaphoreCreateMutex();
    yJoystickSemaphore = xSemaphoreCreateMutex();
    scrollbarSemaphore = xSemaphoreCreateMutex();
    batteryPercentageSemaphore = xSemaphoreCreateMutex();

    if (xJoystickSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the xJoystickSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "xJoystickSemaphore created");
    }

    if (yJoystickSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the yJoystickSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "yJoystickSemaphore created");
    }

    if (scrollbarSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the scrollbarSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "scrollbarSemaphore created");
    }

    if (batteryPercentageSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the batteryPercentageSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "batteryPercentageSemaphore created");
    }

    // Start blink task for testing
    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

    // Initialization of Non-Volitile Storage
    nvs_init();

    // Init WiFI
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_event_group = xEventGroupCreate();
    start_dhcp_server();
    wifi_init();

    xTaskCreate(&battery_percentage_transmit_task, "battery_percentage_transmit_task", 4096, NULL, 5, NULL);
    xTaskCreate(&receive_control_task, "receive_control_task", 4096, NULL, 5, NULL);
    xTaskCreate(&control_syringe_task, "control_syringe_task", 4096, NULL, 5, NULL);
    xTaskCreate(&control_engines_task, "control_engines_task", 4096, NULL, 5, NULL);

    xTaskCreate(&print_sta_info, "print_sta_info", 4096, NULL, 5, NULL);
}
