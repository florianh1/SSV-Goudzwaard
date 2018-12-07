#include <ov7670.h>

static const char* TAG = "OV7670";

//--------- Screen mode -----------------------------------------------

static const struct regval_list vga_OV7670[] = {
    // 640 x 480
    { REG_COM3, 0 }, // No scaling
    { REG_COM14, 0 },
    { REG_SCALING_XSC, 0x3a }, // Horizontal scale factor
    { REG_SCALING_YSC, 0x35 }, // Vertical scale factor
    { REG_SCALING_DCWCTR, 0 }, // Down sampling nothing
    { REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_RSVD | SCALING_PCLK_DIV_1 }, // DSP scale control Clock divide by 1
    { REG_SCALING_PCLK_DELAY, 0x02 },
    { 0xff, 0xff } // END MARKER
};

static const struct regval_list cif_OV7670[] = {
    // 352 x 288
    { REG_COM3, COM3_DCWEN }, // Enable format scaling
    { REG_COM14, COM14_DCWEN | COM14_PCLKDIV_2 }, // divide by 2
    { REG_SCALING_XSC, 0x3a }, // Horizontal scale factor
    { REG_SCALING_YSC, 0x35 }, // Vertical scale factor
    { REG_SCALING_DCWCTR, SCALING_DCWCTR_VDS_by_2 | SCALING_DCWCTR_HDS_by_2 }, // down sampling by 2
    { REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_RSVD | SCALING_PCLK_DIV_2 }, // DSP scale control Clock divide by 2
    { REG_SCALING_PCLK_DELAY, 0x02 },
    { 0xff, 0xff } // END MARKER
};

static const struct regval_list qvga_OV7670[] = {
    // 320 x 240
    { REG_COM3, COM3_DCWEN }, // Enable format scaling
    { REG_COM14, COM14_DCWEN | COM14_PCLKDIV_2 }, // divide by 2
    { REG_SCALING_XSC, 0x3a }, // Horizontal scale factor
    { REG_SCALING_YSC, 0x35 }, // Vertical scale factor
    { REG_SCALING_DCWCTR, SCALING_DCWCTR_VDS_by_2 | SCALING_DCWCTR_HDS_by_2 }, // down sampling by 2
    { REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_RSVD | SCALING_PCLK_DIV_2 }, // DSP scale control Clock divide by 2
    { REG_SCALING_PCLK_DELAY, 0x02 },
    { 0xff, 0xff } // END MARKER
};

static const struct regval_list qqvga_OV7670[] = {
    // 160 x 120
    { REG_COM3, COM3_DCWEN }, // Enable format scaling
    { REG_COM14, COM14_DCWEN | COM14_MANUAL | COM14_PCLKDIV_4 }, // divide by 4
    { REG_SCALING_XSC, 0x3a }, // Horizontal scale factor
    { REG_SCALING_YSC, 0x35 }, // Vertical scale factor
    { REG_SCALING_DCWCTR, SCALING_DCWCTR_VDS_by_4 | SCALING_DCWCTR_HDS_by_4 }, // down sampling by 4
    { REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_RSVD | SCALING_PCLK_DIV_4 }, // DSP scale control Clock divide by 4
    { REG_SCALING_PCLK_DELAY, 0x02 },
    { 0xff, 0xff } // END MARKER
};

static const struct regval_list qcif_OV7670[] = {
    { REG_COM3, COM3_SCALEEN | COM3_DCWEN }, // Enable format scaling
    { REG_COM3, COM3_DCWEN }, // Enable Downsampling/Dropping/Windowing
    { REG_COM14, COM14_DCWEN | COM14_MANUAL | COM14_PCLKDIV_2 }, // divide by 2
    { REG_SCALING_XSC, 0x3a }, // Horizontal scale factor
    { REG_SCALING_YSC, 0x35 }, // Vertical scale factor
    { REG_SCALING_DCWCTR, SCALING_DCWCTR_VDS_by_2 | SCALING_DCWCTR_HDS_by_2 }, // downsample by 2
    { REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_RSVD | SCALING_PCLK_DIV_2 }, // divide by 2
    { REG_SCALING_PCLK_DELAY, 0x52 },
    { 0xff, 0xff } /* END MARKER */
};

