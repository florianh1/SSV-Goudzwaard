#include <wifi.h>

const int CLIENT_CONNECTED_BIT = BIT0;
const int CLIENT_DISCONNECTED_BIT = BIT1;
const int AP_STARTED_BIT = BIT2;

static const char* TAG = "Wifi";

uint8_t number_of_devices_connected = 0;

/**
 * @Handle events triggerd by the ESPs RTOS system. Called automatically on event
 * 
 * @param ctx 
 * @param event 
 * @return esp_err_t 
 */
esp_err_t event_handler(void* ctx, system_event_t* event)
{
    switch (event->event_id) {
    case SYSTEM_EVENT_AP_START:
        printf("Event: ESP32 is started in AP mode\n");
        xEventGroupSetBits(wifi_event_group, AP_STARTED_BIT);
        break;

    case SYSTEM_EVENT_AP_STACONNECTED:
        printf("Event: station connected to AP\n");
        number_of_devices_connected++;
        xEventGroupSetBits(wifi_event_group, CLIENT_CONNECTED_BIT);
        break;

    case SYSTEM_EVENT_AP_STADISCONNECTED:
        printf("Event: station disconnected from AP\n");
        xEventGroupSetBits(wifi_event_group, CLIENT_DISCONNECTED_BIT);
        number_of_devices_connected--;
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
 * @Initialize a Wi-Fi Access Point
 * @Parameters can be changed in the ESP_WIFI_ and AP_ defines
 * 
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

    if (strlen(ESP_WIFI_PASS) < 8) {
        if (strlen(ESP_WIFI_PASS) == 0) {
            ESP_LOGW(TAG, "Password length is 0, changing authmode to OPEN...");
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        } else {
            ESP_LOGE(TAG, "Password length can range from 8 to 64, set length: %u", strlen(ESP_WIFI_PASS));
            ESP_LOGW(TAG, "Changing authmode to OPEN...");
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }
    }

    if (wifi_config.ap.max_connection > 4) {
        ESP_LOGE(TAG, "Max connections can not be exceed 4, set to 4");
        wifi_config.ap.max_connection = 4;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi setup finished. SSID: %s pass: %s",
        wifi_config.ap.ssid, wifi_config.ap.password);
}

/**
 * @Print a list of stations connected to the Access Point
 * 
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

    for (int i = 0; i < adapter_sta_list.num; i++) {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        printf("%d - mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x - IP: %s\n", i + 1,
            station.mac[0], station.mac[1], station.mac[2],
            station.mac[3], station.mac[4], station.mac[5],
            ip4addr_ntoa(&(station.ip)));
    }

    printf("\n");
}

/**
 * @Initialization and start of DHCP server on 192.168.1.1
 * 
 */
void start_dhcp_server()
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
 * @Print connection information of a station 
 * 
 * @param pvParam 
 */
void print_sta_info(void* pvParam)
{
    static const char* TASK_TAG = "print_sta_info";
    ESP_LOGI(TASK_TAG, "task started \n");

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
