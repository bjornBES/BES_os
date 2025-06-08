#pragma once

#include "types.h"

#define S_IRWXU 00700 // Read, write, execute/search by owner
#define S_IRUSR 00400 // Read permission, owner
#define S_IWUSR 00200 // Write permission, owner
#define S_IXUSR 00100 // Execute/search permission, owner
#define S_IRWXG 00070 // Read, write, execute/search by group
#define S_IRGRP 00040 // Read permission, group
#define S_IWGRP 00020 // Write permission, group
#define S_IXGRP 00010 // Execute/search permission, group
#define S_IRWXO 00007 // Read, write, execute/search by others
#define S_IROTH 00004 // Read permission, others
#define S_IWOTH 00002 // Write permission, others
#define S_IXOTH 00001 // Execute/search permission, others
#define S_ISUID 04000 // Set user ID on execution
#define S_ISGID 02000 // Set group ID on execution
#define S_ISVTX 01000 // Save text image after execution

typedef __mode_t mode_t;    /* Type of file attribute bitmasks.  */

#define O_APPEND 0x0008 // Open file in append mode
#define O_ASYNC 0x0010 // Open file for synchronous I/O
#define O_CLOEXEC 0x0020 // Close on exec
#define O_CREAT 0x0040 // Create file if it does not exist
#define O_DIRECT 0x0080 // Direct I/O
#define O_DIRECTORY 0x0100 // Open directory
#define O_DSYNC 0x0200 // Synchronous I/O data integrity
#define O_EXCL 0x0400 // Exclusive use
#define O_LARGEFILE 0x0800 // Allow large files
#define O_NOATIME 0x1000 // Do not update access time
#define O_NOCTTY 0x2000 // Do not assign controlling terminal
#define O_NOFOLLOW 0x4000 // Do not follow symbolic links
#define O_NONBLOCK 0x8000 // Non-blocking mode
#define O_PATH 0x10000 // Open file path
#define O_SYNC 0x20000 // Synchronous I/O
#define O_TMPFILE 0x40000 // Create temporary file
#define O_TRUNC 0x80000 // Truncate file to zero length

#define O_RDWR 0x0002 // Open file for reading and writing
#define O_RDONLY 0x0000 // Open file for reading only
#define O_WRONLY 0x0001 // Open file for writing only
#define O_BINARY 0x0000 // Open file in binary mode