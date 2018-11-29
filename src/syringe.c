#include <syringe.h>

extern SemaphoreHandle_t xJoystickSemaphore;
extern SemaphoreHandle_t yJoystickSemaphore;
extern SemaphoreHandle_t scrollbarSemaphore;
extern SemaphoreHandle_t batteryPercentageSemaphore;

extern uint8_t joystick_y;
extern uint8_t joystick_x;
extern uint8_t scrollbar;
extern uint8_t battery_percentage;

/**
 * Control syringe task
 * 
 * This task is responsible for controlling the syringe. The task is therefore responsible for diving the submarine.
 *
 * @return void
 */
void control_syringe_task(void* pvParameter)
{
    static const char* TASK_TAG = "control_syringe_task";
    ESP_LOGI(TASK_TAG, "task started");

    while (1) {
        if (scrollbarSemaphore != NULL) {
            if (xSemaphoreTake(scrollbarSemaphore, (TickType_t)10) == pdTRUE) {
                ESP_LOGI(TASK_TAG, "scrollbar: %d", scrollbar);
                xSemaphoreGive(scrollbarSemaphore);
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
