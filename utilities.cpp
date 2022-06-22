#ifdef WIN32
#include "stdafx.h"
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#pragma warning(disable:4703)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include "winDir.h"
#include "TGDSVideoConverter/lzss9.h"
#include "TGDSVideoConverter/TGDSVideo.h"
#include "TGDSVideoConverter/bmp.h"
#include "ToolchainGenericDSFS/fatfslayerTGDS.h"
#include "ToolchainGenericDSFS/dldiWin32.h"
#include "tarball.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include "crc32Tool.h"
#include "zlib-1.2.11/zlib.h"
using namespace std; // std::cout, std::cin

#ifdef GCC
#include <dirent.h>
#endif

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#ifdef WIN32
bool cv_snprintf(char* buf, int len, const char* fmt, ...){
    va_list va;
    va_start(va, fmt);
    int res = vsnprintf((char *)buf, len, fmt, va);
    va_end(va);
#if defined _MSC_VER
    // maybe truncation maybe error 
    if(res < 0)
        //check for last errno 
    res = len -1;
    // ensure null terminating on VS2013	
#if def_MSC_VER<=1800
    buf[res] = 0; 
#endif
#endif
    return res >= 0 && res < len;
}
#endif

int convertbin2c(int argc, char *argv[] ){
	char *buf;
    char* ident;
    unsigned int i, file_size, need_comma;

    FILE *f_input, *f_output;

    if (argc < 4) {
        fprintf(stderr, "Wrong parameters. Usage: [BIN2C] binary_file output_file array_name (optional)SectionName \n", argv[0]);
        return -1;
    }

    f_input = fopen(argv[1], "rb");
    if (f_input == NULL) {
        fprintf(stderr, "%s: can't open %s for reading\n", argv[0], argv[1]);
        return -1;
    }

    // Get the file length
    fseek(f_input, 0, SEEK_END);
    file_size = ftell(f_input);
    fseek(f_input, 0, SEEK_SET);

    buf = (char *)malloc(file_size);
    assert(buf);   

    fread(buf, file_size, 1, f_input);
    fclose(f_input);

	// *.c
    f_output = fopen(argv[2], "w");
    if (f_output == NULL)
    {
        fprintf(stderr, "%s: can't open %s for writing\n", argv[0], argv[1]);
        return -1;
    }

    ident = argv[3];
    
    need_comma = 0;

	char sectionName[256];
	memset(sectionName, 0, sizeof(sectionName));
	if (argv[4] != NULL) {
		strcpy(sectionName, argv[4]);
	}
	else {
		strcpy(sectionName, "embeddedBinData");
	}

	fprintf (f_output, "#ifdef ARM9\n__attribute__((section(\".%s\")))\n#endif\nunsigned char %s[%i] = {", sectionName, ident, file_size);
    for (i = 0; i < file_size; ++i)
    {
        if (need_comma) fprintf(f_output, ", ");
        else need_comma = 1;
        if (( i % 11 ) == 0) fprintf(f_output, "\n\t");
        fprintf(f_output, "0x%.2x", buf[i] & 0xff);
    }
    fprintf(f_output, "\n};\n\n");

	char tempName[512];
	
	int offset = strlen((char*)argv[2]);
    
	#ifdef _MSC_VER
	if(offset>1){
		offset = offset - 1;
	}
	cv_snprintf(tempName, offset, "%s", (char*)argv[2]);	//ignore ending ."c"
	#endif

	#ifndef _MSC_VER
	snprintf(tempName, offset ,"%s",(char*)argv[2]);		//ignore ending ."c"
	#endif
	
	char * charEnd = (char*)(&tempName[offset-2]);
    if( *charEnd != (char)'.'){
		strcat(tempName,".");
	}
	strcat(tempName,"h");
	fprintf(f_output, "#ifdef ARM9\n__attribute__((section(\".%s\")))\n#endif\nint %s_size = sizeof(%s);\n",sectionName,ident,ident);
    fclose(f_output);

	//*.h
	f_output = fopen(tempName, "w");
    
	if (f_output == NULL)
    {
        fprintf(stderr, "can't open %s ", tempName);
        return -1;
    }
	
	fprintf(f_output, "#ifdef __cplusplus \nextern \"C\" {\n#endif\n");
	fprintf(f_output, "	extern unsigned char %s[%i];\n", ident, file_size);
	fprintf(f_output, "	extern int %s_size;\n",ident);
	fprintf(f_output, "#ifdef __cplusplus \n} \n#endif \n");
    fclose(f_output);

	return 0;
}

