#pragma once
#include <stddef.h>

typedef char* string;

const char* strchr(const char* str, char chr);
char* strcpy(char* dst, const char* src);
unsigned strlen(const char* str);
int strcmp(const char* a, const char* b);

void atoi(char* str, int* a);

size_t strcrl(string str, const char what, const char with);
size_t str_begins_with(const string str, const string with);
size_t str_backspace(string str, char c);
size_t strcount(string str, char c);
size_t strsplit(string str, char delim);

wchar_t* utf16_to_codepoint(wchar_t* string, int* codepoint);
char* codepoint_to_utf8(int codepoint, char* stringOutput);