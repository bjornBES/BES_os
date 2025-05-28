#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <hal/vfs.h>

char fputc(char c, fd_t file);
int fputs(const char* str, fd_t file);
int vfprintf(fd_t file, const char* fmt, va_list args);
int fprintf(fd_t file, const char* fmt, ...);
void fprint_buffer(fd_t file, const char* msg, const void* buffer, uint32_t count);
int snprintf(char *s, size_t n, const char *format, ...);
int sprintf(char *s, const char *format, ...);

char putc(char c);
int puts(const char* str);
int printf(const char* fmt, ...);
void print_buffer(const char* msg, const void* buffer, uint32_t count);

void debugc(char c);
void debugs(const char* str);
void debugf(const char* fmt, ...);
void debug_buffer(const char* msg, const void* buffer, uint32_t count);

#define panic() { printf("***KERNEL PANIC*** in %s at line %d\r\n", __FILE__, __LINE__); for (;;);}
