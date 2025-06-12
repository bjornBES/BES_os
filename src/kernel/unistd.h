#pragma once

#include "defaultInclude.h"
#include "types.h"

#ifndef __OFF_T_TYPE__

typedef long int off_t;

#endif

#define ROUNDUP(sz, to)  (((sz)+to-1) & ~(to-1))
#define ROUNDDOWN(a, to) (((a)) & ~(to-1))

/* Synchronize at least the data part of a file with the underlying
   media.  */
int fdatasync (int fildes);

void sleepms(uint32_t ms);
void sleep(uint32_t sec);

char* getlogin(void);

int system(const char* command);