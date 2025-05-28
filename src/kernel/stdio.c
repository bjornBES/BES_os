#include <stdio.h>
#include <arch/i686/io.h>

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include <printfDriver/printf.h>

#include <hal/vfs.h>

char fputc(char c, fd_t file)
{
    VFS_Write(file, (uint8_t*)&c, sizeof(c));
    return c;
}

int fputs(const char* str, fd_t file)
{
    int count = 0;
    while(*str)
    {
        fputc(*str, file);
        str++;
        count++;
    }
    return count;
}

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

const char g_HexChars[] = "0123456789abcdef";

void fprintf_unsigned(fd_t file, unsigned long long number, int radix)
{
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do 
    {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        fputc(buffer[pos], file);
}

void fprintf_signed(fd_t file, long long number, int radix)
{
    if (number < 0)
    {
        fputc('-', file);
        fprintf_unsigned(file, -number, radix);
    }
    else fprintf_unsigned(file, number, radix);
}

int vfprintf(fd_t file, const char* fmt, va_list args)
{
    return vprintf(file, fmt, args);
}

int fprintf(fd_t file, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(file, fmt, args);
    va_end(args);
    return ret;
}

void fprint_buffer(fd_t file, const char* msg, const void* buffer, uint32_t count)
{
    const uint8_t* u8Buffer = (const uint8_t*)buffer;
    
    fputs(msg, file);
    for (uint16_t i = 0; i < count; i++)
    {
        fputc(g_HexChars[u8Buffer[i] >> 4], file);
        fputc(g_HexChars[u8Buffer[i] & 0xF], file);
    }
    fputs("\n", file);
}

char putc(char c)
{
    fputc(c, VFS_FD_STDOUT);
    return c;
}

int puts(const char* str)
{
    return fputs(str, VFS_FD_STDOUT);
}

int printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(VFS_FD_STDOUT, fmt, args);
    va_end(args);
    return ret;
}

int sprintf(char *s, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  const int ret = vsprintf(s, format, args);
  va_end(args);
  return ret;
}

int snprintf(char *s, size_t n, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  const int ret = vsnprintf(s, n, format, args);
  va_end(args);
  return ret;
}

void print_buffer(const char* msg, const void* buffer, uint32_t count)
{
    fprint_buffer(VFS_FD_STDOUT, msg, buffer, count);
}

void debugc(char c)
{
    fputc(c, VFS_FD_DEBUG);
}

void debugs(const char* str)
{
    fputs(str, VFS_FD_DEBUG);
}

void debugf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(VFS_FD_DEBUG, fmt, args);
    va_end(args);
}

void debug_buffer(const char* msg, const void* buffer, uint32_t count)
{
    fprint_buffer(VFS_FD_DEBUG, msg, buffer, count);
}
