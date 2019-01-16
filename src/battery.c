
#include <battery.h>
#include "esp_adc_cal.h"

//#define BATTERY_DEMOVALUE

extern SemaphoreHandle_t batteryPercentageSemaphore;

extern uint8_t battery_percentage;

uint8_t calc_average_battery_percentage();

// Will be either 1, 2 or 3.
uint8_t lowest_cell;

/**
 * @task thats takes care of the different battery functionalities in the SSV GoudZwaard
 * 
 * @param pvParameter 
 */
void battery_percentage_transmit_task(void* pvParameter)
{
    static const char* TASK_TAG = "battery_percentage_transmit_task";
    ESP_LOGI(TASK_TAG, "task started");

    char tx_buffer[20];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    adc_power_on();

    adc1_config_width(ADC_WIDTH_BIT_10);
    adc1_config_channel_atten(BATTERY_CELL_1, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(BATTERY_CELL_2, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(BATTERY_CELL_3, ADC_ATTEN_DB_11);

    while (1) {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr("192.168.1.255");
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(TRANSMIT_BATTERY_PERCENTAGE_UDP_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TASK_TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TASK_TAG, "Socket created");

        while (1) {
            battery_percentage = calc_average_battery_percentage();

            sprintf(tx_buffer, "%d", battery_percentage);
            int err = sendto(sock, &tx_buffer, strlen(tx_buffer), 0, (struct sockaddr*)&destAddr, sizeof(destAddr));
            if (err < 0) {
                ESP_LOGE(TASK_TAG, "Error occured during sending pattery percentage: errno %d", errno);
                break;
            }
            ESP_LOGI(TASK_TAG, "battery percentage sent: %d", battery_percentage);

            // transmit every 1 second
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TASK_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
    }
    vTaskDelete(NULL);
}

/**
 * The lowest cell value should be used because when a cell value
 * reaches 0 the battery could die. Therefore the battery percentage
 * should be calculated using the lowest cell value.
 *
 * @return int
 */
int lowest_cell_value()
{
    int cell_value = 0;

    int cell_1_value = adc1_get_raw(BATTERY_CELL_1);
    int cell_2_value = adc1_get_raw(BATTERY_CELL_2);
    int cell_3_value = adc1_get_raw(BATTERY_CELL_3);

    if (cell_1_value > cell_2_value && cell_1_value > cell_3_value) {
        cell_value = cell_1_value;

        lowest_cell = 1;
    } else if (cell_2_value > cell_1_value && cell_2_value > cell_3_value) {
        cell_value = cell_2_value;

        lowest_cell = 2;
    } else if (cell_3_value > cell_1_value && cell_3_value > cell_2_value) {
        cell_value = cell_3_value;

        lowest_cell = 3;
    }

    return cell_value;
}

/**
 * The voltage we read from the battery is after the current has gone through resistors.
 * In this method we convert the voltage from after the resistors to what the voltage
 * would have been before the resistors.
 *
 * @param  voltage_after_resistors
 * @return float
 */
float get_voltage_before_resistors(float voltage_after_resistors)
{
    float voltage_before_resistors = 0;

    if (lowest_cell == 1) {
        voltage_before_resistors = (float) (voltage_after_resistors / 10 * 13.3);
    } else if (lowest_cell == 2) {
        voltage_before_resistors = (float) (voltage_after_resistors / 10 * 28);
    } else {
        voltage_before_resistors = (float) (voltage_after_resistors / 10 * 40);
    }

    return voltage_before_resistors;
}

/**
 * Calculate the average battery percentage based on the 3 cells in the battery.
 *
 * @return uint8_t
 */
uint8_t calc_average_battery_percentage()
{
#ifdef BATTERY_DEMOVALUE
    return 94;
#else
    int cell_value = lowest_cell_value();
    uint8_t battery_percentage = 0;

    esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    uint32_t voltage_after_resistors = esp_adc_cal_raw_to_voltage((uint32_t) cell_value, adc_chars); // mV

    float voltage_before_resistors = get_voltage_before_resistors(voltage_after_resistors);

    if (lowest_cell == 1) {
        battery_percentage = (uint8_t) ((voltage_before_resistors / 4.2) * 100);
    } else if (lowest_cell == 2) {
        battery_percentage = (uint8_t) ((voltage_before_resistors / 8.4) * 100);
    } else {
        battery_percentage = (uint8_t) ((voltage_before_resistors / 12.6) * 100);
    }

    return battery_percentage;
#endif
}