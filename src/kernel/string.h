#pragma once
#include "defaultInclude.h"


#define UNICODE_COMBINING_ACUTE 0x0301
#define UNICODE_COMBINING_CARON 0x030C

#ifdef __i686__
#include "arch/i686/string_i686.h"
#include "arch/i686/memory_i686.h"
#endif
int strcmp_debug(const char *a, const char *b);
int strcasecmp(const char *a, const char *b);
int strncasecmp(const char *a, const char *b, size_t count);

typedef char* string;

void itoa(char *buf, uint32_t n, int base);
void atoi(char* str, int* a);
size_t atou(const char* str);
const char* atou_return(const char *str, size_t* result);

uint32_t strcrl(string str, const char what, const char with);
uint32_t str_begins_with(string str, string with);
uint32_t str_backspace(string str, char c);
uint32_t strcount(string str, char c);
char* strtok(string str, const char* delim);


uint16_t* utf16_to_codepoint(uint16_t* string, int* codepoint);
char* codepoint_to_utf8(int codepoint, char* stringOutput);
