#include <controls.h>

/**
 * @macro for printing the binary value of the payload
 * 
 */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)         \
    (byte & 0x1000 ? '1' : '0'),     \
        (byte & 0x0800 ? '1' : '0'), \
        (byte & 0x0200 ? '1' : '0'), \
        (byte & 0x0100 ? '1' : '0'), \
        (byte & 0x0400 ? '1' : '0'), \
        (byte & 0x0080 ? '1' : '0'), \
        (byte & 0x0040 ? '1' : '0'), \
        (byte & 0x0020 ? '1' : '0'), \
        (byte & 0x0010 ? '1' : '0'), \
        (byte & 0x0008 ? '1' : '0'), \
        (byte & 0x0004 ? '1' : '0'), \
        (byte & 0x0002 ? '1' : '0'), \
        (byte & 0x0001 ? '1' : '0')

extern SemaphoreHandle_t xJoystickSemaphore;
extern SemaphoreHandle_t yJoystickSemaphore;
extern SemaphoreHandle_t scrollbarSemaphore;
extern SemaphoreHandle_t batteryPercentageSemaphore;
extern SemaphoreHandle_t lightStatusSemaphore;

extern uint8_t joystick_y;
extern uint8_t joystick_x;
extern uint8_t scrollbar;
extern uint8_t battery_percentage;
extern uint8_t lightStatus;

/**
 * @TODO:
 * 
 * @param pvParameter 
 */
void receive_control_task(void* pvParameter)
{
    static const char* TASK_TAG = "receive_control_task";
    ESP_LOGI(TASK_TAG, "task started");

    char rx_buffer[20];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    int sock;
    int err;
    int len;
    struct sockaddr_in destAddr;
    struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
    socklen_t socklen;

    int payload;

    while (1) {

        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(RECEIVE_CONTROL_UDP_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TASK_TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TASK_TAG, "Socket created");

        err = bind(sock, (struct sockaddr*)&destAddr, sizeof(destAddr));
        if (err < 0) {
            ESP_LOGE(TASK_TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TASK_TAG, "Socket binded");

        while (1) {
            // ESP_LOGI(TASK_TAG, "receive_control_task waiting for data");

            socklen = sizeof(sourceAddr);
            len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr*)&sourceAddr, &socklen);

            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TASK_TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (sourceAddr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in*)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (sourceAddr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...

                //ESP_LOGI(TASK_TAG, "Received %d bytes from %s:", len, addr_str);
                //ESP_LOGI(TASK_TAG, "%s", rx_buffer);

                // extract rx_buffer to payload
                sscanf(rx_buffer, "%d", &payload);

                ESP_LOGI(TASK_TAG, "payload =" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(payload));
                // extract payload to values

                if (scrollbarSemaphore != NULL) {
                    if (xSemaphoreTake(scrollbarSemaphore, (TickType_t)1) == pdTRUE) {
                        scrollbar = payload & 0x3F;
                        xSemaphoreGive(scrollbarSemaphore);
                    }
                }
                if (yJoystickSemaphore != NULL) {
                    if (xSemaphoreTake(yJoystickSemaphore, (TickType_t)1) == pdTRUE) {
                        joystick_y = (payload >> 6) & 0x7;
                        xSemaphoreGive(yJoystickSemaphore);
                    }
                }
                if (xJoystickSemaphore != NULL) {
                    if (xSemaphoreTake(xJoystickSemaphore, (TickType_t)1) == pdTRUE) {
                        joystick_x = (payload >> 9) & 0x7;
                        xSemaphoreGive(xJoystickSemaphore);
                    }
                }
                if (lightStatusSemaphore != NULL) {
                    if (xSemaphoreTake(lightStatusSemaphore, (TickType_t)1) == pdTRUE) {
                        xSemaphoreGive(lightStatusSemaphore);
                        lightStatus = (payload >> 12) & 0x1;
                    }
                }

                // ESP_LOGI(TASK_TAG, "IP: %s", addr_str);
                int err = sendto(sock, "OK", sizeof("OK"), 0, (struct sockaddr*)&sourceAddr, sizeof(sourceAddr));
                if (err < 0) {
                    ESP_LOGE(TASK_TAG, "Error occured during sending: errno %d", errno);
                    break;
                }
            }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TASK_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    // vTaskDelete(NULL);
    vTaskDelay(100 / portTICK_PERIOD_MS);
}