int convertMP4toTVS(int argc, char *argv[] ){
	//read all .mp4 files in the CWD
	sint8 name[256];
	char cwdPathMP4[256];
	getCWDWin(cwdPathMP4, "\\tvsIn\\");
	
	std::vector<std::string> TVSFilesToConvert = findFiles(std::string(cwdPathMP4), std::string("mp4"));
	
	for(vector<string>::const_iterator it = TVSFilesToConvert.begin(); it != TVSFilesToConvert.end(); ++it) {
		std::string file = getFileName(string(*it), false);
		int nextVideoFrameOffset = -1;
		int nextVideoFrameFileSize = -1;
		u8 * rawVideoFramedeCompressed = NULL;

		sint8 name[256];
		char cwdPath[256];
		getCWDWin(cwdPath, "\\bmpFrames\\");

		std::vector<std::string> BMPframesToClean = findFiles(std::string(cwdPath), std::string("bmp"));
		for(vector<string>::const_iterator itFrame = BMPframesToClean.begin(); itFrame != BMPframesToClean.end(); ++itFrame) {
			std::string frameDeleted = string(*itFrame);
			int res2 = std::remove(frameDeleted.c_str());
		}
		
		std::vector<std::string> IMAAudioToClean = findFiles(std::string(cwdPath), std::string("ima"));
		for(vector<string>::const_iterator itAudio = IMAAudioToClean.begin(); itAudio != IMAAudioToClean.end(); ++itAudio) {
			std::string audioDeleted = string(*itAudio);
			int res2 = std::remove(audioDeleted.c_str());
		}
		
		std::string cmd = string("ffmpeg -async 20 -i tvsIn/"+ file+" -vf scale=256:192,fps=fps=10 -pix_fmt rgb24 -y bmpFrames/yo%03d.bmp -f wav -acodec adpcm_ima_wav -ar 22050 bmpFrames/audio.ima");
		//FFmpeg convert
		system(cmd.c_str());

		////////////////////////////////////////////////////////TVS start

		char cwdPathOut[256];
		getCWDWin(cwdPathOut, "\\tvsout\\");
	
		//Generate TVS stream
		std::vector<std::string> audioTrackFound = findFiles(std::string(cwdPath), std::string("ima"));
		std::vector<std::string> BMPframesFound = findFiles(std::string(cwdPath), std::string("bmp"));
		printf("Current Directory:\n%s", cwdPath);
		if(audioTrackFound.size() <= 0){
			printf("audio track (IMA ADPCM) not found");
			return -1;
		}
		int ret = generateTGDSVideoformatFromBMPDir(BMPframesFound, std::string(cwdPathOut), audioTrackFound.at(0), getFileNameNoExtension(file));
		printf("Processed %d BMP frames\n", ret);
	}

	return 0;
}

