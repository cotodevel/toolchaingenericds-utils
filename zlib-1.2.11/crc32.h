#ifndef ZLIB_CRC32__H_

#include <string.h>

extern unsigned long ZEXPORT crc32_z(unsigned long crc,
    const unsigned char FAR *buf,
    z_size_t len);
	
	
#endif