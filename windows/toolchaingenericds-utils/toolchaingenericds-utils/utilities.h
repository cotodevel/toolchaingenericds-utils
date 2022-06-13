#ifndef __utilities_h__
#define __utilities_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>

#endif


extern bool cv_snprintf(char* buf, int len, const char* fmt, ...);
extern void showMenu(char * appName);


extern int convertbin2c(int argc, char *argv[]);
extern int convertbin2lzss(int argc, char **argv);
	/*----------------------------------------------------------------------------*/
	void  Title(void);
	void  Usage(void);

	char *Load(char *filename, int *length, int min, int max);
	void  Save(char *filename, char *buffer, int length);
	char *Memory(int length, int size);

	void  LZS_Decode(char *filename);
	void  LZS_Encode(char *filename, int mode);
	char *LZS_Code(unsigned char *raw_buffer, int raw_len, int *new_len, int best);

	char *LZS_Fast(unsigned char *raw_buffer, int raw_len, int *new_len);
	void  LZS_InitTree(void);
	void  LZS_InsertNode(int r);
	void  LZS_DeleteNode(int p);