static const struct regval_list qqcif_OV7670[] = {
    { REG_COM3, COM3_SCALEEN | COM3_DCWEN },
    { REG_COM14, COM14_DCWEN | COM14_PCLKDIV_4 }, // divide by 4
    { REG_SCALING_XSC, 0x3a }, // Horizontal scale factor
    { REG_SCALING_YSC, 0x35 }, // Vertical scale factor
    { REG_SCALING_DCWCTR, SCALING_DCWCTR_VDS_by_4 | SCALING_DCWCTR_HDS_by_4 }, // down sampling by 4
    { REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_RSVD | SCALING_PCLK_DIV_4 }, // DSP scale control Clock divide by 4
    { REG_SCALING_PCLK_DELAY, 0x2A },
    { 0xff, 0xff } /* END MARKER */
};

//------- Color mode --------------------------------------
static const struct regval_list yuv422_OV7670[] = {
    { REG_RGB444, 0 }, /* No RGB444 please */
    { REG_COM1, 0 },
    { REG_COM15, COM15_R00FF },
    { REG_COM9, COM9_AGC_GAIN_16x | 0x08 }, /* 16x gain ceiling; 0x08 is reserved bit */
    { REG_MTX1, 0x80 }, /* "matrix coefficient 1" */
    { REG_MTX2, 0x80 }, /* "matrix coefficient 2" */
    { REG_MTX3, 0 }, /* vb */
    { REG_MTX4, 0x22 }, /* "matrix coefficient 4" */
    { REG_MTX5, 0x5e }, /* "matrix coefficient 5" */
    { REG_MTX6, 0x80 }, /* "matrix coefficient 6" */
    { REG_COM13, COM13_GAMMA | COM13_UVSAT | COM13_UVSWAP },
    { 0xff, 0xff } /* END MARKER */
};

static const struct regval_list rgb565_OV7670[] = {
    //	{REG_COM7, COM7_RGB},	// select RGB mode
    { REG_RGB444, 0 }, /* No RGB444 please */
    { REG_COM1, 0x0 },
    { REG_COM15, COM15_R00FF | COM15_RGB565 }, // RGB565
    { REG_TSLB, 0x04 },
    { REG_COM9, COM9_AGC_GAIN_16x | 0x08 }, /* 16x gain ceiling; 0x08 is reserved bit */
    { REG_MTX1, 0xb3 }, /* "matrix coefficient 1" */
    { REG_MTX2, 0xb3 }, /* "matrix coefficient 2" */
    { REG_MTX3, 0 }, /* vb */
    { REG_MTX4, 0x3d }, /* "matrix coefficient 4" */
    { REG_MTX5, 0xa7 }, /* "matrix coefficient 5" */
    { REG_MTX6, 0xe4 }, /* "matrix coefficient 6" */
    { REG_COM13, COM13_GAMMA | COM13_UVSAT },
    { 0xff, 0xff } /* END MARKER */
};

static const struct regval_list bayerRGB_OV7670[] = {
    //	{REG_COM7, COM7_PBAYER},
    { REG_RGB444, R444_DISABLE },
    { REG_COM15, COM15_R00FF },
    { REG_TSLB, 0x04 },
    { REG_COM13, 0x08 }, /* No gamma, magic rsvd bit */
    { REG_COM16, 0x3d }, /* Edge enhancement, denoise */
    { REG_REG76, 0xe1 }, /* Pix correction, magic rsvd */
    { 0xff, 0xff } /* END MARKER */
};

