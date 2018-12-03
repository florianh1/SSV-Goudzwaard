#include <camera.h>

//********* カメラ解像度指定 ***************
//#define CAM_RES     VGA     // カメラ解像度
//#define CAM_WIDTH   640     // カメラ幅
//#define CAM_HEIGHT  480     // カメラ高さ
//#define CAM_DIV      12     // １画面分割数

//#define CAM_RES     CIF     // カメラ解像度
//#define CAM_WIDTH   352     // カメラ幅
//#define CAM_HEIGHT  288     // カメラ高さ
//#define CAM_DIV       4     // １画面分割数

//#define CAM_RES     QVGA    // カメラ解像度
//#define CAM_WIDTH   320     // カメラ幅
//#define CAM_HEIGHT  240     // カメラ高さ
//#define CAM_DIV       3     // １画面分割数

#define CAM_RES QCIF // カメラ解像度
#define CAM_WIDTH 176 // カメラ幅
#define CAM_HEIGHT 144 // カメラ高さ
#define CAM_DIV 1 // １画面分割数

//#define CAM_RES     QQVGA   // カメラ解像度
//#define CAM_WIDTH   160     // カメラ幅
//#define CAM_HEIGHT  120     // カメラ高さ
//#define CAM_DIV       1     // １画面分割数

//******************************************

camera_config_t cam_conf = {
    .D0 = 36,
    .D1 = 39,
    .D2 = 34,
    .D3 = 35,
    .D4 = 32,
    .D5 = 33,
    .D6 = 25,
    .D7 = 26,
    .XCLK = 27, // 27 にすると何故かwebsocket通信時に動かなくなる。
    .PCLK = 14,
    .VSYNC = 13,
    .xclk_freq_hz = 20000000, // XCLK 10MHz
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0
};

//  SSCB_SDA(SIOD)  --> 21(ESP32)
//  SSCB_SCL(SIOC)  --> 22(ESP32)
//  RESET   --> 3.3V
//  PWDN    --> GND
//  HREF    --> NC

/**
 * Camera task
 *
 * @return void
 */
void camera_task(void* pvParameter)
{
    static const char* TASK_TAG = "camera_task";
    ESP_LOGI(TASK_TAG, "task started");

    esp_err_t err = init(&cam_conf, CAM_RES, RGB565); // カメラを初期化 (PCLK 20MHz)

    if (err != ESP_OK) {
        ESP_LOGE(TASK_TAG, "Camera init error");
    }

    ESP_LOGI(TASK_TAG, "cam MID = %X\n\r", getMID());
    ESP_LOGI(TASK_TAG, "cam PID = %X\n\r", getPID());

    while (1) {

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}