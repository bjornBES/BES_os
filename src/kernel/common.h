#ifndef H_COMMON
#define H_COMMON

#define STATUS_GOOD 1
#define STATUS_ERROR 0

#define STATUS_TRUE 1
#define STATUS_FALSE 0

#define BIG_ENDIAN_WORD(x) (((x & 0xFF)<<8) | ((x>>8) & 0xFF))
#define BIG_ENDIAN_DWORD(x) (((x & 0x000000FF)<<24) | ((x & 0x0000FF00)<<8) | ((x & 0x00FF0000)>>8) | ((x & 0xFF000000)>>24))

#endif