const struct regval_list OV7670_default2_regs[] = {
    { REG_TSLB, 0x04 },
    { REG_COM15, COM15_R00FF | COM15_RGB565 },
    { REG_COM7, COM7_FMT_QVGA | COM7_RGB },
    { REG_HREF, 0x80 },
    { REG_HSTART, 0x16 },
    { REG_HSTOP, 0x04 },
    { REG_VSTART, 0x02 },
    { REG_VSTOP, 0x7b },
    { REG_VREF, 0x06 },
    { REG_COM3, COM3_DCWEN },
    { REG_COM14, COM14_DCWEN | COM14_MANUAL | COM14_PCLKDIV_2 },
    { REG_SCALING_XSC, 0x3a },
    { REG_SCALING_YSC, 0x35 },
    { REG_SCALING_DCWCTR, SCALING_DCWCTR_VDS_by_2 | SCALING_DCWCTR_HDS_by_2 },
    { REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_RSVD | SCALING_PCLK_DIV_2 },
    { REG_SCALING_PCLK_DELAY, 0x02 },
    { REG_CLKRC, CLK_RSVD | 0x01 }, // clock divid 2
    // Gamma curve values
    { REG_SLOP, 0x20 }, // SLOP = (256-GAM15)*40/30
    { REG_GAM1, 0x1c },
    { REG_GAM2, 0x28 },
    { REG_GAM3, 0x3c },
    { REG_GAM4, 0x55 },
    { REG_GAM5, 0x68 },
    { REG_GAM6, 0x76 },
    { REG_GAM7, 0x80 },
    { REG_GAM8, 0x88 },
    { REG_GAM9, 0x8f },
    { REG_GAM10, 0x96 },
    { REG_GAM11, 0xa3 },
    { REG_GAM12, 0xaf },
    { REG_GAM13, 0xc4 },
    { REG_GAM14, 0xd7 },
    { REG_GAM15, 0xe8 },

    // AGC(Auto Gain Celling) and AEC(Auto Exposure(露出) Control) parameters.
    // AGC/AEC parameters.  Note we start by disabling those features,then turn them only after tweaking the values.
    { REG_COM8, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT },

    { REG_GAIN, 0x00 },
    { REG_AECH, 0x00 },
    { REG_COM4, COM4_AEC_FULL },
    { REG_COM9, COM9_AGC_GAIN_8x | 0x08 },
    { REG_BD50MAX, 0x05 },
    { REG_BD60MAX, 0x07 },
    { REG_AEW, 0x75 }, // AGC upper limit
    { REG_AEB, 0x63 }, // AGC lower limit
    { REG_VPT, 0xA5 }, // AGC/AEC fast mode op region
    { REG_HAECC1, 0x78 }, // Hist AEC/AGC control 1
    { REG_HAECC2, 0x68 }, // Hist AEC/AGC control 2
    { 0xa1, 0x03 }, // Reserved
    { REG_HAECC3, 0xdf }, // Hist AEC/AGC control 3
    { REG_HAECC4, 0xdf }, // Hist AEC/AGC control 4
    { REG_HAECC5, 0xf0 }, // Hist AEC/AGC control 5
    { REG_HAECC6, 0x90 }, // Hist AEC/AGC control 6
    { REG_HAECC7, 0x94 }, // Hist AEC/AGC control 7

    { REG_COM8, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT | COM8_AGC | COM8_AEC },

    { REG_COM5, 0x61 }, // Reserved
    { REG_COM6, 0x4b }, // Reset all timing when format changes
    { 0x16, 0x02 }, // Reserved
    { REG_MVFP, 0x37 }, //0x07},
    { REG_ADCCTR1, 0x02 }, // Reserved
    { REG_ADCCTR2, 0x91 }, // Reserved
    { 0x29, 0x07 },
    { 0x33, 0x0b },
    { 0x35, 0x0b },
    { 0x37, 0x1d },
    { 0x38, 0x71 },
    { 0x39, 0x2a }, // All Reserved
    { REG_COM12, 0x78 },
    { 0x4d, 0x40 },
    { 0x4e, 0x20 }, // Reserved

    { REG_GFIX, 0x00 }, // AWB Pre gain control
    { REG_DBLV, DBLV_BYPASS }, // PLL control,Regulator control
    { REG_REG74, 0x19 }, // Digital gain manual control
    { 0x8d, 0x4f },
    { 0x8e, 0x00 },
    { 0x8f, 0x00 },
    { 0x90, 0x00 },
    { 0x91, 0x00 }, // Reserved
    { REG_DM_LNL, 0x00 },
    { 0x96, 0x00 },
    { 0x9a, 0x80 },
    { 0xb0, 0x84 },
    { REG_ABLC1, 0x0c },
    { 0xb2, 0x0e },
    { REG_THL_ST, 0x82 },
    { 0xb8, 0x0a },

    { REG_AWBC1, 0x14 }, //AWB Control 1
    { REG_AWBC2, 0xf0 }, //AWB Control 2
    { REG_AWBC3, 0x34 }, //AWB Control 3
    { REG_AWBC4, 0x58 }, //AWB Control 4
    { REG_AWBC5, 0x28 }, //AWB Control 5
    { REG_AWBC6, 0x3a }, //AWB Control 6
    { REG_AWBC7, 0x88 }, //AWB Control 7
    { REG_AWBC8, 0x88 }, //AWB Control 8
    { REG_AWBC9, 0x44 }, //AWB Control 9
    { REG_AWBC10, 0x67 }, //AWB Control 10
    { REG_AWBC11, 0x49 }, //AWB Control 11
    { REG_AWBC12, 0x0e }, //AWB Control 12

    //	{REG_LCC1, 0x00},	// Lens Correction Option 1
    //	{REG_LCC2, 0x00},	// Lens Correction Option 2
    { REG_LCC3, 0x04 }, // Lens Correction Option 3
    { REG_LCC4, 0x20 }, // Lens Correction Option 4
    { REG_LCC5, 0x05 }, // Lens Correction Option 5
    { REG_LCC6, 0x04 }, // Lens Correction Option 6
    { REG_LCC7, 0x08 }, // Lens Correction Option 7
    { REG_AWBCTR3, 0x0a },
    { REG_AWBCTR2, 0x55 },
    { REG_AWBCTR1, 0x11 },
    { REG_AWBCTR0, 0x9f },
    { REG_GGAIN, 0x40 }, // AWB Green gain
    { REG_BLUE, 0x40 }, // AWB Blue gain (00-ff)
    { REG_RED, 0x40 }, // AWB Red gain (00-ff)

    { REG_COM8, COM8_FASTAEC | COM8_RSVD | COM8_AGC | COM8_AWB | COM8_AEC },

    { REG_COM10, COM10_PCLK_HB | COM10_VS_NEG }, // PCLK does not toggle during horizontal blank & VSYNC negative

    { REG_MTX1, 0x80 }, // Matrix Coefficient 1
    { REG_MTX2, 0x80 }, // Matrix Coefficient 2
    { REG_MTX3, 0x00 }, // Matrix Coefficient 3
    { REG_MTX4, 0x22 }, // Matrix Coefficient 4
    { REG_MTX5, 0x5e }, // Matrix Coefficient 5
    { REG_MTX6, 0x80 }, // Matrix Coefficient 6
    { REG_MTXS, 0x9e }, // Matrix Coefficient Sign

    { REG_COM16, COM16_AWBGAIN }, // AWB gain enable
    { REG_EDGE, 0x00 }, // Edge enhancement factor
    { REG_REG75, 0x05 }, // Edge enhanced lower limit
    { REG_REG76, 0xe1 }, // Edge enhanced higher limit ,Black/white pixcel correction enable
    { REG_DNSTH, 0x00 }, // De-noise Threshold
    { REG_REG77, 0x01 }, // Offset, de-noise range control
    { REG_COM13, 0xc2 }, // Gamma enable, UV saturation auto adjustment
    { 0x4b, 0x09 },
    { REG_SATCTR, 0x60 }, // UV saturatin control min
    { REG_COM16, COM16_YUV_ENHANC | COM16_DE_NOISE | COM16_AWBGAIN },
    { REG_CONTRAS, 0x40 }, // Contrast Control
    { 0x34, 0x11 },

    { REG_COM11, COM11_EXP }, // Exposure timing can be less than limit of banding filter when light is too strong
    { REG_NT_CTRL, 0x89 }, // Auto frame rate adjustment dummy row selection

    // Magic setting
    { 0x96, 0x00 },
    { 0x97, 0x30 },
    { 0x98, 0x20 },
    { 0x99, 0x30 },
    { 0x9a, 0x84 },
    { 0x9b, 0x29 },
    { 0x9c, 0x03 },
    { 0x9d, 0x4c },
    { 0x9e, 0x3f },
    { 0x78, 0x04 },
    { 0x79, 0x01 },
    { 0xc8, 0xf0 },
    { 0x79, 0x0f },
    { 0xc8, 0x00 },
    { 0x79, 0x10 },
    { 0xc8, 0x7e },
    { 0x79, 0x0a },
    { 0xc8, 0x80 },
    { 0x79, 0x0b },
    { 0xc8, 0x01 },
    { 0x79, 0x0c },
    { 0xc8, 0x0f },
    { 0x79, 0x0d },
    { 0xc8, 0x20 },
    { 0x79, 0x09 },
    { 0xc8, 0x80 },
    { 0x79, 0x02 },
    { 0xc8, 0xc0 },
    { 0x79, 0x03 },
    { 0xc8, 0x40 },
    { 0x79, 0x05 },
    { 0xc8, 0x30 },
    { 0x79, 0x26 },

    { REG_COM2, COM2_OUT_DRIVE_4x }, // Output Drive Capability 4x
    { REG_COM11, COM11_FR_BY_4 | COM11_EXP }, // 1/4 normal mode frame rate, Exposure timing

    { 0xff, 0xff },
};

