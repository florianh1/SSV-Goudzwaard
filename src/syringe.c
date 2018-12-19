#include <syringe.h>

#include "driver/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern SemaphoreHandle_t scrollbarSemaphore;

extern uint8_t scrollbar;

#define GPIO_ON 1
#define GPIO_OFF 0
#define PULL 0
#define PUSH 1
#define STEPPER_PINS ((1ULL << STEPPIN) | (1ULL << DIRPIN))
#define MILLISECONDS(ms) (ms / portTICK_PERIOD_MS)
#define STEPPER_DELAY MILLISECONDS(10) // 10ms

uint8_t waterML = 0;

/**
 * @initializes the pins of the syringes
 * 
 */
void init()
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = STEPPER_PINS;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void turnStepper(int dir)
{
    gpio_set_level(DIRPIN, dir);
    for (int x = 0; x < 10; x++) {
        gpio_set_level(STEPPIN, GPIO_ON);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(STEPPIN, GPIO_OFF);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/**
 * @pushes water out of the syringes untill they are empty
 * 
 */
void emptyTank()
{
    static const char* TASK_TAG = "emptyTank";
    ESP_LOGI(TASK_TAG, "emptying the balasttank...");
    while (waterML > 1) {
        ESP_LOGW(TASK_TAG, "Emptying tank: Tank volume: %d", waterML);
        turnStepper(PUSH);
        waterML--;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/**
 * @This task is responsible for controlling the syringe. The task is therefore responsible for diving the submarine.
 * 
 * @param pvParameter 
 */
void control_syringe_task(void* pvParameter)
{
    static const char* TASK_TAG = "control_syringe_task";
    ESP_LOGI(TASK_TAG, "Syringe task started...");
    ESP_LOGI(TASK_TAG, "Initializing the scrollbar pins...");
    init();
    while (1) {
        if (scrollbarSemaphore != NULL) {
            if (scrollbar < waterML) {
                turnStepper(PUSH);
                waterML--;
            } else if (scrollbar > waterML) {
                turnStepper(PULL);
                waterML++;
            } else {
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
            ESP_LOGI(TASK_TAG, "scrollbar = %d || waterML = %d", scrollbar, waterML);
        }
    }
}
