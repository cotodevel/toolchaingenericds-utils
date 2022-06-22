/*
 * bmp.h
 *
 *  Created on: 2019. febr. 17.
 *      Author: Benjami
 * https://www.vbforums.com/showthread.php?261522-C-C-Loading-Bitmap-Files-%28Manually%29
 */

#ifdef __cplusplus
#include <vector>
#endif
#include "TGDSTypes.h"

#ifndef __BMPI_H_
#define __BMPI_H_

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#define MAX_BMP_FILESIZE (int)(1*1024*1024)

//WIN32
#ifdef _MSC_VER
typedef struct BMPBITMAPFILEHEADER { PACK(
  uint16_t   bfType;        //bitmap id
  uint32_t   bfSize;        //The size of the BMP file in bytes
  uint16_t   bfReserved1;
  uint16_t   bfReserved2;
  uint32_t   bfOffBits;
  )} tagBMPBITMAPFILEHEADER; 
  // size is 14 bytes

typedef struct BMPBITMAPINFOHEADER { PACK(
  uint32_t  biSize;           //the size of this header, in bytes (40)
  uint32_t  biWidth;          //the bitmap width in pixels (signed integer)
  uint32_t  biHeight;         //the bitmap height in pixels (signed integer)
  uint16_t  biPlanes;         //the number of color planes (must be 1)
  uint16_t  biBitCount;       //the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32. 
  uint32_t  biCompression;
  uint32_t  biSizeImage;      //the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps. 
  uint32_t  biXPelsPerMeter;  //the horizontal resolution of the image
  uint32_t  biYPelsPerMeter;  //the vertical resolution of the image
  uint32_t  biClrUsed;
  uint32_t  biClrImportant;
  )} tagBMPBITMAPINFOHEADER;  
 // size is 40 bytes
#endif

//GCC
#ifndef _MSC_VER
typedef struct BMPBITMAPFILEHEADER { 
  uint16_t   bfType;        //bitmap id
  uint32_t   bfSize;        //The size of the BMP file in bytes
  uint16_t   bfReserved1;
  uint16_t   bfReserved2;
  uint32_t   bfOffBits;
  } tagBMPBITMAPFILEHEADER  __attribute__((__packed__)) ; 
  // size is 14 bytes

typedef struct BMPBITMAPINFOHEADER {
  uint32_t  biSize;           //the size of this header, in bytes (40)
  uint32_t  biWidth;          //the bitmap width in pixels (signed integer)
  uint32_t  biHeight;         //the bitmap height in pixels (signed integer)
  uint16_t  biPlanes;         //the number of color planes (must be 1)
  uint16_t  biBitCount;       //the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32. 
  uint32_t  biCompression;
  uint32_t  biSizeImage;      //the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps. 
  uint32_t  biXPelsPerMeter;  //the horizontal resolution of the image
  uint32_t  biYPelsPerMeter;  //the vertical resolution of the image
  uint32_t  biClrUsed;
  uint32_t  biClrImportant;
  } tagBMPBITMAPINFOHEADER __attribute__((__packed__)) ;  
 // size is 40 bytes
#endif

typedef struct{
    uint8_t r;
    uint8_t g;
    uint8_t b;
}color_rgb888_s;
typedef color_rgb888_s color_rgb_s;

extern unsigned char *LoadBitmapFile(char *filename, struct BMPBITMAPINFOHEADER *bitmapInfoHeader);
extern unsigned char SaveBitmap24File(char * name,uint16_t width,uint16_t height,unsigned char * Buffer);

#endif /* __BMP_H_ */

#ifdef __cplusplus

extern int generateTGDSVideoformatFromBMPDir(const std::vector<std::string> BMPFrames, std::string outDir, std::string imaTrack, std::string targetFilename);
extern uint8_t bmpBuffer[MAX_BMP_FILESIZE];
extern uint16_t color_24_to_16_s(color_rgb_s color);

#ifdef WIN32
extern double round(double d);
#endif

#endif
