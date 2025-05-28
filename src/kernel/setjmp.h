#pragma once

#include "wordsize.h"

#define _SIGSET_NWORDS (1024 / (8 * sizeof (unsigned long int)))
typedef struct
{
    unsigned long int __val[_SIGSET_NWORDS];
} __sigset_t;

#ifndef _ASM

# if __WORDSIZE == 64
typedef long int __jmp_buf[8];
# else
typedef int __jmp_buf[6];
# endif

#endif

/* Calling environment, plus possibly a saved signal mask.  */
struct __jmp_buf_tag
{
	unsigned int regs[32];
};
typedef struct __jmp_buf_tag jmp_buf[1];

int	setjmp( jmp_buf env );

void longjmp( jmp_buf env, int val );