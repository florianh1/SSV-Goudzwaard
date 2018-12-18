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

TaskHandle_t camera_task_handler = NULL;
TaskHandle_t battery_percentage_transmit_task_handler = NULL;
TaskHandle_t receive_control_task_handler = NULL;
TaskHandle_t control_syringe_task_handler = NULL;
TaskHandle_t motor_task_handler = NULL;
TaskHandle_t blink_task_handler = NULL;

extern EventGroupHandle_t wifi_event_group;
extern const int CLIENT_CONNECTED_BIT;
extern uint8_t number_of_devices_connected;
extern uint8_t waterML;

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

    xTaskCreate(&print_sta_info, "print_sta_info", 2048, NULL, 10, NULL);

    /* Wait for the callback to set the CONNECTED_BIT in the
       event group.
    */

    xEventGroupWaitBits(wifi_event_group, CLIENT_CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(APP_MAIN_TAG, "Device connected, starting tasks!");

    // start the tasks
    xTaskCreate(&camera_task, "camera_task", 4096 * 2, NULL, 5, &camera_task_handler);
    xTaskCreate(&battery_percentage_transmit_task, "battery_percentage_transmit_task", 2048, NULL, 5, &battery_percentage_transmit_task_handler);
    xTaskCreate(&receive_control_task, "receive_control_task", 2048, NULL, 5, &receive_control_task_handler);
    xTaskCreate(&control_syringe_task, "control_syringe_task", 4096, NULL, 5, &control_syringe_task_handler);
    xTaskCreate(&motor_task, "motor_task", 2048, NULL, 5, &motor_task_handler);

    while (1) {
        ESP_LOGI(APP_MAIN_TAG, "Number of connected devices: %d", number_of_devices_connected);

        if ((!(number_of_devices_connected > 0)) || (battery_percentage < 5)) {
            // stop the tasks

            if (blink_task_handler != NULL) {
                if (eTaskGetState(blink_task_handler) != eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Suspending blink_task");
                    vTaskSuspend(blink_task_handler);
                }
            }

            if (camera_task_handler != NULL) {
                if (eTaskGetState(camera_task_handler) != eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Suspending camera task");
                    vTaskSuspend(camera_task_handler);
                }
            }

            if (battery_percentage_transmit_task_handler != NULL) {
                if (eTaskGetState(battery_percentage_transmit_task_handler) != eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Suspending battery transmit task");
                    vTaskSuspend(battery_percentage_transmit_task_handler);
                }
            }

            if (receive_control_task_handler != NULL) {
                if (eTaskGetState(receive_control_task_handler) != eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Suspending receive control task");
                    vTaskSuspend(receive_control_task_handler);
                }
            }

            if (motor_task_handler != NULL) {
                if (eTaskGetState(motor_task_handler) != eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Suspending motor control task");
                    vTaskSuspend(motor_task_handler);
                }
            }

            // move submarine to surface
            scrollbar = 0;
            if (waterML == 0) {
                if (control_syringe_task_handler != NULL) {
                    if (eTaskGetState(control_syringe_task_handler) != eSuspended) {
                        ESP_LOGI(APP_MAIN_TAG, "Suspending control syringe task");
                        vTaskSuspend(control_syringe_task_handler);
                    }
                }
            }

        } else {
            // continue the tasks

            if (blink_task_handler != NULL) {
                if (eTaskGetState(blink_task_handler) == eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Continuing blink_task");
                    vTaskResume(blink_task_handler);
                }
            }

            if (camera_task_handler != NULL) {
                if (eTaskGetState(camera_task_handler) == eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Continuing camera task");
                    vTaskResume(camera_task_handler);
                }
            }

            if (battery_percentage_transmit_task_handler != NULL) {
                if (eTaskGetState(battery_percentage_transmit_task_handler) == eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Continuing battery transmit task");
                    vTaskResume(battery_percentage_transmit_task_handler);
                }
            }

            if (receive_control_task_handler != NULL) {
                if (eTaskGetState(receive_control_task_handler) == eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Continuing receive control task");
                    vTaskResume(receive_control_task_handler);
                }
            }

            if (control_syringe_task_handler != NULL) {
                if (eTaskGetState(control_syringe_task_handler) == eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Continuing control syringe task");
                    vTaskResume(control_syringe_task_handler);
                }
            }

            if (motor_task_handler != NULL) {
                if (eTaskGetState(motor_task_handler) == eSuspended) {
                    ESP_LOGI(APP_MAIN_TAG, "Continuing motor control task");
                    vTaskResume(motor_task_handler);
                }
            }
        }

        // Delay 1 seconds
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
