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
        char tmp[1500];
        memcpy(tmp, pData, send_size);
        //WSclient.write(pData, send_size); // websocketデータ送信 ( UNITサイズ以下に区切って送る )

        for (size_t i = 0; i < 1500; i++) {
            printf("%x, ", tmp[i]);
        }

        printf("\n");

        len -= send_size;
        pData += send_size;
    }
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

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    char tx_buffer[20];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        uint16_t y, dy;

        dy = CAM_HEIGHT / CAM_DIV; // １度に送るライン数
        setImgHeader(CAM_WIDTH, dy); // Websocket用ヘッダを用意

        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr("192.168.1.2"); //TODO: set correct address addres is most of time 192.168.1.2
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(3000);
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
            for (y = 0; y < CAM_HEIGHT; y += dy) {
                getLines(y + 1, &WScamData[6], dy); // カメラから dyライン分得る。LineNo(top:1)

                uint16_t len, send_size;
                uint8_t* pData;

                WScamData[4] = (uint8_t)(y % 256);
                WScamData[5] = (uint8_t)(y / 256);

                len = data_size + 4;
                pData = WScamData;
                while (len) {
                    send_size = (len > UNIT_SIZE) ? UNIT_SIZE : len;

                    //WSclient.write(pData, send_size); // websocketデータ送信 ( UNITサイズ以下に区切って送る )

                    int err = sendto(sock, &pData, send_size, 0, (struct sockaddr*)&destAddr, sizeof(destAddr));
                    if (err < 0) {
                        ESP_LOGE(TASK_TAG, "Error occured during sending frame percentage: errno %d", errno);
                        break;
                    }

                    len -= send_size;
                    pData += send_size;
                }

                // if (WS_on) {
                //     if (WSclient) {
                // WS_sendImg(y); // Websocket 画像送信
                //         WS_cmdCheck(); // clientからのコマンドのやり取り
                //     } else {
                //         WSclient.stop(); // 接続が切れたら、ブラウザとコネクション切断する。
                //         WS_on = false;
                //         Serial.println("Client Stop--------------------");
                //     }
                // }
            }
            // if (!WS_on) {
            //     Ini_HTTP_Response();
            // }

            vTaskDelay(1000000 / portTICK_PERIOD_MS);
        }
        free(WScamData);
        vTaskDelete(NULL);
    }
}
