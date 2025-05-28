#pragma once

#include "defaultInclude.h"

#ifndef EOF
#define EOF (-1)
#endif

/* Buffering mode used by setvbuf.  */
#define _IOFBF 0	/* Fully buffered. */
#define _IOLBF 1	/* Line buffered. */
#define _IONBF 2	/* No buffering. */

/* open/fcntl.  */
#define O_ACCMODE	   0003
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#ifndef O_CREAT
# define O_CREAT	   0100	/* Not fcntl.  */
#endif
#ifndef O_EXCL
# define O_EXCL		   0200	/* Not fcntl.  */
#endif
#ifndef O_NOCTTY
# define O_NOCTTY	   0400	/* Not fcntl.  */
#endif
#ifndef O_TRUNC
# define O_TRUNC	  01000	/* Not fcntl.  */
#endif
#ifndef O_APPEND
# define O_APPEND	  02000
#endif
#ifndef O_NONBLOCK
# define O_NONBLOCK	  04000
#endif
#ifndef O_NDELAY
# define O_NDELAY	O_NONBLOCK
#endif
#ifndef O_SYNC
# define O_SYNC	       04010000
#endif
#define O_FSYNC		O_SYNC
#ifndef O_ASYNC
# define O_ASYNC	 020000
#endif
#ifndef __O_LARGEFILE
# define __O_LARGEFILE	0100000
#endif

#ifndef __O_DIRECTORY
# define __O_DIRECTORY	0200000
#endif
#ifndef __O_NOFOLLOW
# define __O_NOFOLLOW	0400000
#endif
#ifndef __O_CLOEXEC
# define __O_CLOEXEC   02000000
#endif
#ifndef __O_DIRECT
# define __O_DIRECT	 040000
#endif
#ifndef __O_NOATIME
# define __O_NOATIME   01000000
#endif
#ifndef __O_PATH
# define __O_PATH     010000000
#endif
#ifndef __O_DSYNC
# define __O_DSYNC	 010000
#endif
#ifndef __O_TMPFILE
# define __O_TMPFILE   (020000000 | __O_DIRECTORY)
#endif

#ifndef F_GETLK
# if !defined __USE_FILE_OFFSET64 && __TIMESIZE != 64
#  define F_GETLK	5	/* Get record locking info.  */
#  define F_SETLK	6	/* Set record locking info (non-blocking).  */
#  define F_SETLKW	7	/* Set record locking info (blocking).  */
# else
#  define F_GETLK	F_GETLK64  /* Get record locking info.  */
#  define F_SETLK	F_SETLK64  /* Set record locking info (non-blocking).*/
#  define F_SETLKW	F_SETLKW64 /* Set record locking info (blocking).  */
# endif
#endif
#ifndef F_GETLK64
# define F_GETLK64	12	/* Get record locking info.  */
# define F_SETLK64	13	/* Set record locking info (non-blocking).  */
# define F_SETLKW64	14	/* Set record locking info (blocking).  */
#endif

# define SEEK_SET	0	/* Seek from beginning of file.  */
# define SEEK_CUR	1	/* Seek from current position.  */
# define SEEK_END	2	/* Seek from end of file.  */

#ifndef FILEDEF
#define FILEDEF 1
typedef struct _FILE {
	char dummy[1];
} FILE;
#endif

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define STDDEBUG_FILENO 3

static __attribute__((unused)) FILE* const stdin  = (FILE*)(intptr_t)~STDIN_FILENO;
static __attribute__((unused)) FILE* const stdout = (FILE*)(intptr_t)~STDOUT_FILENO;
static __attribute__((unused)) FILE* const stderr = (FILE*)(intptr_t)~STDERR_FILENO;
static __attribute__((unused)) FILE* const stddebug = (FILE*)(intptr_t)~STDDEBUG_FILENO;

int remove(const char* filename);
int rename(const char* old, const char* new);

FILE *fdopen(int fd, const char* mode);
FILE *fopen(const char* filename, const char* mode);
int feof (FILE *__stream);
int ferror(FILE* stream);
int fileno(FILE *stream);
int fflush(FILE* stream);
int fclose(FILE* stream);

int ungetc (int c, FILE *stream);

int flockfile(FILE* stream);
int funlockfile(FILE* stream);

int close (int fd);
int open (const char* path, int oflag, ...);

#define getc(stream) fgetc(stream)
int fgetc(FILE* stream);
int getchar();

#define putc(c, stream) fputc(c, stream)
int fputc(char c, FILE *stream);
int putchar(char c);

int _fwrite(const void* buf, size_t size, FILE * stream);
int fwrite(const void* buf, size_t size, size_t nmemb, FILE * stream);

int _fread(const void* buf, size_t size, FILE * stream);
int fread(const void* buf, size_t size, size_t nmemb, FILE * stream);

int fputs(const char* s, FILE* stream);
int puts(const char* s);

char* fgets (char *s, int size, FILE *stream);

int vfprintf(FILE *stream, const char* fmt, va_list args);
int fprintf(FILE *stream, char* fmt, ...);
int printf(char* fmt, ...);

int sprintf(char *s, char *fmt, ...);
int snprintf(char *s, size_t count, char *fmt, ...);

int vsscanf(const char* str, const char* format, va_list args);
int sscanf(const char* str, const char* format, ...);

int fseek (FILE *stream, long int off, int whence);
long int ftell (FILE *stream);
int rewind(FILE* stream);

size_t write(int fd, const void* buf, size_t count);
size_t read(int fd, void* buf, size_t count);
