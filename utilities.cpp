/*
#ifdef WIN32
#include "stdafx.h"
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#pragma warning(disable:4703)
#endif
*/

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include "http/server.h"

#ifdef GCC
#include <dirent.h>
#endif

//network
#if !defined(WIN32)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

using namespace std; // std::cout, std::cin

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

#if defined(WIN32) || !defined(ARM9)
bool existFilePosix(char *fname){
	bool exists = false;
	FILE * f = fopen(fname, "rb"); 
    if (f != NULL) { 
        exists = true; 
        fclose(f);
    }
    return exists;
}
#endif

//deps from TGDS not included in TGDS-utils
#if !defined(ARM9)
int	FS_getFileSize(char *filename){	
	FILE * f = fopen(filename, "rb");
	if (f == NULL){
		return -1;
	}
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fclose(f);
	return size;
}

//Server:
//Open a port and listen through it. Synchronous/blocking.
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
int openServerSyncConn(int SyncPort, struct sockaddr_in * sain){
	int srv_len = sizeof(struct sockaddr_in);
	memset(sain, 0, srv_len);
	sain->sin_port = htons(SyncPort);//default listening port
	sain->sin_addr.s_addr = INADDR_ANY;	//the socket will be bound to all local interfaces (and we just have one up to this point, being the DS Client IP acquired from the DHCP server).
	
	int my_socket = socket(AF_INET, SOCK_STREAM, 0);
	int enable = 0;
	if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&enable, sizeof(int)) < 0){	//socket can be respawned ASAP if it's dropped
		#if defined(_MSC_VER)
		closesocket(my_socket); // remove the socket.
		#endif
		#if !defined(_MSC_VER)
		close(my_socket); // remove the socket.
		#endif
		return -1;
	}
	if(my_socket == -1){
		return -1;
	}
	int retVal = bind(my_socket,(struct sockaddr*)sain, srv_len);
	if(retVal == -1){
		#if defined(_MSC_VER)
		closesocket(my_socket);
		#endif
		#if !defined(_MSC_VER)
		close(my_socket);
		#endif
		return -1;
	}
	
	int MAXCONN = 20;
	retVal = listen(my_socket, MAXCONN);
	if(retVal == -1){
		#if defined(_MSC_VER)
		closesocket(my_socket);
		#endif
		#if !defined(_MSC_VER)
		close(my_socket);
		#endif
		return -1;
	}
	return my_socket;
}

void getMyIP(IP_v4 * myIP){
    #ifdef WIN32
    char szBuffer[1024];
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 0);
    if(::WSAStartup(wVersionRequested, &wsaData) != 0){
        
    }

    if(gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR)
    {
      WSACleanup();
    }

    struct hostent *host_info = gethostbyname(szBuffer);
    if(host_info == NULL)
    {
      WSACleanup();
    }

	#define THIS_IP_SLOT ((int)2)
	struct in_addr **address_list = (struct in_addr **)host_info->h_addr_list;
    for(int i = THIS_IP_SLOT; address_list[i] != NULL; i++){
        struct in_addr thisNetwork = *(address_list[i]);
		//Obtain the computer's IP
		myIP->b1 = thisNetwork.S_un.S_un_b.s_b1;
		myIP->b2 = thisNetwork.S_un.S_un_b.s_b2;
		myIP->b3 = thisNetwork.S_un.S_un_b.s_b3;
		myIP->b4 = thisNetwork.S_un.S_un_b.s_b4;
		break;
    }

    WSACleanup();
    #endif
    
    #if !defined(WIN32)
    char host[256];
    //char *IP;
    struct hostent *host_entry;
    int hostname;
    hostname = gethostname(host, sizeof(host)); //find the host name
    if(hostname == -1){
      return;
    }
    
    host_entry = gethostbyname(host); //find host information
    if(host_entry == NULL){
      return;
    }
    //IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); //Convert into IP string
    //printf("Current Host Name: %s\n", host);
    //printf("Host IP: %s\n", IP);
    
    struct in_addr * thisAddr = ((struct in_addr*) host_entry->h_addr_list[0]);
    u32 ipNumv4 = (u32)thisAddr->s_addr;;
    myIP->b1 = ((ipNumv4>>0)&0xFF);
    myIP->b2 = ((ipNumv4>>8)&0xFF);
    myIP->b3 = ((ipNumv4>>16)&0xFF);
    myIP->b4 = ((ipNumv4>>24)&0xFF);
    #endif
}

