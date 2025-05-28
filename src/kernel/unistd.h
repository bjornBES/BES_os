#pragma once

#include "defaultInclude.h"
#include "types.h"

#ifndef __OFF_T_TYPE__

typedef long int off_t;

#endif

/* Synchronize at least the data part of a file with the underlying
   media.  */
int fdatasync (int fildes);

void sleepms(uint32_t ms);
void sleep(uint32_t sec);

int access(const char* name, int type);

int rmdir(const char* name);
int chdir(const char* name);
char* getcwd(const char* buf, size_t size);
int unlink(const char* name);
off_t lseek(int fd, off_t offset, int whence);