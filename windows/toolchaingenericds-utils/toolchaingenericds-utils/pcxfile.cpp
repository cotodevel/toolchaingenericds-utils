/////////////////////////////////////
// PCX Loading routines for CImageDIB
/////////////////////////////////////

#include "image.h"

/////////////////////////
// Load DIB from PCX file
/////////////////////////
void CImageDIB::LoadPCX(const char *pszFilename)
{
   /* ------- Open file ------- */
	ifstream fil(pszFilename, ios::binary|ios::_Nocreate);
   //Make sure file was opened successfully
   if (fil.fail())
   {
      throw CGfxExcept(ERR_OPEN);
   }

   /* ------- Read header ------- */
   PCXFILEHEADER hdr;
   if (!fil.read((char*)&hdr, sizeof(PCXFILEHEADER)))
   {
      fil.close();
      throw CGfxExcept(ERR_READ);
   }
   //Check if this is valid PCX
   if (hdr.Manufacturer != 10)
   {
      fil.close();
      throw CGfxExcept(ERR_INVALID);
   }

   /* ------- Obtain required info and create DIB  ------- */
   DWORD width, height, paddedwidth, imageSize;
   BYTE bitcount;

   //Calculate image dimensions and bit depth
   width = DWORD(hdr.XMax - hdr.XMin + 1);
   height = DWORD(hdr.YMax - hdr.YMin + 1);
   bitcount = BYTE(hdr.BitsPerPixel * hdr.NPlanes);
   paddedwidth = BMPWIDTHBYTES(bitcount * width);

   //Calculate number of palette entries
   if (bitcount > 8)
      PalEntries = 0;
   else
      PalEntries = 1 << bitcount;

   //Calculate DIB data size including BITMAPINFOHEADER and RGBQUAD structures
   // and the size of array that will hold bitmap bits
   imageSize = paddedwidth * height;
   DibSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PalEntries + imageSize;

   //Attempt to create DIB
   if (!CreateDIB())
   {
      fil.close();
      throw CGfxExcept(ERR_CREATEDIB);
   }

   //Set bitmap info data
   pInfo->biSizeImage = imageSize;
   pInfo->biWidth = (LONG)width;
   pInfo->biHeight = (LONG)height;
   pInfo->biBitCount = (WORD)bitcount;

   /* ------- Check whether there is palette and load it in ------- */
   if (PalEntries != 0)
   {
      BYTE *pPal = new BYTE [PalEntries * 3];
      if (bitcount == 1 || bitcount == 4) //Palette info stored in file header
      {
         memcpy(pPal, &hdr.Colormap, PalEntries * 3);
      }
      else if (bitcount == 8) //256 color palette is appended to the end of file
      {
         fil.seekg(-769, ios::end);
         char dat;
         //first byte must be 12
         if (!fil.get(dat) || dat != 12)
         {
            Free();
            fil.close();
            throw CGfxExcept(ERR_PALETTE);
         }
         else //Ok to proceed
            if(!fil.read((char*)pPal, 768))
            {
               delete [] pPal;
               Free();
               fil.close();
               throw CGfxExcept(ERR_READ);
            }

         fil.seekg(sizeof(PCXFILEHEADER)); //Go to the beginning of picture data
      }

      for (short i = 0; i < PalEntries; i++) //Set RGB palette entries
      {
         pRGB[i].rgbBlue = pPal[i * 3 + 2]; //Blue
         pRGB[i].rgbGreen = pPal[i * 3 + 1]; //Green
         pRGB[i].rgbRed = pPal[i * 3]; //Red
      }
      delete [] pPal;

      CreatePal(); //Create logical palette
   }

   /* ------- Decode the image ------- */
   char data;
   int count;
   LONG l, offset = 0, linesize = LONG(hdr.NPlanes * hdr.BytesPerLine);
   BYTE *pBuff = new BYTE [linesize];
   for (LONG y = hdr.YMax; y > hdr.YMin - 1; /* Nothing in here */)
   {
      count = 1; //Default run-length

      if (!encgetc(fil, count, data)) break;

      for (char i = 0; i < count; i++)
      {
         pBuff[offset++] = data;
         if (offset >= linesize) //Reached end of scanline
         {
            if (bitcount == 24) //24-bit image scanline consists of three planes
            {
               l = paddedwidth * y;
               for (DWORD j = 0; j < width; j++)
               {
                  pBits[l + j * 3] = pBuff[j + hdr.BytesPerLine * 2]; //Blue
                  pBits[l + j * 3 + 1] = pBuff[j + hdr.BytesPerLine]; //Green
                  pBits[l + j * 3 + 2] = pBuff[j]; //Red
               }
            }
            else //Simple one-plane scanline here (same as BMP)
            {
               memcpy(&pBits[paddedwidth * y], pBuff, linesize);
            }
            offset = 0;
            y--;
         }
      }
   }
   delete [] pBuff;

   /* ------- All done ------- */
   fil.close();
}

