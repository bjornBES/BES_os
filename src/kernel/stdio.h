#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <hal/vfs.h>


static __attribute__((unused)) fd_t const stdin = (fd_t)VFS_FD_STDIN;
static __attribute__((unused)) fd_t const stdout = (fd_t)VFS_FD_STDOUT;
static __attribute__((unused)) fd_t const stderr = (fd_t)VFS_FD_STDERR;
static __attribute__((unused)) fd_t const stddebug = (fd_t)VFS_FD_DEBUG;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifndef EOF
#define EOF (-1)
#endif

char fputc(char c, fd_t file);
char putc(char c);

int fputs(const char *str, fd_t file);
int puts(const char *str);
#define putchar(c) fputc(c, stdout)

int fgetc(fd_t file);
int getc(fd_t file);
#define getchar() fgetc(stdin)

int fprintf(fd_t file, const char *fmt, ...);
int vfprintf(fd_t file, const char *fmt, va_list args);
int printf(const char *fmt, ...);
int snprintf(char *s, size_t n, const char *format, ...);
int sprintf(char *s, const char *format, ...);
void fprint_buffer(fd_t file, const char *msg, const void *buffer, uint32_t count);

int sscanf(const char *s, const char *format, ...);

void print_buffer(const char *msg, const void *buffer, uint32_t count);

fd_t fileno(fd_t stream);

int lseek(fd_t stream, int offset, int whence);
int fseek(fd_t stream, int offset, int whence);
void rewind(fd_t stream);

int ftell(fd_t stream);

int fread(const void *buf, size_t size, size_t count, fd_t stream);
size_t read(fd_t stream, void *buf, size_t count);

int fwrite(void *buf, size_t size, size_t count, fd_t stream);
size_t write(fd_t stream, void *buf, size_t count);

fd_t open(const char *filename, int flags, ...);
fd_t fopen(const char *filename, const char *mode);
fd_t fdopen(fd_t fildes, const char *mode);

int close(fd_t stream);
int fclose(fd_t stream);

int fflush(fd_t stream);

int rename(const char *oldname, const char *newname);
int remove(const char *filename);

#define panic()                                                                \
    {                                                                          \
        printf("***KERNEL PANIC*** in %s at line %d\r\n", __FILE__, __LINE__); \
        for (;;)                                                               \
            ;                                                                  \
    }
