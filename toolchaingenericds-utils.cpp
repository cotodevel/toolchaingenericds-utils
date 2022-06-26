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

#include <string>
#include <vector>
using namespace std;


void showMenu(char * appName){
	printf("Usage: README.MD \n");
	printf("%s [Command] arg0 arg1 arg2 arg3 ... argN \n", getFileName(string(appName), false).c_str());
	printf("	Commands: \n");
	printf("	[bin2c] Binfile.bin Binfile.c Binfile (optional)SectionName\n");
	printf("	[bin2lzss] command2 filename [filename [...]]\n");
	printf("	[mp4totvs] (put files in /tvsin folder)\n");
	printf("	[pkgbuilder]  TGDSProjectName [/baseTargetDecompressorDirectory] [/TGDSLibrarySourceDirectory] [/TGDSPKGOutDirectory] [/TGDSProjectSourceFolder] \n");
}

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
	else{
		printf("\nMissing or Wrong Command\n");
	}
	return 0;
}