int Wifi_GetIP(){
	struct IP_v4 v4;
	getMyIP(&v4);
	return (int)( ((v4.b1 << 0)&0xFF) | ((v4.b2 << 8)&0xFF00) | ((v4.b3 << 16)&0xFF0000) | ((v4.b4 << 24)&0xFF000000) );
}

char * print_ip(uint32 ip, char * outBuf){
    uint8 bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    #ifdef WIN32
    cv_snprintf(outBuf, sizeof(outBuf), "%d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
    #endif
    #if !defined(WIN32)
    snprintf(outBuf, sizeof(outBuf), "%d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
    #endif
    return outBuf;
}

#endif

//All args starting from 1 are arranged to start from 0
void orderArgs(int argc, char *argv[]){
	vector<char *> vect;
	int i = 1;
	for(i = 1; i < argc; i++){
		vect.push_back(argv[i]);
	}
	i = 0;
	for (char*& arr : vect) {
		argv[i] = arr; //printf("new arg:%d %s\n", i, arr);//current arg char* buffer (string value)
		i++;
	}
	vect.empty();
}

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
		std::string outputFile = "bmpFrames/output.txt";
		std::remove(outputFile.c_str());
		//FFmpeg convert
		std::string cmd = string("ffmpeg -async 20 -i tvsIn/"+ file+" -vf scale=256:192,fps=fps=10,showinfo -pix_fmt rgb24 -y bmpFrames/yo%03d.bmp -f wav -acodec adpcm_ima_wav -ar 22050 -af apad=pad_dur=1,lowpass=f=11000,afftdn=nf=-25,dynaudnorm=p=0.9:s=5 bmpFrames/audio.ima > bmpFrames/output.txt 2>&1");
		system(cmd.c_str());

		////////////////////////////////////////////////////////TVS start
		std::vector<struct videoFrameTimeStamp> ts_list = parseTimeStampFromFFMPEGOutput(outputFile);
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
		int ret = generateTGDSVideoformatFromBMPDir(BMPframesFound, std::string(cwdPathOut), audioTrackFound.at(0), getFileNameNoExtension(file), ts_list);
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
	
	//printf("DEBUGGER: argv : %s\n", argv[0]);	// TGDSPKGBuilder.exe
	//printf("DEBUGGER: argv : %s\n", argv[1]);	// ToolchainGenericDS-template
	//printf("DEBUGGER: argv : %s\n", argv[2]);	// /
	//printf("DEBUGGER: argv : %s\n", argv[3]);	// c:/toolchain_generic/6.2_2016q4/arm-eabi/lib/
	//printf("DEBUGGER: argv : %s\n", argv[4]);	// src directory {basedir}/release/arm7dldi-ntr (fullpath)
	//printf("DEBUGGER: argv : %s\n", argv[5]);	// NTR / TWL Mode
	//printf("DEBUGGER: argv : %s\n", argv[6]);	// TGDSPKG filename
	//printf("DEBUGGER: argv : %s\n", argv[7]);	// "gdbenable" / "nogdb"
	//printf("DEBUGGER: argv : %s\n", argv[8]);	// src directory /release/arm7dldi-ntr (relative path)
	//printf("\nDEBUGGER: argv : %s \n", argv[9]);	// //TGDS-multiboot's remoteboot port number
	//printf("\nDEBUGGER: argv : %s \n", argv[10]);	// //TGDS-multiboot's remoteboot IP
	//exit(0);

	/* avoid end-of-line conversions */
    SET_BINARY_MODE(stdin);
    SET_BINARY_MODE(stdout);

	if (argc < 3){
		TGDSPackageBuilderHelp();
		return -1;
	}

	// "/TGDSLibrarySourceDirectory"
	char TGDSLibrarySourceDirectory[256+1];
	
	#ifdef _MSC_VER
	strcpy(TGDSLibrarySourceDirectory, argv[3]); //strcpy(TGDSLibrarySourceDirectory, "C:\\toolchain_generic\\6.2_2016q4\\arm-eabi\\lib\\newlib-nano-2.1-nds\\");
	#endif

	//fix up for linux
	#ifndef _MSC_VER
	char * TGDSLibDir = argv[3];
	//TGDSLibDir++;
	strcpy(TGDSLibrarySourceDirectory, TGDSLibDir); //strcpy(TGDSLibrarySourceDirectory, "C:\\toolchain_generic\\6.2_2016q4\\arm-eabi\\lib\\newlib-nano-2.1-nds\\");
	strcat(TGDSLibrarySourceDirectory, "/");
	#endif
	

	// "/baseTargetDecompressorDirectory"
	char baseTargetDecompressorDirectory[256+1];
	if((argv[2] != NULL) && (strlen(argv[2]) > 1)){
		strcpy(baseTargetDecompressorDirectory, argv[2]); //strcpy(baseTargetDecompressorDirectory, "TGDSbaseTargetDecompressorDirectory/");
		strcat(baseTargetDecompressorDirectory, "/");
	}
	else{
		strcpy(baseTargetDecompressorDirectory, "");
	}
	
	// "remotegdbflag"
	char remotegdbflag[256+1];
	strcpy(remotegdbflag, argv[6]); //"gdbenable" / "nogdb"
	
	//TGDS-multiboot's remoteboot port number
	//printf("\n port:%s \n",argv[9]);
	//exit(1);

	char TarName[256];
	strcpy(TarName, argv[1]);
	char TGDSMainApp[256];
	strcpy(TGDSMainApp, TarName);

	if( (argv[5] != NULL) && (strncmp(argv[5], "twl_mode", strlen("twl_mode")) == 0)){
		strcat(TGDSMainApp, ".srl");
	}
	else if( (argv[5] != NULL) && (strncmp(argv[5], "ntr_mode", strlen("ntr_mode")) == 0)){
		strcat(TGDSMainApp, ".nds");
	}
	
	//Output Directory
	char converted[256];	
	char outputPKGPath[256];

	//VS2012? full path
	#ifdef _MSC_VER
	strcpy(outputPKGPath, (string(argv[4]) + string("\\")).c_str() ); //strcpy(outputPKGPath, (string(converted) + string("\\..\\Debug")).c_str() );
	#endif

	//Linux? relative path
	#ifndef _MSC_VER
	char * relpath = argv[8];
	relpath++;
	strcpy(outputPKGPath, (string(relpath) + string("/")).c_str() );
	#endif

	printf("\nSource files Directory: %s\n", argv[8]); //relative path

	std::vector<dirItem> vec = list_directoryByType(string(argv[4]));	//fullpath
	
	printf("\nSource files Count: %d\n", vec.size());
	
	int crc32mainApp =-1;
	int crc32TGDSSDK =-1;

	char fullPathOuttar[256+1];
	memset(fullPathOuttar, 0, sizeof(fullPathOuttar));
	strcpy(fullPathOuttar, outputPKGPath);
	//override package name? If not, use the default name from the Main App
	if(argv[6] != NULL){
		strcpy(TarName, ("..\\" + string(argv[6]) + string(".tar")).c_str());
		strcat(fullPathOuttar, TarName);
	}
	else{
		strcpy(TarName, ("..\\" + string(argv[1]) + string(".tar")).c_str());
		strcat(fullPathOuttar, TarName);
	}

	char zipArgs[10][256];
	/* open file for writing */
	string zipList = "";
	int argStart = 0;
	for(int i = 0; i < vec.size(); i++){
		dirItem item = vec.at(i);
		char * filename = (char*)item.path.c_str();
		if( (string(filename) != "..") && (string(TarName) != string(filename)) && ((string(TarName) + string(".gz")) != string(filename)) && (string("toolchaingenericds-multiboot-config.txt") != string(filename)) ){
			/* Add a file */
			if(item.type == FT_FILE){
				char fullPathIn[256+1];
				strcpy(fullPathIn, outputPKGPath);
				strcat(fullPathIn, filename);
				printf("TAR: Add File: %d: %s \n", i, fullPathIn);
				
				//copy files to make them relative to root path in upcoming zip archive
				remove(filename);
				
				//debug start
				//strcpy(fullPathIn, "release/arm7dldi-ntr/ToolchainGenericDS-template.nds");
				//debug end
				
				ifstream infile(fullPathIn, std::ios::out | std::ios::binary);
				if (infile)
				{
					istreambuf_iterator<char> ifit(infile);
					ofstream outfile(filename, std::ios::out | std::ios::binary);
					ostreambuf_iterator<char> ofit(outfile);
					if (outfile)
					{
						copy(ifit, istreambuf_iterator<char>(), ofit);
						outfile.close();
						cout << "file" << fullPathIn << "open OK \n";
					}
					else
					{
						cerr << "Could not open output file" << "\n";
					}
					infile.close();
					zipList+=(" "+string(filename));
					strcpy(&zipArgs[argStart][0], filename);
					argStart++;
				}
				else
				{
					cerr << "Could not open input file" << fullPathIn << "\n";
				}
				
				//Found mainApp?
				if(string(TGDSMainApp) == string(filename)){
					//unsigned long crc32 = -1;
					FILE* TGDSLibraryFile = fopen(filename,"rb");
					int err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32mainApp);
					fclose(TGDSLibraryFile);
					printf("mainApp[%s] CRC32: %x\n", filename, crc32mainApp);
				}
			}
			else if(item.type == FT_DIR){
				 
			}
		}
	}
	
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

	//Windows requires this
	#ifdef _MSC_VER
	strcat(TGDSLibrarySourceDirectory, "\\");
	#endif

	//Linux doesn't access full paths from CWD
	#ifndef _MSC_VER
	char thisCWD[256];
	getCWDWin(thisCWD, "");
	chdir(TGDSLibrarySourceDirectory);
	printf("TGDS Libraries directory:[%s]", TGDSLibrarySourceDirectory);
	strcpy(TGDSLibrarySourceDirectory, "");
	#endif

	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("libcnano7.a")).c_str(),"r");
	if(TGDSLibraryFile != NULL){
		err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32TGDSSDKlibcnano7);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libcnano7.a missing. Make sure you build newlib-nds first!");
		return -1;
	}

	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("libcnano9.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32TGDSSDKlibcnano9);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libcnano9.a missing. Make sure you build newlib-nds first!");
		return -1;
	}

	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("libtoolchaingen7.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32TGDSSDKlibtoolchaingen7);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libtoolchaingen7.a missing. Make sure you build ToolchainGenericDS first!");
		return -1;
	}

	TGDSLibraryFile = fopen((string(TGDSLibrarySourceDirectory) + string("libtoolchaingen9.a")).c_str(),"rb");
	if(TGDSLibraryFile != NULL){	
		err = Crc32_ComputeFile(TGDSLibraryFile, (uint32_t*)&crc32TGDSSDKlibtoolchaingen9);
		fclose(TGDSLibraryFile);
	}
	else{
		printf("libtoolchaingen9.a missing. Make sure you build ToolchainGenericDS first!");
		return -1;
	}

	#ifndef _MSC_VER
	chdir(thisCWD);
	#endif
	

	// Write the descriptor 
	{
		char TGDSDescriptorBuffer[256+1];
		#ifdef _MSC_VER
		cv_snprintf(TGDSDescriptorBuffer, sizeof(TGDSDescriptorBuffer), "[Global]\n\nmainApp = %s\n\nmainAppCRC32 = %x\n\nTGDSSdkCrc32 = %x\n\nbaseTargetPath = %s\n\nremotegdbflag = %s\n\n", TGDSMainApp, crc32mainApp, (crc32TGDSSDKlibcnano7 + crc32TGDSSDKlibcnano9 + crc32TGDSSDKlibtoolchaingen7 + crc32TGDSSDKlibtoolchaingen9), baseTargetDecompressorDirectory, remotegdbflag);
		#endif
		
		#ifndef _MSC_VER
		snprintf(TGDSDescriptorBuffer, sizeof(TGDSDescriptorBuffer), "[Global]\n\nmainApp = %s\n\nmainAppCRC32 = %x\n\nTGDSSdkCrc32 = %x\n\nbaseTargetPath = %s\n\nremotegdbflag = %s\n\n", TGDSMainApp, crc32mainApp, (crc32TGDSSDKlibcnano7 + crc32TGDSSDKlibcnano9 + crc32TGDSSDKlibtoolchaingen7 + crc32TGDSSDKlibtoolchaingen9), baseTargetDecompressorDirectory, remotegdbflag);
		#endif
		ofstream ofs;
		ofs.open("descriptor.txt", ofstream::out | std::ios::binary);
		if (ofs)
		{
			cout << "\ndescriptor.txt create OK \n";
		}
		else
		{
			cerr << "\nCould not create descriptor.txt \n";
		}
		
		ofs << TGDSDescriptorBuffer;
		ofs << endl;
		ofs.close();

		zipList+=(" "+string("descriptor.txt"));
		strcpy(&zipArgs[argStart][0], "descriptor.txt");
		argStart++;
	}


	// Write toolchaingenericds-multiboot-config.txt
	{
		char TGDSMBCFGBuffer[256];
		char fullNDSPath[256];
		strcpy(fullNDSPath, "0:/");
		strcat(fullNDSPath, TGDSMainApp);
		#ifdef _MSC_VER
		cv_snprintf(TGDSMBCFGBuffer, sizeof(TGDSMBCFGBuffer), "[Global]\n# ToolchainGenericDS-multiboot global settings \n\ntgdsutilsremotebooteripaddr = %s\ntgdsutilsremotebooterport = %s\ntgdsmultitbootlasthomebrew = %s\n", argv[10], argv[9], fullNDSPath);
		#endif		
		#ifndef _MSC_VER
		snprintf(TGDSMBCFGBuffer, sizeof(TGDSMBCFGBuffer), "[Global]\n# ToolchainGenericDS-multiboot global settings \n\ntgdsutilsremotebooteripaddr = %s\ntgdsutilsremotebooterport = %s\ntgdsmultitbootlasthomebrew = %s\n", argv[10], argv[9], fullNDSPath);
		#endif
		ofstream ofs;
		ofs.open("toolchaingenericds-multiboot-config.txt", ofstream::out | std::ios::binary);
		if (ofs)
		{
			cout << "\ntoolchaingenericds-multiboot-config.txt create OK \n";
		}
		else
		{
			cerr << "\nCould not create toolchaingenericds-multiboot-config.txt \n";
		}
		
		ofs << TGDSMBCFGBuffer;
		ofs << endl;
		ofs.close();

		zipList+=(" "+string("toolchaingenericds-multiboot-config.txt"));
		strcpy(&zipArgs[argStart][0], "toolchaingenericds-multiboot-config.txt");
		argStart++;
	}

	argc = argStart + 3; 	//descriptor.txt & toolchaingenericds-multiboot-config.txt & dummy end

	char argvZip[MAX_ARGV_BUFFER_SIZE_TGDSUTILS][MAX_TGDSFILENAME_LENGTH]; 
	memset(argvZip, 0, sizeof(argvZip));
	strcpy((char*)&argvZip[0][0], (char*)"thisApp"); //unused
	strcpy((char*)&argvZip[1][0], (char*)"-o "); //emit zip
	strcpy((char*)&argvZip[2][0], (char*)"remotepackage.zip"); //.zip filename to create
	
	for(int i = 0; i < 12; i++){
		argv[i] = NULL;
	}

	argv[0] = (char*)&argvZip[0][0];
	argv[1] = (char*)&argvZip[1][0];
	argv[2] = (char*)&argvZip[2][0];
	
	for(int i = 0; i < argc; i++){
		if( strlen(&zipArgs[i][0]) > 4 ){
			strcpy((char*)&argvZip[3+i][0], (char*)&zipArgs[i][0]); //.zip filename to create
			argv[3+i] = (char*)&argvZip[3+i][0];
		}
	}
	
	for(int i = 0; i < argc; i++){
		printf("\n current zip arg: %s \n", (char*)argv[i]);
	}

	mainZIPBuild(argc, argv);

	//cleanup
	for(int i = 0; i < (argc); i++){
		char buf[256];
		getCWDWin(buf, "/");
		strcat(buf, (char*)argvZip[3+i]);
		//printf("\n erasing: %s \n", buf);
		remove(buf);
	}
	
	printf("TGDSPKG %s build OK \n", "remotepackage.zip");
    return 0;
}

