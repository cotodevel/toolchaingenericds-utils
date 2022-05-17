////////////////////////////////////////
// Bitmap Loading routines for CImageDIB
////////////////////////////////////////

#include "image.h"

/////////////////////////
// Load DIB from BMP file
/////////////////////////
void CImageDIB::LoadBMP(const char *pszFilename)
{
   /* ------- Open file ------- */
	ifstream fil(pszFilename, ios::binary|ios::_Nocreate);
   if (fil.fail())
   {
      throw CGfxExcept(ERR_OPEN);
   }

   /* ------- Read file headers ------- */
   BITMAPFILEHEADER BMfh;
   BITMAPINFOHEADER BMih;
   if (!fil.read((char*)&BMfh, sizeof(BITMAPFILEHEADER)) ||
         !fil.read((char*)&BMih, sizeof(BITMAPINFOHEADER)))
   {
      fil.close();
      throw CGfxExcept(ERR_READ);
   }
   //Check if a valid BMP file
   if (BMfh.bfType != 0x4D42)
   {
      fil.close();
      throw CGfxExcept(ERR_INVALID);
   }
   //Check whether this is supported bitmap
   //Non-Windows and 16 or 32 bit are not supported
   if (BMih.biSize != sizeof(BITMAPINFOHEADER) || BMih.biCompression == BI_BITFIELDS ||
          BMih.biBitCount == 16 || BMih.biBitCount == 32)
   {
      fil.close();
      throw CGfxExcept(ERR_UNSUPPORTED);
   }

   /* ------- Create DIB ------- */
   DWORD imageSize = BMPWIDTHBYTES(BMih.biWidth * BMih.biBitCount) * BMih.biHeight;

   //Calculate the number of colors used
   if (BMih.biBitCount > 8)
      PalEntries = 0;
   else if (BMih.biClrUsed != 0)
      PalEntries = BMih.biClrUsed;
   else
      PalEntries = 1 << BMih.biBitCount;

   //Calculate total DIB size
   DibSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PalEntries + imageSize;

   //Attempt to create DIB
   if (!CreateDIB())
   {
      fil.close();
      throw CGfxExcept(ERR_CREATEDIB);
   }

   //Set DIB info
   CopyMemory(pInfo, &BMih, sizeof(BITMAPINFOHEADER));
   pInfo->biCompression = BI_RGB;
   pInfo->biSizeImage = imageSize;

   /* ------- Read DIB from file ------- */
   if (PalEntries != 0) //If there is palette, read it in!
   {
      if (!fil.read((char*)pRGB, sizeof(RGBQUAD) * PalEntries))
      {
         Free();
         fil.close();
         throw CGfxExcept(ERR_READ);
      }
      CreatePal(); //Create logical palette
   }
   //Decompress image, if needed
   BOOL ret = FALSE;
   switch (BMih.biCompression)
   {
   case BI_RLE8:
      ret = DecRLE8(fil, pBits);
      break;

   case BI_RLE4:
      ret = DecRLE4(fil, pBits);
      break;

   default:
      if (fil.read((char*)pBits, imageSize)) ret = TRUE;
   }
   //Make sure there was no error
   if (!ret)
   {
      Free();
      fil.close();
      throw CGfxExcept(ERR_READ);
   }

   /* ------- All done ------- */
   fil.close();
}

///////////////////////
// Save DIB as BMP file
///////////////////////
void CImageDIB::SaveBMP(const char *pszFilename)
{
   //Make sure we have some data before saving!
   if (pDibData == NULL) return;
   
   /* ------- Create file ------- */
   ofstream fil(pszFilename, ios::binary|ios::trunc);
   if (fil.fail())
   {
      throw CGfxExcept(ERR_SAVE);
   }

   /* ------- Prepare file header ------- */
   BITMAPFILEHEADER BMfh;
   memset(&BMfh, 0, sizeof(BITMAPFILEHEADER));
   BMfh.bfType = 0x4D42;
   BMfh.bfSize = sizeof(BITMAPFILEHEADER) + DibSize;
   BMfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
     + sizeof(RGBQUAD) * PalEntries;

   /* ------- Write header and DIB to file ------- */
   if (!fil.write((char*)&BMfh, sizeof(BITMAPFILEHEADER)) ||
           !fil.write((char*)pDibData, DibSize))
   {
      fil.close();
      remove(pszFilename); //Delete file if failed
      throw CGfxExcept(ERR_WRITE);
   }

   /* ------- All done ------- */
   fil.close();
}