//---------------------------------------------------------

void conf_setFrameSize(uint8_t res)
{
    switch (res) {
    case VGA:
        cam_conf.frame_width = 640;
        cam_conf.frame_height = 480;
        break;
    case QVGA:
        cam_conf.frame_width = 320;
        cam_conf.frame_height = 240;
        break;
    case QQVGA:
        cam_conf.frame_width = 160;
        cam_conf.frame_height = 120;
        break;

    case CIF:
        cam_conf.frame_width = 352;
        cam_conf.frame_height = 288;
        break;
    case QCIF:
        cam_conf.frame_width = 176;
        cam_conf.frame_height = 144;
        break;
    case QQCIF:
        cam_conf.frame_width = 88;
        cam_conf.frame_height = 72;
        break;
    }
}

void reset(void)
{
    wrReg(REG_COM7, COM7_RESET); // All reg reset

    ESP_LOGI(TAG, "--- Default setting -----");
    wrRegs(OV7670_default2_regs); // Camera Default setting
    ESP_LOGI(TAG, "--- Resolution setting -----");
    setResolution(_resolution); // Resolution setting
    ESP_LOGI(TAG, "--- ColorMode setting -----");
    setColor(_colormode); // Color mode setting
    setPCLK(1, DBLV_CLK_x4); // PCLK setting : 10MHz / (pre+1) * 4 --> 20MHz
}