//Test case: 
//toolchaingenericds-utils remotebooter /release 192.168.43.82 twl_mode ToolchainGenericDS-multimediaplayer / C:/toolchain_generic/6.2_2016q4/arm-eabi/lib/newlib-nano-2.1-nds/ remotepackage

//Packages a destination directory and sends it to ToolchainGenericDS-multiboot remoteboot command
int TGDSRemoteBooter(int argc, char *argv[]){
	printf("TGDSRemoteBooter Start\n");
	//debug start
	//arg 0: [remotebooter]
	//arg 1: [/TGDSProjectSourceDirectory]
	//arg 2: [NintendoDS IP:xxx.xxx.xxx.xxx format]
	/*
	char arg0[256], arg1[256], arg2[256], arg3[256];
	{
		argc = 3;
		strcpy(arg0, "remotebooter");
		getCWDWin(arg1, "/Debug/release/"); //debug
		argv[1] = (char*)&arg1[0];

		strcpy(arg2, "192.168.43.61");
		strcpy(arg3, "twl_mode");
		argv[0] = (char*)&arg0[0];
		argv[1] = (char*)&arg1[0];
		argv[2] = (char*)&arg2[0];
		argv[3] = (char*)&arg3[0];
	}
	*/
	//debug end
	
	//Build TGDS Package
	int argcPackage = MAX_ARGV_BUFFER_SIZE_TGDSUTILS;
	char * argvPackage[MAX_ARGV_BUFFER_SIZE_TGDSUTILS];
	char TGDSProjectName[256];
	char baseTargetDecompressorDirectory[256];
	char TGDSLibrarySourceDirectory[256];
	char TGDSProjectNTRorTWLMode[256];
	char TGDSProjectSourceDirectory[256];

	//arg7  = remotebooter
	//arg6 = $(LIBPATTH)
	//arg5 = should be "/" but it's C:/MinGW/msys/1.0
	//arg4 = TGDS Main App entrypoint
	//arg3 = twl_mode / ntr_mode
	//arg2 = Remotebooter IP
	//arg1 = TGDS Project source directory
	//arg0 = remotebooter (shell cmd from tgds-utils binary)

	//debug
		//strcpy(TGDSProjectName, "ToolchainGenericDS-multimediaplayer");
		//strcpy(baseTargetDecompressorDirectory, "/"); //where PKG is decompressed on target directory on NDS FS
		//strcpy(TGDSLibrarySourceDirectory, "C:/toolchain_generic/6.2_2016q4/arm-eabi/lib/newlib-nano-2.1-nds/"); //$(LIBPATH)
	
	//release
		strcpy(TGDSProjectName, argv[4]);
		strcpy(baseTargetDecompressorDirectory, argv[5]);
		strcpy(TGDSLibrarySourceDirectory, argv[6]);

	getCWDWin(TGDSProjectSourceDirectory, argv[1]); //release
	strcat(TGDSProjectSourceDirectory, "/");
	strcpy(TGDSProjectNTRorTWLMode, argv[3]); 
	argvPackage[0] = argv[0];
	argvPackage[1] = (char*)&TGDSProjectName[0];
	argvPackage[2] = (char*)&baseTargetDecompressorDirectory[0];
	argvPackage[3] = (char*)&TGDSLibrarySourceDirectory[0];
	argvPackage[4] = (char*)&TGDSProjectSourceDirectory[0]; //{basedir}/release/arm7dldi-ntr (src directory fullpath)
	argvPackage[5] = (char*)&TGDSProjectNTRorTWLMode[0]; //ntr_mode or twl_mode
	argvPackage[6] = argv[8]; //override TGDS Package name if provided or use the default Main App one
	argvPackage[7] = (char*)argv[9]; //is remoteboot or gdb debug session
	argvPackage[8] = (char*)argv[1];	// /release/arm7dldi-ntr (src directory relative path)
	argvPackage[9] = (char*)argv[10]; //remoteboot port
	argvPackage[10] = (char*)argv[2]; //remoteboot IP
	
	int result = TGDSPKGBuilder(argcPackage, argvPackage);
	
	//now send to NDS
    char fullPath[256];
	getCWDWin(fullPath, (char*)"/"); 
	argcPackage = 4;
	argvPackage[0] = "httpserver";
	argvPackage[1] = (char*)"-quit";
	argvPackage[2] = (char*)argv[10]; //remoteboot port
	argvPackage[3] = (char*)argv[2]; //remoteboot IP

	mainHTTPServer(argcPackage, argvPackage);
	printf("TGDSRemoteBooter End\n");
	return 0;
}


