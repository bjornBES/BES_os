#pragma once

#define __LIBCOB_VERSION	3
#define __LIBCOB_VERSION_MINOR		2
#define __LIBCOB_VERSION_PATCHLEVEL	0	/* Note: possibly differs from patchelvel shown with cobc --version! */

#define __LIBCOB_RELEASE (__LIBCOB_VERSION * 10000 + __LIBCOB_VERSION_MINOR * 100 + __LIBCOB_VERSION_PATCHLEVEL)

#ifndef COB_EXPIMP
#if	((defined(_WIN32) || defined(__CYGWIN__)) && !defined(__clang__))
#define COB_EXPIMP	__declspec(dllimport) extern
#else
#define COB_EXPIMP	extern
#endif
#endif

COB_EXPIMP const char *libcob_version (void);
COB_EXPIMP int		set_libcob_version (int *, int *, int *);

COB_EXPIMP void		print_version (void);
COB_EXPIMP void		print_version_summary (void);