esp_err_t init_camera(const camera_config_t* value, uint8_t res, uint8_t colmode)
{
    esp_err_t err;
    memcpy(&cam_conf, value, sizeof(cam_conf));

    _resolution = res;
    _colormode = colmode;

    gpio_pad_select_gpio(cam_conf.XCLK);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(cam_conf.XCLK, GPIO_MODE_OUTPUT);

    gpio_set_level(cam_conf.XCLK, 0);

    periph_module_enable(PERIPH_LEDC_MODULE);

    ledc_timer_config_t timer_conf;
    timer_conf.duty_resolution = 1;
    timer_conf.freq_hz = cam_conf.xclk_freq_hz;
    timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num = cam_conf.ledc_timer;
    err = ledc_timer_config(&timer_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed, rc=%x", err);
        return err;
    }

    ledc_channel_config_t ch_conf;
    ch_conf.channel = cam_conf.ledc_channel;
    ch_conf.timer_sel = cam_conf.ledc_timer;
    ch_conf.intr_type = LEDC_INTR_DISABLE;
    ch_conf.duty = 1;
    ch_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    ch_conf.gpio_num = cam_conf.XCLK;

    ch_conf.hpoint = 0;

    err = ledc_channel_config(&ch_conf);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed, rc=%x", err);
        return err;
    }

    conf_setFrameSize(res);

    switch (colmode) {
    case YUV422:
        break;
    case RGB565:
        cam_conf.pixel_byte_num = 2;
        break;
    case BAYER_RAW:
        cam_conf.pixel_byte_num = 1;
        break;
    case PBAYER_RAW:
        cam_conf.pixel_byte_num = 1;
        break;
    }

    i2c_init(CAMERA_SDA, CAMERA_SCL, 100000);

    err = I2S_camera_init(&cam_conf);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, " I2S Camera init ERROR");
        return err;
    }

    reset();

    ESP_LOGI(TAG, "---- Camera init ok! ----");
    return err;
}