/*
//Unit test: consume TVS stream using NDS DLDI FAT Driver
strcat(cwdPath, "fileOut.tvs");
FILE * outFileGen = fopen(cwdPath, "rb");
if(outFileGen != NULL){
	fseek(outFileGen, 0, SEEK_END);
	int TVSFileSize = ftell(outFileGen);
	fseek(outFileGen, 0, SEEK_SET);
		
	u8 * TVSFileStream = (u8*)malloc(TVSFileSize);
	if(fread(TVSFileStream, 1, TVSFileSize, outFileGen) == TVSFileSize){
		
		///////////////////////////////////////////DLDI testcase start ///////////////////////////////////////
		char cwdPath[256];
		getCWDWin(cwdPath, "\\..\\virtualDLDI\\");
		std::vector<std::string> virtualDiskImgFiles = findFiles(std::string(cwdPath), std::string("img"));
		std::string virtualDiskImgFile = virtualDiskImgFiles.at(0);
	
		std::vector<std::string> virtualDldiFiles = findFiles(std::string(cwdPath), std::string("dldi"));
		std::string virtualDldiFile = virtualDldiFiles.at(0);
	
		virtualDLDIDISKImg = fopen(virtualDiskImgFile.c_str(), "rb+");
		FILE * virtualDLDIFH = fopen(virtualDldiFile.c_str(), "rb");

		if((virtualDLDIDISKImg != NULL) && (virtualDLDIFH != NULL)){
			printf("file: %s open OK\n\n\n\n", virtualDiskImgFile.c_str());
			printf("file: %s open OK\n\n\n\n", virtualDldiFile.c_str());

			if(fread((char*)&_io_dldi_stub[0], 1, sizeof(_io_dldi_stub), virtualDLDIFH) == sizeof(_io_dldi_stub)){
				printf("read DLDI OK.");

				//Initialize TGDS FS
				int ret=FS_init();
				if (ret == 0)
				{
					printf("FS Init ok.");
						
					//DLDI tasks

					//sort list alphabetically
					struct FileClassList * playlistfileClassListCtx = NULL;
					playlistfileClassListCtx = initFileList();
					cleanFileList(playlistfileClassListCtx);
					bool ret = readDirectoryIntoFileClass("/", playlistfileClassListCtx);
					if(ret == true){
						char scratchPadMem[76800];
						bool ignoreFirstFileClass = true;
						sortFileClassListAsc(playlistfileClassListCtx, (char**)&scratchPadMem[0], ignoreFirstFileClass);
					}
					else{
						printf("fail");
					}

					//remove("0:/fileOut.tvs");

					//copy fileOut.tvs to disk image
					int tgdsfd = -1;
					//int res = TGDSFSUserfatfs_open_file("0:/fileOut.tvs", "w+", &tgdsfd);
					//if(res >= 0){
					//	struct fd * fdOpened = getStructFD(tgdsfd);
					//	//char bufferTestToRead[512];
					//	//int read = ARM7FS_ReadBuffer_ARM9ImplementationTGDSFD((u8*)&bufferTestToRead[0], 0, fdOpened, 100);
					//	//printf("%s ", bufferTestToRead);
					//	int fileOffset = 0;
					//	//int writtenTest = fatfs_write(fdOpened->cur_entry.d_ino, (u8 *)TVSFileStream, TVSFileSize);
					//	fsync(tgdsfd);
					//	int res = TGDSFSUserfatfs_close(fdOpened);
					//	printf("");
					//}
					
					//read it
					tgdsfd = -1;
					char * tvsFile = "0:/fileOut.tvs";
					fatfs_open_fileIntoTargetStructFD(tvsFile, "r", &tgdsfd, NULL);
					videoHandleFD = getStructFD(tgdsfd);
					if(parseTGDSVideoFile(videoHandleFD, tvsFile) > 0){
						int readFileSize = FS_getFileSizeFromOpenStructFD(videoHandleFD);
						int predictedClusterCount = (readFileSize / (getDiskClusterSize() * getDiskSectorSize())) + 2;
						nextVideoFrameOffset = TGDSVideoFrameContextReference->videoFrameStartFileOffset;
						nextVideoFrameFileSize = TGDSVideoFrameContextReference->videoFrameStartFileSize;			
						TGDSVideoPlayback = true;

						//render each frame
						nextVideoFrameOffset = TGDSVideoFrameContextReference->videoFrameStartFileOffset;
						nextVideoFrameFileSize = TGDSVideoFrameContextReference->videoFrameStartFileSize;
								
						//some vblanks
						vblankCount++;
						vblankCount++;

						TGDSVideoPlayback = true; //render
						for (int j = 0; j < TGDSVideoFrameContextReference->videoFramesTotalCount; j++){	
							TGDSVideoRender();
							printf("debug videoFrame: %d", j);
							vblankCount++;
						}

						printf("\nFrame decoding ended.");
					}
					else{
						TGDSVideoPlayback = false;
						printf(".TVS File: %s not found ", tvsFile);
						while(1==1){
						}
					}
				}
				else
				{
					printf("FS Init error.");
				}
			}
			else{
				printf("read DLDI FAIL");
			}
			fclose(virtualDLDIDISKImg);
			fclose(virtualDLDIFH);
		}
		else{
			printf("failure opening: %s", virtualDiskImgFile.c_str());
			printf("failure opening: %s", virtualDldiFile.c_str());
		}
		///////////////////////////////////////////DLDI testcase end ///////////////////////////////////////
	}
	else{
		printf("failure reading TVS generated");
		while(1==1){}
	}
	free(TVSFileStream);
	fclose(outFileGen);
}
else{
	printf("Not a .TVS File. ");
	while(1==1){}
}
*/
////////////////////////////////////////////////////////TVS end

