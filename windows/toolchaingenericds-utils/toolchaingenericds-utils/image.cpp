/////////////////////////////////
// CImageDIB class implementation
/////////////////////////////////

#include "image.h"

///////////////////
// Copy constructor
///////////////////
CImageDIB::CImageDIB (const CImageDIB &src)
{
   InitializeDIB();
   CopyDIB(src);
}

///////////////////////
// Initialize variables
///////////////////////
void CImageDIB::InitializeDIB ()
{
   pLogPal = NULL;
   pDibData = NULL;
   pBits = NULL;
   pInfo = NULL;
   pRGB = NULL;
   DibSize = 0;
}

////////////////////////////////
// Clear DIB by erasing all data
////////////////////////////////
void CImageDIB::Free()
{
   if (pDibData != NULL)
   {
      delete [] pDibData;
      pDibData = NULL;
   }
   if (pLogPal != NULL)
   {
      delete [] pLogPal;
      pLogPal = NULL;
   }
   pBits = NULL;
   pInfo = NULL;
   pRGB = NULL;
}

////////////////////////////////////
// Create logical palette, if needed
////////////////////////////////////
void CImageDIB::CreatePal ()
{
   if (PalEntries == 0) return;
   
   // Create logical palette
   pLogPal = (LOGPALETTE*) new BYTE [sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * PalEntries];
   pLogPal->palVersion = 0x300;
   pLogPal->palNumEntries = (WORD)PalEntries;
   // Set palette entries
   for (int i = 0; i < PalEntries; i++)
   {
      pLogPal->palPalEntry[i].peRed = pRGB[i].rgbRed;
      pLogPal->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
      pLogPal->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
      pLogPal->palPalEntry[i].peFlags = 0;
   }
}

/////////////////////////////////
// Create array to hold DIB data
/////////////////////////////////
BOOL CImageDIB::CreateDIB ()
{
   if (DibSize == 0) return FALSE;

   Free(); //delete previous DIB

   //Create array to hold DIB data
   pDibData = new BYTE [DibSize];
   if (pDibData == 0) return FALSE;

   //make sure array is nice and clean
   memset(pDibData, 0, DibSize);

   //Set pointer to bitmap info header
   pInfo = (BITMAPINFOHEADER*)pDibData; //Beginning of the array
   pInfo->biSize = sizeof(BITMAPINFOHEADER);
   pInfo->biPlanes = 1;

   //Palette comes next
   pRGB = (RGBQUAD*)&pDibData[sizeof(BITMAPINFOHEADER)];

   //And finally the bits
   pBits = &pDibData[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PalEntries];

   return TRUE;
}

//////////////////////////////////////
// Create bitmap and return its handle
//////////////////////////////////////
HBITMAP CImageDIB::CreateBitmap (HDC hDC)
{
   //No bits!
   if (pDibData == NULL) return NULL;

   //Create DDB and return its handle
   // do not forget to delete it when done!
   return CreateDIBitmap(hDC, pInfo, CBM_INIT, pBits, (BITMAPINFO*)pDibData, BI_RGB);
}

////////////////////////////////////////////////
// Copy device dependent bitmap to the clipboard
////////////////////////////////////////////////
BOOL CImageDIB::CopyToClipboard (HWND hWndOwner)
{
   // No data!
   if (pDibData == NULL) return FALSE;

   if (OpenClipboard(hWndOwner))
   {
      EmptyClipboard(); //Clear clipboard

      //Create device dependent bitmap
      HDC hDC = GetDC(hWndOwner);
      HBITMAP hBmp = CreateBitmap(hDC);
      ReleaseDC(hWndOwner, hDC);

      SetClipboardData(CF_BITMAP, hBmp);

      return CloseClipboard();
   }

   return FALSE;
}

////////////////////////////////////
// Blit bitmap to the destination DC
////////////////////////////////////
BOOL CImageDIB::Blit (HDC hDC, int XDest, int YDest, int Width, int Height, int XSrc, int YSrc)
{
   //No bitmap bits!
   if (pDibData == NULL) return FALSE;

   if (Width == -1)  //Use default
      Width = pInfo->biWidth;
   if (Height == -1) //Use default
      Height = pInfo->biHeight;

   //Draw image on the destination DC
   return SetDIBitsToDevice(hDC, XDest, YDest, Width, Height, XSrc, YSrc, 0,
                                Height, pBits, (BITMAPINFO*)pDibData, BI_RGB);
}

//////////////////////////////////////
// Stretch image to the destination DC
//////////////////////////////////////
BOOL CImageDIB::Stretch (HDC hDC, int XDest, int YDest, int Width, int Height,
                         int XSrc, int YSrc, int SrcWidth, int SrcHeight, DWORD rop)
{
   //No bitmap bits!
   if (pDibData == NULL) return FALSE;

   if (Width == -1)  //Use default
      Width = pInfo->biWidth;
   if (Height == -1) //Use default
      Height = pInfo->biHeight;
   if (SrcWidth == -1) //Use default
      SrcWidth = pInfo->biWidth;
   if (SrcHeight == -1) //Use default
      SrcHeight = pInfo->biHeight;

   return StretchDIBits(hDC, XDest, YDest, Width, Height, XSrc, YSrc,
                           SrcWidth, SrcHeight, pBits, (BITMAPINFO*)pInfo, BI_RGB, rop);
}

/////////////////////////////////////////
// Make current DIB a copy of another one
/////////////////////////////////////////
void CImageDIB::CopyDIB (const CImageDIB &src)
{
   //Make sure we can copy
   if (&src != this && src.DibSize != 0)
   {
      Free();
      DibSize = src.DibSize; //Copy DIB size
      PalEntries = src.PalEntries; //Copy number of palette entries
      // Attempt to create DIB
      if (!CreateDIB()) throw CGfxExcept(ERR_CREATEDIB);
      memcpy(pDibData, src.pDibData, DibSize); //Copy DIB data
      CreatePal(); //Create logical palette
   }
}

/////////////////////////////////////////////
// Utility functions; return various DIB info
/////////////////////////////////////////////
LONG CImageDIB::GetWidth () const
{
   if (pDibData == NULL)
      return 0;
   else
      return pInfo->biWidth;
}

LONG CImageDIB::GetHeight () const
{
   if (pDibData == NULL)
      return 0;
   else
      return pInfo->biHeight;
}

WORD CImageDIB::GetBitCount () const
{
   if (pDibData == NULL)
      return 0;
   else
      return pInfo->biBitCount;
}
