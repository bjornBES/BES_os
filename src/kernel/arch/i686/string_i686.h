#pragma once
#include "defaultInclude.h"

#define ASMCALL __attribute__((cdecl))

ASMCALL char* strcpy(char* dst, const char* src);
ASMCALL char* strncpy(char* dst, const char* src, size_t num);
ASMCALL char* strcat(char* dest, const char* src);
ASMCALL char* strncat(char* dest, const char* src, size_t num);
ASMCALL int strcmp(const char* a, const char* b);
ASMCALL int strcoll(const char* a, const char* b);
ASMCALL int strncmp(const char* a, const char* b, size_t num);
ASMCALL const char* strchr(const char* str, char chr);
ASMCALL uint32_t strlen(const char* str);
ASMCALL size_t strnlen(const char *str, size_t maxsize);
