#pragma once

#include "types.h"

#ifdef __x86_64__
# define __O_LARGEFILE	0
#endif

/* For posix fcntl() and `l_type' field of a `struct flock' for lockf().  */
# define F_RDLCK		0	/* Read lock.  */
# define F_WRLCK		1	/* Write lock.  */
# define F_UNLCK		2	/* Remove lock.  */

struct flock
{
    short int l_type;   /* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
    short int l_whence; /* Where `l_start' is relative to (like `lseek').  */
#ifndef __USE_FILE_OFFSET64
    __off_t l_start; /* Offset where the lock begins.  */
    __off_t l_len;   /* Size of the locked area; zero means until EOF.  */
#else
    __off64_t l_start; /* Offset where the lock begins.  */
    __off64_t l_len;   /* Size of the locked area; zero means until EOF.  */
#endif
    __pid_t l_pid; /* Process holding the lock.  */
};