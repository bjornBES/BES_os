#include "stdio2.h"

#include "hal/vfs.h"
#include "printfDriver/printf.h"

FILE *fdopen(int fd, const char *mode)
{
    return (FILE *)(intptr_t)~0xFFFFFFFF; // Return a dummy FILE pointer
}

int fileno(FILE *stream)
{
    intptr_t i = (intptr_t)stream;
    if (i >= 0)
    {
        return -1;
    }
    return ~i;
}

int fflush(FILE *stream)
{
    intptr_t i = (intptr_t)stream;

    if (i > 0)
    {
        return -1;
    }
    return 0;
}

int feof (FILE *__stream)
{
    return 0;
}

int ferror(FILE* stream)
{
    return 0;
}

int fclose(FILE *stream)
{
    return 0;
}

int flockfile(FILE* stream)
{
    return 0;
}

int funlockfile(FILE* stream)
{
    return 0;
}

int ungetc (int c, FILE *stream)
{
    // ungetc is not implemented, so we return EOF
    // as per the standard, this is not an error
    return EOF;
}

int fgetc(FILE* stream)
{
    return 0; // Return EOF for now, as we don't have a real implementation
}
int getchar()
{
    return fgetc(stdin);
}

int _fwrite(const void *buf, size_t size, FILE *stream)
{
    size_t ret;
    int fd = fileno(stream);
    while (size)
    {
        uint8_t* u8Buffer = (uint8_t*)buf;
        ret = VFS_Write((fd_t)fd, u8Buffer, size);
        if (ret <= 0)
        {
            return EOF;
        }
        size -= ret;
        buf += ret;
    }
    return 0;
}

int fwrite(const void *buf, size_t size, size_t nmemb, FILE *stream)
{
    size_t written;

    for (written = 0; written < nmemb; written++)
    {
        if (_fwrite(buf, size, stream) != 0)
            break;
        buf += size;
    }
    return written;
}

char *fgets(char *s, int size, FILE *stream)
{
    return NULL;
}

int vsscanf(const char *str, const char *format, va_list args)
{
    return -1;
}

int sscanf(const char *str, const char *format, ...)
{
    fprintf(stddebug, "called function sscanf");
    return -1;
}
