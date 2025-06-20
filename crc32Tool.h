#ifndef __crc32_h__
#define __crc32_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#if !defined(WIN32)
#include <unistd.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int Crc32_ComputeFile(FILE *file, uint32_t *outCrc32 );
extern unsigned long Crc32_ComputeBuf( unsigned long inCrc32, const void *buf,size_t bufLen );

#ifdef __cplusplus
}
#endif
