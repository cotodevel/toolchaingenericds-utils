// toolchaingenericds-utils.cpp : Defines the entry point for the console application.

#ifdef _MSC_VER
#include "stdafx.h"
#include "winDir.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include "utilities.h"
#include "winDir.h"
#include "TGDSVideoConverter/lzss9.h"
#include "http/server.h"

#include <string>
#include <vector>
using namespace std;


void showMenu(char * appName){
	printf("Usage: README.MD \n");
	printf("%s [Command] arg0 arg1 arg2 arg3 ... argN \n", getFileName(string(appName), false).c_str());
	printf("	Commands: \n");
	printf("	[bin2c] Binfile.bin Binfile.c Binfile (optional)SectionName\n");
	printf("	[bin2lzss] command2 filename [filename [...]]\n");
	printf("	[mp4totvs] TGDS-Videoconverter, emits *.TVS video+audio streams playable in ToolchainGenericDS-multimediaplayer (cmd only) \n");
	printf("	[pkgbuilder]  TGDSProjectName [/baseTargetDecompressorDirectory] [/TGDSLibrarySourceDirectory] [/TGDSProjectSourceDirectory] [ntr_mode/twl_mode] \n");
	printf("	[remotebooter]  [/TGDSProjectSourceDirectory] [NintendoDS IP:xxx.xxx.xxx.xxx format] [ntr_mode/twl_mode] [TGDSProjectName] [baseTargetDecompressorDirectory] [TGDSLibrarySourceDirectory] \n");
	printf("    [httpserver] [-quit]");
}

int main( int argc, char *argv[] ){
	if(argc < 2){
		showMenu(argv[0]);
	}

	if( (argv[1] != NULL) && (strncmp(argv[1], "bin2c", strlen("bin2c")) == 0)){
		orderArgs(argc, argv);
		return convertbin2c(argc, argv);
	}
	else if( (argv[1] != NULL) && (strncmp(argv[1], "bin2lzss", strlen("bin2lzss")) == 0)){
		orderArgs(argc, argv);
		return convertbin2lzss(argc, argv);
	}
	else if( (argv[1] != NULL) && (strncmp(argv[1], "mp4totvs", strlen("mp4totvs")) == 0)){
		orderArgs(argc, argv);
		return convertMP4toTVS(argc, argv);
	}
	else if( (argv[1] != NULL) && (strncmp(argv[1], "pkgbuilder", strlen("pkgbuilder")) == 0)){
		orderArgs(argc, argv);
		return TGDSPKGBuilder(argc, argv);
	}
	else if( (argv[1] != NULL) && (strncmp(argv[1], "remotebooter", strlen("remotebooter")) == 0)){
		orderArgs(argc, argv);
		return TGDSRemoteBooter(argc, argv);
	}
	else if( (argv[1] != NULL) && (strncmp(argv[1], "httpserver", strlen("httpserver")) == 0)){
		orderArgs(argc, argv);
		return mainHTTPServer(argc, argv);
	}
	else{
		printf("\nMissing or Wrong Command\n");
	}
	return 0;
}

