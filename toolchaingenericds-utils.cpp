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

#include <string>
#include <vector>
using namespace std;


void showMenu(char * appName){
	printf("Usage: README.MD \n");
	printf("%s [Command] arg0 arg1 arg2 arg3 ... argN \n", getFileName(string(appName), false).c_str());
	printf("	Commands: \n");
	printf("	[bin2c] Binfile.bin Binfile.c Binfile (optional)SectionName\n");
	printf("	[bin2lzss] command2 filename [filename [...]]\n");

}

int main( int argc, char *argv[] ){
	if(argc < 2){
		showMenu(argv[0]);
	}

	if( (argv[1] != NULL) && (strncmp(argv[1], "bin2c", strlen("bin2c")) == 0)){
		return convertbin2c(argc, argv);
	}
	else if( (argv[1] != NULL) && (strncmp(argv[1], "bin2lzss", strlen("bin2lzss")) == 0)){
		return convertbin2lzss(argc, argv);
	}

	return 0;
}

