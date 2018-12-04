#include <camera.h>

#define CAM_VGA 
//#define CAM_CIF
//#define CAM_QVGA
//#define CAM_QCIF
//#define CAM_QQVGA

//********* カメラ解像度指定 ***************
#ifdef CAM_VGA
#define CAM_RES     VGA     // カメラ解像度
#define CAM_WIDTH   640     // カメラ幅
#define CAM_HEIGHT  480     // カメラ高さ
#define CAM_DIV      12     // １画面分割数
#endif

#ifdef CAM_CIF
#define CAM_RES     CIF     // カメラ解像度
#define CAM_WIDTH   352     // カメラ幅
#define CAM_HEIGHT  288     // カメラ高さ
#define CAM_DIV       4     // １画面分割数
#endif

#ifdef CAM_QVGA
#define CAM_RES     QVGA    // カメラ解像度
#define CAM_WIDTH   320     // カメラ幅
#define CAM_HEIGHT  240     // カメラ高さ
#define CAM_DIV       3     // １画面分割数
#endif

#ifdef CAM_QCIF
#define CAM_RES     QCIF    // カメラ解像度
#define CAM_WIDTH   176     // カメラ幅
#define CAM_HEIGHT  144     // カメラ高さ
#define CAM_DIV     1       // １画面分割数
#endif

#ifdef CAM_QQVGA
#define CAM_RES     QQVGA   // カメラ解像度
#define CAM_WIDTH   160     // カメラ幅
#define CAM_HEIGHT  120     // カメラ高さ
#define CAM_DIV       1     // １画面分割数
#endif

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

uint8_t v_gain = 0;
uint8_t v_awbb = 0;
uint8_t v_awbr = 0;
uint8_t v_awbg = 0;
uint16_t v_aec = 0;
int8_t v_bright = 0;
uint8_t v_cnt = 0;
uint16_t v_hstart = 0;
uint16_t v_vstart = 0;
bool b_agc = false;
bool b_awb = false;
bool b_aec = false;

uint8_t* WScamData;
uint16_t data_size;
uint16_t line_size;
uint16_t line_h;

#define WS_FIN 0x80
#define OP_TEXT 0x81
#define OP_BIN 0x82
#define OP_CLOSE 0x88
#define OP_PING 0x89
#define OP_PONG 0x8A
#define WS_MASK 0x80;

#define UNIT_SIZE 1414 // websocketで１度に送信する最大バイト数

static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace; boundary=123456789000000000000987654321";

static const char* STREAM_BOUNDARY = "--123456789000000000000987654321";

bool setImgHeader(uint16_t w, uint16_t h)
{
    line_h = h;
    line_size = w * 2;
    data_size = 2 + line_size * h; // (LineNo + img) バイト数
    WScamData = (uint8_t*)malloc(data_size + 4); // + head size
    if (WScamData == NULL) {
        ESP_LOGI("setImgHeader", "******** Memory allocate Error! ***********");
        return false;
    }
    WScamData[0] = OP_BIN; // バイナリデータ送信ヘッダ
    WScamData[1] = 126; // 126:この後に続く２バイトがデータ長。127なら８バイトがデータ長
    WScamData[2] = (uint8_t)(data_size / 256); // 送信バイト数(Hi)
    WScamData[3] = (uint8_t)(data_size % 256); // 送信バイト数(Lo)
    return true;
}

void WS_sendImg(uint16_t lineNo)
{
    uint16_t len, send_size;
    uint8_t* pData;

    WScamData[4] = (uint8_t)(lineNo % 256);
    WScamData[5] = (uint8_t)(lineNo / 256);

    len = data_size + 4;
    pData = WScamData;
    while (len) {
        send_size = (len > UNIT_SIZE) ? UNIT_SIZE : len;
        char tmp[send_size];
        memcpy(tmp, pData, send_size);
        //WSclient.write(pData, send_size); // websocketデータ送信 ( UNITサイズ以下に区切って送る )

        for (size_t i = 0; i < send_size; i++) {
            printf("%x", tmp[i]);
            // printf("\n");
        }

        printf("\n\n\n");

        len -= send_size;
        pData += send_size;
    }
}

