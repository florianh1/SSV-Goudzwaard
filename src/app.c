#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "../include/motor.h"
// #include "motor.c"


/* Bling GPIO pin */
#define BLINK_GPIO 2

/* Wi-Fi SSID, note a maximum character length of 32 */
#define ESP_WIFI_SSID "SSV GoudZwaard"

/* Wi-Fi password, note a minimal length of 8 and a maximum length of 64 characters */
#define ESP_WIFI_PASS "12345678"

/* Wi-Fi channel, note a range from 0 to 13 */
#define ESP_WIFI_CHANNEL 0

/* Amount of Access Point connections, note a maximum amount of 4 */
#define AP_MAX_CONNECTIONS 4

/* Port number of TCP-server */
#define TCP_PORT 3000

/* Maximum queue length of pending connections */
#define LISTEN_QUEUE 2

/* Size of the recvieve buffer (amount of charcters) */
#define RECV_BUF_SIZE 64

/* Port number of UDP-servers */
#define RECEIVE_CONTROL_UDP_PORT 3333
#define TRANSMIT_BATTERY_PERCENTAGE_UDP_PORT 3334

/* Pre-defined delay times, in seconds */
#define TASK_DELAY_1 1000
#define TASK_DELAY_5 5000

const int CLIENT_CONNECTED_BIT = BIT0;
const int CLIENT_DISCONNECTED_BIT = BIT1;
const int AP_STARTED_BIT = BIT2;

static const char *TAG = "Wifi";
static EventGroupHandle_t wifi_event_group;

SemaphoreHandle_t xJoystickSemaphore = NULL;
SemaphoreHandle_t yJoystickSemaphore = NULL;
SemaphoreHandle_t scrollbarSemaphore = NULL;
SemaphoreHandle_t batteryPercentageSemaphore = NULL;

uint8_t joystick_y = 4;
uint8_t joystick_x = 4;
uint8_t scrollbar = 0;
uint8_t battery_percentage = 94;

/**
 * Handle events triggerd by the ESPs RTOS system. Called automatically on event
 *
 * @param  system_event_t *event
 * @param  void *ctx
 * 
 * @return esp_err_t ESP_OK to be used in ESP_ERROR_CHECK
 */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_AP_START:
        printf("Event: ESP32 is started in AP mode\n");
        xEventGroupSetBits(wifi_event_group, AP_STARTED_BIT);
        break;

    case SYSTEM_EVENT_AP_STACONNECTED:
        printf("Event: station connected to AP\n");
        xEventGroupSetBits(wifi_event_group, CLIENT_CONNECTED_BIT);
        break;

    case SYSTEM_EVENT_AP_STADISCONNECTED:
        printf("Event: station disconnected from AP\n");
        xEventGroupSetBits(wifi_event_group, CLIENT_DISCONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        printf("Event: probe request recieved\n");
        break;
    default:
        break;
    }
    return ESP_OK;
}

/**
 * Initialization of Non-Volitile Storage
 * 
 * @return void
 */
void nvs_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("NVS init", "NVS setup finished...");
}

/**
 * Initialization and start of DHCP server on 192.168.1.1
 *
 * @return void
 */
static void start_dhcp_server()
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));

    tcpip_adapter_ip_info_t info;
    memset(&info, 0, sizeof(info));
    IP4_ADDR(&info.ip, 192, 168, 1, 1);
    IP4_ADDR(&info.gw, 192, 168, 1, 1);
    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));

    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
    ESP_LOGI(TAG, "DHCP server started");
}

/**
 * Initialize a Wi-Fi Access Point
 * Parameters can be changed in the ESP_WIFI_ and AP_ defines
 *
 * @return void
 */
void wifi_init()
{
    //esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi debug log
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .password = ESP_WIFI_PASS,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = AP_MAX_CONNECTIONS,
        },
    };

    if (strlen(ESP_WIFI_SSID) == 0)
    {
        ESP_LOGW(TAG, "Password lenght is 0, changing authmode to OPEN...");
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi setup finished. SSID: %s pass: %s",
             wifi_config.ap.ssid, wifi_config.ap.password);
}

/**
 * Start TCP-server
 * PORT can be changed in the TCP_ define
 *
 * @param  void *
 * @return void
 */
