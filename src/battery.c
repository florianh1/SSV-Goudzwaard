#include <battery.h>

/* Port number of UDP-servers */
#define TRANSMIT_BATTERY_PERCENTAGE_UDP_PORT 3334

extern SemaphoreHandle_t xJoystickSemaphore;
extern SemaphoreHandle_t yJoystickSemaphore;
extern SemaphoreHandle_t scrollbarSemaphore;
extern SemaphoreHandle_t batteryPercentageSemaphore;

extern uint8_t joystick_y;
extern uint8_t joystick_x;
extern uint8_t scrollbar;
extern uint8_t battery_percentage;

/**
 * battery percentage transmit task
 *
 * @return void
 */
void battery_percentage_transmit_task(void *pvParameter)
{
    static const char *TASK_TAG = "battery_percentage_transmit_task";
    ESP_LOGI(TASK_TAG, "task started");

    char tx_buffer[20];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1)
    {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr("192.168.1.2"); //TODO: set correct address addres is most of time 192.168.1.2
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(TRANSMIT_BATTERY_PERCENTAGE_UDP_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TASK_TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TASK_TAG, "Socket created");

        while (1)
        {
            sprintf(tx_buffer, "%d", battery_percentage);
            int err = sendto(sock, &tx_buffer, strlen(tx_buffer), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
            if (err < 0)
            {
                ESP_LOGE(TASK_TAG, "Error occured during sending pattery percentage: errno %d", errno);
                break;
            }
            ESP_LOGI(TASK_TAG, "battery percentage sent");

            // transmit every 10 seconds
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }

        if (sock != -1)
        {
            ESP_LOGE(TASK_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}