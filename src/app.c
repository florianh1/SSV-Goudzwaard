#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <battery.h>
#include <blink.h>
#include <camera.h>
#include <controls.h>
#include <motor.h>
#include <syringe.h>
#include <wifi.h>

SemaphoreHandle_t xJoystickSemaphore = NULL;
SemaphoreHandle_t yJoystickSemaphore = NULL;
SemaphoreHandle_t scrollbarSemaphore = NULL;
SemaphoreHandle_t batteryPercentageSemaphore = NULL;

uint8_t joystick_y = 4;
uint8_t joystick_x = 4;
uint8_t scrollbar = 0;
uint8_t battery_percentage = 94;

extern EventGroupHandle_t wifi_event_group;
extern const int CLIENT_CONNECTED_BIT;

/**
 * @Initialization of Non-Volitile Storage
 * 
 */
void nvs_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("NVS init", "NVS setup finished...");
}

/**
 * @main loop
 * 
 */
void app_main()
{
    static const char* APP_MAIN_TAG = "app_main";

    esp_log_level_set("camera_task", ESP_LOG_INFO);
    esp_log_level_set("Wifi", ESP_LOG_INFO);

    xJoystickSemaphore = xSemaphoreCreateMutex();
    yJoystickSemaphore = xSemaphoreCreateMutex();
    scrollbarSemaphore = xSemaphoreCreateMutex();
    batteryPercentageSemaphore = xSemaphoreCreateMutex();

    if (xJoystickSemaphore == NULL) {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    } else {
        /* The semaphore can now be used. Its handle is stored in the xJoystickSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "xJoystickSemaphore created");
    }

    if (yJoystickSemaphore == NULL) {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    } else {
        /* The semaphore can now be used. Its handle is stored in the yJoystickSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "yJoystickSemaphore created");
    }

    if (scrollbarSemaphore == NULL) {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    } else {
        /* The semaphore can now be used. Its handle is stored in the scrollbarSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "scrollbarSemaphore created");
    }

    if (batteryPercentageSemaphore == NULL) {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    } else {
        /* The semaphore can now be used. Its handle is stored in the batteryPercentageSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "batteryPercentageSemaphore created");
    }

    // Initialization of Non-Volitile Storage
    nvs_init();

    // Init WiFI
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_event_group = xEventGroupCreate();
    start_dhcp_server();
    wifi_init();

    /* Wait for the callback to set the CONNECTED_BIT in the
       event group.
    */
    xEventGroupWaitBits(wifi_event_group, CLIENT_CONNECTED_BIT, false, true, portMAX_DELAY);

    ESP_LOGI(APP_MAIN_TAG, "Device connected, starting tasks!");

    xTaskCreate(&camera_task, "camera_task", 4096 * 2, NULL, 5, NULL);

    // xTaskCreate(&battery_percentage_transmit_task, "battery_percentage_transmit_task", 4096, NULL, 5, NULL);
    // xTaskCreate(&receive_control_task, "receive_control_task", 4096, NULL, 5, NULL);
    // xTaskCreate(&control_syringe_task, "control_syringe_task", 4096, NULL, 5, NULL);
    // xTaskCreate(&motor_task, "motor_task", 4096, NULL, 5, NULL);
    // xTaskCreate(&print_sta_info, "print_sta_info", 4096, NULL, 5, NULL);
}
