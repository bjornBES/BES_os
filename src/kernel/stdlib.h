#pragma once
#include "defaultInclude.h"

/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

void exit (int __status);

void abort();

void qsort(void* base,
           size_t num,
           size_t size,
           int (*compar)(const void*, const void*));

char *getenv(const char *match);
char *getenvOW(const char *match, int overwrite);
int setenv(const char* var, const char* val);
int setenvOW(const char* var, const char* val, int overwrite);
int putenv(const char* var);
int unsetenv(const char* var);