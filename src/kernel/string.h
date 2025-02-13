#pragma once
#include <stddef.h>
#include <stdint.h>

typedef char* string;

const char* strchr(const char* str, char chr);
char* strcpy(char* dst, const char* src);
uint32_t strlen(const char* str);
int strcmp(const char* a, const char* b);

void atoi(char* str, int* a);

uint32_t strcrl(string str, const char what, const char with);
uint32_t str_begins_with(string str, string with);
uint32_t str_backspace(string str, char c);
uint32_t strcount(string str, char c);
uint32_t strsplit(string str, char delim);

wchar_t* utf16_to_codepoint(wchar_t* string, int* codepoint);
char* codepoint_to_utf8(int codepoint, char* stringOutput);