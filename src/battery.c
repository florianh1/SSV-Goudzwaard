
#include <battery.h>

#define BATTERY_DEMOVALUE

extern SemaphoreHandle_t batteryPercentageSemaphore;

extern uint8_t battery_percentage;

uint8_t calc_average_battery_percentage();

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
 * Calculate the average battery percentage based on the 3 cells in the battery.
 *
 * @return uint8_t
 */
uint8_t calc_average_battery_percentage()
{
#ifdef BATTERY_DEMOVALUE
    return 94;
#else
    int cell_1_value = adc1_get_raw(BATTERY_CELL_1);
    int cell_2_value = adc1_get_raw(BATTERY_CELL_2);
    int cell_3_value = adc1_get_raw(BATTERY_CELL_3);
    int cell_value = 0;

    // The lowest cell value should be used because when a cell value
    // reaches 0 the battery could die. Therefore the battery percentage
    // should be calculated using the lowest cell value.
    if (cell_1_value > cell_2_value && cell_1_value > cell_3_value) {
        cell_value = cell_1_value;
    } else if (cell_2_value > cell_1_value && cell_2_value > cell_3_value) {
        cell_value = cell_2_value;
    } else if (cell_3_value > cell_1_value && cell_3_value > cell_2_value) {
        cell_value = cell_3_value;
    }

    // Since we read a 10 bit value from adc, the maximum value will be 1024.
    return (uint8_t)((cell_value / 1024) * 100);
#endif
}