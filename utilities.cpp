#ifdef WIN32


#include "stdafx.h"
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http/server.h"
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
		closesocket(my_socket); // remove the socket.
		return -1;
	}
	if(my_socket == -1){
		return -1;
	}
	int retVal = bind(my_socket,(struct sockaddr*)sain, srv_len);
	if(retVal == -1){
		closesocket(my_socket);
		return -1;
	}
	
	int MAXCONN = 20;
	retVal = listen(my_socket, MAXCONN);
	if(retVal == -1){
		closesocket(my_socket);
		return -1;
	}
	return my_socket;
}

void getMyIP(IP_v4 * myIP)
{
    char szBuffer[1024];

    #ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 0);
    if(::WSAStartup(wVersionRequested, &wsaData) != 0){
        
	}
    #endif


    if(gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR)
    {
      #ifdef WIN32
      WSACleanup();
      #endif
    }

    struct hostent *host = gethostbyname(szBuffer);
    if(host == NULL)
    {
      #ifdef WIN32
      WSACleanup();
      #endif
    }

    //Obtain the computer's IP
    myIP->b1 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b1;
    myIP->b2 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b2;
    myIP->b3 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b3;
    myIP->b4 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b4;

    #ifdef WIN32
    WSACleanup();
    #endif
}

int Wifi_GetIP(){
	#ifndef WIN32
	//todo: Linux
	char host[256];
	char *IP;
	struct hostent *host_entry;
	int hostname;
	hostname = gethostname(host, sizeof(host)); //find the host name
	check_host_name(hostname);
	host_entry = gethostbyname(host); //find host information
	check_host_entry(host_entry);
	IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); //Convert into IP string
	#endif
	
	#ifdef WIN32
	struct IP_v4 v4;
	getMyIP(&v4);
	return (int)( (v4.b1 << 0) | (v4.b2 << 8) | (v4.b3 << 16) | (v4.b4 << 24) ); //order may be wrong
	#endif
	return 0;
}

