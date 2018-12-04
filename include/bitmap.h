#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define _bitsperpixel 16
#define _planes 1
#define _compression 0
#define _xpixelpermeter 0x130B //2835 , 72 DPI
#define _ypixelpermeter 0x130B //2835 , 72 DPI
#define pixel 0xFF

typedef struct __attribute__((packed, aligned(1))) {
    uint8_t signature[2];
    uint32_t filesize;
    uint32_t reserved;
    uint32_t fileoffset_to_pixelarray;
} fileheader;

typedef struct __attribute__((packed, aligned(1))) {
    uint32_t dibheadersize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsperpixel;
    uint32_t compression;
    uint32_t imagesize;
    uint32_t ypixelpermeter;
    uint32_t xpixelpermeter;
    uint32_t numcolorspallette;
    uint32_t mostimpcolor;
    uint32_t r;
    uint32_t g;
    uint32_t b;
} bitmapinfoheader;

typedef struct {
    fileheader fileheader;
    bitmapinfoheader bitmapinfoheader;
} bitmap_header_t;

/******************************************************************************
  Function prototypes
******************************************************************************/
bitmap_header_t* bmp_create_header(int w, int h);

#endif //_BITMAP_H_