void setResolution(uint8_t res)
{
    uint8_t temp;
    uint16_t vstart = 0, hstart = 0;
    uint8_t pclkdiv = 0;

    conf_setFrameSize(res);

    temp = rdReg(REG_COM7);
    temp &= 0b01000111;

    switch (res) {
    case VGA:
        wrReg(REG_COM7, temp | COM7_FMT_VGA); // set Resolution
        wrRegs(vga_OV7670);
        hstart = 158;
        vstart = 10;
        pclkdiv = 4;
        break;
    case QVGA:
        wrReg(REG_COM7, temp | COM7_FMT_QVGA); // set Resolution
        wrRegs(qvga_OV7670);
        hstart = 180;
        vstart = 12;
        pclkdiv = 2;
        break;
    case QQVGA:
        wrReg(REG_COM7, temp | COM7_FMT_QVGA); // set Resolution
        wrRegs(qqvga_OV7670);
        hstart = 190;
        vstart = 10;
        pclkdiv = 1;
        break;

    case CIF:
        wrReg(REG_COM7, temp | COM7_FMT_CIF); // set Resolution
        wrRegs(cif_OV7670);
        hstart = 178;
        vstart = 14;
        pclkdiv = 2;
        break;
    case QCIF:
        wrReg(REG_COM7, temp | COM7_FMT_QCIF); // set Resolution
        wrRegs(qcif_OV7670);
        hstart = 456;
        vstart = 14;
        pclkdiv = 1;
        break;
    case QQCIF:
        wrReg(REG_COM7, temp | COM7_FMT_QCIF); // set Resolution
        wrRegs(qcif_OV7670);
        hstart = 456;
        vstart = 14;
        pclkdiv = 1;
        break;
    }

    setPCLK(pclkdiv, DBLV_CLK_x4);
    setHStart(hstart);
    setVStart(vstart);
}

void setHStart(uint16_t hstart)
{
    uint16_t hstop = 0;
    switch (_resolution) {
    case VGA:
    case QVGA:
    case QQVGA:
        hstop = 640;
        break;
    case CIF:
        hstop = 704;
        break;
    case QCIF:
        hstop = 352;
        break;
    case QQCIF:
        hstop = 176;
        break;
    }
    hstop = (hstart + hstop) % 784;
    wrReg(REG_HSTART, (uint8_t)(hstart / 8));
    wrReg(REG_HSTOP, (uint8_t)(hstop / 8));
    wrReg(REG_HREF, 0x80 | (uint8_t)((hstop % 8) << 3) | (uint8_t)(hstart % 8));
    rewrCLKRC();
}

void setVStart(uint16_t vstart)
{
    uint16_t vstop;
    vstop = vstart + 480;
    wrReg(REG_VSTART, (uint8_t)(vstart / 4));
    wrReg(REG_VSTOP, (uint8_t)(vstop / 4));
    wrReg(REG_VREF, (uint8_t)((vstop % 4) << 2) | (uint8_t)(vstart % 4));
    rewrCLKRC();
}

