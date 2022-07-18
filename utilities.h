#ifndef __utilities_h__
#define __utilities_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include "zlib-1.2.11/zlib.h"
#include "zlib-1.2.11/ioapi.h"
#include "TGDSVideoConverter/TGDSVideo.h"

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
#define MAX_ARGV_BUFFER_SIZE_TGDSUTILS ((int)128) //max stacked argvs 

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

#ifdef __cplusplus
extern vector<struct videoFrameTimeStamp> parseTimeStampFromFFMPEGOutput(const string &filename);

extern "C"{
#endif
extern int untgzmain(int argc,char **argv);
extern bool existFilePosix(char *fname);

//zlib
extern int gz_compress(FILE   *in, gzFile out);
extern voidpf  fopen_file_func (voidpf opaque, const char* filename, int mode);
extern uLong   fread_file_func (voidpf opaque, voidpf stream, void* buf, uLong size);
extern uLong   fwrite_file_func (voidpf opaque, voidpf stream, const void* buf,uLong size);
extern ZPOS64_T ftell64_file_func (voidpf opaque, voidpf stream);
extern voidpf fopen64_file_func (voidpf opaque, const void* filename, int mode);
extern long    fseek64_file_func (voidpf opaque, voidpf stream, ZPOS64_T offset, int origin);
extern int     fclose_file_func (voidpf opaque, voidpf stream);
extern int     ferror_file_func (voidpf opaque, voidpf stream);
extern int mainZIPBuild(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
