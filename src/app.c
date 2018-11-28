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
