#include <camera.h>

//#define CAM_VGA
//#define CAM_CIF
//#define CAM_QVGA
#define CAM_QQVGA
//#define CAM_QCIF

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

uint8_t* camData;
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

static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace; boundary=123456789000000000000987654321";

static const char* STREAM_BOUNDARY = "--123456789000000000000987654321";

/**
 * @TODO:
 * 
 * @param w 
 * @param h 
 * @return true 
 * @return false 
 */
bool allocateMemory(uint16_t w, uint16_t h)
{
    line_h = h;
    line_size = w * 2;
    data_size = 2 + line_size * h;
    camData = (uint8_t*)malloc(data_size);
    if (camData == NULL) {
        ESP_LOGI("allocateMemory", "******** Memory allocate Error! ***********");
        return false;
    }
    return true;
}

/**
 * @TODO:
 * 
 * @param http_ctx 
 * @return esp_err_t 
 */
static esp_err_t write_frame(http_context_t http_ctx)
{
    http_buffer_t fb_data = {
        .data = camData,
        .size = data_size,
        .data_is_persistent = true
    };
    return http_response_write(http_ctx, &fb_data);
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

    esp_err_t err = init_camera(&cam_conf, CAM_RES, RGB565);

    setPCLK(2, DBLV_CLK_x4);
    vflip(false);

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

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    http_server_t server;
    http_server_options_t http_options = HTTP_SERVER_OPTIONS_DEFAULT();
    ESP_ERROR_CHECK(http_server_start(&http_options, &server));

    ESP_ERROR_CHECK(http_register_handler(server, "/bmp.bmp", HTTP_GET, HTTP_HANDLE_RESPONSE, &handle_rgb_bmp, NULL));
    ESP_LOGI(TASK_TAG, "Open http://192.168.1.1/bmp.bmp for single image/bitmap image");
    ESP_ERROR_CHECK(http_register_handler(server, "/bmp_stream", HTTP_GET, HTTP_HANDLE_RESPONSE, &handle_rgb_bmp_stream, NULL));
    ESP_LOGI(TASK_TAG, "Open http://192.168.1.1/bmp_stream for single image/bitmap image");

    allocateMemory(CAM_WIDTH, (CAM_HEIGHT / CAM_DIV));

    while (1) {
        vTaskDelay(1000000 / portTICK_PERIOD_MS);
    }
}

/**
 * @TODO:
 * 
 * @param http_ctx 
 * @param ctx 
 */
void handle_rgb_bmp(http_context_t http_ctx, void* ctx)
{
    bitmap_header_t* header = bmp_create_header(CAM_WIDTH, CAM_HEIGHT);
    if (header == NULL) {
        return;
    }

    http_response_begin(http_ctx, 200, "image/bmp", sizeof(*header) + data_size * CAM_DIV);
    http_buffer_t bmp_header = {
        .data = header,
        .size = sizeof(*header)
    };
    http_response_set_header(http_ctx, "Content-disposition", "inline; filename=capture.bmp");

    http_response_write(http_ctx, &bmp_header);

    uint16_t y, dy;
    dy = CAM_HEIGHT / CAM_DIV;

    for (y = 0; y < CAM_HEIGHT; y += dy) {
        getLines(y + 1, &camData[0], dy);

        write_frame(http_ctx);
    }
    free(header);

    http_response_end(http_ctx);
}

/**
 * @TODO:
 * 
 * @param http_ctx 
 * @param ctx 
 */
void handle_rgb_bmp_stream(http_context_t http_ctx, void* ctx)
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
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_err_t err = http_response_begin_multipart(http_ctx, "image/bitmap",
            data_size + sizeof(*header));
        if (err != ESP_OK) {
            break;
        }
        err = http_response_write(http_ctx, &bmp_header);
        if (err != ESP_OK) {
            break;
        }

        uint16_t y, dy;
        dy = CAM_HEIGHT / CAM_DIV;

        for (y = 0; y < CAM_HEIGHT; y += dy) {
            getLines(y + 1, &camData[0], dy);

            err = write_frame(http_ctx);
            if (err != ESP_OK) {
                break;
            }
        }

        err = http_response_end_multipart(http_ctx, STREAM_BOUNDARY);
        if (err != ESP_OK) {
            break;
        }
    }

    free(header);
    http_response_end(http_ctx);
}