uint16_t getHStart(void)
{
    uint16_t hstart;
    hstart = (uint16_t)rdReg(REG_HSTART) * 8 + (uint16_t)(rdReg(REG_HREF) & 0x07);
    return hstart;
}

uint16_t getVStart(void)
{
    uint16_t vstart;
    vstart = (uint16_t)rdReg(REG_VSTART) * 4 + (uint16_t)(rdReg(REG_VREF) & 0x03);
    return vstart;
}

uint16_t* getLine(uint16_t lineno)
{
    uint16_t* p_buf;
    p_buf = camera_getLine(lineno);
    return p_buf;
}

bool getLines(uint16_t lineno, uint8_t* buf, uint16_t n)
{
    uint16_t i, *p_buf;
    uint16_t wb = cam_conf.frame_width * cam_conf.pixel_byte_num;

    for (i = 0; i < n; i++) {
        p_buf = camera_getLine(lineno + i);

        if (p_buf == NULL)
            return false;

        memcpy(&buf[i * wb], (uint8_t*)p_buf, wb);
    }

    return true;
}

void getFrame(uint8_t* buf)
{
    getLines(1, buf, cam_conf.frame_height);
}

void setColor(uint8_t colormode)
{
    uint8_t temp;

    temp = rdReg(REG_COM7) & 0b01111010;
    wrReg(REG_COM7, temp | colormode); // set colormode

    switch (colormode) {
    case YUV422:
        wrRegs(yuv422_OV7670); // ather reg set
        cam_conf.pixel_byte_num = 2;
        break;

    case RGB565:
        wrRegs(rgb565_OV7670);
        cam_conf.pixel_byte_num = 2;
        break;

    case BAYER_RAW:
        wrRegs(bayerRGB_OV7670);
        cam_conf.pixel_byte_num = 1;
        break;

    case PBAYER_RAW:

        cam_conf.pixel_byte_num = 1; // ???
        break;
    }
    rewrCLKRC(); //according to the Linux kernel driver PCLK needs rewriting
}

void setPCLK(uint8_t pre, uint8_t pll)
{
    uint8_t temp;
    temp = rdReg(REG_CLKRC);
    wrReg(REG_CLKRC, (temp & 0b10000000) | pre); // F(internal clock) = F(input clock) / (Bit[0-5] + 1)
    temp = rdReg(REG_DBLV);
    wrReg(REG_DBLV, (temp & 0b00111111) | pll); // val=0:Bypass PLL,1:Input clock x4,2: x6,3:x8
    rewrCLKRC();
}

void rewrCLKRC(void)
{
    uint8_t temp;
    temp = rdReg(REG_CLKRC);
    wrReg(REG_CLKRC, temp); //according to the Linux kernel driver rgb565 PCLK needs rewriting
}

void vflip(bool enable)
{
    uint8_t temp;
    temp = rdReg(REG_MVFP) & 0b11001111;
    if (enable)
        temp |= MVFP_MIRROR | MVFP_FLIP;
    wrReg(REG_MVFP, temp);
}

uint16_t getMID(void)
{
    return ((uint16_t)rdReg(REG_MIDH) << 8 | (uint16_t)rdReg(REG_MIDL));
}

uint16_t getPID(void)
{
    return ((uint16_t)rdReg(REG_PID) << 8 | (uint16_t)rdReg(REG_VER));
}

void setGain(uint16_t val)
{
    if (val > 1023)
        val = 1023;
    uint8_t temp;
    wrReg(REG_GAIN, val % 256);
    temp = rdReg(REG_VREF) & 0x3F;
    wrReg(REG_VREF, temp | ((val / 256) << 6));
}

uint16_t getGain(void)
{
    return ((uint16_t)rdReg(REG_GAIN) + (uint16_t)((rdReg(REG_VREF) & 0x3F)) * 256);
}

