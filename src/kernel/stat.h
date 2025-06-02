#pragma once
#include "types.h"

struct stat
{
#ifdef __USE_TIME_BITS64
#include <bits/struct_stat_time64_helper.h>
#else
    __dev_t st_dev; /* Device.  */
#ifndef __x86_64__
    unsigned short int __pad1;
#endif
#if defined __x86_64__ || !defined __USE_FILE_OFFSET64
    __ino_t st_ino; /* File serial number.	*/
#else
    __ino_t __st_ino; /* 32bit file serial number.	*/
#endif
#ifndef __x86_64__
    __mode_t st_mode;   /* File mode.  */
    __nlink_t st_nlink; /* Link count.  */
#else
    __nlink_t st_nlink; /* Link count.  */
    __mode_t st_mode;   /* File mode.  */
#endif
    __uid_t st_uid; /* User ID of the file's owner.	*/
    __gid_t st_gid; /* Group ID of the file's group.*/
#ifdef __x86_64__
    int __pad0;
#endif
    __dev_t st_rdev; /* Device number, if device.  */
#ifndef __x86_64__
    unsigned short int __pad2;
#endif
#if defined __x86_64__ || !defined __USE_FILE_OFFSET64
    __off_t st_size; /* Size of file, in bytes.  */
#else
    __off64_t st_size; /* Size of file, in bytes.  */
#endif
    __blksize_t st_blksize; /* Optimal block size for I/O.  */
#if defined __x86_64__ || !defined __USE_FILE_OFFSET64
    __blkcnt_t st_blocks; /* Number 512-byte blocks allocated. */
#else
    __blkcnt64_t st_blocks; /* Number 512-byte blocks allocated. */
#endif
#ifdef __USE_XOPEN2K8
    /* Nanosecond resolution timestamps are stored in a format
       equivalent to 'struct timespec'.  This is the type used
       whenever possible but the Unix namespace rules do not allow the
       identifier 'timespec' to appear in the <sys/stat.h> header.
       Therefore we have to handle the use of this header in strictly
       standard-compliant sources special.  */
    struct timespec st_atim; /* Time of last access.  */
    struct timespec st_mtim; /* Time of last modification.  */
    struct timespec st_ctim; /* Time of last status change.  */
#define st_atime st_atim.tv_sec /* Backward compatibility.  */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
#else
    __time_t st_atime;              /* Time of last access.  */
    __syscall_ulong_t st_atimensec; /* Nscecs of last access.  */
    __time_t st_mtime;              /* Time of last modification.  */
    __syscall_ulong_t st_mtimensec; /* Nsecs of last modification.  */
    __time_t st_ctime;              /* Time of last status change.  */
    __syscall_ulong_t st_ctimensec; /* Nsecs of last status change.  */
#endif
#ifdef __x86_64__
    __syscall_slong_t __glibc_reserved[3];
#else
#ifndef __USE_FILE_OFFSET64
    unsigned long int __glibc_reserved4;
    unsigned long int __glibc_reserved5;
#else
    __ino64_t st_ino; /* File serial number.	*/
#endif
#endif
#endif /* __USE_TIME_BITS64  */
};

/* Encoding of the file mode.  */

#define	__S_IFMT	0170000	/* These bits determine file type.  */

/* File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFIFO	0010000	/* FIFO.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */


#if defined __USE_MISC || defined __USE_XOPEN
#define S_IFMT __S_IFMT
#define S_IFDIR __S_IFDIR
#define S_IFCHR __S_IFCHR
#define S_IFBLK __S_IFBLK
#define S_IFREG __S_IFREG
#ifdef __S_IFIFO
#define S_IFIFO __S_IFIFO
#endif
#ifdef __S_IFLNK
#define S_IFLNK __S_IFLNK
#endif
#if (defined __USE_MISC || defined __USE_XOPEN_EXTENDED) && defined __S_IFSOCK
#define S_IFSOCK __S_IFSOCK
#endif
#endif

/* Test macros for file types.	*/

#define __S_ISTYPE(mode, mask) (((mode) & __S_IFMT) == (mask))

#define S_ISDIR(mode) __S_ISTYPE((mode), __S_IFDIR)
#define S_ISCHR(mode) __S_ISTYPE((mode), __S_IFCHR)
#define S_ISBLK(mode) __S_ISTYPE((mode), __S_IFBLK)
#define S_ISREG(mode) __S_ISTYPE((mode), __S_IFREG)
#ifdef __S_IFIFO
#define S_ISFIFO(mode) __S_ISTYPE((mode), __S_IFIFO)
#endif
#ifdef __S_IFLNK
#define S_ISLNK(mode) __S_ISTYPE((mode), __S_IFLNK)
#endif

int stat(const char* path, struct stat* buf);
int fstat(int fd, struct stat* buf);