void tcp_server(void *pvParam)
{
    ESP_LOGI(TAG, "tcp_server task started");

    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons(TCP_PORT);

    ESP_LOGI(TAG, "%u TCP server on: %u:%u with family: %u", tcpServerAddr.sin_len, tcpServerAddr.sin_addr.s_addr, tcpServerAddr.sin_port, tcpServerAddr.sin_family);

    int s, r, cs; // socket, recieve, client socket
    char recv_buf[64];
    static struct sockaddr_in remote_addr;
    static unsigned int socklen;
    socklen = sizeof(remote_addr);

    xEventGroupWaitBits(wifi_event_group, AP_STARTED_BIT, false, true, portMAX_DELAY);

    while (1)
    {
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0)
        {
            ESP_LOGE(TAG, "Failed to allocate socket");
            vTaskDelay(TASK_DELAY_1 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Allocated socket");
        if (bind(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0)
        {
            ESP_LOGE(TAG, "Socket bind failed, errno:%d", errno);
            close(s);
            vTaskDelay(TASK_DELAY_1 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Socket bind done");
        if (listen(s, LISTEN_QUEUE) != 0)
        {
            ESP_LOGE(TAG, "Socket listen failed errno:%d", errno);
            close(s);
            vTaskDelay(TASK_DELAY_1 / portTICK_PERIOD_MS);
            continue;
        }

        while (1)
        {
            ESP_LOGI(TAG, "Now accepting socket connections...");
            cs = accept(s, (struct sockaddr *)&remote_addr, &socklen);
            ESP_LOGI(TAG, "New connection request, request data:");
            fcntl(cs, F_SETFL, O_NONBLOCK);
            bzero(recv_buf, sizeof(recv_buf));
            do
            {
                r = recv(cs, recv_buf, sizeof(recv_buf) - 1, 0);
                for (int i = 0; i < r; i++)
                {
                    putchar(recv_buf[i]);
                }
            } while (r > 0);
            printf("\n");

            ESP_LOGI(TAG, "Done reading from socket. Last read return:%d, errno:%d", r, errno);

            if (write(cs, recv_buf, sizeof(recv_buf)) < 0)
            { // TODO: dont send whole recv_buf, only filled part
                ESP_LOGE(TAG, "Send failed");
                close(s);
                vTaskDelay(TASK_DELAY_1 / portTICK_PERIOD_MS);
                continue;
            }

            ESP_LOGI(TAG, "Socket send success");
            close(cs);
        }

        ESP_LOGI(TAG, "Server will be opend in 5 seconds");
        vTaskDelay(TASK_DELAY_5 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "tcp_client task closed");
}

/**
 * Print a list of stations connected to the Access Point
 *
 * @return void
 */
void printStationList()
{
    printf("\nConnected stations:\n");
    printf("--------------------------------------------------\n");

    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;

    memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
    memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

    ESP_ERROR_CHECK(esp_wifi_ap_get_sta_list(&wifi_sta_list));
    ESP_ERROR_CHECK(tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list));

    for (int i = 0; i < adapter_sta_list.num; i++)
    {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        printf("%d - mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x - IP: %s\n", i + 1,
               station.mac[0], station.mac[1], station.mac[2],
               station.mac[3], station.mac[4], station.mac[5],
               ip4addr_ntoa(&(station.ip)));
    }

    printf("\n");
}

/**
 * Print connection information of a station 
 *
 * @param  void *
 * @return void
 */
void print_sta_info(void *pvParam)
{
    ESP_LOGI(TAG, "print_sta_info task started \n");
    while (1)
    {
        EventBits_t staBits = xEventGroupWaitBits(wifi_event_group, CLIENT_CONNECTED_BIT | CLIENT_DISCONNECTED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);

        if ((staBits & CLIENT_CONNECTED_BIT) != 0)
        {
            ESP_LOGI(TAG, "New station connected");
        }
        else
        {
            ESP_LOGI(TAG, "A station disconnected");
        }

        printStationList();
    }
}

/**
 * Blink task
 *
 * @return void
 */
void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while (1)
    {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * Control syringe task
 * 
 * This task is responsible for controlling the syringe. The task is therefore responsible for diving the submarine.
 *
 * @return void
 */
void control_syringe_task(void *pvParameter)
{
    static const char *TASK_TAG = "control_syringe_task";
    ESP_LOGI(TASK_TAG, "task started");

    while (1)
    {
        if (scrollbarSemaphore != NULL)
        {
            if (xSemaphoreTake(scrollbarSemaphore, (TickType_t)10) == pdTRUE)
            {
                ESP_LOGI(TASK_TAG, "scrollbar: %d", scrollbar);
                xSemaphoreGive(scrollbarSemaphore);
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * Control engines task
 * 
 * This task is responsible for controlling the engines. This task determines which way the submarine will move and how fast.
 *
 * @return void
 */
void control_engines_task(void *pvParameter)
{
    static const char *TASK_TAG = "control_engines_task";
    ESP_LOGI(TASK_TAG, "task started");

    while (1)
    {
        if (yJoystickSemaphore != NULL)
        {
            if (xSemaphoreTake(yJoystickSemaphore, (TickType_t)10) == pdTRUE)
            {
                ESP_LOGI(TASK_TAG, "joystick_y: %d", joystick_y);
                xSemaphoreGive(yJoystickSemaphore);
            }
        }
        if (xJoystickSemaphore != NULL)
        {
            if (xSemaphoreTake(xJoystickSemaphore, (TickType_t)10) == pdTRUE)
            {
                ESP_LOGI(TASK_TAG, "joystick_x: %d", joystick_x);
                xSemaphoreGive(xJoystickSemaphore);
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * receive control task
 *
 * @return void
 */
void receive_control_task(void *pvParameter)
{
    static const char *TASK_TAG = "receive_control_task";
    ESP_LOGI(TASK_TAG, "task started");

    char rx_buffer[20];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    int payload;

    while (1)
    {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(RECEIVE_CONTROL_UDP_PORT);
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

        int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err < 0)
        {
            ESP_LOGE(TASK_TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TASK_TAG, "Socket binded");

        while (1)
        {
            //ESP_LOGI(TASK_TAG, "receive_control_task waiting for data");

            struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);

            // Error occured during receiving
            if (len < 0)
            {
                ESP_LOGE(TASK_TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else
            {
                // Get the sender's ip address as string
                if (sourceAddr.sin6_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                }
                else if (sourceAddr.sin6_family == PF_INET6)
                {
                    inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...

                //ESP_LOGI(TASK_TAG, "Received %d bytes from %s:", len, addr_str);
                //ESP_LOGI(TASK_TAG, "%s", rx_buffer);

                // extract rx_buffer to payload
                sscanf(rx_buffer, "%d", &payload);

                // extract payload to values

                if (scrollbarSemaphore != NULL)
                {
                    if (xSemaphoreTake(scrollbarSemaphore, (TickType_t)1) == pdTRUE)
                    {
                        scrollbar = payload & 0x3F;
                        xSemaphoreGive(scrollbarSemaphore);
                    }
                }
                if (yJoystickSemaphore != NULL)
                {
                    if (xSemaphoreTake(yJoystickSemaphore, (TickType_t)1) == pdTRUE)
                    {
                        joystick_y = (payload >> 6) & 0x7;
                        xSemaphoreGive(yJoystickSemaphore);
                    }
                }
                if (xJoystickSemaphore != NULL)
                {
                    if (xSemaphoreTake(xJoystickSemaphore, (TickType_t)1) == pdTRUE)
                    {
                        joystick_x = (payload >> 9) & 0x7;
                        xSemaphoreGive(xJoystickSemaphore);
                    }
                }

                ESP_LOGI(TASK_TAG, "IP: %s", addr_str);
                int err = sendto(sock, "OK", sizeof("OK"), 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                if (err < 0)
                {
                    ESP_LOGE(TASK_TAG, "Error occured during sending: errno %d", errno);
                    break;
                }
            }
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

/**
 * Main loop
 *
 * @return void
 */
void app_main()
{
    xTaskCreate(motor_task, "motor_task", 4096, NULL, 5, NULL);

    while(1);

    static const char *APP_MAIN_TAG = "app_main";

    xJoystickSemaphore = xSemaphoreCreateMutex();
    yJoystickSemaphore = xSemaphoreCreateMutex();
    scrollbarSemaphore = xSemaphoreCreateMutex();
    batteryPercentageSemaphore = xSemaphoreCreateMutex();

    if (xJoystickSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the xJoystickSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "xJoystickSemaphore created");
    }

    if (yJoystickSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the yJoystickSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "yJoystickSemaphore created");
    }

    if (scrollbarSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the scrollbarSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "scrollbarSemaphore created");
    }

    if (batteryPercentageSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to be created successfully. */
    }
    else
    {
        /* The semaphore can now be used. Its handle is stored in the batteryPercentageSemaphore variable.  Calling xSemaphoreTake() on the semaphore here will fail until the semaphore has first been given. */
        ESP_LOGI(APP_MAIN_TAG, "batteryPercentageSemaphore created");
    }

    // Start blink task for testing
    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

    // Initialization of Non-Volitile Storage
    nvs_init();

    // Init WiFI
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_event_group = xEventGroupCreate();
    start_dhcp_server();
    wifi_init();

    xTaskCreate(&battery_percentage_transmit_task, "battery_percentage_transmit_task", 4096, NULL, 5, NULL);
    xTaskCreate(&receive_control_task, "receive_control_task", 4096, NULL, 5, NULL);
    xTaskCreate(&control_syringe_task, "control_syringe_task", 4096, NULL, 5, NULL);
    xTaskCreate(&control_engines_task, "control_engines_task", 4096, NULL, 5, NULL);

    //xTaskCreate(&tcp_server, "tcp_server", 4096, NULL, 5, NULL);
    //xTaskCreate(&print_sta_info, "print_sta_info", 4096, NULL, 5, NULL);
}