void setAGC(uint8_t val)
{
    uint8_t temp;
    temp = rdReg(REG_COM8) & ~(COM8_FASTAEC | COM8_AGC);
    if (val == 1)
        temp |= COM8_FASTAEC | COM8_AGC;
    wrReg(REG_COM8, temp);
}

bool getAGC(void)
{
    uint8_t temp;
    temp = rdReg(REG_COM8);
    if ((temp & COM8_AGC) != 0)
        return true;
    return false;
}

void setAWB(uint8_t val)
{
    uint8_t temp;
    temp = rdReg(REG_COM8) & ~COM8_AWB;
    if (val == 1)
        temp |= COM8_AWB;
    wrReg(REG_COM8, temp);
}

bool getAWB(void)
{
    uint8_t temp;
    temp = rdReg(REG_COM8);
    if ((temp & COM8_AWB) != 0)
        return true;
    return false;
}

void setAWBB(uint8_t val)
{
    wrReg(REG_BLUE, val);
}

void setAWBR(uint8_t val)
{
    wrReg(REG_RED, val);
}

void setAWBG(uint8_t val)
{
    wrReg(REG_GGAIN, val);
}

void setAEC(uint8_t val)
{
    uint8_t temp;
    temp = rdReg(REG_COM8) & ~(COM8_FASTAEC | COM8_AEC);
    if (val == 1)
        temp |= COM8_FASTAEC | COM8_AEC;
    wrReg(REG_COM8, temp);
}

bool getAEC(void)
{
    uint8_t temp;
    temp = rdReg(REG_COM8);
    if ((temp & COM8_AEC) != 0)
        return true;
    return false;
}

void setBright(int8_t val)
{
    wrReg(REG_BRIGHT, (uint8_t)val);
}

int8_t getBright(void)
{
    int8_t temp;
    temp = (int8_t)rdReg(REG_BRIGHT);
    return temp;
}

void setContrast(uint8_t val)
{
    wrReg(REG_CONTRAS, val);
}

uint8_t getContrast(void)
{
    return rdReg(REG_CONTRAS);
}

void setExposure(uint16_t val)
{
    uint8_t temp;
    temp = rdReg(REG_COM1) & 0x03;
    wrReg(REG_COM1, temp | (uint8_t)(val % 4));
    wrReg(REG_AECH, (uint8_t)((val / 4) % 256));
    temp = rdReg(REG_AECHH) & 0x3F;
    wrReg(REG_AECHH, temp | (uint8_t)(val / 1024));
}

void colorbar(bool on)
{
    uint8_t temp;
    temp = rdReg(REG_COM17);
    if (on)
        wrReg(REG_COM17, temp | COM17_CBAR);
    else
        wrReg(REG_COM17, temp & ~COM17_CBAR);
}

void colorbar_super(bool on)
{
    uint8_t temp;
    temp = rdReg(REG_COM7);
    if (on)
        wrReg(REG_COM7, temp | COM7_CBAR);
    else
        wrReg(REG_COM7, temp & ~COM7_CBAR);
}

//----------------------------------------------

void i2c_init(uint8_t sda, uint8_t scl, uint32_t clk_speed)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = clk_speed,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
}

void wrReg(uint8_t reg, uint8_t dat)
{
    i2c_cmd_handle_t cmd;
    esp_err_t ret;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (0x21 << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, dat, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "i2c write reg:%02X data:%02X", reg, dat);
}

uint8_t rdReg(uint8_t reg)
{
    uint8_t dat = 0x0;
    i2c_cmd_handle_t cmd;
    esp_err_t ret;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (0x21 << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (0x21 << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &dat, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "i2c read reg:%02X data:%02X", reg, dat);

    return dat;
}

void wrRegs(const struct regval_list* reglist)
{
    const struct regval_list* next = reglist;

    for (;;) {
        uint8_t reg_addr = next->reg_num;
        uint8_t reg_val = next->value;
        if ((reg_addr == 0xff) && (reg_val == 0xff)) // end markers
            break;
        wrReg(reg_addr, reg_val);
        next++;
    }
}