#include <fstream>
#include <iostream>

string extractInformation(size_t p, string key, const string& theEntireString)
{
  auto p1 = theEntireString.find(key);
  if (string::npos != p1)
    p1 += key.size();
  auto p2 = theEntireString.find_first_of('p',p1);
  if (string::npos != p2)
    return theEntireString.substr(p1,p2-p1);
  return "";
}

std::vector<struct videoFrameTimeStamp> parseTimeStampFromFFMPEGOutput(const std::string &filename){
	std::vector<struct videoFrameTimeStamp> ts_list;
	std::ifstream file(filename);
	int videoFrameIndex = 0;
	if (file.is_open()) {
		std::string line;
			while (std::getline(file, line)) {
				string res = extractInformation(0,"pts_time:", line);
				if(res != ""){
					videoFrameTimeStamp tsFrame;
					tsFrame.frameIndex = videoFrameIndex;
					res.erase(res.find_last_not_of(" \n\r\t")+1); //trim
					double val = atof(res.c_str());
					int ms = (((double)val)*1000);
					tsFrame.extractedElapsedTimeStampInMilliseconds = ms;
					ts_list.push_back(tsFrame);
					videoFrameIndex++;
				}
			}
		file.close();
	}
	else{
		printf("parseTimeStampFromFFMPEGOutput():failure opening: %s\n", filename.c_str());
	}
	return ts_list;
}