char * print_ip(uint32 ip, char * outBuf){
    uint8 bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;	
    cv_snprintf(outBuf, sizeof(outBuf), "%d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
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
		
		std::string cmd = string("ffmpeg -async 20 -i tvsIn/"+ file+" -vf scale=256:192,fps=fps=10,showinfo -pix_fmt rgb24 -y bmpFrames/yo%03d.bmp -f wav -acodec adpcm_ima_wav -ar 22050 bmpFrames/audio.ima > output.txt 2>&1");
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
	
	//printf("DEBUGGER: argv 0: %s\n", argv[0]);	// TGDSPKGBuilder.exe
	//printf("DEBUGGER: argv 1: %s\n", argv[1]);	// ToolchainGenericDS-template
	//printf("DEBUGGER: argv 2: %s\n", argv[2]);	// /
	//printf("DEBUGGER: argv 3: %s\n", argv[3]);	// c:/toolchain_generic/6.2_2016q4/arm-eabi/lib/
	//printf("DEBUGGER: argv 5: %s\n", argv[4]);	// /release/arm7dldi-ntr
	//printf("DEBUGGER: argv 6: %s\n", argv[5]);	// NTR / TWL Mode
	
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
	strcpy(outputPKGPath, (string(argv[4]) + string("\\")).c_str() ); //strcpy(outputPKGPath, (string(converted) + string("\\..\\Debug")).c_str() );
	
	printf("\nSource files Directory: %s\n", argv[4]);
	std::vector<dirItem> vec = list_directoryByType(string(argv[4]));	
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
	int argStart = 3;
	for(int i = 0; i < vec.size(); i++){
		dirItem item = vec.at(i);
		char * filename = (char*)item.path.c_str();
		if( (string(filename) != "..") && (string(TarName) != string(filename)) && ((string(TarName) + string(".gz")) != string(filename)) ){
			/* Add a file */
			if(item.type == FT_FILE){
				char fullPathIn[256+1];
				strcpy(fullPathIn, outputPKGPath);
				strcat(fullPathIn, filename);
				//printf("TAR: Add File: %d: %s \n", i, fullPathIn);
				//printf("into: [%s] \n", (string(baseTargetDecompressorDirectory) + string(filename)).c_str());
				//tarball.putFile(fullPathIn, (string(baseTargetDecompressorDirectory) + string(filename)).c_str());
				
				//copy files to make them relative to root path in upcoming zip archive
				remove(filename);
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
					}
					else
					{
						cerr << "Could not open output file" << "\n";
					}
					infile.close();
					zipList+=(" "+string(filename));
					strcpy(&zipArgs[argStart-3][0], filename);
					argv[argStart] = (char*)&zipArgs[argStart-3][0];
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
				//todo: read dir, iterate contents, then create dir and files inside, in TAR 
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

	/* Write the descriptor */
	char TGDSDescriptorBuffer[256+1];
	cv_snprintf(TGDSDescriptorBuffer, sizeof(TGDSDescriptorBuffer), "[Global]\n\nmainApp = %s\n\nmainAppCRC32 = %x\n\nTGDSSdkCrc32 = %x\n\nbaseTargetPath = %s\n\n", TGDSMainApp, crc32mainApp, (crc32TGDSSDKlibcnano7 + crc32TGDSSDKlibcnano9 + crc32TGDSSDKlibtoolchaingen7 + crc32TGDSSDKlibtoolchaingen9), baseTargetDecompressorDirectory);
	ofstream ofs;
	ofs.open("descriptor.txt", ofstream::out | std::ios::binary);
	ofs << TGDSDescriptorBuffer;
	ofs << endl;
	ofs.close();

	zipList+=(" "+string("descriptor.txt"));

	strcpy(&zipArgs[vec.size()-1][0], "descriptor.txt");
	/*
	//Example
		remove("remotepackage2.zip");

		//todo: copy files from another path into this path, list them below, add descriptor, call zip then delete them
		argc = 4;
		argv[0] = "thisApp"; //unused
		argv[1] = "remotepackage2.zip"; //.zip filename to create
		argv[2] = "Debug/release/tgds_multiboot_payload_twl.bin"; //arg 0
		argv[3] = "Debug/release/ToolchainGenericDS-multimediaplayer.srl"; //arg 1
								//arg n
		mainZIPBuild(argc, argv);
		
	note: filepaths are relative to current working directory
	*/
	
	argc = vec.size() + 3; 
	argv[0] = "thisApp"; //unused
	argv[1] = "-o ";						//arg n
	argv[2] = "remotepackage.zip"; //.zip filename to create
	
	for(int i = 0; i < (vec.size() + 1); i++){
		argv[3+i] = &zipArgs[i][0];
	}

	mainZIPBuild(argc, argv);

	//cleanup
	for(int i = 0; i < (vec.size() + 1); i++){
		remove((char*)&zipArgs[i][0]);
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
	argvPackage[4] = (char*)&TGDSProjectSourceDirectory[0];
	argvPackage[5] = (char*)&TGDSProjectNTRorTWLMode[0]; //ntr_mode or twl_mode
	argvPackage[6] = argv[8]; //override TGDS Package name if provided or use the default Main App one
	int result = TGDSPKGBuilder(argcPackage, argvPackage);
	
	//now send to NDS
    string TGDSPKGFile = string(argv[1]) + string("/"); //+ string(argv[4]) + string(".tar.gz");
	char fullPath[256];
	getCWDWin(fullPath, (char*)TGDSPKGFile.c_str()); //debug
	strcat(fullPath, (string(argv[4]) + string(".tar.gz")).c_str());
	argcPackage = 2;
	argvPackage[0] = "httpserver";
	argvPackage[1] = (char*)"-quit";
	
	mainHTTPServer(argcPackage, argvPackage);
	printf("TGDSRemoteBooter End\n");
	return 0;
}
