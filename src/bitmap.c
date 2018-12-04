#include <bitmap.h>

bitmap_header_t* bmp_create_header(int w, int h)
{
    bitmap_header_t* pbitmap = (bitmap_header_t*)calloc(1, sizeof(bitmap_header_t));
    int _pixelbytesize = w * h * 16 / 8;
    int _filesize = _pixelbytesize + sizeof(bitmap_header_t);
    strcpy((char*)pbitmap->fileheader.signature, "BM");
    pbitmap->fileheader.filesize = _filesize;
    pbitmap->fileheader.fileoffset_to_pixelarray = sizeof(bitmap_header_t);
    pbitmap->bitmapinfoheader.dibheadersize = sizeof(bitmapinfoheader) - (sizeof(uint32_t) * 3);
    pbitmap->bitmapinfoheader.width = w;
    pbitmap->bitmapinfoheader.height = h;
    pbitmap->bitmapinfoheader.planes = 1;
    pbitmap->bitmapinfoheader.bitsperpixel = _bitsperpixel;
    pbitmap->bitmapinfoheader.compression = 3;
    pbitmap->bitmapinfoheader.imagesize = _pixelbytesize;
    pbitmap->bitmapinfoheader.ypixelpermeter = _ypixelpermeter;
    pbitmap->bitmapinfoheader.xpixelpermeter = _xpixelpermeter;
    pbitmap->bitmapinfoheader.numcolorspallette = 0;
    pbitmap->bitmapinfoheader.mostimpcolor = 0;
    pbitmap->bitmapinfoheader.r = 0xF800;
    pbitmap->bitmapinfoheader.g = 0x7E0;
    pbitmap->bitmapinfoheader.b = 0x1F;
    return pbitmap;
}