/////////////////////////
// Save DIB in PCX format
/////////////////////////
void CImageDIB::SavePCX(const char *pszFilename)
{
   //Make sure we have some data before saving!
   if (pDibData == NULL) return;

   /* ------- Create file ------- */
   ofstream fil(pszFilename, ios::binary|ios::trunc);
   //Make sure file was created successfully
   if (fil.fail())
   {
      throw CGfxExcept(ERR_SAVE);
   }

   /* ------- Create file header ------- */
   PCXFILEHEADER hdr;
   memset(&hdr, 0, sizeof(PCXFILEHEADER));
   hdr.Manufacturer = 10; //ZSoft, of course ;)
   hdr.Version = 5;       //The latest one
   hdr.Encoding = 1;      //Run-length encoding
   hdr.PaletteInfo = 1;   // Colored/BW palette
   hdr.XMax = WORD(pInfo->biWidth - 1);
   hdr.YMax = WORD(pInfo->biHeight - 1);
   hdr.NPlanes = BYTE(pInfo->biBitCount == 24 ? 3 : 1); //Number of planes
   //Should be eight for 24 bit bitmaps
   hdr.BitsPerPixel = BYTE(pInfo->biBitCount > 8 ? 8 : pInfo->biBitCount);
   hdr.BytesPerLine = (WORD)PCXWIDTHBYTES(hdr.BitsPerPixel * pInfo->biWidth);

   /* ------- Write header ------- */
   if(!fil.write((char*)&hdr, sizeof(PCXFILEHEADER)))
   {
      fil.close();
      remove(pszFilename);
      throw CGfxExcept(ERR_WRITE);
   }

   /* ------- Encode and write image data ------- */
   DWORD paddedwidth = BMPWIDTHBYTES(pInfo->biWidth * pInfo->biBitCount);
   LONG linesize = LONG(hdr.NPlanes * hdr.BytesPerLine);

   BYTE *pBuff = new BYTE [linesize];
   for (LONG y = hdr.YMax; y > hdr.YMin - 1; y--)
   {
      //Create the scanline:
      if (pInfo->biBitCount == 24)//...Multi-plane
      {
         LONG l = paddedwidth * y;
         for (int j = 0; j < hdr.XMax + 1; j++)
         {
            pBuff[j + hdr.BytesPerLine * 2] = pBits[l + j * 3]; //Blue
            pBuff[j + hdr.BytesPerLine] = pBits[l + j * 3 + 1]; //Green
            pBuff[j] = pBits[l + j * 3 + 2]; //Red
         }
      }
      else //...Single
      {
         memcpy(pBuff, &pBits[paddedwidth * y], linesize);
      }

      //Write the scanline to the file
      if (!encline(fil, linesize, pBuff))
      {
         fil.close();
         remove(pszFilename);
         throw CGfxExcept(ERR_WRITE);
      }
   }
   delete [] pBuff;

   /* ------- Write palette ------- */
   if (pInfo->biBitCount == 8) //Append palette to the end of file for 8 bit images
   {
      //Put palette identifier
      if(!fil.put(char(12)))
      {
         fil.close();
         remove(pszFilename);
         throw CGfxExcept(ERR_WRITE);
      }
      
      for (int i = 0; i < 256; i++)
      {
         if (i >= PalEntries) //ran out of palette, so fill the rest with zeros
         {
            if(!fil.write("\0\0\0", 3))
            {
               fil.close();
               remove(pszFilename);
               throw CGfxExcept(ERR_WRITE);
            }
         }
         else
         {
            if(!fil.put(pRGB[i].rgbRed) || !fil.put(pRGB[i].rgbGreen) ||
                    !fil.put(pRGB[i].rgbBlue))
            {
               fil.close();
               remove(pszFilename);
               throw CGfxExcept(ERR_WRITE);
            }
         }
      }
   }
   else if (pInfo->biBitCount < 8) //Put palette into the header for other formats
   {
      for (short i = 0; i < PalEntries; i++)
      {
         if (i >= 48) break; //No more than 16 triples!
          
         hdr.Colormap[i * 3] = pRGB[i].rgbRed;
         hdr.Colormap[i * 3 + 1] = pRGB[i].rgbGreen;
         hdr.Colormap[i * 3 + 2] = pRGB[i].rgbBlue;
      }
      //Save header
      fil.seekp(0);
      if(!fil.write((char*)&hdr, sizeof(PCXFILEHEADER)))
      {
         fil.close();
         remove(pszFilename);
         throw CGfxExcept(ERR_WRITE);
      }
   }

   /* ------- All done ------- */
   fil.close();
}

