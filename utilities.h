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

#endif

extern void showMenu(char * appName);
extern int convertbin2c(int argc, char *argv[]);
extern int convertMP4toTVS(int argc, char *argv[] );
extern int TGDSPKGBuilder(int argc, char *argv[] );
