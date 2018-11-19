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

#define ESP_WIFI_SSID       "SSV GOLDENSWORD"
#define ESP_WIFI_PASS       "Manderijn"          /* Minimal 8, maximal 64 characters */
#define AP_MAX_CONNECTIONS  4
#define PORT                3000
#define LISTEN_Q            2
#define MESSAGE             "Hello test TCP client"

const int CLIENT_CONNECTED_BIT = BIT0;
const int CLIENT_DISCONNECTED_BIT = BIT1;
const int AP_STARTED_BIT = BIT2;

static const char *TAG = "Wifi";
static EventGroupHandle_t wifi_event_group;


/**
 * TODO: make function discription
 *
 * @param  system_event_t *
 * @param  void *
 * @return esp_err_t
 */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
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
 * TODO: make function discription
 *
 * @return void
 */
void nvs_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("NVS init", "NVS setup finished...");
}

/**
 * TODO: make function discription
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
 * TODO: make function discription
 *
 * @return void
 */
void wifi_init()
{
    //esp_log_level_set("wifi", ESP_LOG_NONE);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .channel = 0,
            .password = ESP_WIFI_PASS,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = AP_MAX_CONNECTIONS,
        },
    };

    if (strlen(ESP_WIFI_SSID) == 0) {
        ESP_LOGW(TAG, "Password lenght is 0, changing authmode to OPEN...");
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi setup finished. SSID: %s pass: %s",
            wifi_config.ap.ssid, wifi_config.ap.password);
}

/**
 * TODO: make function discription
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
    tcpServerAddr.sin_port = htons(PORT);

    ESP_LOGI(TAG, "%u TCP server on: %u:%u with family: %u", tcpServerAddr.sin_len, tcpServerAddr.sin_addr.s_addr, tcpServerAddr.sin_port, tcpServerAddr.sin_family);

    int s, r, cs; // socket, recieve, client socket
    char recv_buf[64];
    static struct sockaddr_in remote_addr;
    static unsigned int socklen;
    socklen = sizeof(remote_addr);

    xEventGroupWaitBits(wifi_event_group, AP_STARTED_BIT, false, true, portMAX_DELAY);

    while(1) {
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            ESP_LOGE(TAG, "Failed to allocate socket");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Allocated socket");
        if (bind(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
            ESP_LOGE(TAG, "Socket bind failed, errno:%d", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Socket bind done");
        if (listen(s, LISTEN_Q) != 0) {
            ESP_LOGE(TAG, "Socket listen failed errno:%d", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        while (1) {
            ESP_LOGI(TAG, "Now accepting socket connections...");
            cs = accept(s, (struct sockaddr *)&remote_addr, &socklen);
            ESP_LOGI(TAG, "New connection request, request data:");
            fcntl(cs, F_SETFL, O_NONBLOCK);
            do {
                bzero(recv_buf, sizeof(recv_buf));
                r = recv(cs, recv_buf, sizeof(recv_buf) - 1, 0);
                for (int i = 0; i < r; i++) {
                    putchar(recv_buf[i]);
                }
            } while (r > 0);

            ESP_LOGI(TAG, "Done reading from socket. Last read return:%d, errno:%d", r, errno);
            
            if (write(cs, MESSAGE, strlen(MESSAGE)) < 0) {
                ESP_LOGE(TAG, "Send failed");
                close(s);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                continue;
            }

            ESP_LOGI(TAG, "Socket send success");
            close(cs);
        }

        ESP_LOGI(TAG, "Server will be opend in 5 seconds");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "tcp_client task closed");
}

/**
 * TODO: make function discription
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

    for(int i = 0; i < adapter_sta_list.num; i++) {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        printf("%d - mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x - IP: %s\n", i + 1,
                station.mac[0], station.mac[1], station.mac[2],
				station.mac[3], station.mac[4], station.mac[5],
                ip4addr_ntoa(&(station.ip)));
    }

    printf("\n");
}

/**
 * TODO: make function discription
 *
 * @param  void *
 * @return void
 */
void print_sta_info(void *pvParam)
{
    ESP_LOGI(TAG, "print_sta_info task started \n");
    while (1) {
        EventBits_t staBits = xEventGroupWaitBits(wifi_event_group, CLIENT_CONNECTED_BIT | CLIENT_DISCONNECTED_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        
        if ((staBits & CLIENT_CONNECTED_BIT) != 0) {
            ESP_LOGI(TAG, "New station connected");
        } else {
            ESP_LOGI(TAG, "A station disconnected");
        }

        printStationList();
    }
}


/**
 * TODO: make function discription
 *
 * @return void
 */
void app_main()
{
    nvs_init();

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_event_group = xEventGroupCreate();
    start_dhcp_server();
    wifi_init();

    xTaskCreate(&tcp_server, "tcp_server", 4096, NULL, 5, NULL);
    xTaskCreate(&print_sta_info, "print_sta_info", 4096, NULL, 5, NULL);

}
