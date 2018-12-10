#include <syringe.h>

#include "driver/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern SemaphoreHandle_t scrollbarSemaphore;

extern uint8_t scrollbar;

#define GPIO_ON 1
#define GPIO_OFF 0
#define STEPPER_PINS ((1ULL << SYRINGE_IN1) | (1ULL << SYRINGE_IN2) | (1ULL << SYRINGE_IN3) | (1ULL << SYRINGE_IN4))
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

/**
 * @sets the syringe pins HIGH or LOW
 * 
 * @param state1 
 * @param state2 
 * @param state3 
 * @param state4 
 */
void pinSet(int state1, int state2, int state3, int state4)
{
    gpio_set_level(SYRINGE_IN1, state1);
    gpio_set_level(SYRINGE_IN2, state2);
    gpio_set_level(SYRINGE_IN3, state3);
    gpio_set_level(SYRINGE_IN4, state4);
}

/**
 * @pushes approximately 1mL of water out of each syringe attached
 * 
 */
void pushML()
{
    for (int i = 0; i < 56; i++) {
        pinSet(GPIO_ON, GPIO_OFF, GPIO_OFF, GPIO_OFF);
        ets_delay_us(1000);
        pinSet(GPIO_ON, GPIO_ON, GPIO_OFF, GPIO_OFF);
        ets_delay_us(1000);

        pinSet(GPIO_OFF, GPIO_ON, GPIO_OFF, GPIO_OFF);
        ets_delay_us(1000);
        pinSet(GPIO_OFF, GPIO_ON, GPIO_ON, GPIO_OFF);
        ets_delay_us(1000);

        pinSet(GPIO_OFF, GPIO_OFF, GPIO_ON, GPIO_OFF);
        ets_delay_us(1000);
        pinSet(GPIO_OFF, GPIO_OFF, GPIO_ON, GPIO_ON);
        ets_delay_us(1000);

        pinSet(GPIO_OFF, GPIO_OFF, GPIO_OFF, GPIO_ON);
        ets_delay_us(1000);
        pinSet(GPIO_ON, GPIO_OFF, GPIO_OFF, GPIO_ON);
        ets_delay_us(1000);
    }
}

/**
 * @pulls approximately 1mL of water out of each syringe attached
 * 
 */
void pullML()
{
    for (int i = 0; i < 56; i++) {
        pinSet(GPIO_ON, GPIO_OFF, GPIO_OFF, GPIO_ON);
        ets_delay_us(1000);
        pinSet(GPIO_OFF, GPIO_OFF, GPIO_OFF, GPIO_ON);
        ets_delay_us(1000);

        pinSet(GPIO_OFF, GPIO_OFF, GPIO_ON, GPIO_ON);
        ets_delay_us(1000);
        pinSet(GPIO_OFF, GPIO_OFF, GPIO_ON, GPIO_OFF);
        ets_delay_us(1000);

        pinSet(GPIO_OFF, GPIO_ON, GPIO_ON, GPIO_OFF);
        ets_delay_us(1000);
        pinSet(GPIO_OFF, GPIO_ON, GPIO_OFF, GPIO_OFF);
        ets_delay_us(1000);

        pinSet(GPIO_ON, GPIO_ON, GPIO_OFF, GPIO_OFF);
        ets_delay_us(1000);
        pinSet(GPIO_ON, GPIO_OFF, GPIO_OFF, GPIO_OFF);
        ets_delay_us(1000);
    }
}

/**
 * @sets all output pins of the syringes LOW
 * 
 */
void syringeStop()
{
    pinSet(GPIO_OFF, GPIO_OFF, GPIO_OFF, GPIO_OFF);
}

/**
 * @pushes water out of the syringes untill they are empty
 * 
 */
void emptyTank()
{
    static const char* TASK_TAG = "emptyTank";
    ESP_LOGI(TASK_TAG, "emptying the balasttank...");
    while (waterML > 0) {
        ESP_LOGI(TASK_TAG, "Emptying tank: Tank volume: %d", waterML);
        pushML();
        waterML--;
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
        // if (scrollbarSemaphore != NULL) {
        if (scrollbar < waterML) {
            pushML();
            waterML--;
        } else if (scrollbar > waterML) {
            pullML();
            waterML++;
        } else {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            syringeStop();
        }
        // ESP_LOGI(TASK_TAG, "scrollbar = %d || waterML = %d", scrollbar, waterML);
    }
}