//////////////////////////////////////
// Decompress 8 bpp rle-encoded bitmap
//////////////////////////////////////
BOOL CImageDIB::DecRLE8(ifstream &fil, BYTE *pDest)
{
   DWORD x, y, paddedwidth = BMPWIDTHBYTES(pInfo->biWidth * pInfo->biBitCount);
   BYTE FirstByte, SecondByte;
   WORD data;

   x = y = 0;

   /* ------- Decode ------- */
   while (!fil.eof()) //Make sure we have some data to read!
   {
      // Read data
      if (!fil.read((char*)&data, 2)) return FALSE;

      // Obtain first and second byte values
      FirstByte = LOBYTE(data);
      SecondByte = HIBYTE(data);

      if (FirstByte == 0) // Second byte will be a special command
      {
         switch (SecondByte)
         {
         case 0: //End of scanline
            x = 0;
            y++;
            break;

         case 1: //End of bitmap
            return TRUE;

         case 2: //Delta
            if (!fil.read((char*)&data, 2)) return FALSE;
            x += LOBYTE(data);
            y += HIBYTE(data);
            break;

         default: //Absolute mode
            char ch;
            for (BYTE i = 0; i < SecondByte; i++)
            {
               if (!fil.get(ch)) return FALSE;
               pDest[paddedwidth * y + x++] = (BYTE)ch;
            }
            //Number of bytes read from file must be even
            if ((SecondByte % 2) == 1)
               if (!fil.get(ch)) return FALSE;
         }
      }
      else //RLE encoded
      {
         for (BYTE i = 0; i < FirstByte; i++)
            pDest[paddedwidth * y + x++] = SecondByte;
      }
   }
   
   return FALSE; //Unexpected end of file
}

//////////////////////////////////////
// Decompress 4 bpp rle-encoded bitmap
//////////////////////////////////////
BOOL CImageDIB::DecRLE4(ifstream &fil, BYTE *pDest)
{
   DWORD x, y, paddedwidth = BMPWIDTHBYTES(pInfo->biWidth * pInfo->biBitCount);
   BYTE FirstByte, SecondByte;
   WORD data;

   x = y = 0;

   /* ------- Decode ------- */
   while (!fil.eof()) //Make sure we have some data to read!
   {
      //Read data
      if (!fil.read((char*)&data, 2)) return FALSE;

      //Obtain first and second byte values
      FirstByte = LOBYTE(data);
      SecondByte = HIBYTE(data);

      if (FirstByte == 0) //Second byte will be a special command
      {
         switch(SecondByte)
         {
         case 0: //End of scanline
            x = 0;
            y++;
            break;

         case 1: //End of bitmap
            return TRUE;

         case 2: //Delta
            if (!fil.read((char*)&data, 2)) return FALSE;
            x += LOBYTE(data);
            y += HIBYTE(data);
            break;

         default: //Absolute mode
            char ch;
            for (BYTE i = 0; i < SecondByte; i++)
            {
               //Read data only on even run
               if ((i & 1) == 0)
               {
                  if (!fil.get(ch)) return FALSE;
               }
			   BYTE temp = BYTE(ch); //VC++ does not accept BYTE(ch) for some reason...
               SetByte(pDest[paddedwidth * y + x / 2], temp, i, x);
            }
            //Number of bytes read from file must be even
            if ((SecondByte & 3) == 1 || (SecondByte & 3) == 2)
               if (!fil.get(ch)) return FALSE;
         }
      }
      else //RLE encoded
      {
         for (BYTE i = 0; i < FirstByte; i++)
            SetByte(pDest[paddedwidth * y + x / 2], SecondByte, i, x);
      }
   }

   return FALSE; //Unexpected end of file
}

//////////////////////////////////
// Put color byte into 4-bit image
//////////////////////////////////
void CImageDIB::SetByte(BYTE &Dest, BYTE &Data, BYTE &run, DWORD &col)
{
   BYTE lo = BYTE(Data >> 4);
   BYTE hi = BYTE(Data & 0xF);

   if ((col & 1) == 0) //If the column is even
   {
      if ((run & 1) == 0)
         Dest = BYTE(lo << 4);
      else
         Dest = BYTE(hi << 4);
   }
   else
   {
      if ((run & 1) == 0)
         Dest |= lo;
      else
         Dest |= hi;
   }

   col++; //Move on
}