///////////////////////////////////////
// Encode and read a character from PCX
// (from PCX documentation by ZSoft)
///////////////////////////////////////
BOOL CImageDIB::encgetc(ifstream &fil, int &cnt, char &ch)
{
   cnt = 1; //default run-length

   //get a byte from file and check its top two bits are set
   if(!fil.get(ch)) return FALSE;
   if ((0xC0 & ch) == 0xC0)
   {
      cnt = 0x3F & ch; //the remaining six bits tell us how many
      // times to duplicate the next byte
      if(!fil.get(ch)) return FALSE;
   }

   return TRUE;
}

/////////////////////////////////////
// Encode and write character to file
// (from PCX documentation by ZSoft)
/////////////////////////////////////
BOOL CImageDIB::encputc(ofstream &fil, int &cnt, char &ch)
{
   if (cnt != 0)
   {
      if ((cnt == 1) && (0xC0 != (0xC0 & ch)))
      {
         //Write data byte
         if(!fil.put(ch)) return FALSE;
         return TRUE;
      }
      else
      {
         //Write special byte
         if(!fil.put(char(0xC0 | cnt)) || !fil.put(ch)) return FALSE;
         return TRUE;
      }
   }

   return FALSE;
}

////////////////////////////////////
// Encode and write scanline to file
// (from PCX documentation by ZSoft)
////////////////////////////////////
BOOL CImageDIB::encline(ofstream &fil, int linesize, BYTE *pData)
{
   char curr, last = pData[0];
   int runCount = 1;

   for (int dec = 1; dec < linesize; dec++)
   {
      curr = pData[dec]; //Assign the character to current one
      if (curr == last) //If the current and previous match
      {
         runCount++; //Update counter
         if (runCount == 63) //the run-count value cannot exceed 00111111 in base 2,
         {                   // which is 63
            if (!encputc(fil, runCount, last)) return FALSE;
            runCount = 0;
         }
      }
      else //Characters do not match
      {
         if (runCount)
         {
            if (!encputc(fil, runCount, last)) return FALSE;
         }
         last = curr;
         runCount = 1;
      }
   }

   //Finish up
   if (runCount)
   {
      if (!encputc(fil, runCount, last)) return FALSE;
   }

   return TRUE;
}

