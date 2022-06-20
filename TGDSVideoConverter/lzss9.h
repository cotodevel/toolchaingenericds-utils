/*----------------------------------------------------------------------------*/
/*--  lzss.c - LZSS coding for Nintendo GBA/DS                              --*/
/*--  Copyright (C) 2011 CUE                                                --*/
/*--                                                                        --*/
/*--  This program is free software: you can redistribute it and/or modify  --*/
/*--  it under the terms of the GNU General Public License as published by  --*/
/*--  the Free Software Foundation, either version 3 of the License, or     --*/
/*--  (at your option) any later version.                                   --*/
/*--                                                                        --*/
/*--  This program is distributed in the hope that it will be useful,       --*/
/*--  but WITHOUT ANY WARRANTY; without even the implied warranty of        --*/
/*--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          --*/
/*--  GNU General Public License for more details.                          --*/
/*--                                                                        --*/
/*--  You should have received a copy of the GNU General Public License     --*/
/*--  along with this program. If not, see <http://www.gnu.org/licenses/>.  --*/
/*----------------------------------------------------------------------------*/


#ifndef __lzss9_h__
#define __lzss9_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TGDSTypes.h"
//LZSS
struct LZSSContext {
	u8 * bufferSource;
	int bufferSize;
};

/*----------------------------------------------------------------------------*/
#define CMD_DECODE    0x00       // decode
#define CMD_CODE_10   0x10       // LZSS magic number

#define LZS_NORMAL    0x00       // normal mode, (0)
#define LZS_FAST      0x80       // fast mode, (1 << 7)
#define LZS_BEST      0x40       // best mode, (1 << 6)

#define LZS_WRAM      0x00       // VRAM not compatible (LZS_WRAM | LZS_NORMAL)
#define LZS_VRAM      0x01       // VRAM compatible (LZS_VRAM | LZS_NORMAL)
#define LZS_WFAST     0x80       // LZS_WRAM fast (LZS_WRAM | LZS_FAST)
#define LZS_VFAST     0x81       // LZS_VRAM fast (LZS_VRAM | LZS_FAST)
#define LZS_WBEST     0x40       // LZS_WRAM best (LZS_WRAM | LZS_BEST)
#define LZS_VBEST     0x41       // LZS_VRAM best (LZS_VRAM | LZS_BEST)

#define LZS_SHIFT     1          // bits to shift
#define LZS_MASK      0x80       // bits to check:
                                 // ((((1 << LZS_SHIFT) - 1) << (8 - LZS_SHIFT)

#define LZS_THRESHOLD 2          // max number of bytes to not encode
#define LZS_N         0x1000     // max offset (1 << 12)
#define LZS_F         0x12       // max coded ((1 << 4) + LZS_THRESHOLD)
#define LZS_NIL       LZS_N      // index for root of binary search trees

#define RAW_MINIM     0x00000000 // empty file, 0 bytes
#define RAW_MAXIM     0x00FFFFFF // 3-bytes length, 16MB - 1

#define LZS_MINIM     0x00000004 // header only (empty RAW file)
#define LZS_MAXIM     0x01400000 // 0x01200003, padded to 20MB:
                                 // * header, 4
                                 // * length, RAW_MAXIM
                                 // * flags, (RAW_MAXIM + 7) / 8
                                 // 4 + 0x00FFFFFF + 0x00200000 + padding

//LZSS end

#define BREAK(text) { printf(text); return; }
#define EXIT(text)  { printf(text); while(1==1); }

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void  Title(void);
extern void  Usage(void);

extern char *Load(char *filenameIn, int *length, int min, int max);
extern void  Save(char *filenameOut, char *buffer, int length);
extern char *Memory(int length, int size);

extern bool LZS_Decode(const char *filenameIn, const char *filenameOut);
extern void LZS_Encode(const char *filenameIn, const char *filenameOut);
extern char *LZS_Code(unsigned char *raw_buffer, int raw_len, int *new_len, int best);

extern char *LZS_Fast(unsigned char *raw_buffer, int raw_len, int *new_len);
extern void  LZS_InitTree(void);
extern void  LZS_InsertNode(int r);
extern void  LZS_DeleteNode(int p);

extern unsigned char ring[LZS_N + LZS_F - 1];
extern int           dad[LZS_N + 1], lson[LZS_N + 1], rson[LZS_N + 1 + 256];
extern int           pos_ring, len_ring, lzs_vram;
extern void LZS_EncodeFromBuffer(unsigned char *raw_buffer, unsigned int raw_len, unsigned char *pak_buffer, unsigned int *pak_len);
extern void LZS_DecodeFromBuffer(unsigned char *pak_buffer, unsigned int pak_len, unsigned char * raw_buffer, unsigned int * raw_len);
#ifdef __cplusplus
}
#endif
