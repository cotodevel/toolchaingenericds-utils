#ifndef __utilities_h__
#define __utilities_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

#if !defined(WIN32)
#include <stdbool.h>
#endif
#ifdef __cplusplus
#include <string>
#include <vector>
using namespace std;
#endif

#ifndef _MSC_VER
//strncasecmp() — Compare Strings without Case Sensitivity
//strncmp() — Compare Strings
//la función strcmpi() no distingue entre mayúsculas y minúsculas
//la función strcmp() distingue entre mayúsculas y minúsculas.
#define strcmpi strcasecmp
//#define strcmp  strcasecmp //already defined
#define MAX_TGDSFILENAME_LENGTH ((int)256)
#endif

#define SIZE 256

#if defined(WIN32)
#include "TGDSVideoConverter/TGDSTypes.h"
#endif
#if defined(WIN32) || !defined(ARM9)
#define TGDSDirectorySeparator ((char*)"/")
#endif

typedef struct IP_v4
{
    unsigned char b1, b2, b3, b4;
} IP_v4;
#endif

extern void showMenu(char * appName);
extern void orderArgs(int argc, char *argv[]);
extern int convertbin2c(int argc, char *argv[]);
extern int convertMP4toTVS(int argc, char *argv[] );
extern int TGDSPKGBuilder(int argc, char *argv[] );
extern int TGDSRemoteBooter(int argc, char *argv[]);

#if !defined(ARM9)
extern int	FS_getFileSize(char *filename);
extern int Wifi_GetIP();
extern void getMyIP(IP_v4 * myIP);
extern char * print_ip(uint32 ip, char * outBuf);
#endif
