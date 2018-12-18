#include <camera.h>

#ifdef CAM_VGA
#define CAM_RES VGA
#define CAM_WIDTH 640
#define CAM_HEIGHT 480
#define CAM_DIV 12
#endif

#ifdef CAM_CIF
#define CAM_RES CIF
#define CAM_WIDTH 352
#define CAM_HEIGHT 288
#define CAM_DIV 4
#endif

#ifdef CAM_QVGA
#define CAM_RES QVGA
#define CAM_WIDTH 320
#define CAM_HEIGHT 240
#define CAM_DIV 3
#endif

#ifdef CAM_QCIF
#define CAM_RES QCIF
#define CAM_WIDTH 176
#define CAM_HEIGHT 144
#define CAM_DIV 1
#endif

#ifdef CAM_QQVGA
#define CAM_RES QQVGA
#define CAM_WIDTH 160
#define CAM_HEIGHT 120
#define CAM_DIV 1
#endif

#define PACKET_SIZE 1200 // For exactly 16 packets

//******************************************

camera_config_t cam_conf = {
    .D0 = CAMERA_D0,
    .D1 = CAMERA_D1,
    .D2 = CAMERA_D2,
    .D3 = CAMERA_D3,
    .D4 = CAMERA_D4,
    .D5 = CAMERA_D5,
    .D6 = CAMERA_D6,
    .D7 = CAMERA_D7,
    .XCLK = CAMERA_XCLK,
    .PCLK = CAMERA_PCLK,
    .VSYNC = CAMERA_VSYNC,
    .xclk_freq_hz = 20000000, // XCLK 10MHz
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0
};

//  SSCB_SDA(SIOD)  --> 21(ESP32)
//  SSCB_SCL(SIOC)  --> 22(ESP32)
//  RESET   --> 3.3V
//  PWDN    --> GND
//  HREF    --> NC

uint8_t* camData;
uint16_t data_size;
uint16_t line_size;
uint16_t line_h;

/**
 * @TODO:
 *
 * @param w
 * @param h
 * @uint8_t
 */
bool allocateMemory(uint16_t w, uint16_t h)
{

#ifdef CONVERT_RGB565_TO_RGB332
    line_h = h;
    line_size = w;
    data_size = 2 + line_size * h;
#else
    line_h = h;
    line_size = w * 2;
    data_size = 2 + line_size * h;
#endif // CONVERT_RGB565_TO_RGB332
    camData = (uint8_t*)malloc(data_size + 4);
    if (camData == NULL) {
        ESP_LOGI("allocateMemory", "******** Memory allocate Error! ***********");
        return false;
    }

    return true;
}

/**
 * @TODO:
 *
 * @param pvParameter
 */
void camera_task(void* pvParameter)
{
    static const char* TASK_TAG = "camera_task";
    ESP_LOGI(TASK_TAG, "task started");

    esp_err_t err = init_camera(&cam_conf, CAM_RES, CAM_COLOR);

    setPCLK(2, DBLV_CLK_x4);
    vflip(false);

    if (err != ESP_OK) {
        ESP_LOGE(TASK_TAG, "Camera init error");
    }

    ESP_LOGI(TASK_TAG, "cam MID = %X\n\r", getMID());
    ESP_LOGI(TASK_TAG, "cam PID = %X\n\r", getPID());

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    allocateMemory(CAM_WIDTH, (CAM_HEIGHT / CAM_DIV));

    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr("192.168.1.255");
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(5000);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TASK_TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TASK_TAG, "Socket created");

        int success_messages = 0;

        while (1) {

            uint16_t y, dy;
            dy = CAM_HEIGHT / CAM_DIV;

            for (y = 0; y < CAM_HEIGHT; y += dy) {

                getLines(y + 1, &camData[4], dy);

                uint8_t parts = (data_size % PACKET_SIZE == 0) ? (data_size / PACKET_SIZE) : (data_size / PACKET_SIZE) + 1;

                for (uint8_t part = 0; part <= parts; part++) {
                    // Start pos in camData, length of new packet
                    int startPos = (part == 0) ? (part * PACKET_SIZE) : (part * PACKET_SIZE) - 1;
                    int length = (part == parts) ? (data_size % PACKET_SIZE) : PACKET_SIZE;

                    // Header
                    camData[startPos] = part;
                    camData[startPos + 1] = parts;
                    camData[startPos + 2] = (int16_t)(PACKET_SIZE & 0xFF00) >> 8;
                    camData[startPos + 3] = (int16_t)PACKET_SIZE & 0xFF;

                    int err = sendto(sock, &camData[startPos], length + 4, 0, (struct sockaddr*)&destAddr, sizeof(destAddr));

                    if (err < 0) {
                        ESP_LOGE(TASK_TAG, "Error occured during sending video frame: errno %d size %d", errno, (data_size + 2));
                        success_messages = 0;
                        break;
                    } else {
                        success_messages++;

                        if (success_messages == (16 * 20)) {
                            ESP_LOGI(TASK_TAG, "320 messages send!");
                            success_messages = 0;
                        }
                    }
                }
            }

            // transmit 4 times per second
            vTaskDelay(250 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TASK_TAG, "Shutting down socket and restarting after 10 seconds...");
            shutdown(sock, 0);
            close(sock);
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}