static void TGDSPackageBuilderHelp(){
	printf("Usage: \n");
	printf(" TGDSProjectName [/baseTargetDecompressorDirectory] [/TGDSLibrarySourceDirectory] [/TGDSPKGOutDirectory] [/TGDSProjectSourceFolder] \n");
}

#ifdef __cplusplus
extern "C" {
#endif
//Build the TAR
extern int file_compress(char  *file, char  *mode);
#ifdef __cplusplus
}
#endif

int TGDSPKGBuilder(int argc, char *argv[] ){
	int k;
	
	//printf("DEBUGGER: argv 0: %s\n", argv[0]);	// C:\toolchain_generic\6.2_2016q4\bin\TGDSPKGBuilder.exe
	//printf("DEBUGGER: argv 1: %s\n", argv[1]);	// ToolchainGenericDS-template
	//printf("DEBUGGER: argv 2: %s\n", argv[2]);	// TGDSPKG_template
	//printf("DEBUGGER: argv 3: %s\n", argv[3]);	// c:/toolchain_generic/6.2_2016q4/arm-eabi/lib/
	//printf("DEBUGGER: argv 5: %s\n", argv[4]);	// /release/arm7dldi-ntr
	
	/* avoid end-of-line conversions */
    	SET_BINARY_MODE(stdin);
    	SET_BINARY_MODE(stdout);

	if (argc < 3){
		TGDSPackageBuilderHelp();
		return -1;
	}

	// "/TGDSLibrarySourceDirectory"
	char TGDSLibrarySourceDirectory[256+1];
	strcpy(TGDSLibrarySourceDirectory, argv[3]); //strcpy(TGDSLibrarySourceDirectory, "C:\\toolchain_generic\\6.2_2016q4\\arm-eabi\\lib\\newlib-nano-2.1-nds\\");

	// "/baseTargetDecompressorDirectory"
	char baseTargetDecompressorDirectory[256+1];
	if((argv[2] != NULL) && (strlen(argv[2]) > 1)){
		strcpy(baseTargetDecompressorDirectory, argv[2]); //strcpy(baseTargetDecompressorDirectory, "TGDSbaseTargetDecompressorDirectory/");
		strcat(baseTargetDecompressorDirectory, "/");
	}
	else{
		strcpy(baseTargetDecompressorDirectory, "");
	}
	
	char TGDSProjectName[256+1];
	strcpy(TGDSProjectName, argv[1]); //strcpy(TGDSProjectName, "ToolchainGenericDS-template");
	char TGDSMainApp[256+1];
	strcpy(TGDSMainApp, TGDSProjectName);
	strcat(TGDSMainApp, ".nds");
	
	//ok

	//Output Directory
	char converted[256];	
	char outputPKGPath[256];
	getcwd(converted, sizeof(converted));
	strcpy(outputPKGPath, (string(converted) + string("\\")).c_str() ); //strcpy(outputPKGPath, (string(converted) + string("\\..\\Debug")).c_str() );
	
	//ok

	printf("Source files Directory: %s\n", argv[4]);
	std::vector<std::string> vec = list_directory(string(converted) + string(argv[4]));	
	printf("Source files Count: %d\n", vec.size());
	
	int crc32mainApp =-1;
	int crc32TGDSSDK =-1;

	/* open file for writing */
	char fullPathOuttar[256+1];
		//strcpy(fullPathOuttar, "..\\Debug\\");
		//strcat(fullPathOuttar, (string(TGDSProjectName) + string(".tar")).c_str());
		
		memset(fullPathOuttar, 0, sizeof(fullPathOuttar));
		strcpy(fullPathOuttar, outputPKGPath);
		strcat(fullPathOuttar, (string(argv[1]) + string(".tar")).c_str());

	
	/* open file for writing */
	std::fstream out(fullPathOuttar,std::ios::out | std::ios::binary);
	printf("Tar Out: %s\n", fullPathOuttar);
	
	//ok

	if(!out.is_open())
		{
		std::cerr << "Cannot open out: " << string(fullPathOuttar) << std::endl;
		return EXIT_FAILURE;
		}
	/* create the tar file */
	lindenb::io::Tar tarball(out);
	
	for(int i = 0; i < vec.size(); i++){
		char * filename = (char*)vec.at(i).c_str();
		if( (string(filename) != "..") && ((string(TGDSProjectName) + string(".tar")) != string(filename)) && ((string(TGDSProjectName) + string(".tar.gz")) != string(filename)) ){
			
			/* add a file */
			char fullPathIn[256+1];
			//strcpy(fullPathIn, "..\\Debug\\");
			//strcat(fullPathIn, filename);

			strcpy(fullPathIn, outputPKGPath);
			strcat(fullPathIn, (argv[4] + 1));
			strcat(fullPathIn, filename);
			
			printf("TAR: Add File: %d: %s \n", i, fullPathIn);
			printf("into: [%s] \n", (string(baseTargetDecompressorDirectory) + string(filename)).c_str());
			
			tarball.putFile(fullPathIn, (string(baseTargetDecompressorDirectory) + string(filename)).c_str());
			
			//Found mainApp?
			if(string(TGDSMainApp) == string(filename)){
				//unsigned long crc32 = -1;
				FILE* TGDSLibraryFile = fopen(fullPathIn,"rb");
				int err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32mainApp);
				fclose(TGDSLibraryFile);

				printf("mainApp[%s] CRC32: %x\n", filename, crc32mainApp);
			}
			
		}
	}
	
	//ok
	
	//crc32TGDSSDK
	//libcnano7.a
	//libcnano9.a
	//libtoolchaingen7.a
	//libtoolchaingen9.a
	int crc32TGDSSDKlibcnano7 = -1;
	int crc32TGDSSDKlibcnano9 = -1;
	int crc32TGDSSDKlibtoolchaingen7 = -1;
	int crc32TGDSSDKlibtoolchaingen9 = -1;
	int err = 0;
	FILE* TGDSLibraryFile = NULL;
	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("\\") + string("libcnano7.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){
		err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32TGDSSDKlibcnano7);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libcnano7.a missing. Make sure you build newlib-nds first!");
		return -1;
	}

	
	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("\\") + string("libcnano9.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32TGDSSDKlibcnano9);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libcnano9.a missing. Make sure you build newlib-nds first!");
		return -1;
	}

	
	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("\\") + string("libtoolchaingen7.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32TGDSSDKlibtoolchaingen7);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libtoolchaingen7.a missing. Make sure you build ToolchainGenericDS first!");
		return -1;
	}

	
	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("\\") + string("libtoolchaingen9.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32TGDSSDKlibtoolchaingen9);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libtoolchaingen9.a missing. Make sure you build ToolchainGenericDS first!");
		return -1;
	}
	//ok

	/* Write the descriptor */
	char TGDSDescriptorBuffer[256+1];
	sprintf(TGDSDescriptorBuffer, "[Global]\n\nmainApp = %s\n\nmainAppCRC32 = %x\n\nTGDSSdkCrc32 = %x\n\nbaseTargetPath = %s\n\n", TGDSMainApp, crc32mainApp, (crc32TGDSSDKlibcnano7 + crc32TGDSSDKlibcnano9 + crc32TGDSSDKlibtoolchaingen7 + crc32TGDSSDKlibtoolchaingen9), baseTargetDecompressorDirectory);
	
	try{
		tarball.put( (string("descriptor.txt")).c_str(), TGDSDescriptorBuffer);
	}
	catch(exception ex){
		printf("descriptor.txt creating fail");
		return -1;
	}

	/* finalize the tar file */
	tarball.finish();
	/* close the file */
	out.close();
	/* we're done */

	//ok

	//gz zip the tarball
	if(file_compress(fullPathOuttar, "w+b") == Z_OK){
		rename(fullPathOuttar, (string(fullPathOuttar)+string(".gz")).c_str());
		printf("TGDSPKG %s build OK \n", (string(fullPathOuttar)+string(".gz")).c_str());
	}
	else
	{
		printf("TGDSPKG %s build ERROR \n", (string(fullPathOuttar)+string(".gz")).c_str());
	}

    return 0;
}
