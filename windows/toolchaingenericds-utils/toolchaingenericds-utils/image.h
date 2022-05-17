//////////////////////////////
// FILE: image.h
// CImageDIB class declaration
//////////////////////////////

#ifndef __image_h__
#define __image_h__

#include <windows.h>
#include <fstream>
#include "gfxexcept.h"
using namespace std;

//Calculate BMP scanline size
#define BMPWIDTHBYTES(bits) (((bits + 31) >> 5) << 2)
//Calculate PCX scanline size
#define PCXWIDTHBYTES(bits) (((bits + 15) >> 4) << 1)

//PCX file header
typedef struct tagPCXFILEHEADER
{
   BYTE Manufacturer,
        Version,
        Encoding,
        BitsPerPixel;
   WORD XMin,
        YMin,
        XMax,
        YMax,
        HDpi,
        VDpi;
   BYTE Colormap[48],
        Reserved,
        NPlanes;
   WORD BytesPerLine,
        PaletteInfo,
        HScreenSize,
        VScreenSize;
   BYTE Filler[54];

} PCXFILEHEADER;

class CImageDIB
{
public:
   CImageDIB () {InitializeDIB();} //Default constructor
   CImageDIB (const CImageDIB &src); //Copy constructor
   ~CImageDIB() {Free();} //Destructor

   void Free (); //Erase all previous DIB data

   HPALETTE GetPalette() {return (HPALETTE) pLogPal;} //Get pointer to logical palette

   HBITMAP CreateBitmap (HDC hDC = NULL); //Create bitmap from DIB

   BOOL CopyToClipboard (HWND hWndOwner = NULL); //Copy bitmap to clipboard

   // Drawing functions
   BOOL Blit (HDC hDC, int XDest = 0, int YDest = 0, int Width = -1, int Height = -1, int XSrc = 0, int YSrc = 0);
   BOOL Stretch (HDC hDC, int XDest = 0, int YDest = 0, int Width = -1, int Height = -1, int XSrc = 0, int YSrc = 0, int SrcWidth = -1, int SrcHeight = -1, DWORD rop = SRCCOPY);

   // --- BMP Related ---
   void LoadBMP (const char *pszFilename);
   void SaveBMP (const char *pszFilename);

   // --- PCX Related ---
   void LoadPCX (const char *pszFilename);
   void SavePCX (const char *pszFilename);

   // Utility functions
   LONG GetWidth () const;
   LONG GetHeight () const;
   WORD GetBitCount () const;

   // Assignment operator
   void operator= (const CImageDIB &src) {CopyDIB(src);}

protected:
   void InitializeDIB (); // Init variables
   void CopyDIB (const CImageDIB &src); // Copy data from another DIB

   BOOL CreateDIB ();
   void CreatePal ();

   // --- BMP Related ---
   BOOL DecRLE8 (ifstream &fil, BYTE *pDest);
   BOOL DecRLE4 (ifstream &fil, BYTE *pDest);
   void SetByte (BYTE &Dest, BYTE &Data, BYTE &run, DWORD &col);

   // --- PCX Related ---
   BOOL encgetc(ifstream &fil, int &cnt, char &ch);
   BOOL encputc(ofstream &fil, int &cnt, char &ch);
   BOOL encline(ofstream &fil, int linesize, BYTE *pData);

   DWORD DibSize;          //Size of dib data
   int PalEntries;         //Number of palette entries
   LOGPALETTE *pLogPal;    //Logical palette
   RGBQUAD *pRGB;          //Pointer to bitmap palette
   BYTE *pDibData, *pBits; //Pointer to bitmap bits
   BITMAPINFOHEADER *pInfo;//Pointer to bitmap info

};

#endif