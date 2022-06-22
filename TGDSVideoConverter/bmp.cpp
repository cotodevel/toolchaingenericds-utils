//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)

/*
 * bmp.h
 *
 *  Created on: 2019. febr. 17.
 *      Author: Benjami
 * https://www.vbforums.com/showthread.php?261522-C-C-Loading-Bitmap-Files-%28Manually%29
 */

#if defined (MSDOS) || defined(OS2) || defined(WIN32) || defined(_CYGWIN_)
#include <fcntl.h>
#include<io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), _O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif


#if defined(WIN32)
#include <windows.h> // WinApi header
#include "shlwapi.h"
#endif

#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std; // std::cout, std::cin
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bmp.h"
#include "../winDir.h"
#include "TGDSVideo.h"
#include "lzss9.h"
#include <iostream>
#include <cmath>

uint8_t bmpBuffer[MAX_BMP_FILESIZE];

double round(double d)
{
	return floor(d + 0.5);
}

uint16_t color_24_to_16_s(color_rgb_s color)
{
    uint16_t cc=0;
	cc|=(((int)round((color.r*(float)0.1215686)))&0x1f)<<10;
	cc|=(((int)round((color.b*(float)0.1215686)))&0x1f)<<5 ;
    cc|=(((int)round((color.g*(float)0.1215686)))&0x1f)<<0 ;
    return cc;
}

unsigned char *LoadBitmapFile(char *filename, struct BMPBITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *filePtr; //our file pointer
    tagBMPBITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
    unsigned char *bitmapImage;  //store image data
    int imageIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable

    //open filename in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
        return NULL;

    //read the bitmap file header
    fread(&bitmapFileHeader, sizeof(tagBMPBITMAPFILEHEADER),1,filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.bfType !=0x4D42)
    {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(tagBMPBITMAPINFOHEADER),1,filePtr); // small edit. forgot to add the closing bracket at sizeof

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char*)&bmpBuffer[0];

	if(bitmapInfoHeader->biSizeImage > MAX_BMP_FILESIZE){
		fclose(filePtr);
		return NULL;
	}

    //read in the bitmap image data
    fread(bitmapImage,bitmapInfoHeader->biSizeImage,1,filePtr);

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        return NULL;
    }

    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0; imageIdx < (int)(bitmapInfoHeader->biSizeImage);imageIdx+=3) // fixed semicolon
    {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    //close file and return bitmap iamge data
    fclose(filePtr);
    return bitmapImage;
}

unsigned char SaveBitmap24File(char * name,uint16_t width,uint16_t height,unsigned char * Buffer)
{
    int i,j,k;
    uint8_t *Buf,rname[64];
    FILE *img;
    uint32_t ImgMByte=(((width*3)+(width%4))*height);

    struct BMPBITMAPFILEHEADER info;
    struct BMPBITMAPINFOHEADER header;
    
	memset(&info,0,sizeof(info));

    info.bfType=0x4d42;
    info.bfSize=0x36+ImgMByte;
    info.bfOffBits=0x36;

    memset(&header,0,sizeof(header));

    header.biSize=0x28;
    header.biWidth=width;
    header.biHeight=height;
    header.biPlanes=0x01;
    header.biBitCount=0x18;
    header.biSizeImage=ImgMByte;
    header.biXPelsPerMeter=0x0b13;
    header.biYPelsPerMeter=0x0b13;

    sprintf((char*)rname,"%s.bmp",name);
    img=fopen((char*)rname,"wb");
    if(img==NULL)return 0;

    Buf=(uint8_t *)calloc(ImgMByte,sizeof(unsigned char));
    memset(Buf,0x00,ImgMByte);
    
    k=width%4;

    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            Buf[(i*((width*3)+(k)))+(j*3)+0]=Buffer[(i*width*3)+(j*3)+0];
            Buf[(i*((width*3)+(k)))+(j*3)+1]=Buffer[(i*width*3)+(j*3)+1];
            Buf[(i*((width*3)+(k)))+(j*3)+2]=Buffer[(i*width*3)+(j*3)+2];
        }
    }

    fwrite(&info, 1, sizeof(struct BMPBITMAPFILEHEADER), img);
    fwrite(&header, 1, sizeof(struct BMPBITMAPINFOHEADER), img);
    fwrite(Buf, 1, ImgMByte, img);

    fclose(img);
    free(Buf);
    return 1;
}

