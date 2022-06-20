#ifdef WIN32

#ifndef __TGDSTypes_h__
#define __TGDSTypes_h__

#include <stdio.h>
#include <stdint.h>
#define TGDSARM9Malloc malloc
#define TGDSARM9Calloc calloc
#define TGDSARM9Realloc realloc
#define TGDSARM9Free free

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef s8 sint8;
typedef s16	sint16;
typedef s32	sint32;

typedef s32	_ssize_t;

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef u32	mode_t;

typedef u8	uint8;
typedef u16	uint16;
typedef u32	uint32;

#ifdef WIN32
#ifndef __cplusplus
typedef unsigned char bool;
static bool false = 0;
static bool true = 1;
#endif
#endif

#endif

#endif
