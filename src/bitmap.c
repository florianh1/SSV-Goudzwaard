#include <bitmap.h>

/**
 * @sets the header of the bitmap image
 * 
 * @param w 
 * @param h 
 * @return bitmap_header_t* 
 */
bitmap_header_t* bmp_create_header(int w, int h)
{
    bitmap_header_t* pbitmap = (bitmap_header_t*)calloc(1, sizeof(bitmap_header_t));
#ifdef CONVERT_RGB565_TO_RGB332
    int _pixelbytesize = w * h * 8 / 8;
#else
    int _pixelbytesize = w * h * 16 / 8;
#endif // CONVERT_RGB565_TO_RGB332
    int _filesize = _pixelbytesize + sizeof(bitmap_header_t);
    strcpy((char*)pbitmap->fileheader.signature, "BM");
    pbitmap->fileheader.filesize = _filesize;
    pbitmap->fileheader.fileoffset_to_pixelarray = sizeof(bitmap_header_t);
    pbitmap->bitmapinfoheader.dibheadersize = sizeof(bitmapinfoheader) - (sizeof(uint32_t) * 3);
    pbitmap->bitmapinfoheader.width = w;
    pbitmap->bitmapinfoheader.height = h;
    pbitmap->bitmapinfoheader.planes = 1;
#ifdef CONVERT_RGB565_TO_RGB332
    pbitmap->bitmapinfoheader.bitsperpixel = 8;
#else
    pbitmap->bitmapinfoheader.bitsperpixel = _bitsperpixel;
#endif // CONVERT_RGB565_TO_RGB332
    pbitmap->bitmapinfoheader.compression = 0;
    pbitmap->bitmapinfoheader.imagesize = _pixelbytesize;
    pbitmap->bitmapinfoheader.ypixelpermeter = _ypixelpermeter;
    pbitmap->bitmapinfoheader.xpixelpermeter = _xpixelpermeter;
    pbitmap->bitmapinfoheader.numcolorspallette = 0;
    pbitmap->bitmapinfoheader.mostimpcolor = 0;
#ifdef CONVERT_RGB565_TO_RGB332
    pbitmap->bitmapinfoheader.r = 0xE0;
    pbitmap->bitmapinfoheader.g = 0x1C;
    pbitmap->bitmapinfoheader.b = 0x3;
#else
    pbitmap->bitmapinfoheader.r = 0xF800;
    pbitmap->bitmapinfoheader.g = 0x7E0;
    pbitmap->bitmapinfoheader.b = 0x1F;
#endif // CONVERT_RGB565_TO_RGB332
    return pbitmap;
}
