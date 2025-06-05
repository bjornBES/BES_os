#include <stdio.h>
#include <arch/i686/io.h>

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "debug.h"

#include <printfDriver/printf.h>

#include <hal/vfs.h>

#define MODULE "stdio"

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

int fgetc(fd_t file)
{
    uint8_t c;
    int ret = VFS_Read(file, &c, sizeof(c));
    if (ret <= 0)
        return EOF; // Error or EOF
    return c; // Return the character read
}
int getc(fd_t file)
{
    return fgetc(file);
}

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

int sscanf(const char *s, const char *format, ...)
{
    FUNC_NOT_IMPLEMENTED(MODULE, "sscanf");
    return EOF; // Placeholder for sscanf function, should be implemented
}

void print_buffer(const char* msg, const void* buffer, uint32_t count)
{
    fprint_buffer(VFS_FD_STDOUT, msg, buffer, count);
}

fd_t fileno(fd_t stream)
{
    if (stream < VFS_FD_START || stream >= MAX_FILE_HANDLES)
    {
        log_err(MODULE, "fileno: Invalid file descriptor: %d", stream);
        return VFS_INVALID_FD; // Invalid file descriptor
    }
    return stream; // In this case, stream is already a file descriptor
}

int fread(const void* buf, size_t size, size_t count, fd_t stream)
{
    size_t total_read = 0;
    size_t bytes_to_read = size * count;
    uint8_t* u8Buffer = (uint8_t*)buf;

    while (bytes_to_read > 0)
    {
        size_t read = VFS_Read(stream, u8Buffer + total_read, bytes_to_read);
        if (read <= 0)
            break; // EOF or error
        total_read += read;
        bytes_to_read -= read;
    }

    return total_read / size; // return number of items read
}

size_t read(fd_t stream, void* buf, size_t count)
{
    fread(buf, 1, count, stream);
    return count; // Return the number of bytes written
}

int fwrite(void* buf, size_t size, size_t count, fd_t stream)
{
    size_t total_read = 0;
    size_t bytes_to_read = size * count;
    uint8_t* u8Buffer = (uint8_t*)buf;

    while (bytes_to_read > 0)
    {
        size_t read = VFS_Write(stream, u8Buffer + total_read, bytes_to_read);
        if (read <= 0)
            break; // EOF or error
        total_read += read;
        bytes_to_read -= read;
    }

    return total_read / size; // return number of items read
}

size_t write(fd_t stream, void* buf, size_t count)
{
    fwrite(buf, 1, count, stream);
    return count; // Return the number of bytes written
}

int lseek(fd_t stream, int offset, int whence)
{
    uint64_t new_offset = 0;

    

    switch (whence)
    {
        case SEEK_SET:
            new_offset = offset; // Set to absolute position
            break;
        case SEEK_CUR:
            new_offset = VFS_GetOffset(stream) + offset; // Move relative to current position
            break;
        case SEEK_END:
            new_offset = VFS_GetSize(stream) + offset; // Move relative to end of file
            break;
        default:
            log_err(MODULE, "lseek: Invalid whence value: %d", whence);
            return -1; // Invalid whence value
    }

    if (!VFS_Seek(stream, new_offset))
    {
        log_err(MODULE, "lseek: Failed to seek to offset %llu in stream %d", new_offset, stream);
        return -1; // Seek failed
    }

    return new_offset; // Success
}

int fseek(fd_t stream, int offset, int whence)
{
    log_debug(MODULE, "fseek: stream = %d, offset = %d, whence = %d", stream, offset, whence);
    return lseek(stream, offset, whence); // Delegate to lseek
}

void rewind(fd_t stream)
{
    VFS_Seek(stream, 0); // Reset the file offset to the beginning
}

int ftell(fd_t stream)
{
    log_debug(MODULE, "ftell: stream = %d", stream);
    int offset = VFS_GetOffset(stream);
    if (offset < 0)
    {
        log_err(MODULE, "ftell: Failed to get offset for stream %d", stream);
        return -1; // Error getting offset
    }
    return offset; // Return the current file offset
}

fd_t open(const char* filename, int flags, ...)
{
    va_list args;
    va_start(args, flags);
    log_debug(MODULE, "open: filename = %s, flags = %d", filename, flags);
    FUNC_NOT_IMPLEMENTED(MODULE, "open");
    va_end(args);
    return VFS_INVALID_FD; // Placeholder for open function, should be implemented    
}

fd_t fopen(const char* filename, const char* mode)
{
    log_debug(MODULE, "fopen: filename = %s, mode = %s", filename, mode);
    FUNC_NOT_IMPLEMENTED(MODULE, "fopen");
    return VFS_INVALID_FD; // Placeholder for fopen function, should be implemented
}

fd_t fdopen(fd_t fildes, const char* mode)
{
    log_debug(MODULE, "fdopen: fildes = %d, mode = %s", fildes, mode);
    FUNC_NOT_IMPLEMENTED(MODULE, "fdopen");
    return VFS_INVALID_FD; // Placeholder for fdopen function, should be implemented
}

int close(fd_t stream)
{
    if (VFS_Close(stream) == false)
    {
        log_err(MODULE, "fclose: Failed to close stream %d", stream);
        return EOF; // Return EOF on failure
    }
    return 0; // Return 0 on success, as per the standard
}

int fclose(fd_t stream)
{
    return close(stream); // Delegate to close
}

int fflush(fd_t stream)
{
    log_debug(MODULE, "fflush: stream = %d", stream);
    if (stream < VFS_FD_START || stream >= MAX_FILE_HANDLES)
    {
        log_err(MODULE, "fflush: Invalid file descriptor: %d", stream);
        return EOF; // Invalid file descriptor
    }
    // In this case, we assume that flushing is a no-op for the VFS
    return 0; // Return 0 on success, as per the standard
}

int rename(const char* oldname, const char* newname)
{
    log_debug(MODULE, "rename: oldname = %s, newname = %s", oldname, newname);
    FUNC_NOT_IMPLEMENTED(MODULE, "rename");
    return -1; // Placeholder for rename function, should be implemented
}
int remove(const char* filename)
{
    log_debug(MODULE, "remove: filename = %s", filename);
    FUNC_NOT_IMPLEMENTED(MODULE, "remove");
    return -1; // Placeholder for remove function, should be implemented
}