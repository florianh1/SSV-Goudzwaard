#include <taskControl.h>

extern EventGroupHandle_t wifi_event_group;
extern const int CLIENT_CONNECTED_BIT;
extern uint8_t number_of_devices_connected;
extern uint8_t waterML;

extern TaskHandle_t camera_task_handler;
extern TaskHandle_t battery_percentage_transmit_task_handler;
extern TaskHandle_t receive_control_task_handler;
extern TaskHandle_t control_syringe_task_handler;
extern TaskHandle_t motor_task_handler;
extern TaskHandle_t blink_task_handler;

extern uint8_t joystick_y;
extern uint8_t joystick_x;
extern uint8_t scrollbar;
extern uint8_t battery_percentage;

void task_control(void* pvParameter)
{
    static const char* TASK_TAG = "task_control";
    ESP_LOGI(TASK_TAG, "Task control task started...");

    while (1) {
        ESP_LOGI(TASK_TAG, "Number of connected devices: %d", number_of_devices_connected);

        if ((!(number_of_devices_connected > 0)) || (battery_percentage < 5)) {
            // stop the tasks

            // if (blink_task_handler != NULL) {
            //     if (eTaskGetState(blink_task_handler) != eSuspended) {
            //         ESP_LOGI(TASK_TAG, "Suspending blink_task");
            //         vTaskSuspend(blink_task_handler);
            //     }
            // }

            if (camera_task_handler != NULL) {
                if (eTaskGetState(camera_task_handler) != eSuspended) {
                    ESP_LOGI(TASK_TAG, "Suspending camera task");
                    vTaskSuspend(camera_task_handler);
                }
            }

            if (battery_percentage_transmit_task_handler != NULL) {
                if (eTaskGetState(battery_percentage_transmit_task_handler) != eSuspended) {
                    ESP_LOGI(TASK_TAG, "Suspending battery transmit task");
                    vTaskSuspend(battery_percentage_transmit_task_handler);
                }
            }

            if (receive_control_task_handler != NULL) {
                if (eTaskGetState(receive_control_task_handler) != eSuspended) {
                    ESP_LOGI(TASK_TAG, "Suspending receive control task");
                    vTaskSuspend(receive_control_task_handler);
                }
            }

            if (motor_task_handler != NULL) {
                if (eTaskGetState(motor_task_handler) != eSuspended) {
                    ESP_LOGI(TASK_TAG, "Suspending motor control task");
                    vTaskSuspend(motor_task_handler);
                }
            }

            // move submarine to surface
            scrollbar = 0;
            if (waterML == 0) {
                if (control_syringe_task_handler != NULL) {
                    if (eTaskGetState(control_syringe_task_handler) != eSuspended) {
                        ESP_LOGI(TASK_TAG, "Suspending control syringe task");
                        vTaskSuspend(control_syringe_task_handler);
                    }
                }
            }

        } else {
            // continue the tasks

            // if (blink_task_handler != NULL) {
            //     if (eTaskGetState(blink_task_handler) == eSuspended) {
            //         ESP_LOGI(TASK_TAG, "Continuing blink_task");
            //         vTaskResume(blink_task_handler);
            //     }
            // }

            if (camera_task_handler != NULL) {
                if (eTaskGetState(camera_task_handler) == eSuspended) {
                    ESP_LOGI(TASK_TAG, "Continuing camera task");
                    vTaskResume(camera_task_handler);
                }
            }

            if (battery_percentage_transmit_task_handler != NULL) {
                if (eTaskGetState(battery_percentage_transmit_task_handler) == eSuspended) {
                    ESP_LOGI(TASK_TAG, "Continuing battery transmit task");
                    vTaskResume(battery_percentage_transmit_task_handler);
                }
            }

            if (receive_control_task_handler != NULL) {
                if (eTaskGetState(receive_control_task_handler) == eSuspended) {
                    ESP_LOGI(TASK_TAG, "Continuing receive control task");
                    vTaskResume(receive_control_task_handler);
                }
            }

            if (control_syringe_task_handler != NULL) {
                if (eTaskGetState(control_syringe_task_handler) == eSuspended) {
                    ESP_LOGI(TASK_TAG, "Continuing control syringe task");
                    vTaskResume(control_syringe_task_handler);
                }
            }

            if (motor_task_handler != NULL) {
                if (eTaskGetState(motor_task_handler) == eSuspended) {
                    ESP_LOGI(TASK_TAG, "Continuing motor control task");
                    vTaskResume(motor_task_handler);
                }
            }
        }

        // Delay 5 seconds
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}