int generateTGDSVideoformatFromBMPDir(const std::vector<std::string> BMPFrames, std::string outDir, std::string imaTrack, std::string targetFilename){
	struct TGDSVideoFrameContext TGDSVideoFrameContextOut;
	int bmpFramesDone=0;
	int lastVideoFrameOffset=-1;
	int lastVideoFrameSize=-1;
	int nextVideoFrameOffset=-1;
	int nextVideoFrameSize=-1;
	char outPath[256];
	strcpy(outPath, (outDir + string(targetFilename) + string(".tvs")).c_str());
	FILE *out=fopen((char*)outPath,"wb+");
	int fileOutOffset = 0;
	
	//Write last
	fileOutOffset+=(sizeof(struct TGDSVideoFrameContext));
	//point to u32 * frameOffsetCollection
	//fileOutOffset-=(4);
	fseek(out, fileOutOffset, SEEK_SET);
	
	//Generate how much frameOffsetCollection items there will be + create frameOffsetCollection in FILE Handle
	int frameCollectionItems = BMPFrames.size();
	u32 * frameCollection = (u32 *)malloc(frameCollectionItems*sizeof(u32));
	fileOutOffset+=(frameCollectionItems*sizeof(u32));
	fseek(out, fileOutOffset, SEEK_SET);

	for(vector<string>::const_iterator it = BMPFrames.begin(); it != BMPFrames.end(); ++it) {
		char FileName[256]; //full path filename
		unsigned char *BMP_Data,*Buffer;
		struct BMPBITMAPINFOHEADER BMP_Header;
		uint16_t BMP_WidthByteSize,BMP_Width,BMP_Hight;
		std::string file = string(*it);
		int i=0,j=0,k=0;
	
		strcpy(FileName, file.c_str());
		BMP_Data=LoadBitmapFile((char*)FileName,&BMP_Header);
		if(BMP_Data==NULL){
			printf("ERROR: Can't Open %s / Not 256x192 nor BMP24 / BMP exceeds max filesize : %d \r\n", FileName, MAX_BMP_FILESIZE);
			return 1;
		}
		//Generate RGB565 16-bit frame here
		BMP_Width=BMP_Header.biWidth;
		BMP_Hight=BMP_Header.biHeight;
		printf("Size = %d X %d\r\n",BMP_Width,BMP_Hight);
		BMP_WidthByteSize=BMP_Width*3;
		Buffer=(unsigned char *)calloc(BMP_WidthByteSize,sizeof(unsigned char));
		u16 * frameBuffer = (u16 *)malloc(NDS_HEIGHT*NDS_WIDTH*2);
		int frameBufferOffset = 0;
		for(j=1;j<=BMP_Hight;j++){
			memcpy(Buffer,&BMP_Data[j*((-1*BMP_WidthByteSize)-(BMP_Width%4))+ BMP_Header.biSizeImage],BMP_WidthByteSize);
			for(i=0;i<BMP_WidthByteSize;i+=3){
				color_rgb_s RGBColor;
				RGBColor.g=Buffer[i];
				RGBColor.b=Buffer[i+1];
				RGBColor.r=Buffer[i+2];
				uint16_t Color=color_24_to_16_s(RGBColor);
				uint8_t red = (Color & 31) << 3;
				uint8_t green = ((Color >> 5) & 31) << 3;
				uint8_t blue = ((Color >> 10) & 31) << 3;
				uint16_t ColorGBA = (uint16_t)(((red >> 3) & 31) | (((green >> 3) & 31) << 5) | (((blue >> 3) & 31) << 10));
				frameBuffer[frameBufferOffset] = ColorGBA;
				if(k>=16){
					k=0;
				}
				else {
					k++;
				}
				frameBufferOffset++;
			}
		}
		
		//LZSS Compress it
		int compressedFrameSize = 0;
		u32 compressedBufferGenerated = NULL;
		LZS_EncodeFromBuffer((unsigned char *)frameBuffer,(unsigned int)frameBufferOffset*2, (unsigned char*)&compressedBufferGenerated, (unsigned int*)&compressedFrameSize);
		if(compressedBufferGenerated!= NULL){
			//reserve struct videoFrame
			fileOutOffset+=(sizeof(struct videoFrame));
			fseek(out, fileOutOffset, SEEK_SET);

			compressedFrameSize = (compressedFrameSize + (4 - 1)) & -4; //align videoFrame boundary to 4 bytes
			
			//write fileOffset collection
			char * compressedBufferGeneratedPtr = (char*)compressedBufferGenerated;
			if(fwrite(compressedBufferGeneratedPtr, 1, compressedFrameSize, out) == compressedFrameSize){
				frameCollection[bmpFramesDone] = (int)(ftell(out) - compressedFrameSize - sizeof(struct videoFrame));

				//copy last videoFrame offset and videoFrame size into struct videoFrame *
				if(bmpFramesDone > 0){
					lastVideoFrameOffset = frameCollection[bmpFramesDone-1];
					
					struct videoFrame lastVideoFrame;
					memset(&lastVideoFrame, 0, sizeof(struct videoFrame));
					int curPos = ftell(out);
					fseek(out, (int)lastVideoFrameOffset, SEEK_SET);
					fread((u8*)&lastVideoFrame, 1, sizeof(struct videoFrame), out);
					lastVideoFrameSize = lastVideoFrame.frameSize;
					fseek(out, (int)curPos, SEEK_SET);
				}
				else{
					lastVideoFrameOffset = -1;
					lastVideoFrameSize = -1;
				}

				struct videoFrame processedVideoFrame;
				memset(&processedVideoFrame, 0, sizeof(struct videoFrame));
				processedVideoFrame.currentFrameIndex = bmpFramesDone;
				processedVideoFrame.frameSize = compressedFrameSize;
				processedVideoFrame.LZSSWRAMCompressedFormat = TGDSVideoFrameLZSSWRAMCompression;
				processedVideoFrame.rawVideoFrame = (u32*)(ftell(out) - compressedFrameSize); //VideoFrame saved physically into file, then get offset, save it here.
				processedVideoFrame.lastVideoFrameOffsetInFile = lastVideoFrameOffset;
				processedVideoFrame.lastVideoFrameFileSize = lastVideoFrameSize;
				fseek(out, (int)frameCollection[bmpFramesDone], SEEK_SET);
				if(fwrite((u8*)&processedVideoFrame, 1, sizeof(struct videoFrame), out) == sizeof(struct videoFrame)){
					free((char*)compressedBufferGenerated);	
					fileOutOffset+=(compressedFrameSize + sizeof(struct videoFrame));
					fseek(out, fileOutOffset, SEEK_SET); //end of physical compressed videoFrame

					printf("Processed %s (%d)\r\n", file.c_str(),bmpFramesDone);
				}
				else{
					printf("Error saving %d Frame\r\n", bmpFramesDone);
				}
			}
			else{
				printf("Error saving %d Frame\r\n", bmpFramesDone);
			}
		}
		else{
			printf("Error saving %d Frame\r\n", bmpFramesDone);
		}

		free(frameBuffer);
		free(Buffer);
		bmpFramesDone++;
	}

	//generate TGDSVideoplayer context
	memset(&TGDSVideoFrameContextOut, 0, sizeof(struct TGDSVideoFrameContext));
	TGDSVideoFrameContextOut.framesPerSecond = 15;
	TGDSVideoFrameContextOut.videoFramesTotalCount = bmpFramesDone;
	
	if(frameCollectionItems <= 0){
		TGDSVideoFrameContextOut.videoFrameStartFileOffset = -1;
	}
	else{
		TGDSVideoFrameContextOut.videoFrameStartFileOffset = frameCollection[0];
		fseek(out, TGDSVideoFrameContextOut.videoFrameStartFileOffset, SEEK_SET);
		struct videoFrame tempVideoFrame;
		memset(&tempVideoFrame, 0, sizeof(struct videoFrame));
		fread(&tempVideoFrame, 1, sizeof(struct videoFrame), out);
		TGDSVideoFrameContextOut.videoFrameStartFileSize = tempVideoFrame.frameSize;
	}

	fileOutOffset = 0;
	fseek(out, fileOutOffset, SEEK_SET);
	fwrite((char*)&TGDSVideoFrameContextOut, 1, sizeof(struct TGDSVideoFrameContext) - 4, out);

	//write physical fileOffset collection
	for (int j = 0; j < frameCollectionItems; j++){
		u32 structVideoFrameOffsetInFile = frameCollection[j];
		fwrite((char*)&structVideoFrameOffsetInFile, 1, 4, out);

		//now go to each struct videoFrame * generated and add nextFrame offsets
		//processedVideoFrame.nextVideoFrameOffsetInFile = nextVideoFrameOffset;
		if((j+1) < frameCollectionItems){
			nextVideoFrameOffset = frameCollection[j+1];

			struct videoFrame nextVideoFrame;
			memset(&nextVideoFrame, 0, sizeof(struct videoFrame));
			int curPos = ftell(out);
			fseek(out, (int)nextVideoFrameOffset, SEEK_SET);
			fread((u8*)&nextVideoFrame, 1, sizeof(struct videoFrame), out);
			nextVideoFrameSize = nextVideoFrame.frameSize;
			fseek(out, (int)curPos, SEEK_SET);

		}
		else{
			nextVideoFrameOffset = -1;
			nextVideoFrameSize = -1;
		}

		int curPos = ftell(out);
		fseek(out, structVideoFrameOffsetInFile, SEEK_SET);
		struct videoFrame processedVideoFrame;
		memset(&processedVideoFrame, 0, sizeof(struct videoFrame));
		fread(&processedVideoFrame, 1, sizeof(struct videoFrame), out);
		processedVideoFrame.nextVideoFrameOffsetInFile = nextVideoFrameOffset;
		processedVideoFrame.nextVideoFrameFileSize = nextVideoFrameSize;
		fseek(out, structVideoFrameOffsetInFile, SEEK_SET); //rewind and write
		fwrite(&processedVideoFrame, 1, sizeof(struct videoFrame), out);
		fseek(out, curPos, SEEK_SET);
	}
	
	fclose(out);
	free(frameCollection);

	//Copy audio track
	ifstream src;
	ofstream dst;

	src.open(imaTrack, ios::in | ios::binary);
	dst.open((outDir + targetFilename + string(".ima")), ios::out | ios::binary);
	dst << src.rdbuf();

	src.close();
	dst.close();

	printf("End\r\n");
	return bmpFramesDone;
}