static esp_err_t write_frame(http_context_t http_ctx)
{
    http_buffer_t fb_data = {
        .data = WScamData,
        .size = data_size,
        .data_is_persistent = true
    };
    return http_response_write(http_ctx, &fb_data);
}

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

    setPCLK(2, DBLV_CLK_x4); // PCLK変更 : 10MHz / (pre+1) * 4 --> 13.3MHz
    vflip(false); // 画面１８０度回転

    if (err != ESP_OK) {
        ESP_LOGE(TASK_TAG, "Camera init error");
    }

    ESP_LOGI(TASK_TAG, "cam MID = %X\n\r", getMID());
    ESP_LOGI(TASK_TAG, "cam PID = %X\n\r", getPID());

    v_gain = getGain();
    v_awbb = rdReg(REG_BLUE);
    v_awbr = rdReg(REG_RED);
    v_bright = getBright();
    v_cnt = getContrast();
    v_hstart = getHStart();
    v_vstart = getVStart();
    b_agc = getAGC();
    b_awb = getAWB();
    b_aec = getAEC();

    http_server_t server;
    http_server_options_t http_options = HTTP_SERVER_OPTIONS_DEFAULT();
    ESP_ERROR_CHECK(http_server_start(&http_options, &server));

    ESP_ERROR_CHECK(http_register_handler(server, "/bmp.bmp", HTTP_GET, HTTP_HANDLE_RESPONSE, &handle_rgb_bmp, NULL));
    ESP_LOGI(TASK_TAG, "Open http://192.168.1.1/bmp.bmp for single image/bitmap image");
    ESP_ERROR_CHECK(http_register_handler(server, "/bmp_stream", HTTP_GET, HTTP_HANDLE_RESPONSE, &handle_rgb_bmp_stream, NULL));
    ESP_LOGI(TASK_TAG, "Open http://192.168.1.1/bmp_stream for single image/bitmap image");

    ESP_LOGI(TASK_TAG, "Free heap: %u", xPortGetFreeHeapSize());
    ESP_LOGI(TASK_TAG, "Camera demo ready");

    while (1) {
        uint16_t y, dy;

        dy = CAM_HEIGHT / CAM_DIV; // １度に送るライン数
        setImgHeader(CAM_WIDTH, dy); // Websocket用ヘッダを用意

        while (1) {
            for (y = 0; y < CAM_HEIGHT; y += dy) {
                getLines(y + 1, &WScamData[6], dy); // カメラから dyライン分得る。LineNo(top:1)

                uint16_t len, send_size;
                uint8_t* pData;

                WScamData[4] = (uint8_t)(y % 256);
                WScamData[5] = (uint8_t)(y / 256);

                len = data_size + 4;
                pData = WScamData;

                int q = 0;

                while (len) {
                    send_size = (len > UNIT_SIZE) ? UNIT_SIZE : len;

                    char tmp[send_size];
                    memcpy(tmp, pData, send_size);
                    //WSclient.write(pData, send_size); // websocketデータ送信 ( UNITサイズ以下に区切って送る )

                    // for (size_t i = 0; i < send_size; i = i + 4) {
                    //     printf("%.2x%.2x ", tmp[i], tmp[i + 1]);
                    //     vTaskDelay(10 / portTICK_PERIOD_MS);

                    //     q++;
                    //     if (q >= 8) {
                    //         q = 0;
                    //         printf("\n");
                    //     }

                    //     // printf("\n");
                    // }

                    // printf("\n\n\n");

                    len -= send_size;
                    pData += send_size;
                }
            }

            vTaskDelay(10000000 / portTICK_PERIOD_MS);
        }
        free(WScamData);
        vTaskDelay(1000000 / portTICK_PERIOD_MS);
    }
}

static void handle_rgb_bmp(http_context_t http_ctx, void* ctx)
{
    printf("size: %d", sizeof(bitmapinfoheader));
    getLines(0, &WScamData[0], (CAM_HEIGHT));
    // if (err != ESP_OK) {
    //     ESP_LOGD("camera_task", "Camera capture failed with error = %d", err);
    //     return;
    // }

    bitmap_header_t* header = bmp_create_header(CAM_WIDTH, CAM_HEIGHT);
    if (header == NULL) {
        return;
    }

    http_response_begin(http_ctx, 200, "image/bmp", sizeof(*header) + data_size);
    http_buffer_t bmp_header = {
        .data = header,
        .size = sizeof(*header)
    };
    http_response_set_header(http_ctx, "Content-disposition", "inline; filename=capture.bmp");
    http_response_write(http_ctx, &bmp_header);
    free(header);

    write_frame(http_ctx);
    http_response_end(http_ctx);
}

static void handle_rgb_bmp_stream(http_context_t http_ctx, void* ctx)
{
    http_response_begin(http_ctx, 200, STREAM_CONTENT_TYPE, HTTP_RESPONSE_SIZE_UNKNOWN);

    bitmap_header_t* header = bmp_create_header(CAM_WIDTH, CAM_HEIGHT);
    if (header == NULL) {
        return;
    }
    http_buffer_t bmp_header = {
        .data = header,
        .size = sizeof(*header)
    };

    while (true) {
        getLines(1, &WScamData[0], (CAM_HEIGHT));
        // esp_err_t err = camera_run();
        // if (err != ESP_OK) {
        //     ESP_LOGD("camera_task", "Camera capture failed with error = %d", err);
        //     return;
        // }

        esp_err_t err = http_response_begin_multipart(http_ctx, "image/bitmap",
            data_size + sizeof(*header));
        if (err != ESP_OK) {
            break;
        }
        err = http_response_write(http_ctx, &bmp_header);
        if (err != ESP_OK) {
            break;
        }
        err = write_frame(http_ctx);
        if (err != ESP_OK) {
            break;
        }
        err = http_response_end_multipart(http_ctx, STREAM_BOUNDARY);
        if (err != ESP_OK) {
            break;
        }
    }

    free(header);
    http_response_end(http_ctx);
}
