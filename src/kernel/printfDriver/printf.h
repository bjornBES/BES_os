
#pragma once

#include "defaultInclude.h"

#ifndef fd_t
typedef int fd_t;
#endif  
// The printf()-family functions return an `int`; it is therefore
// unnecessary/inappropriate to use size_t - often larger than int
// in practice - for non-negative related values, such as widths,
// precisions, offsets into buffers used for printing and the sizes
// of these buffers. instead, we use:
#define PRINTF_MAX_POSSIBLE_BUFFER_SIZE INT_MAX

#define PRINTF_ALIAS_STANDARD_FUNCTION_NAMES 0

// If you want to include this implementation file directly rather than
// link against, this will let you control the functions' visibility,
// e.g. make them static so as not to clash with other objects also
// using them.
#define PRINTF_VISIBILITY

// wrapper (used as buffer) for output function type
//
// One of the following must hold:
// 1. max_chars is 0
// 2. buffer is non-null
// 3. function is non-null
//
// ... otherwise bad things will happen.
typedef struct
{
    void *extra_function_arg;
    char *buffer;
    size_t pos;
    size_t max_chars;
    fd_t file;
} output_gadget_t;

void append_termination_with_gadget(output_gadget_t *gadget);
void format_string_loop(output_gadget_t *output, const char *format, va_list args);
output_gadget_t discarding_gadget(void);

output_gadget_t extern_putchar_gadget(void);

output_gadget_t buffer_gadget(char *buffer, size_t buffer_size);

output_gadget_t function_gadget();

// internal vsnprintf - used for implementing _all library functions
int vsnprintf_impl(output_gadget_t *output, const char *format, va_list args);

int vprintf(fd_t file, const char *format, va_list arg);

int vsnprintf(char *s, size_t n, const char *format, va_list arg);

int vsprintf(char *s, const char *format, va_list arg);

int vfctprintf(void (*out)(char c, void *extra_arg), void *extra_arg, const char *format, va_list arg);
