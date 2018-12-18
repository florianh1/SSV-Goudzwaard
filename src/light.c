#include <light.h>

extern SemaphoreHandle_t lightStatusSemaphore;

extern uint8_t lightStatus;

/**
 * @light up a led to see if that is set in the application
 * 
 * @param pvParameter 
 */
void light_task(void* pvParameter)
{
    gpio_pad_select_gpio(LIGHT_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LIGHT_GPIO, GPIO_MODE_OUTPUT);
    static const char* TASK_TAG = "light_task";
    ESP_LOGI(TASK_TAG, "Light task started...");
    while (1) {
        ESP_LOGI(TASK_TAG, "Lightstatus: %d", lightStatus);
        gpio_set_level(LIGHT_GPIO, lightStatus);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}