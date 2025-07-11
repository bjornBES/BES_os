/*
   Copyright (C) 2001-2012, 2014-2023 Free Software Foundation, Inc.
   Written by Keisuke Nishida, Roger While, Simon Sobisch, Ron Norman

   This file is part of GnuCOBOL.

   The GnuCOBOL runtime library is free software: you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   GnuCOBOL is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GnuCOBOL.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "tarstamp.h"
#include "libcob-config.h"

#include <stdio.h>
#include "printfDriver/printf.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#ifdef	HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>
#include <types.h>
#include <stat.h>
#include <errno.h>
#include "malloc.h"

#include <math.h>
#ifdef HAVE_FINITE_IEEEFP_H
#include <ieeefp.h>
#endif

#include <unistd.h>
#ifdef	HAVE_UNISTD_H
#else
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif
#endif
#ifdef	HAVE_SYS_TIME_H
#include "time.h"
#endif
#ifdef	HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef	_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef MOUSE_MOVED
#include <process.h>
#include <io.h>
#include <fcntl.h>	/* for _O_BINARY only */
#endif

#ifdef	HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifndef SIGFPE
#ifndef NSIG
#define NSIG 240
#endif
#define SIGFPE NSIG + 1
#endif

#ifdef	HAVE_LOCALE_H
#include <locale.h>
#endif

/* library headers for version output */
#ifdef _WIN32
#ifndef __GMP_LIBGMP_DLL
#define __GMP_LIBGMP_DLL 1
#endif
#endif
#ifdef	HAVE_GMP_H
#include "libs/GMP/gmp-impl.h"
#elif defined HAVE_MPIR_H
#include <mpir.h>
#else
#error either HAVE_GMP_H or HAVE_MPIR_H needs to be defined
#endif

#ifdef	WITH_DB
#include <db.h>
#endif

#if defined (HAVE_NCURSESW_NCURSES_H)
#include <ncursesw/ncurses.h>
#define COB_GEN_SCREENIO
#elif defined (HAVE_NCURSESW_CURSES_H)
#include <ncursesw/curses.h>
#define COB_GEN_SCREENIO
#elif defined (HAVE_NCURSES_H)
#include <ncurses.h>
#define COB_GEN_SCREENIO
#elif defined (HAVE_NCURSES_NCURSES_H)
#include <ncurses/ncurses.h>
#define COB_GEN_SCREENIO
#elif defined (HAVE_PDCURSES_H)
#define PDC_NCMOUSE		/* use ncurses compatible mouse API */
#include <pdcurses.h>
#define COB_GEN_SCREENIO
#elif defined (HAVE_CURSES_H)
#define PDC_NCMOUSE	/* see comment above */
#include <curses/curses.h>
#define COB_GEN_SCREENIO
#ifndef PDC_MOUSE_MOVED
#undef PDC_NCMOUSE
#endif
#endif

#if defined (__PDCURSES__)
/* Note: PDC will internally define NCURSES_MOUSE_VERSION with
   a recent version when PDC_NCMOUSE was defined;
   for older version define manually! */
#endif

#if defined (WITH_XML2)
#include <libxml/xmlversion.h>
#include <libxml/xmlwriter.h>
#endif

#if defined (WITH_CJSON)
#if defined (HAVE_CJSON_CJSON_H)
#include <cjson/cJSON.h>
#elif defined (HAVE_CJSON_H)
#include <cJSON.h>
#else
#error CJSON without necessary header
#endif
#elif defined (WITH_JSON_C)
#if defined (HAVE_JSON_C_JSON_H)
#include <json-c/json_c_version.h>
#elif defined (HAVE_JSON_H)
#include <json_c_version.h>
#else
#error JSON-C without necessary header
#endif
#endif

/* end of library headers */

/* include internal and external libcob definitions, forcing exports */
#define	COB_LIB_EXPIMP
#include "coblocal.h"

#include "cobgetopt.h"

/* sanity checks */
#if COB_MAX_WORDLEN > 255
#error COB_MAX_WORDLEN is too big, must be less than 256
#endif
#if COB_MAX_NAMELEN > COB_MAX_WORDLEN
#error COB_MAX_NAMELEN is too big, must be less than COB_MAX_WORDLEN
#endif

#define	CB_IMSG_SIZE	24
#define	CB_IVAL_SIZE	(80 - CB_IMSG_SIZE - 4)

/* Stringify macros */
#define CB_STRINGIFY(s)			#s
#define CB_XSTRINGIFY(s)		CB_STRINGIFY (s)

/* C version info */
#ifdef	__VERSION__
#if		! defined (_MSC_VER)
#if		defined (__MINGW32__)
#define GC_C_VERSION_PRF	"(MinGW) "
#elif	defined (__DJGPP__)
#define GC_C_VERSION_PRF	"(DJGPP) "
#elif	defined (__ORANGEC__)
#define GC_C_VERSION_PRF	"(OrangeC) "
#else
#define GC_C_VERSION_PRF	""
#endif
#elif	defined (__c2__)
#define GC_C_VERSION_PRF	"(Microsoft C2) "
#elif	defined (__llvm__)
#define GC_C_VERSION_PRF	"(LLVM / MSC) "
#else
#define GC_C_VERSION_PRF	"(Microsoft) "
#endif
#define GC_C_VERSION	CB_XSTRINGIFY (__VERSION__)
#elif	defined (__xlc__)
#define GC_C_VERSION_PRF	"(IBM XL C/C++) "
#define GC_C_VERSION	CB_XSTRINGIFY (__xlc__)
#elif	defined (__SUNPRO_C)
#define GC_C_VERSION_PRF	"(Sun C) "
#define GC_C_VERSION	CB_XSTRINGIFY (__SUNPRO_C)
#elif	defined (_MSC_VER)
#define GC_C_VERSION_PRF	"(Microsoft) "
#define GC_C_VERSION	CB_XSTRINGIFY (_MSC_VER)
#elif	defined (__BORLANDC__)
#define GC_C_VERSION_PRF	"(Borland) "
#define GC_C_VERSION	CB_XSTRINGIFY (__BORLANDC__)
#elif	defined (__WATCOMC__)
#define GC_C_VERSION_PRF	"(Watcom) "
#define GC_C_VERSION	CB_XSTRINGIFY (__WATCOMC__)
#elif	defined (__INTEL_COMPILER)
#define GC_C_VERSION_PRF	"(Intel) "
#define GC_C_VERSION	CB_XSTRINGIFY (__INTEL_COMPILER)
#elif	defined(__TINYC__)
#define GC_C_VERSION_PRF	"(Tiny C) "
#define GC_C_VERSION	CB_XSTRINGIFY(__TINYC__)
#elif  defined(__HP_cc)
#define GC_C_VERSION_PRF       "(HP aC++/ANSI C) "
#define GC_C_VERSION   CB_XSTRINGIFY(__HP_cc) 
#elif  defined(__hpux) || defined(_HPUX_SOURCE)
#if  defined(__ia64)
#define GC_C_VERSION_PRF       "(HPUX IA64) "
#else
#define GC_C_VERSION_PRF       "(HPUX PA-RISC) "
#endif
#define GC_C_VERSION   " C"  
#else
#define GC_C_VERSION_PRF	""
#define GC_C_VERSION	_("unknown")
#endif

#if COB_MAX_UNBOUNDED_SIZE > COB_MAX_FIELD_SIZE
#define COB_MAX_ALLOC_SIZE COB_MAX_UNBOUNDED_SIZE
#else
#define COB_MAX_ALLOC_SIZE COB_MAX_FIELD_SIZE
#endif

/* Global variables */
#define SPACE_16	"                "
#define SPACE_64	SPACE_16 SPACE_16 SPACE_16 SPACE_16
#define SPACE_256	SPACE_64 SPACE_64 SPACE_64 SPACE_64
#define SPACE_1024	SPACE_256 SPACE_256 SPACE_256 SPACE_256
const char *COB_SPACES_ALPHABETIC = SPACE_1024;
#undef SPACE_16
#undef SPACE_64
#undef SPACE_256
#undef SPACE_1024
#define ZERO_16 	"0000000000000000"
#define ZERO_64 	ZERO_16 ZERO_16 ZERO_16 ZERO_16
#define ZERO_256	ZERO_64 ZERO_64 ZERO_64 ZERO_64
const char *COB_ZEROES_ALPHABETIC = ZERO_256;
#undef ZERO_16
#undef ZERO_64
#undef ZERO_256

struct cob_alloc_cache {
	struct cob_alloc_cache	*next;		/* Pointer to next */
	void			*cob_pointer;	/* Pointer to malloced space */
	size_t			size;		/* Item size */
};
const int	MAX_MODULE_ITERS = 10240;

struct cob_alloc_module {
	struct cob_alloc_module	*next;		/* Pointer to next */
	void			*cob_pointer;	/* Pointer to malloced space */
};

/* EXTERNAL structure */

struct cob_external {
	struct cob_external	*next;		/* Pointer to next */
	void			*ext_alloc;	/* Pointer to malloced space */
	char			*ename;		/* External name */
	int			esize;		/* Item size */
};

#define COB_ERRBUF_SIZE		1024

/* Local variables */

static int			cob_initialized = 0;
static int			check_mainhandle = 1;
static int			cob_argc = 0;
static char			**cob_argv = NULL;
static struct cob_alloc_cache	*cob_alloc_base = NULL;
static struct cob_alloc_module	*cob_module_list = NULL;
static cob_module		*cob_module_err = NULL;
static const char		*cob_last_sfile = NULL;
static const char		*cob_last_progid = NULL;

static cob_global		*cobglobptr = NULL;
static cob_settings		*cobsetptr = NULL;

static int			last_exception_code;	/* Last exception: code */
static int			active_error_handler = 0;

static int			cannot_check_subscript = 0;

static const cob_field_attr	const_alpha_attr =
				{COB_TYPE_ALPHANUMERIC, 0, 0, 0, NULL};
static const cob_field_attr	const_bin_nano_attr =
				{COB_TYPE_NUMERIC_BINARY, 20, 9,
				 COB_FLAG_HAVE_SIGN, NULL};

static char			*cob_local_env = NULL;
static int			current_arg = 0;
static unsigned char		*commlnptr = NULL;
static size_t			commlncnt = 0;
static size_t			cob_local_env_size = 0;

static struct cob_external	*basext = NULL;

static size_t			sort_nkeys = 0;
static cob_file_key		*sort_keys = NULL;
static const unsigned char	*sort_collate = NULL;

static const char		*cob_source_file = NULL;
static unsigned int		cob_source_line = 0;

Page libcobPage;

#ifdef	HAVE_DESIGNATED_INITS
char *cob_statement_name[STMT_MAX_ENTRY] = {
	[STMT_UNKNOWN] = "UNKNOWN"
	/* note: STMT_UNKNOWN left out here */
#define COB_STATEMENT(ename,str)	, [ename] = str
#include "statement.def"	/* located and installed next to common.h */
#undef COB_STATEMENT
};
#else
const char	*cob_statement_name[STMT_MAX_ENTRY];
static void init_statement_list (void);
#endif
int 		cob_statement_hash[STMT_MAX_ENTRY] = { 0 };

#ifdef COB_DEBUG_LOG
static int			cob_debug_log_time = 0;
static FILE			*cob_debug_file = NULL;
static int			cob_debug_level = 9;
static char			*cob_debug_mod = NULL;
static char			cob_debug_modules[12][4] = {"", "", "", "", "", "", "", "", "", "", "", ""};
static char			*cob_debug_file_name = NULL;
#endif

static char			*strbuff = NULL;

static int		cob_process_id = 0;
static int		cob_temp_iteration = 0;

static unsigned int	conf_runtime_error_displayed = 0;
static unsigned int	last_runtime_error_line = 0;
static const char	*last_runtime_error_file = NULL;

static char			runtime_err_str[COB_ERRBUF_SIZE] = { 0 };	/* emergency buffer */
static int			exit_code = 0;	/* exit code when leaving libcob */

#ifndef COB_WITHOUT_JMP
/* longjmp buffer used instead of exit() if cob_init_with_return is used */
static jmp_buf	return_jmp_buf;
static int		return_jmp_buffer_set = 0;
#endif

#if	defined (HAVE_SIGNAL_H) && defined (HAVE_SIG_ATOMIC_T)
volatile sig_atomic_t	sig_is_handled = 0;
#endif

/* Function Pointer for external signal handling */
static void		(*cob_ext_sighdl) (int) = NULL;

#if defined (_MSC_VER) && COB_USE_VC2008_OR_GREATER
static VOID		(WINAPI *time_as_filetime_func) (LPFILETIME) = NULL;
#endif

#undef	COB_EXCEPTION
#define COB_EXCEPTION(code, tag, name, critical)	name,
static const char		* const cob_exception_tab_name[] = {
	"None",		/* COB_EC_ZERO */
#include "exception.def"
	"Invalid"	/* COB_EC_MAX */
};

#undef	COB_EXCEPTION
#define COB_EXCEPTION(code, tag, name, critical)	0x##code,
static const int		cob_exception_tab_code[] = {
	0,		/* COB_EC_ZERO */
#include "exception.def"
	0		/* COB_EC_MAX */
};

#undef	COB_EXCEPTION

#define EXCEPTION_TAB_SIZE	sizeof (cob_exception_tab_code) / sizeof (int)

/* Switches */
#define	COB_SWITCH_MAX	36  /* maximum switches, must match cobc/tree.h! */

static int		cob_switch[COB_SWITCH_MAX + 1];

/* BCD to Integer translation (full byte -> 0 - 99) */
static unsigned char   b2i[]= {
     0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 255, 255, 255, 255, 255, 255,
    10,  11,  12,  13,  14,  15,  16,  17,  18,  19, 255, 255, 255, 255, 255, 255,
    20,  21,  22,  23,  24,  25,  26,  27,  28,  29, 255, 255, 255, 255, 255, 255,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39, 255, 255, 255, 255, 255, 255,
    40,  41,  42,  43,  44,  45,  46,  47,  48,  49, 255, 255, 255, 255, 255, 255,
    50,  51,  52,  53,  54,  55,  56,  57,  58,  59, 255, 255, 255, 255, 255, 255,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69, 255, 255, 255, 255, 255, 255,
    70,  71,  72,  73,  74,  75,  76,  77,  78,  79, 255, 255, 255, 255, 255, 255,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89, 255, 255, 255, 255, 255, 255,
    90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };

#define IS_INVALID_BCD_DATA(c)	(b2i[(unsigned char)c] == 255)

/* note: use of table d2i was tested and seen to be
   slower for checking validity/invalidity
   and only 0.2% faster for GET_D2I than COB_D2I */
#define IS_INVALID_DIGIT_DATA(c)	((unsigned char)(c - '0') > 9)	/* not valid digits '0' - '9' */
#define IS_VALID_DIGIT_DATA(c)	((unsigned char)(c - '0') <= 9)	/* valid digits '0' - '9' */

/* Runtime exit handling */
static struct exit_handlerlist {
	struct exit_handlerlist	*next;
	int			(*proc)(void);
	unsigned char priority;
} *exit_hdlrs;

/* Runtime error handling */
static struct handlerlist {
	struct handlerlist	*next;
	int			(*proc)(char *s);
} *hdlrs;

/* note: set again (translated) in print_runtime_conf */
static const char *setting_group[] = {" hidden setting ", "CALL configuration",
					"File I/O configuration", "Screen I/O configuration", "Miscellaneous",
					"System configuration"};

static struct config_enum lwrupr[] = {{"LOWER", "1"}, {"UPPER", "2"}, {"not set", "0"}, {NULL, NULL}};
#if 0	/* boolean "not set" - used for file specific settings (4.x feature) */
static struct config_enum notset[] = {{"not set", "!"}, {NULL, NULL}};
#endif
static struct config_enum never[] = {{"never", "!"}, {NULL, NULL}};
static struct config_enum beepopts[] = {{"FLASH", "1"}, {"SPEAKER", "2"}, {"FALSE", "9"}, {"BEEP", "0"}, {NULL, NULL}};
static struct config_enum timeopts[] = {{"0", "1000"}, {"1", "100"}, {"2", "10"}, {"3", "1"}, {NULL, NULL}};
static struct config_enum syncopts[] = {{"P", "1"}, {NULL, NULL}};
static struct config_enum varseqopts[] = {{"0", "0"}, {"1", "1"}, {"2", "2"}, {"3", "3"}, {NULL, NULL}};
static struct config_enum coeopts[] = {{"0", "0"}, {"1", "1"}, {"2", "2"}, {"3", "3"}, {NULL, NULL}};
static char	varseq_dflt[8] = "0";
static unsigned char min_conf_length = 0;
static const char *not_set;

/*
 * Table of possible environment variables and/or runtime.cfg parameters:
   Env Var name, Name used in run-time config file, Default value (NULL for aliases), Table of Alternate values,
   Grouping for display of run-time options, Data type, Location within structure (adds computed length of referenced field),
   Set by which runtime.cfg file, value set by a different keyword,
   optional: Minimum accepted value, Maximum accepted value
 */
static struct config_tbl gc_conf[] = {
	{"COB_LOAD_CASE", "load_case", 		"0", 	lwrupr, GRP_CALL, ENV_UINT | ENV_ENUMVAL, SETPOS (name_convert)},
	{"COB_PHYSICAL_CANCEL", "physical_cancel", 	"0", 	never, GRP_CALL, ENV_BOOL | ENV_ENUMVAL, SETPOS (cob_physical_cancel)},
	{"default_cancel_mode", "default_cancel_mode", 	NULL, NULL, GRP_HIDE, ENV_BOOL | ENV_NOT, SETPOS (cob_physical_cancel)},
	{"LOGICAL_CANCELS", "logical_cancels", 	NULL, NULL, GRP_HIDE, ENV_BOOL | ENV_NOT, SETPOS (cob_physical_cancel)},
	{"COB_LIBRARY_PATH", "library_path", 	NULL, 	NULL, GRP_CALL, ENV_PATH, SETPOS (cob_library_path)}, /* default value set in cob_init_call() */
	{"COB_PRE_LOAD", "pre_load", 		NULL, 	NULL, GRP_CALL, ENV_STR, SETPOS (cob_preload_str)},
	{"COB_BELL", "bell", 			"0", 	beepopts, GRP_SCREEN, ENV_UINT | ENV_ENUMVAL, SETPOS (cob_beep_value)},
	{"COB_DEBUG_LOG", "debug_log", 		NULL, 	NULL, GRP_HIDE, ENV_FILE, SETPOS (cob_debug_log)},
	{"COB_DISABLE_WARNINGS", "disable_warnings", "0", 	NULL, GRP_MISC, ENV_BOOL | ENV_NOT, SETPOS (cob_display_warn)},
	{"COB_ENV_MANGLE", "env_mangle", 		"0", 	NULL, GRP_MISC, ENV_BOOL, SETPOS (cob_env_mangle)},
	{"COB_REDIRECT_DISPLAY", "redirect_display", "0", 	NULL, GRP_SCREEN, ENV_BOOL, SETPOS (cob_disp_to_stderr)},
	{"COB_SCREEN_ESC", "screen_esc", 		"0", 	NULL, GRP_SCREEN, ENV_BOOL, SETPOS (cob_use_esc)},
	{"COB_SCREEN_EXCEPTIONS", "screen_exceptions", "0", NULL, GRP_SCREEN, ENV_BOOL, SETPOS (cob_extended_status)},
	{"COB_TIMEOUT_SCALE", "timeout_scale", 	"0", 	timeopts, GRP_SCREEN, ENV_UINT, SETPOS (cob_timeout_scale)},
	{"COB_INSERT_MODE", "insert_mode", "0", NULL, GRP_SCREEN, ENV_BOOL, SETPOS (cob_insert_mode)},
	{"COB_MOUSE_FLAGS", "mouse_flags", "1", NULL, GRP_SCREEN, ENV_UINT, SETPOS (cob_mouse_flags)},
	{"MOUSE_FLAGS", "mouse_flags", NULL, NULL, GRP_HIDE, ENV_UINT, SETPOS (cob_mouse_flags)},
#ifdef HAVE_MOUSEINTERVAL	/* possibly add an internal option for mouse support, too */
	{"COB_MOUSE_INTERVAL", "mouse_interval", "100", NULL, GRP_SCREEN, ENV_UINT, SETPOS (cob_mouse_interval), 0, 166},
#endif
	{"COB_SET_DEBUG", "debugging_mode", 		"0", 	NULL, GRP_MISC, ENV_BOOL, SETPOS (cob_debugging_mode)},
	{"COB_SET_TRACE", "set_trace", 		"0", 	NULL, GRP_MISC, ENV_BOOL, SETPOS (cob_line_trace)},
	{"COB_TRACE_FILE", "trace_file", 		NULL, 	NULL, GRP_MISC, ENV_FILE, SETPOS (cob_trace_filename)},
	{"COB_TRACE_FORMAT", "trace_format",	"%P %S Line: %L", NULL, GRP_MISC, ENV_STR, SETPOS (cob_trace_format)},
	{"COB_STACKTRACE", "stacktrace", 	"1", 	NULL, GRP_MISC, ENV_BOOL, SETPOS (cob_stacktrace)},
	{"COB_CORE_ON_ERROR", "core_on_error", 	"0", 	coeopts, GRP_MISC, ENV_UINT | ENV_ENUMVAL, SETPOS (cob_core_on_error)},
	{"COB_CORE_FILENAME", "core_filename", 	"./core.libcob", 	NULL, GRP_MISC, ENV_STR, SETPOS (cob_core_filename)},
	{"COB_DUMP_FILE", "dump_file",		NULL,	NULL, GRP_MISC, ENV_FILE, SETPOS (cob_dump_filename)},
	{"COB_DUMP_WIDTH", "dump_width",		"100",	NULL, GRP_MISC, ENV_UINT, SETPOS (cob_dump_width)},
#ifdef  _WIN32
	/* checked before configuration load if set from environment in cob_common_init() */
	{"COB_UNIX_LF", "unix_lf", 		"0", 	NULL, GRP_FILE, ENV_BOOL, SETPOS (cob_unix_lf)},
#endif
	{"USERNAME", "username", 			NULL, 	NULL, GRP_SYSENV, ENV_STR, SETPOS (cob_user_name)},	/* default set in cob_init() */
	{"LOGNAME", "logname", 			NULL, 	NULL, GRP_HIDE, ENV_STR, SETPOS (cob_user_name)},
#if !defined (_WIN32) || defined (__MINGW32__) /* cygwin does not define _WIN32 */
	{"LANG", "lang", 				NULL, 	NULL, GRP_SYSENV, ENV_STR, SETPOS (cob_sys_lang)},
#if defined (__linux__) || defined (__CYGWIN__) || defined (__MINGW32__)
	{"OSTYPE", "ostype", 			NULL, 	NULL, GRP_SYSENV, ENV_STR, SETPOS (cob_sys_type)},
#endif
	{"TERM", "term", 				NULL, 	NULL, GRP_SYSENV, ENV_STR, SETPOS (cob_sys_term)},
#endif
#if defined (_WIN32) && !defined (__MINGW32__)
	{"OS", "ostype", 			NULL, 	NULL, GRP_SYSENV, ENV_STR, SETPOS (cob_sys_type)},
#endif
	{"COB_FILE_PATH", "file_path", 		NULL, 	NULL, GRP_FILE, ENV_PATH, SETPOS (cob_file_path)},
	{"COB_VARSEQ_FORMAT", "varseq_format", 	varseq_dflt, varseqopts, GRP_FILE, ENV_UINT | ENV_ENUM, SETPOS (cob_varseq_type)},
	{"COB_LS_FIXED", "ls_fixed", 		"0", 	NULL, GRP_FILE, ENV_BOOL, SETPOS (cob_ls_fixed)},
	{"STRIP_TRAILING_SPACES", "strip_trailing_spaces", 		NULL, 	NULL, GRP_HIDE, ENV_BOOL | ENV_NOT, SETPOS (cob_ls_fixed)},
	{"COB_LS_VALIDATE","ls_validate",	"1",	NULL,GRP_FILE,ENV_BOOL,SETPOS(cob_ls_validate)},
	{"COB_LS_NULLS", "ls_nulls", 		"0", 	NULL, GRP_FILE, ENV_BOOL, SETPOS (cob_ls_nulls)},
	{"COB_LS_SPLIT", "ls_split", 		"1", 	NULL, GRP_FILE, ENV_BOOL, SETPOS (cob_ls_split)},
    {"COB_SEQ_CONCAT_NAME","seq_concat_name","0",NULL,GRP_FILE,ENV_BOOL,SETPOS(cob_concat_name)},
    {"COB_SEQ_CONCAT_SEP","seq_concat_sep","+",NULL,GRP_FILE,ENV_CHAR,SETPOS(cob_concat_sep),1},
	{"COB_SORT_CHUNK", "sort_chunk", 		"256K", 	NULL, GRP_FILE, ENV_SIZE, SETPOS (cob_sort_chunk), (128 * 1024), (16 * 1024 * 1024)},
	{"COB_SORT_MEMORY", "sort_memory", 	"128M", 	NULL, GRP_FILE, ENV_SIZE, SETPOS (cob_sort_memory), (1024*1024), 4294967294UL /* max. guaranteed - 1 */},
	{"COB_SYNC", "sync", 			"0", 	syncopts, GRP_FILE, ENV_BOOL, SETPOS (cob_do_sync)},
#ifdef  WITH_DB
	{"DB_HOME", "db_home", 			NULL, 	NULL, GRP_FILE, ENV_FILE, SETPOS (bdb_home)},
#endif
	{"COB_COL_JUST_LRC", "col_just_lrc", "true", 	NULL, GRP_FILE, ENV_BOOL, SETPOS (cob_col_just_lrc)},
	{"COB_DISPLAY_PRINT_PIPE", "display_print_pipe",		NULL,	NULL, GRP_SCREEN, ENV_STR, SETPOS (cob_display_print_pipe)},
	{"COBPRINTER", "printer",		NULL,	NULL, GRP_HIDE, ENV_STR, SETPOS (cob_display_print_pipe)},
	{"COB_DISPLAY_PRINT_FILE", "display_print_file",		NULL,	NULL, GRP_SCREEN, ENV_STR,SETPOS (cob_display_print_filename)},
	{"COB_DISPLAY_PUNCH_FILE", "display_punch_file",		NULL,	NULL, GRP_SCREEN, ENV_STR,SETPOS (cob_display_punch_filename)},
	{"COB_LEGACY", "legacy", 			NULL, 	NULL, GRP_SCREEN, ENV_BOOL, SETPOS (cob_legacy)},
	{"COB_EXIT_WAIT", "exit_wait", 		"1", 	NULL, GRP_SCREEN, ENV_BOOL, SETPOS (cob_exit_wait)},
	{"COB_EXIT_MSG", "exit_msg", 		NULL, NULL, GRP_SCREEN, ENV_STR, SETPOS (cob_exit_msg)},	/* default set in cob_init_screenio() */
	{"COB_CURRENT_DATE" ,"current_date",	NULL,	NULL, GRP_MISC, ENV_STR, SETPOS (cob_date)},
	{"COB_DATE", "date",			NULL,	NULL, GRP_HIDE, ENV_STR, SETPOS (cob_date)},
	{NULL, NULL, 0, 0}
};
#define NUM_CONFIG (sizeof (gc_conf) /sizeof (struct config_tbl) - 1)
#define FUNC_NAME_IN_DEFAULT NUM_CONFIG + 1

/* 
 * Table of 'signal' supported by this system 
 */
static struct signal_table {
	short		sig;			/* Signal number */
	short		for_set;		/* Set via 'cob_set_signal' 1=set if not SIG_IGN */
	short		for_dump;		/* set via 'cob_set_dump_signal' */
	short		unused;
	const char	*shortname;		/* Short signal name */
	const char	*description;	/* Longer desciption message */
} signals[] = {
#ifdef	SIGINT
	{SIGINT,1,0,0,"SIGINT"},
#endif
#ifdef	SIGHUP
	{SIGHUP,1,0,0,"SIGHUP"},
#endif
#ifdef	SIGQUIT
	{SIGQUIT,1,0,0,"SIGQUIT"},
#endif
#ifdef	SIGTERM
	{SIGTERM,1,0,0,"SIGTERM"},
#endif
#ifdef	SIGEMT
	{SIGEMT,1,0,0,"SIGEMT"},
#endif
#ifdef	SIGPIPE
	{SIGPIPE,1,0,0,"SIGPIPE"},
#endif
#ifdef	SIGIO
	{SIGIO,1,0,0,"SIGIO"},
#endif
#ifdef	SIGSEGV
	{SIGSEGV,2,1,0,"SIGSEGV"},
#endif
#ifdef	SIGBUS
	{SIGBUS,2,1,0,"SIGBUS"},
#endif
	{SIGFPE,1,1,0,"SIGFPE"},	/* always defined, if missing */
#ifdef	SIGILL
	{SIGILL,0,0,0,"SIGILL"},
#endif
#ifdef	SIGABRT
	{SIGABRT,0,0,0,"SIGABRT"},
#endif
#ifdef	SIGKILL
	{SIGKILL,0,0,0,"SIGKILL"},
#endif
#ifdef	SIGALRM
	{SIGALRM,0,0,0,"SIGALRM"},
#endif
#ifdef	SIGSTOP
	{SIGSTOP,0,0,0,"SIGSTOP"},
#endif
#ifdef	SIGCHLD
	{SIGCHLD,0,0,0,"SIGCHLD"},
#endif
#ifdef	SIGCLD
	{SIGCLD,0,0,0,"SIGCLD"},
#endif
	{-1,0,0,0,"unknown"}
};
#define NUM_SIGNALS (int)((sizeof (signals) / sizeof (struct signal_table)) - 1)
const char *signal_msgid = "signal";
const char *warning_msgid = "warning: ";
const char *more_stack_frames_msgid = "(more COBOL runtime elements follow...)";

/* Local functions */
static int		translate_boolean_to_int	(const char* ptr);
static cob_s64_t	get_sleep_nanoseconds	(cob_field *nano_seconds);
static cob_s64_t	get_sleep_nanoseconds_from_seconds	(cob_field *decimal_seconds);
static void		internal_nanosleep	(cob_s64_t nsecs);

static int		set_config_val	(char *value, int pos);
static char		*get_config_val	(char *value, int pos, char *orgvalue);

static void		cob_dump_module (char *reason);
static char		abort_reason[COB_MINI_BUFF] = { 0 };
static unsigned int 	dump_trace_started;	/* ensures that we dump/stacktrace only once */
#define 		DUMP_TRACE_DONE_DUMP 		(1U << 0)
#define 		DUMP_TRACE_DONE_TRACE		(1U << 1)
#define 		DUMP_TRACE_ACTIVE_TRACE		(1U << 2)
static void		cob_stack_trace_internal (fd_t target, int verbose, int count);

#ifdef COB_DEBUG_LOG
static void		cob_debug_open	(void);
#endif
static void		conf_runtime_error_value	(const char *value, const int conf_pos);
static void		conf_runtime_error	(const int finish_error, const char *fmt, ...);

static void
cob_exit_common (void)
{
	struct cob_external	*p;
	struct cob_external	*q;
	struct cob_alloc_cache	*x;
	struct cob_alloc_cache	*y;

#ifdef	HAVE_SETLOCALE
	if (cobglobptr->cob_locale_orig) {
		(void) setlocale (LC_ALL, cobglobptr->cob_locale_orig);
		cob_free (cobglobptr->cob_locale_orig);
	}
	if (cobglobptr->cob_locale) {
		cob_free (cobglobptr->cob_locale);
	}
	if (cobglobptr->cob_locale_ctype) {
		cob_free (cobglobptr->cob_locale_ctype);
	}
	if (cobglobptr->cob_locale_collate) {
		cob_free (cobglobptr->cob_locale_collate);
	}
	if (cobglobptr->cob_locale_messages) {
		cob_free (cobglobptr->cob_locale_messages);
	}
	if (cobglobptr->cob_locale_monetary) {
		cob_free (cobglobptr->cob_locale_monetary);
	}
	if (cobglobptr->cob_locale_numeric) {
		cob_free (cobglobptr->cob_locale_numeric);
	}
	if (cobglobptr->cob_locale_time) {
		cob_free (cobglobptr->cob_locale_time);
	}
#endif

	if (commlnptr) {
		cob_free (commlnptr);
	}
	if (cob_local_env) {
		cob_free (cob_local_env);
	}

	/* Free library routine stuff */

	if (cobglobptr->cob_term_buff) {
		cob_free (cobglobptr->cob_term_buff);
	}

	/* Free cached externals */
	for (p = basext; p;) {
		q = p;
		p = p->next;
		if (q->ename) {
			cob_free (q->ename);
		}
		if (q->ext_alloc) {
			cob_free (q->ext_alloc);
		}
		cob_free (q);
	}

	/* Free cached mallocs */
	for (x = cob_alloc_base; x;) {
		y = x;
		x = x->next;
		cob_free (y->cob_pointer);
		cob_free (y);
	}

	/* Free last stuff */
	if (cob_last_sfile) {
		cob_free ((void *)cob_last_sfile);
	}
	if (cobglobptr) {
		if (cobglobptr->cob_main_argv0) {
			cob_free ((void *)(cobglobptr->cob_main_argv0));
		}
		cob_free (cobglobptr);
		cobglobptr = NULL;
	}
	if (cobsetptr) {
		void 	*data;
		char	*str;
		unsigned int	i;
		if (cobsetptr->cob_config_file) {
			for (i = 0; i < cobsetptr->cob_config_num; i++) {
				if (cobsetptr->cob_config_file[i]) {
					cob_free ((void *)cobsetptr->cob_config_file[i]);
				}
			}
			cob_free ((void *)cobsetptr->cob_config_file);
		}
		/* Free all strings pointed to by cobsetptr */
		for (i = 0; i < NUM_CONFIG; i++) {
			if ((gc_conf[i].data_type & ENV_STR)
			||  (gc_conf[i].data_type & ENV_FILE)
			||  (gc_conf[i].data_type & ENV_PATH)) {	/* String/Path to be stored as a string */
				data = (void *)((char *)cobsetptr + gc_conf[i].data_loc);
				memcpy (&str, data, sizeof (char *));
				if (str != NULL) {
					cob_free ((void *)str);
					str = NULL;
					memcpy (data, &str, sizeof (char *));	/* Reset pointer to NULL */
				}
			}
		}
		if (cobsetptr->cob_preload_str_set) {
			cob_free((void*)(cobsetptr->cob_preload_str_set));
		}
		cob_free (cobsetptr);
		cobsetptr = NULL;
	}
	cob_initialized = 0;
}

static void
cob_exit_common_modules (void)
{
	cob_module	*mod;
	struct cob_alloc_module	*ptr, *nxt;
	int		(*cancel_func)(const int);

	/* Call each module to release local memory
	   - currently used for: decimals -
	   and remove it from the internal module list */
	for (ptr = cob_module_list; ptr; ptr = nxt) {
		mod = ptr->cob_pointer;
		nxt = ptr->next;
		if (mod && mod->module_cancel.funcint) {
			mod->module_active = 0;
			cancel_func = mod->module_cancel.funcint;
			(void)cancel_func (-20);	/* Clear just decimals */
		}
		cob_free (ptr);
	}
	cob_module_list = NULL;
}

static void
ss_terminate_routines (void)
{
	if (!cob_initialized || !cobglobptr) {
		return;
	}
	cob_exit_fileio_msg_only ();
	cob_exit_screen_from_signal(1);

	if (COB_MODULE_PTR && abort_reason[0] != 0) {
		if (cobsetptr->cob_stacktrace) {
			if (!(dump_trace_started & (DUMP_TRACE_DONE_TRACE | DUMP_TRACE_ACTIVE_TRACE))) {
				dump_trace_started |= DUMP_TRACE_DONE_TRACE;
				dump_trace_started |= DUMP_TRACE_ACTIVE_TRACE;
				cob_stack_trace_internal (stderr, 1, 0);
				dump_trace_started ^= DUMP_TRACE_ACTIVE_TRACE;
			}
		}
		/* note: no dump code here as getting all the data out of the modules
		   isn't signal-safe at all; the code to write it out isn't either */
	}
}

static void
cob_terminate_routines (void)
{
	if (!cob_initialized || !cobglobptr) {
		return;
	}
	fflush (stderr);

	cob_exit_fileio_msg_only ();

	if (COB_MODULE_PTR && abort_reason[0] != 0) {
		if (cobsetptr->cob_stacktrace) {
			if (!(dump_trace_started & (DUMP_TRACE_DONE_TRACE | DUMP_TRACE_ACTIVE_TRACE))) {
				dump_trace_started |= DUMP_TRACE_DONE_TRACE;
				dump_trace_started |= DUMP_TRACE_ACTIVE_TRACE;
				cob_stack_trace_internal (stderr, 1, 0);
				dump_trace_started ^= DUMP_TRACE_ACTIVE_TRACE;
			}
		}
		if (!(dump_trace_started & DUMP_TRACE_DONE_DUMP)) {
			dump_trace_started |= DUMP_TRACE_DONE_DUMP;
			cob_dump_module (abort_reason);
		}
	}

	if (cobsetptr->cob_dump_file == cobsetptr->cob_trace_file
	 || cobsetptr->cob_dump_file == stderr) {
		cobsetptr->cob_dump_file = VFS_INVALID_FD;
	}

	if (cobsetptr->cob_dump_file) {
		fclose (cobsetptr->cob_dump_file);
		cobsetptr->cob_dump_file = VFS_INVALID_FD;
	}

#ifdef COB_DEBUG_LOG
	/* close debug log (delete file if empty) */
	if (cob_debug_file
	 && cob_debug_file != stderr) {
		/* note: cob_debug_file can only be identical to cob_trace_file
		         if same file name was used, not with external_trace_file */
		if (cob_debug_file == cobsetptr->cob_trace_file) {
			cobsetptr->cob_trace_file = NULL;
		}
		if (cob_debug_file_name != NULL
		 && ftell (cob_debug_file) == 0) {
			fclose (cob_debug_file);
			unlink (cob_debug_file_name);
		} else {
			fclose (cob_debug_file);
		}
	}
	cob_debug_file = NULL;
	if (cob_debug_file_name) {
		cob_free (cob_debug_file_name);
		cob_debug_file_name = NULL;
	}
#endif

	if (cobsetptr->cob_trace_file
	 && cobsetptr->cob_trace_file != stderr
	 && !cobsetptr->external_trace_file	/* note: may include stdout */) {
		fclose (cobsetptr->cob_trace_file);
	}
	cobsetptr->cob_trace_file = VFS_INVALID_FD;

	/* close punch file if self-opened */
	if (cobsetptr->cob_display_punch_file
	 && cobsetptr->cob_display_punch_filename) {
		fclose (cobsetptr->cob_display_punch_file);
		cobsetptr->cob_display_punch_file = VFS_INVALID_FD;
	}

	cob_exit_screen ();
	cob_exit_fileio ();
	cob_exit_reportio ();
	cob_exit_mlio ();

	cob_exit_intrinsic ();
	cob_exit_strings ();
	cob_exit_numeric ();

	cob_exit_common_modules ();
	cob_exit_call ();
	cob_exit_common ();
}

static void
cob_get_source_line ()
{
	if (cobglobptr
	 && COB_MODULE_PTR) {
		cob_module	*mod = COB_MODULE_PTR;
		while (mod && mod->module_stmt == 0) {
			mod = mod->next;
		}
		if (mod
		 && mod->module_stmt != 0
		 && mod->module_sources) {
			cob_source_file =
				mod->module_sources[COB_GET_FILE_NUM (mod->module_stmt)];
			cob_source_line = COB_GET_LINE_NUM (mod->module_stmt);
		}
#if 0	/* note: those are also manually set
		         and therefore may not be reset here */
	} else {
		cob_source_file = NULL;
		cob_source_line = 0;
#endif
	}
}

/* reentrant version of strerror */
static char *
cob_get_strerror (void)
{
	size_t size;
	char *ret;
#ifdef HAVE_STRERROR
	char *msg = strerror (errno);
	size = strlen (msg);
#else
	char msg[COB_ERRBUF_SIZE];
	size = snprintf (msg, COB_ERRBUF_SIZE, _("system error %d"), errno);
	if (size >= COB_ERRBUF_SIZE) {
		/* very unlikely, but if that would be a bad msg catalog
		   we don't want to memcpy from invalid data */
		size = COB_ERRBUF_SIZE - 1;
	}
#endif
	ret = cob_cache_malloc (size + 1);
	memcpy (ret, msg, size + 1);
	return ret;
}


static char ss_itoa_buf[12];
/* signal safe integer to string conversion,;
   operates on ss_itoa_buff, returns length;
   uses ideas of Shaoquan's
   https://github.com/wsq003/itoa_for_linux/blob/master/itoa.c */
static size_t
ss_itoa_u10 (int value)
{
	const unsigned int radix = 10;
	char *p, *p2;
	unsigned int digit;
	unsigned int u;
	size_t len;

	p = ss_itoa_buf;

	if (value < 0) {
		*p++ = '-';
		value = 0 - value;
	}
	u = (unsigned int)value;

	p2 = p;

	do {
		digit = u % radix;
		u /= radix;

		*p++ = COB_I2D (digit);

	} while (u > 0);

	len = p - ss_itoa_buf;
	*p-- = 0;

	/* swap around */
	do {
		char temp = *p;
		*p-- = *p2;
		*p2++ = temp;

	} while (p2 < p);

	return len;
}

static COB_INLINE void
set_source_location (const char **file, unsigned int *line)
{
	*file = cob_source_file; /* may be NULL */
	*line = cob_source_line; /* may be zero */
	if (cobglobptr) {
		const cob_module *mod = COB_MODULE_PTR;
		if (mod
		 && mod->module_stmt != 0
		 && mod->module_sources) {
			*file = mod->module_sources[COB_GET_FILE_NUM(mod->module_stmt)];
			*line = COB_GET_LINE_NUM(mod->module_stmt);
		}
	}
}

/* write integer to stderr using fixed buffer */
#define write_to_stderr_or_return_int(i) \
	if (write (stderr, ss_itoa_buf, ss_itoa_u10 (i)) == -1) return
/* write char array (constant C string) to stderr */
#define write_to_stderr_or_return_arr(ch_arr) \
	if (write (stderr, ch_arr, sizeof (ch_arr) - 1) == -1) return
/* write string to stderr, byte count computed with strlen,
   str is evaluated twice */
#define write_to_stderr_or_return_str(str) \
	if (write (stderr, (char*)str, strlen (str)) == -1) return

/* write integer to fileno using fixed buffer */
#define write_or_return_int(fileno,i) \
	if (write (fileno, ss_itoa_buf, ss_itoa_u10 (i)) == -1) return
/* write char array (constant C string) to fileno */
#define write_or_return_arr(fileno, ch_arr) \
	if (write (fileno, ch_arr, sizeof (ch_arr) - 1) == -1) return
/* write string to fileno, byte count computed with strlen,
   str is evaluated twice */
#define write_or_return_str(fileno,str) \
	if (write (fileno, (char*)str, strlen (str)) == -1) return

#if 0 /* unused */
/* write buffer with given byte count to stderr */
#define write_to_stderr_or_return_buf(buff,count) \
	if (write (STDERR_FILENO, buff, count) == -1) return
/* write buffer with given byte count to fileno */
#define write_or_return_buf(fileno,buff,count) \
	if (write (fileno, buff, count) == -1) return
#endif

static void
output_source_location (void)
{
	const char		*source_file;
	unsigned int	 source_line;
	set_source_location (&source_file, &source_line);
	
	if (source_file) {
		write_to_stderr_or_return_str (source_file);
		if (source_line) {
			write_to_stderr_or_return_arr (":");
			write_to_stderr_or_return_int ((int)source_line);
		}
		write_to_stderr_or_return_arr (": ");
	}
}

static int
create_dumpfile (void)
{
	/* Note: this function is not called from a signal handler so may use stdio */

	char cmd [COB_NORMAL_BUFF];
	const char *default_name = "./core.libcob";
	const char *envval;
	size_t content_size;
	int ret;

	if (cobsetptr) {
		envval = cobsetptr->cob_core_filename;
	} else {
		envval = cob_getenv_direct ("COB_CORE_FILENAME");
	}
	if (envval == NULL) {
		envval = default_name;
	}

	content_size = snprintf (cmd, COB_NORMAL_BUFF, "gcore -a -o %s %d", envval, cob_sys_getpid ());
	/* LCOV_EXCL_START */
	if (content_size >= COB_NORMAL_BUFF) {
		/* in unlikely case of "cob_core_filename is too long" use the simple one */
		sprintf (cmd, "gcore -a -o %s %d", default_name, cob_sys_getpid ());
	}
	/* LCOV_EXCL_STOP */

	ret = system (cmd);
	if (ret) {
		fprintf (stderr, "\nlibcob: ");
		fprintf (stderr, _("requested coredump creation failed with status %d"), ret);
		fprintf (stderr, "\n\t%s\t%s\n", _("executing:"), (char *)cmd);
	}
	return ret;
}


#ifdef	HAVE_SIGNAL_H
static void
cob_sig_handler (int sig)
{
	const char *signal_name;
	const char *msg;
	char signal_text[COB_MINI_BUFF] = { 0 };
	size_t str_len;

#if	defined (HAVE_SIGACTION) && !defined (SA_RESETHAND)
	struct sigaction	sa;
#endif

#ifdef	HAVE_SIG_ATOMIC_T
	 if (sig_is_handled) {
#ifdef	HAVE_RAISE
		raise (sig);
#else
		kill (getpid (), sig);
#endif
		exit (sig);
	}
	sig_is_handled = 1;
#endif

#if 0	/* We do not generally flush whatever we may have in our streams
     	   as stdio is not signal-safe;
		   we _may_ do this if not SIGSEGV/SIGBUS/SIGABRT */
	fflush (stdout);
	fflush (stderr);
#endif

	signal_name = cob_get_sig_name (sig);
	/* LCOV_EXCL_START */
	if (signal_name == signals[NUM_SIGNALS].shortname) {
		/* not translated as it is a very unlikely error case */
		signal_name = signals[NUM_SIGNALS].description;	/* translated unknown */
		write_to_stderr_or_return_arr ("\ncob_sig_handler caught not handled signal: ");
		write_to_stderr_or_return_int (sig);
		write_to_stderr_or_return_arr ("\n");
	}
	/* LCOV_EXCL_STOP */

	/* Skip dumping for SIGTERM, SIGINT and "other process" issue's SIGHUP, SIGPIPE */
	switch (sig) {
	case -1:
#ifdef	SIGTERM
	case SIGTERM:
#endif
#ifdef	SIGINT
	case SIGINT:
#endif
#ifdef	SIGHUP
	case SIGHUP:
#endif
#ifdef	SIGPIPE
	case SIGPIPE:
#endif
		dump_trace_started |= DUMP_TRACE_DONE_DUMP;
		/* Fall-through */
	default:
		break;
	}

#ifdef	HAVE_SIGACTION
#ifndef	SA_RESETHAND
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = SIG_DFL;
	(void)sigemptyset (&sa.sa_mask);
	(void)sigaction (sig, &sa, NULL);
#endif
#else
	(void)signal (sig, SIG_DFL);
#endif
	cob_exit_screen ();

	write_to_stderr_or_return_arr ("\n");
	cob_get_source_line ();
	output_source_location ();

	msg = cob_get_sig_description (sig);
	write_to_stderr_or_return_str (msg);

	/* setup "signal %s" */
	str_len = strlen (signal_msgid);
	memcpy (signal_text, signal_msgid, str_len++);
	signal_text[str_len] = ' ';
	memcpy (signal_text + str_len, signal_name, strlen (signal_name));

	write_to_stderr_or_return_arr (" (");
	write_to_stderr_or_return_str (signal_text);
	write_to_stderr_or_return_arr (")\n\n");

	if (cob_initialized) {
		if (abort_reason[0] == 0) {
			memcpy (abort_reason, signal_text, COB_MINI_BUFF);
#if 0	/* Is there a use in this message ?*/
			write_to_stderr_or_return_str (abnormal_termination_msgid);
			write_to_stderr_or_return_arr ("\n");
#endif
		}
	}

	/* early coredump if requested would be nice,
	   but that is not signal-safe so do SIGABRT later... */
	if (cobsetptr && cobsetptr->cob_core_on_error == 3)  {
		cobsetptr->cob_core_on_error = 4;
	}
	switch (sig) {
	case -1:
#ifdef	SIGSEGV
	case SIGSEGV:
#endif
#ifdef	SIGBUS
	case SIGBUS:
#endif
#ifdef	SIGABRT
	case SIGABRT:
#endif
		if (cobsetptr && cobsetptr->cob_core_on_error != 0)  {
			ss_terminate_routines ();
			break;
		}
		/* Fall-through */
	default:
		cob_terminate_routines ();
		break;
	}

	/* call external signal handler if registered */
	if (cob_ext_sighdl != NULL) {
		(*cob_ext_sighdl) (sig);
		cob_ext_sighdl = NULL;
	}
	exit_code = sig;
#ifndef COB_WITHOUT_JMP
	if (return_jmp_buffer_set) {
		longjmp (return_jmp_buf, -3);
	}
#endif
	/* if an explicit requested coredump could not
	   be created - raise SIGABRT here */
	if (cobsetptr && cobsetptr->cob_core_on_error == 4) {
		sig = SIGABRT;
	}
	signal (sig, SIG_DFL);
#ifdef	HAVE_RAISE
	raise (sig);
#else
	kill (cob_sys_getpid (), sig);
#endif
	
#if 0 /* we don't necessarily want the OS to handle this,
         so exit in all other cases*/
	exit (sig);
#endif
}
#endif /* HAVE_SIGNAL_H */

/* Raise signal (run both internal and external handlers)
   may return, depending on the signal
*/
void
cob_raise (int sig)
{
#ifdef	HAVE_SIGNAL_H
	/* let the registered signal handlers do their work */
#ifdef	HAVE_RAISE
	raise (sig);
#else
	kill (cob_sys_getpid (), sig);
#endif
	/* else: at least call external signal handler if registered */
#else
	if (cob_ext_sighdl != NULL) {
		(*cob_ext_sighdl) (sig);
		cob_ext_sighdl = NULL;
	}
#endif
}

static COB_INLINE struct signal_table
get_signal_entry (int sig)
{
	unsigned int	k;
	for (k = 0; k < NUM_SIGNALS; k++) {
		if (signals[k].sig == sig)
			break;
	}
	/* if we didn't found a matching signal we end with the max: "unknown" */
	return signals[k];
}


const char *
cob_get_sig_name (int sig)
{
	struct signal_table	signal_entry = get_signal_entry (sig);
	return signal_entry.shortname;
}

const char *
cob_get_sig_description (int sig)
{
	struct signal_table	signal_entry = get_signal_entry (sig);
	/* LCOV_EXCL_START */
	if (!signal_entry.description) {
		/* in theory we could get here before pre-initialization was finished,
		   and would want to have _anything_ to return then */
		return "unknown";
	}
	/* LCOV_EXCL_STOP */
	return signal_entry.description;
}

static void
cob_init_sig_descriptions (void)
{
	int	k;
	for (k = 0; k <= NUM_SIGNALS; k++) {
		/* always defined, if missing */ 
		if (signals[k].sig == SIGFPE) {
			signals[k].description = _("fatal arithmetic error");
	#ifdef	SIGINT
		} else if (signals[k].sig == SIGINT) {
			signals[k].description = _("interrupt from keyboard");
	#endif
	#ifdef	SIGHUP
		} else if (signals[k].sig == SIGHUP) {
			signals[k].description = _("hangup");
	#endif
	#ifdef	SIGQUIT
		} else if (signals[k].sig == SIGQUIT) {
			signals[k].description = _("quit");
	#endif
	#ifdef	SIGTERM
		} else if (signals[k].sig == SIGTERM) {
			signals[k].description = _("termination");
	#endif
	#ifdef	SIGEMT
		} else if (signals[k].sig == SIGEMT) {
			signals[k].description = _("emt termination");
	#endif
	#ifdef	SIGPIPE
		} else if (signals[k].sig == SIGPIPE) {
			signals[k].description = _("broken pipe");
	#endif
	#ifdef	SIGIO
		} else if (signals[k].sig == SIGIO) {
			signals[k].description = _("I/O signal");
	#endif
	#ifdef	SIGSEGV
		} else if (signals[k].sig == SIGSEGV) {
			signals[k].description = _("attempt to reference invalid memory address");
	#endif
	#ifdef	SIGBUS
		} else if (signals[k].sig == SIGBUS) {
			signals[k].description = _("bus error");
	#endif
	#ifdef	SIGILL
		} else if (signals[k].sig == SIGILL) {
			signals[k].description = _("illegal instruction");
	#endif
	#ifdef	SIGABRT
		} else if (signals[k].sig == SIGABRT) {
			signals[k].description = _("abort");
	#endif
	#ifdef	SIGKILL
		} else if (signals[k].sig == SIGKILL) {
			signals[k].description = _("process killed");
	#endif
	#ifdef	SIGALRM
		} else if (signals[k].sig == SIGALRM) {
			signals[k].description = _("alarm signal");
	#endif
	#ifdef	SIGSTOP
		} else if (signals[k].sig == SIGSTOP) {
			signals[k].description = _("stop process");
	#endif
	#ifdef	SIGCHLD
		} else if (signals[k].sig == SIGCHLD) {
			signals[k].description = _("child process stopped");
	#endif
	#ifdef	SIGCLD
		} else if (signals[k].sig == SIGCLD) {
			signals[k].description = _("child process stopped");
	#endif
		} else {
			signals[k].description = _("unknown");
		}
	}
	/* TRANSLATORS: This msgid is used for an OS signal like SIGABRT. */
	signal_msgid = _("signal");
	/* TRANSLATORS: This msgid is shown for a requested but not complete stack trace. */
	more_stack_frames_msgid = _("(more COBOL runtime elements follow...)");
#if 0	/* Is there a use in this message ?*/
	abnormal_termination_msgid = (_("abnormal termination - file contents may be incorrect");
#endif
	warning_msgid = _("warning: ");
}

static void
cob_set_signal (void)
{
#if	defined (HAVE_SIGNAL_H)
	int k;
#ifdef	HAVE_SIGACTION
	struct sigaction	sa;
	struct sigaction	osa;

	memset (&sa, 0, sizeof (sa));
	memset (&osa, 0, sizeof (osa));
	sa.sa_handler = cob_sig_handler;
#ifdef	SA_RESETHAND
	sa.sa_flags = SA_RESETHAND;
#endif
#ifdef	SA_NOCLDSTOP
	sa.sa_flags |= SA_NOCLDSTOP;
#endif

	for (k = 0; k < NUM_SIGNALS; k++) {
		if (signals[k].for_set) {
			/* Take direct control of some hard errors */
			if (signals[k].for_set == 2) {
				(void)sigemptyset (&sa.sa_mask);
				(void)sigaction (signals[k].sig, &sa, NULL);
			} else {
				/* for the others: only register if not configured
				   from the OS side to be ignored */
				(void)sigaction (signals[k].sig, NULL, &osa);
				if (osa.sa_handler != SIG_IGN) {
					(void)sigemptyset (&sa.sa_mask);
					(void)sigaction (signals[k].sig, &sa, NULL);
				}
				/* CHECKME: how should we handle externally registered
				            error handlers (handler != SIG_DFL)? */
			}
		}
	}
#else	/* still defined (HAVE_SIGNAL_H) */
	for (k = 0; k < NUM_SIGNALS; k++) {
		if (signals[k].for_set) {
			/* Take direct control of some hard errors */
			if (signals[k].for_set == 2) {
				(void)signal (signals[k].sig, cob_sig_handler);
			} else {
				/* for the others: only register if not configured
				   from the OS side to be ignored */
				if (signal (signals[k].sig, SIG_IGN) != SIG_IGN) {
					(void)signal (signals[k].sig, cob_sig_handler);
				}
				/* CHECKME: how should we handle externally registered
				            error handlers (handler != SIG_DFL)? */
			}
		}
	}
#endif
#endif
}

/* ASCII Sign - Reading and undo the "overpunch";
 * Note: if used on an EBCDIC machine this is actually _not_ an overpunch
 * but a replacement!
 *   positive: 0123456789
 *   negative: pqrstuvwxy
 * returns one of: 1 = positive (non-negative), -1 = negative
 */

static int
cob_get_sign_ascii (unsigned char *p)
{
#ifdef	COB_EBCDIC_MACHINE
	switch (*p) {
	case 'p':
		*p = (unsigned char)'0';
		return -1;
	case 'q':
		*p = (unsigned char)'1';
		return -1;
	case 'r':
		*p = (unsigned char)'2';
		return -1;
	case 's':
		*p = (unsigned char)'3';
		return -1;
	case 't':
		*p = (unsigned char)'4';
		return -1;
	case 'u':
		*p = (unsigned char)'5';
		return -1;
	case 'v':
		*p = (unsigned char)'6';
		return -1;
	case 'w':
		*p = (unsigned char)'7';
		return -1;
	case 'x':
		*p = (unsigned char)'8';
		return -1;
	case 'y':
		*p = (unsigned char)'9';
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		/* already without sign */
		return -1;
	}
	*p = (unsigned char)'0';
	return 1;
#else
	if (*p >= (unsigned char)'p' && *p <= (unsigned char)'y') {
		*p &= ~0x40;	/* 0x71 'q' -> 0x31 -> '1' */
		return -1;
	}
	if (IS_VALID_DIGIT_DATA (*p)) {
		/* already without sign */
		return 1;
	}
	*p = (unsigned char)'0';
	return 1;
#endif
}

/* overpunches the pointer 'p' with the sign
 * Note: if used on an EBCDIC machine this is actually _not_ an overpunch
 * but a replacement! */
static void
cob_put_sign_ascii (unsigned char *p)
{
#ifdef	COB_EBCDIC_MACHINE
	switch (*p) {
	case '0':
		*p = (unsigned char)'p';
		return;
	case '1':
		*p = (unsigned char)'q';
		return;
	case '2':
		*p = (unsigned char)'r';
		return;
	case '3':
		*p = (unsigned char)'s';
		return;
	case '4':
		*p = (unsigned char)'t';
		return;
	case '5':
		*p = (unsigned char)'u';
		return;
	case '6':
		*p = (unsigned char)'v';
		return;
	case '7':
		*p = (unsigned char)'w';
		return;
	case '8':
		*p = (unsigned char)'x';
		return;
	case '9':
		*p = (unsigned char)'y';
		return;
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
		/* already signed */
		return;
	default:
		*p = (unsigned char)'0';
	}
#else
	*p |= 0x40;
#endif
}

/* EBCDIC Sign - Reading and undo the "overpunch";
 * Note: if used on an ASCII machine this is actually _not_ an overpunch
 * but a replacement!
 *   positive: {ABCDEFGHI
 *   unsigned: 0123456789
 *   negative: }JKLMNOPQR
 * returns one of: 1 = positive (non-negative), -1 = negative, 0 = unsigned
 */

static int
cob_get_sign_ebcdic (unsigned char *p)
{
#ifdef	COB_EBCDIC_MACHINE
	char sign_nibble = *p & 0xF0;
	/* What to do here outside of 0 - 9? */
	if ((*p & 0x0F) > 9) {
		*p = sign_nibble;
	}
	switch (sign_nibble) {
	/* negative */
	case 0xC0:
	/* negative, non-preferred */
	case 0xA0:
	case 0xE0:
		return 1;
	/* positive */
	case 0xD0:
	/* positive, non-preferred */
	case 0xB0:
		return -1;
	/* unsigned  */
	case 0xF0:
		return 0;
		return -1;
	default:
		/* What to do here outside of sign nibbles? */
		return 1;
	}
#else
	switch (*p) {
	case '{':
		*p = (unsigned char)'0';
		return 1;
	case 'A':
		*p = (unsigned char)'1';
		return 1;
	case 'B':
		*p = (unsigned char)'2';
		return 1;
	case 'C':
		*p = (unsigned char)'3';
		return 1;
	case 'D':
		*p = (unsigned char)'4';
		return 1;
	case 'E':
		*p = (unsigned char)'5';
		return 1;
	case 'F':
		*p = (unsigned char)'6';
		return 1;
	case 'G':
		*p = (unsigned char)'7';
		return 1;
	case 'H':
		*p = (unsigned char)'8';
		return 1;
	case 'I':
		*p = (unsigned char)'9';
		return 1;
	case '}':
		*p = (unsigned char)'0';
		return -1;
	case 'J':
		*p = (unsigned char)'1';
		return -1;
	case 'K':
		*p = (unsigned char)'2';
		return -1;
	case 'L':
		*p = (unsigned char)'3';
		return -1;
	case 'M':
		*p = (unsigned char)'4';
		return -1;
	case 'N':
		*p = (unsigned char)'5';
		return -1;
	case 'O':
		*p = (unsigned char)'6';
		return -1;
	case 'P':
		*p = (unsigned char)'7';
		return -1;
	case 'Q':
		*p = (unsigned char)'8';
		return -1;
	case 'R':
		*p = (unsigned char)'9';
		return -1;
	default:
		if (*p >= '0' && *p <= '9') {
			return 0;
		}
		/* What to do here outside of 0 - 9? */
		if ((*p & 0x0F) > 9) {
			*p = (unsigned char)'0';
		} else {
			*p = (unsigned char)COB_I2D (COB_D2I (*p));
		}
		return 1;
	}
#endif
}

static void
cob_put_sign_ebcdic (unsigned char *p, const int sign)
{
	if (sign == -1) {
		switch (*p) {
		case '0':
			*p = (unsigned char)'}';
			return;
		case '1':
			*p = (unsigned char)'J';
			return;
		case '2':
			*p = (unsigned char)'K';
			return;
		case '3':
			*p = (unsigned char)'L';
			return;
		case '4':
			*p = (unsigned char)'M';
			return;
		case '5':
			*p = (unsigned char)'N';
			return;
		case '6':
			*p = (unsigned char)'O';
			return;
		case '7':
			*p = (unsigned char)'P';
			return;
		case '8':
			*p = (unsigned char)'Q';
			return;
		case '9':
			*p = (unsigned char)'R';
			return;
		case '}':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
			/* already signed */
			return;
		default:
			/* What to do here */
			*p = (unsigned char)'}';
			return;
		}
	}

	switch (*p) {
	case '0':
		*p = (unsigned char)'{';
		return;
	case '1':
		*p = (unsigned char)'A';
		return;
	case '2':
		*p = (unsigned char)'B';
		return;
	case '3':
		*p = (unsigned char)'C';
		return;
	case '4':
		*p = (unsigned char)'D';
		return;
	case '5':
		*p = (unsigned char)'E';
		return;
	case '6':
		*p = (unsigned char)'F';
		return;
	case '7':
		*p = (unsigned char)'G';
		return;
	case '8':
		*p = (unsigned char)'H';
		return;
	case '9':
		*p = (unsigned char)'I';
		return;
	case '{':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
		/* already signed */
		return;
	default:
		/* What to do here */
		*p = (unsigned char)'{';
		return;
	}
}

/* compare up to 'size' characters from buffer 'p'
   to a single character 'c',
   using collation 'col' */
static int
common_cmpc (const unsigned char *p, const unsigned int c,
	     const size_t size, const unsigned char *col)
{
	register const unsigned char *end = p + size;
	int			ret;

	const unsigned char c_col = col[c];
	while (p < end) {
		if ((ret = col[*p] - c_col) != 0) {
			return ret;
		}
		p++;
	}
	return 0;
}

/* compare up to 'size' characters in 's1' to 's2'
   using collation 'col' */
static int
common_cmps (const unsigned char *s1, const unsigned char *s2,
	     const size_t size, const unsigned char *col)
{
	register const unsigned char *end = s1 + size;
	int			ret;
	while (s1 < end) {
		if ((ret = col[*s1] - col[*s2]) != 0) {
			return ret;
		}
		s1++, s2++;
	}
	return 0;
}

/* compare up to 'size' characters in 'data' to characters
   in 'c' with size 'compare_size' */
static int
compare_character (const unsigned char *data, size_t size,
	const unsigned char *c, size_t compare_size)
{
	int 		ret;

	/* compare date with compare-data up to max compare-size */
	if ((ret = memcmp (data, c, compare_size)) != 0) {
		return ret;
	}

	/* first bytes in "data" are identical to "compare-data",
	   so use first bytes of "data" for next comparisons,
	   increasing it up to the complete data-size */
	{
		register const unsigned char	*p;	/* position to compare from */
		size -= compare_size;

		while (size > compare_size) {
			p = data + compare_size;
			if ((ret = memcmp (p, data, compare_size)) != 0) {
				return ret;
			}
			size -= compare_size;
			compare_size *= 2;
		}
		if (size > 0) {
			p = data + compare_size;
			return memcmp (p, data, size);
		}
	}

	return 0;
}

/* compare up to 'size' characters in 'data' to spaces */
static int
compare_spaces (const unsigned char *data, size_t size)
{
	if (size <= COB_SPACES_ALPHABETIC_BYTE_LENGTH) {
		return memcmp (data, COB_SPACES_ALPHABETIC, size);
	}
	return compare_character (data, size,
		(const unsigned char *)COB_SPACES_ALPHABETIC,
		COB_SPACES_ALPHABETIC_BYTE_LENGTH);
}

/* compare up to 'size' characters in 'data' to zeroes */
static int
compare_zeroes (const unsigned char *data, size_t size)
{
	if (size <= COB_ZEROES_ALPHABETIC_BYTE_LENGTH) {
		return memcmp (data, COB_ZEROES_ALPHABETIC, size);
	}
	return compare_character (data, size,
		(const unsigned char *)COB_ZEROES_ALPHABETIC,
		COB_ZEROES_ALPHABETIC_BYTE_LENGTH);
}

/* compare content of field 'f1' to repeated content of 'f2' */
static int
cob_cmp_all (cob_field *f1, cob_field *f2)
{
	const unsigned char	*col = COB_MODULE_PTR->collating_sequence;
	const unsigned char	*data1 = COB_FIELD_DATA (f1);
	const unsigned char	*data2 = f2->data;
	const size_t		size1 = COB_FIELD_SIZE (f1);
	const size_t		size2 = f2->size;
	/* CHECKME: What should be returned if f1 is negative? */
	/* CHECKME: Do we ever get here with f2 being numeric? */
	const int	sign = COB_GET_SIGN_ADJUST (f1);
	int ret = 0;

	/* check without collation */
	if (col == NULL) {
		if (size2 == 1) {
			if (*data2 == ' ') {
				/* check for IF VAR = [ALL] SPACE[S] */
				ret = compare_spaces (data1, size1);
			} else
			if (*data2 == '0') {
				/* check for IF VAR = [ALL] ZERO[ES] */
				ret = compare_zeroes (data1, size1);
			} else {
				/* check for IF VAR = ALL '5' / HIGH-VALUE / ... */
				ret = compare_character (data1, size1, data2, 1);
			}
		} else
		/* check for IF VAR = ALL ... / ... */
		if (size1 > size2) {
			ret = compare_character (data1, size1, data2, size2);
		} else {
			ret = compare_character (data1, size1, data2, size1);
		}
	} else
	/* check with collation */
	if (size2 == 1) {
		/* check for IF VAR = ALL "9" / IF VAR = ZERO */
		ret = common_cmpc (data1, *data2, size1, col);
	} else {
		/* check for IF VAR = ALL "AB" ... */
		const size_t	chunk_size = size2;
		size_t		size_loop = size1;
		while (size_loop >= chunk_size) {
			if ((ret = common_cmps (data1, data2, chunk_size, col)) != 0) {
				break;
			}
			size_loop -= chunk_size;
			data1 += chunk_size;
		}
		if (!ret
		 && size1 > 0) {
			ret = common_cmps (data1, data2, size_loop, col);
		}
	}

	COB_PUT_SIGN_ADJUSTED (f1, sign);
	return ret;
}

/* compare content of field 'f1' to content of 'f2', space padded,
   using the optional collating sequence of the program */
static int
cob_cmp_alnum (cob_field *f1, cob_field *f2)
{
	const unsigned char	*col = COB_MODULE_PTR->collating_sequence;
	const unsigned char	*data1 = COB_FIELD_DATA (f1);
	const unsigned char	*data2 = COB_FIELD_DATA (f2);
	const size_t		size1 = COB_FIELD_SIZE (f1);
	const size_t		size2 = COB_FIELD_SIZE (f2);	
	const size_t	min = (size1 < size2) ? size1 : size2;
	int		ret;

	if (col == NULL) {		/* check without collation */

		/* Compare common substring */
		if ((ret = memcmp (data1, data2, min)) != 0) {
			return ret;
		}

		/* Compare the rest (if any) with spaces */
		if (size1 > size2) {
			const size_t spaces_to_test = size1 - min;
			return compare_spaces (data1 + min, spaces_to_test);
		} else if (size1 < size2) {
			const size_t spaces_to_test = size2 - min;
			return -compare_spaces (data2 + min, spaces_to_test);
		}
	
	} else {		/* check with collation */

		/* Compare common substring */
		if ((ret = common_cmps (data1, data2, min, col)) != 0) {
			return ret;
		}

		/* Compare the rest (if any) with spaces */
		if (size1 > size2) {
			const size_t spaces_to_test = size1 - min;
			return common_cmpc (data1 + min, ' ', spaces_to_test, col);
		} else if (size1 < size2) {
			const size_t spaces_to_test = size2 - min;
			return -common_cmpc (data2 + min, ' ', spaces_to_test, col);
		}

	}

	return 0;
}

/* comparision of all key fields for SORT (without explicit collation)
   in records pointed to by 'data1' and 'data2' */
static int
sort_compare (const void *data1, const void *data2)
{
	size_t		i;
	int		res;
	cob_field	f1;
	cob_field	f2;

	for (i = 0; i < sort_nkeys; ++i) {
		f1 = f2 = *sort_keys[i].field;
		f1.data = (unsigned char *)data1 + sort_keys[i].offset;
		f2.data = (unsigned char *)data2 + sort_keys[i].offset;
		if (COB_FIELD_IS_NUMERIC (&f1)) {
			res = cob_numeric_cmp (&f1, &f2);
		} else {
			res = memcmp (f1.data, f2.data, f1.size);
		}
		if (res != 0) {
			return (sort_keys[i].flag == COB_ASCENDING) ? res : -res;
		}
	}
	return 0;
}

/* comparision of all key fields for SORT (with explicit collation)
   in records pointed to by 'data1' and 'data2' */
static int
sort_compare_collate (const void *data1, const void *data2)
{
	size_t		i;
	int		res;
	cob_field	f1;
	cob_field	f2;

	for (i = 0; i < sort_nkeys; ++i) {
		f1 = f2 = *sort_keys[i].field;
		f1.data = (unsigned char *)data1 + sort_keys[i].offset;
		f2.data = (unsigned char *)data2 + sort_keys[i].offset;
		if (COB_FIELD_IS_NUMERIC (&f1)) {
			res = cob_numeric_cmp (&f1, &f2);
		} else {
			res = common_cmps (f1.data, f2.data, f1.size, sort_collate);
		}
		if (res != 0) {
			return (sort_keys[i].flag == COB_ASCENDING) ? res : -res;
		}
	}
	return 0;
}

/* intermediate move using USAGE DISPLAY field to 'dst' using
   buffer 'src' with given 'size' as source */
static void
cob_move_intermediate (cob_field *dst, const void *src, const size_t size)
{
	cob_field	intermediate;
	intermediate.size = size;
	intermediate.data = (cob_u8_ptr)src;
	/* note: if the target is numeric then cob_move will convert
	         on the fly to numeric as necessary */
	intermediate.attr = &const_alpha_attr;
	cob_move (&intermediate, dst);
}

/* intermediate move from 'src' to 'dst'
   as if it would be of COB_TYPE_ALPANUMERIC */
static void
cob_move_to_group_as_alnum (cob_field *src, cob_field *dst)
{
	cob_field	intermediate;
	cob_field_attr	attr;
	/* group moves are defined as memcpy + fill, so move shaddow field with
	   same attributes and data storage but type alnum instead, which will
	   lead to "unpacked" numeric data in the group */
	intermediate = *dst;
	intermediate.attr = &attr;
	attr = *dst->attr;
	attr.type = COB_TYPE_ALPHANUMERIC;
	cob_move (src, &intermediate);
}

/* open file using mode according to cob_unix_lf and
   filename (append when starting with +) */
static fd_t cob_open_logfile (const char *filename)
{
	const char *mode;

	if (!cobsetptr->cob_unix_lf) {
		if (*filename == '+') {
			filename++;
			mode = "a";
		} else {
			mode = "w";
		}
	} else {
		if (*filename == '+') {
			filename++;
			mode = "ab";
		} else {
			mode = "wb";
		}
	}
	return fopen (filename, mode);
}

/* ensure that cob_trace_file is available for writing */
static void
cob_check_trace_file (void)
{
	if (cobsetptr->cob_trace_file) {
		return;
	}
	if (cobsetptr->cob_trace_filename) {
		cobsetptr->cob_trace_file = cob_open_logfile (cobsetptr->cob_trace_filename);
		if (!cobsetptr->cob_trace_file) {
			/* could not open the file
			   unset the filename for not referencing it later */
			cobsetptr->cob_trace_filename = NULL;
			cobsetptr->cob_trace_file = stderr;
		}
	} else {
		cobsetptr->cob_trace_file = stderr;
	}
}

/* close current trace file (if open) and open/attach a new one */
static void
cob_new_trace_file (void)
{
	fd_t old_trace_file = cobsetptr->cob_trace_file;

	if (!cobsetptr->cob_trace_file
	 || cobsetptr->external_trace_file
	 || cobsetptr->cob_trace_file == stderr) {
		cobsetptr->cob_trace_file = VFS_INVALID_FD;
		cob_check_trace_file ();
		return;
	}

	fclose (cobsetptr->cob_trace_file);
	cobsetptr->cob_trace_file = VFS_INVALID_FD;

	cob_check_trace_file ();
	if (cobsetptr->cob_display_print_file
	 && cobsetptr->cob_display_print_file == old_trace_file) {
		cobsetptr->cob_display_print_file = cobsetptr->cob_trace_file;
	}
	if (cobsetptr->cob_dump_file
	 && cobsetptr->cob_dump_file == old_trace_file) {
		cobsetptr->cob_dump_file = cobsetptr->cob_trace_file;
	}
#ifdef COB_DEBUG_LOG
	if (cob_debug_file
	 && cob_debug_file == old_trace_file) {
		cob_debug_file = cobsetptr->cob_trace_file;
	}
#endif
}

int
cob_check_env_true (char * s)
{
	return s && ((strlen (s) == 1 && (*s == 'Y' || *s == 'y' || *s == '1'))
	          || strcasecmp (s, "YES") == 0 || strcasecmp (s, "ON") == 0
	          || strcasecmp (s, "TRUE") == 0);
}

int
cob_check_env_false (char * s)
{
	return s && ((strlen (s) == 1 && (*s == 'N' || *s == 'n' || *s == '0'))
	          || strcasecmp (s, "NO") == 0 || strcasecmp (s, "NONE") == 0
	          || strcasecmp (s, "OFF") == 0 || strcasecmp (s, "FALSE") == 0);
}

static void
cob_rescan_env_vals (void)
{
	int	i;
	int	j;
	int	old_type;
	char	*env;
	char	*save_source_file = (char *) cob_source_file;

	cob_source_file = NULL;
	cob_source_line = 0;

	/* Check for possible environment variables */
	for (i = 0; i < NUM_CONFIG; i++) {
		if (gc_conf[i].env_name
		 && (env = getenv (gc_conf[i].env_name)) != NULL) {
			old_type = gc_conf[i].data_type;
			gc_conf[i].data_type |= STS_ENVSET;

			if (*env != '\0' && set_config_val (env, i)) {
				gc_conf[i].data_type = old_type;

				/* Remove invalid setting */
				(void)cob_unsetenv (gc_conf[i].env_name);
			} else if (gc_conf[i].env_group == GRP_HIDE) {
				/* Any alias present? */
				for (j = 0; j < NUM_CONFIG; j++) {
					if (j != i
					 && gc_conf[i].data_loc == gc_conf[j].data_loc) {
						gc_conf[j].data_type |= STS_ENVSET;
						gc_conf[j].set_by = i;
					}
				}
			}
		}
	}
	cob_source_file = save_source_file;

	/* Extended ACCEPT status returns */
	if (cobsetptr->cob_extended_status == 0) {
		cobsetptr->cob_use_esc = 0;
	}
}

static COB_INLINE int
one_indexed_day_of_week_from_monday (int zero_indexed_from_sunday)
{
	return ((zero_indexed_from_sunday + 6) % 7) + 1;
}

#if defined (_MSC_VER) && COB_USE_VC2008_OR_GREATER
static void
set_cob_time_ns_from_filetime (const FILETIME filetime, struct cob_time *cb_time)
{
	ULONGLONG	filetime_int;

	filetime_int = (((ULONGLONG) filetime.dwHighDateTime) << 32)
		+ filetime.dwLowDateTime;
	/* FILETIMEs are accurate to 100 nanosecond intervals */
	cb_time->nanosecond = (filetime_int % (ULONGLONG) 10000000) * 100;
}
#endif

/* Global functions */

/* get last exception (or 0 if not active) */
int
cob_get_last_exception_code (void)
{
	return last_exception_code;
}

/* get exception name for last raised exception */
const char *
cob_get_last_exception_name (void)
{
	size_t	n;

	/* direct match */
	for (n = 1; n < EXCEPTION_TAB_SIZE; ++n) {
		if (last_exception_code == cob_exception_tab_code[n]) {
			return cob_exception_tab_name[n];
		}
	}
	/* any match (last to first to get most specific one,
	   checking missing/not supported first) */
	if (cob_last_exception_is (COB_EC_IMP_FEATURE_MISSING)) {
		return cob_exception_tab_name[COB_EC_IMP_FEATURE_MISSING];
	}
	if (cob_last_exception_is (COB_EC_IMP_FEATURE_DISABLED)) {
		return cob_exception_tab_name[COB_EC_IMP_FEATURE_DISABLED];
	}
	for (n = EXCEPTION_TAB_SIZE - 1; n != 0; --n) {
		if ((last_exception_code & cob_exception_tab_code[n])
		 == cob_exception_tab_code[n]) {
			return cob_exception_tab_name[n];
		}
	}
	return NULL;
}

/* check if last exception is set and includes the given exception */
int
cob_last_exception_is (const int exception_to_check)
{
	if ((last_exception_code & cob_exception_tab_code[exception_to_check])
	 == cob_exception_tab_code[exception_to_check]) {
		return 1;
	} else {
		return 0;
	}
}

/* set last exception,
   used for EXCEPTION- functions and for cob_accept_exception_status,
   only reset on SET LAST EXCEPTION TO OFF */
void
cob_set_exception (const int id)
{
	cobglobptr->cob_exception_code = cob_exception_tab_code[id];
	last_exception_code = cobglobptr->cob_exception_code;

	cobglobptr->last_exception_statement = STMT_UNKNOWN;
	cobglobptr->last_exception_id = NULL;
	cobglobptr->last_exception_section = NULL;
	cobglobptr->last_exception_paragraph = NULL;

	if (id) {
		static char excp_mod[COB_MAX_WORDLEN + 1];
		static char excp_sec[COB_MAX_WORDLEN + 1];
		static char excp_para[COB_MAX_WORDLEN + 1];
		cob_module	*mod = COB_MODULE_PTR;
		cobglobptr->cob_got_exception = 1;
#if 0	/* consider addition for 4.x */
		cobglobptr->last_exception_source = cob_source_file;	/* needs to be strdup'd */
#endif
		cobglobptr->last_exception_line = cob_source_line;
		if (mod) {
#if 0	/* consider addition for 4.x */
			if (mod->module_sources
			 && mod->module_stmt != 0) {
				/* note: it is likely best to not copy name + line but store the name+line
				   together, maybe even comma-separated when we have copy->copy-prog */
				cobglobptr->last_exception_source = mod->module_sources
					[COB_GET_FILE_NUM (mod->module_stmt)];	/* needs to be strdup'd */
				cobglobptr->last_exception_line = COB_GET_LINE_NUM (mod->module_stmt);
#else
			if (mod->module_stmt != 0) {
				cobglobptr->last_exception_line = COB_GET_LINE_NUM (mod->module_stmt);
#endif
			}
			cobglobptr->last_exception_statement = mod->statement;
			if (mod->module_name) {
				strcpy (excp_mod, mod->module_name);
				cobglobptr->last_exception_id = excp_mod;
			}
			if (mod->section_name) {
				strcpy (excp_sec, mod->section_name);
				cobglobptr->last_exception_section = excp_sec;
			}
			if (mod->paragraph_name) {
				strcpy (excp_para, mod->paragraph_name);
				cobglobptr->last_exception_paragraph = excp_para;
			}
		}
	} else {
		cobglobptr->cob_got_exception = 0;
#if 0	/* consider addition for 4.x */
		if (cobglobptr->last_exception_source) {
			cobglobptr->last_exception_source = NULL;
			cob_free (cobglobptr->last_exception_source);
		}
#endif
		cobglobptr->last_exception_line = 0;
	}
}

/* add to last exception, set if empty */
void
cob_add_exception (const int id)
{
	if (!cobglobptr->cob_exception_code) {
		cob_set_exception (id);
		return;
	}
	cobglobptr->cob_exception_code |= cob_exception_tab_code[id];
	last_exception_code |= cob_exception_tab_code[id];
}

/* return the last exception value */
void
cob_accept_exception_status (cob_field *f)
{
	int exception = last_exception_code;
	/* Note: MF set this to a 9(3) item, we do a translation here (only works for USAGE DISPLAY!);
	   MF: intended for CALL only, 0=ok, 1=ENOMEM, 2=module not found, 128 other CALL failure,
	   "unpredictable if the last statement was not a CALL, especially: adjusted by fileio" */
	if (exception
	 && f->size == 3	/* FIXME: current code works only for DISPLAY, adjust to work for other usages */
	 && COB_FIELD_TYPE (f) == COB_TYPE_NUMERIC_DISPLAY) {
		if (exception == cob_exception_tab_code[COB_EC_PROGRAM_RESOURCES]) {
			exception = 1;
		} else if (exception == cob_exception_tab_code[COB_EC_PROGRAM_NOT_FOUND]) {
			exception = 2;
		} else if (exception || cob_exception_tab_code[COB_EC_PROGRAM]) {
			exception = 128;
		}
	}
	cob_set_int (f, exception);
}

void
cob_accept_user_name (cob_field *f)
{
	if (cobsetptr->cob_user_name) {
		cob_move_intermediate (f, cobsetptr->cob_user_name,
			    strlen (cobsetptr->cob_user_name));
	} else {
		cob_move_intermediate (f, " ", (size_t)1);
	}
}

void *
cob_malloc (const size_t size)
{
	void	*mptr;

	mptr = calloc ((size_t)1, size, &libcobPage);
	/* LCOV_EXCL_START */
	if (unlikely (!mptr)) {
		cob_fatal_error (COB_FERROR_MEMORY);
	}
	/* LCOV_EXCL_STOP */
	return mptr;
}

void *
cob_realloc (void * optr, const size_t osize, const size_t nsize)
{
	void	*mptr;

	/* LCOV_EXCL_START */
	if (unlikely (!optr)) {
		cob_fatal_error (COB_FERROR_FREE);
	}
	/* LCOV_EXCL_STOP */

	if (unlikely (osize == nsize)) {	/* No size change */
		return optr;
	} 
	if (unlikely (osize > nsize)) {		/* Reducing size */
		return realloc (optr, nsize, &libcobPage);
	}

	mptr = calloc ((size_t)1, nsize, &libcobPage);	/* New memory, past old is cleared */
	/* LCOV_EXCL_START */
	if (unlikely (!mptr)) {
		cob_fatal_error (COB_FERROR_MEMORY);
	}
	/* LCOV_EXCL_STOP */
	memcpy (mptr, optr, osize);
	cob_free (optr);
	return mptr;
}

void
cob_free (void * mptr)
{
#ifdef _DEBUG
	/* LCOV_EXCL_START */
	if (unlikely (!mptr)) {
		cob_fatal_error (COB_FERROR_FREE);
	}
	/* LCOV_EXCL_STOP */
#endif
	free (mptr, &libcobPage);

}

void *
cob_fast_malloc (const size_t size)
{
	void	*mptr;

	mptr = malloc (size, &libcobPage);
	/* LCOV_EXCL_START */
	if (unlikely (!mptr)) {
		cob_fatal_error (COB_FERROR_MEMORY);
	}
	/* LCOV_EXCL_STOP */
	return mptr;
}

char *
cob_strdup (const char *p)
{
	char	*mptr;
	size_t	len;

	len = strlen (p) + 1;
	mptr = (char *)cob_fast_malloc (len);
	memcpy (mptr, p, len);
	return mptr;
}

char *
cob_strndup (const char *p, const size_t max_len)
{
	char	*mptr;
	size_t	len;

	len = strlen (p);
	if (len > max_len) {
		len = max_len;
	}
	mptr = (char *)cob_fast_malloc (len + 1);
	memcpy (mptr, p, len);
	*(mptr + len) = 0;
	return mptr;
}

/* Caching versions of malloc/free */
void *
cob_cache_malloc (const size_t size)
{
	struct cob_alloc_cache	*cache_ptr;
	void			*mptr;

	cache_ptr = cob_malloc (sizeof (struct cob_alloc_cache));
	mptr = cob_malloc (size);
	cache_ptr->cob_pointer = mptr;
	cache_ptr->size = size;
	cache_ptr->next = cob_alloc_base;
	cob_alloc_base = cache_ptr;
	return mptr;
}

void *
cob_cache_realloc (void *ptr, const size_t size)
{
	struct cob_alloc_cache	*cache_ptr;
	void			*mptr;

	if (!ptr) {
		return cob_cache_malloc (size);
	}
	cache_ptr = cob_alloc_base;
	for (; cache_ptr; cache_ptr = cache_ptr->next) {
		if (ptr == cache_ptr->cob_pointer) {
			if (size <= cache_ptr->size) {
				return ptr;
			}
			mptr = cob_malloc (size);
			memcpy (mptr, cache_ptr->cob_pointer, cache_ptr->size);
			cob_free (cache_ptr->cob_pointer);
			cache_ptr->cob_pointer = mptr;
			cache_ptr->size = size;
			return mptr;
		}
	}
	return ptr;
}

void
cob_cache_free (void *ptr)
{
	struct cob_alloc_cache	*cache_ptr;
	struct cob_alloc_cache	*prev_ptr;

	if (!ptr) {
		return;
	}
	cache_ptr = cob_alloc_base;
	prev_ptr = cob_alloc_base;
	for (; cache_ptr; cache_ptr = cache_ptr->next) {
		if (ptr == cache_ptr->cob_pointer) {
			cob_free (cache_ptr->cob_pointer);
			if (cache_ptr == cob_alloc_base) {
				cob_alloc_base = cache_ptr->next;
			} else {
				prev_ptr->next = cache_ptr->next;
			}
			cob_free (cache_ptr);
			return;
		}
		prev_ptr = cache_ptr;
	}
}

static COB_INLINE int
hash (const char *s)
{
	register const char *p = s;
	register int	val = 0;

	while (*p) {
		val += *p++;
	}
	return val;
}

static COB_INLINE void
init_statement_hashlist (void)
{
	cob_statement_hash[STMT_UNKNOWN] = hash("UNKNOWN");
#define COB_STATEMENT(ename,str)	\
	cob_statement_hash[ename] = hash(str);
#include "statement.def"	/* located and installed next to common.h */
#undef COB_STATEMENT
}

/* this function is a compat-only function for pre 3.2 */
static enum cob_statement
get_stmt_from_name (const char *stmt_name)
{
	/* Note: When called from the same module then we commonly will
	   get the same pointer for the same statement name;
	   this allows us to use that for a pre-lookup instead of doing
	   the more expensive hash + compare each time */
#define STMT_CACHE_NUM 10
	static const char		*last_stmt_name[STMT_CACHE_NUM] = { 0 };
	static enum cob_statement	last_stmt[STMT_CACHE_NUM] = { 0 };
	static unsigned int 		last_stmt_entry = STMT_CACHE_NUM - 1;
	static unsigned int 		last_stmt_i = 0;

	/* nothing to resolve */
	if (!stmt_name) {
		return STMT_UNKNOWN;
	}

	/* check if stmt_name was previously resolved, then
	   just go with the old result */
	if (stmt_name == last_stmt_name[last_stmt_i]) {
		return last_stmt[last_stmt_i];
	} else {
		unsigned int i;
		for (i = 0; i < STMT_CACHE_NUM; i++) {
			if (stmt_name == last_stmt_name[i]) {
				last_stmt_i = i;
				return last_stmt[i];
			}
		}
	}
	
	/* statement name not resolved - build hash and compare using
	   hash + strcmp; then add to previously resolved list */
	{
		const int stmt_hash = hash (stmt_name);
		enum cob_statement stmt;

		if (cob_statement_hash[STMT_UNKNOWN] == 0) {
			init_statement_hashlist ();
		}

		/* overwrite */
		if (last_stmt_entry == STMT_CACHE_NUM - 1) {
			last_stmt_entry = 0;
		} else {
			last_stmt_entry++;
		}
		last_stmt_i = last_stmt_entry;

#define COB_STATEMENT(ename,str)                   \
		if (stmt_hash == cob_statement_hash[ename] \
		        && strcmp (str, stmt_name) == 0) { \
			stmt = ename;                          \
		} else
#include "statement.def"	/* located and installed next to common.h */
#undef COB_STATEMENT
		{
			cob_runtime_warning ("not handled statement: %s", stmt_name);
			return STMT_UNKNOWN;
		}
		
		last_stmt_name[last_stmt_i] = stmt_name;
		last_stmt[last_stmt_i] = stmt;
		return stmt;
	}

#undef STMT_CACHE_NUM

}

/* cob_set_location is kept for backward compatibility (pre 3.0);
   it stored the location for exception handling and related
   intrinsic functions and did tracing, depending on a global flag */
void
cob_set_location (const char *sfile, const unsigned int sline,
		  const char *csect, const char *cpara,
		  const char *cstatement)
{
	const enum cob_statement stmt = get_stmt_from_name (cstatement);
	cob_module	*mod = COB_MODULE_PTR;

	mod->section_name = csect;
	mod->paragraph_name = cpara;
	cob_source_file = sfile;
	cob_source_line = sline;

	/* compat: add to module structure */
	mod->statement = stmt;

	if (cobsetptr->cob_line_trace) {
		const char	*s;
		if (!cobsetptr->cob_trace_file) {
			cob_check_trace_file ();
#if _MSC_VER /* fix dumb warning */
			if (!cobsetptr->cob_trace_file) {
				return;
			}
#endif
		}
		if (!cob_last_sfile || strcmp (cob_last_sfile, sfile)) {
			if (cob_last_sfile) {
				cob_free ((void *)cob_last_sfile);
			}
			cob_last_sfile = cob_strdup (sfile);
			fprintf (cobsetptr->cob_trace_file, "Source :    '%s'\n", sfile);
		}
		if (COB_MODULE_PTR->module_name) {
			s = COB_MODULE_PTR->module_name;
		} else {
			s = _("unknown");
		}
		fprintf (cobsetptr->cob_trace_file,
			 "Program-Id: %-16s Statement: %-21.21s  Line: %u\n",
			 s, cstatement ? (char *)cstatement : _("unknown"),
			 sline);
		fflush (cobsetptr->cob_trace_file);
	}
}

/* cob_trace_section is kept for backward compatibility,
   and will be removed in 4.x */
void
cob_trace_section (const char *para, const char *source, const int line)
{
	const char	*s;

	if (cobsetptr->cob_line_trace) {
		const cob_module *mod = COB_MODULE_PTR;
		int pline = line;
		if (!cobsetptr->cob_trace_file) {
			cob_check_trace_file ();
#if _MSC_VER /* fix dumb warning */
			if (!cobsetptr->cob_trace_file) {
				return;
			}
#endif
		}
		if (source &&
		    (!cob_last_sfile || strcmp (cob_last_sfile, source))) {
			if (cob_last_sfile) {
				cob_free ((void *)cob_last_sfile);
			}
			cob_last_sfile = cob_strdup (source);
			fprintf (cobsetptr->cob_trace_file, "Source:     '%s'\n", source);
		}
		if (mod->module_name) {
			s = mod->module_name;
			if (pline == 0 && mod->module_stmt) {
				pline = COB_GET_LINE_NUM (mod->module_stmt);
			}
		} else {
			s = _("unknown");
		}
		fprintf (cobsetptr->cob_trace_file, "Program-Id: %-16s ", s);
		if (pline) {
			fprintf (cobsetptr->cob_trace_file, "%-34.34sLine: %d\n", para, pline);
		} else {
			fprintf (cobsetptr->cob_trace_file, "%s\n", para);
		}
		fflush (cobsetptr->cob_trace_file);
	}
}

/* New routines for handling 'trace' follow */
/* Note: As oposed to the old tracing these functions are only called
         if the following vars are set:
         COB_MODULE_PTR + ->module_stmt + ->module_sources
*/
static int
cob_trace_prep (void)
{
	const char	*s;
	if (!cobsetptr->cob_trace_file) {
		cob_check_trace_file ();
		if (!cobsetptr->cob_trace_file)
			return 1; 	/* silence warnings */
	}
	cob_get_source_line ();
	if (cob_source_file
	 && (!cob_last_sfile || strcmp (cob_last_sfile, cob_source_file))) {
		if (cob_last_sfile) {
			cob_free ((void *)cob_last_sfile);
		}
		cob_last_sfile = cob_strdup (cob_source_file);
		fprintf (cobsetptr->cob_trace_file, "Source: '%s'\n", cob_source_file);
	}
	if (COB_MODULE_PTR->module_name) {
		s = COB_MODULE_PTR->module_name;
	} else {
		s = _("unknown");
	}
	if (!cob_last_progid
	 || strcmp (cob_last_progid, s)) {
		cob_last_progid = s;
		if (COB_MODULE_PTR->module_type == COB_MODULE_TYPE_FUNCTION) {
			fprintf (cobsetptr->cob_trace_file, "Function-Id: %s\n", cob_last_progid);
		} else {
			fprintf (cobsetptr->cob_trace_file, "Program-Id:  %s\n", cob_last_progid);
		}
	}
	return 0;
}

static void
cob_trace_print (char *val)
{
	int	i;
	int last_pos = (int)(strlen (cobsetptr->cob_trace_format) - 1);

	/* note: only executed after cob_trace_prep (), so
	         call to cob_get_source_line () already done */
	for (i = 0; cobsetptr->cob_trace_format[i] != 0; i++) {
		if (cobsetptr->cob_trace_format[i] == '%') {
			i++;
			switch (cobsetptr->cob_trace_format[i]) {
			case 'P':
			case 'p':
				if (COB_MODULE_PTR && COB_MODULE_PTR->module_type == COB_MODULE_TYPE_FUNCTION) {
					if (i != last_pos) {
						fprintf (cobsetptr->cob_trace_file, "Function-Id: %-16s", cob_last_progid);
					} else {
						fprintf (cobsetptr->cob_trace_file, "Function-Id: %s", cob_last_progid);
					}
				} else {
					if (i != last_pos) {
						fprintf (cobsetptr->cob_trace_file, "Program-Id:  %-16s", cob_last_progid);
					} else {
						fprintf (cobsetptr->cob_trace_file, "Program-Id:  %s", cob_last_progid);
					}
				}
				break;
			case 'I':
			case 'i':
				fprintf (cobsetptr->cob_trace_file, "%s", cob_last_progid);
				break;
			case 'L':
			case 'l':
				fprintf (cobsetptr->cob_trace_file, "%6u", cob_source_line);
				break;
			case 'S':
			case 's':
				if (i != last_pos) {
					fprintf (cobsetptr->cob_trace_file, "%-42.42s", val);
				} else {
					fprintf (cobsetptr->cob_trace_file, "%s", val);
				}
				break;
			case 'F':
			case 'f':
				if (i != last_pos) {
					fprintf (cobsetptr->cob_trace_file, "Source: %-*.*s",
						-COB_MAX_NAMELEN, COB_MAX_NAMELEN, cob_last_sfile);
				} else {
					fprintf (cobsetptr->cob_trace_file, "Source: %s", cob_last_sfile);
				}
				break;
			default:
				fputc ('%', cobsetptr->cob_trace_file);
				fputc (cobsetptr->cob_trace_format[i], cobsetptr->cob_trace_file);
			}
		} else {
			fputc (cobsetptr->cob_trace_format[i], cobsetptr->cob_trace_file);
		}
	}
	fputc ('\n', cobsetptr->cob_trace_file);
	fflush (cobsetptr->cob_trace_file);
}

void
cob_trace_sect (const char *name)
{
	/* compat for pre 3.2 */
	COB_MODULE_PTR->section_name = name;

	/* actual tracing, if activated */
	if (cobsetptr->cob_line_trace
	 && (COB_MODULE_PTR->flag_debug_trace & COB_MODULE_TRACE)) {
		char	val[60];
		if (cob_trace_prep ()
		 || name == NULL) {
			return;
		}
		snprintf (val, sizeof (val), "  Section: %s", name);
		cob_trace_print (val);
		return;
	}
}

void
cob_trace_para (const char *name)
{
	/* compat for pre 3.2 */
	COB_MODULE_PTR->paragraph_name = name;

	/* actual tracing, if activated */
	if (cobsetptr->cob_line_trace
	 && (COB_MODULE_PTR->flag_debug_trace & COB_MODULE_TRACE)) {
		char	val[60];
		if (cob_trace_prep ()
		 || name == NULL) {
			return;
		}
		snprintf (val, sizeof (val), "Paragraph: %s", name);
		cob_trace_print (val);
		return;
	}
}

void
cob_trace_entry (const char *name)
{
	/* actual tracing, if activated */
	if (cobsetptr->cob_line_trace
	 && (COB_MODULE_PTR->flag_debug_trace & COB_MODULE_TRACE)) {
		char	val[60];
		if (cob_trace_prep ()
		 || name == NULL) {
			return;
		}
		snprintf (val, sizeof (val), "    Entry: %s", name);
		cob_trace_print (val);
		return;
	}
}

void
cob_trace_exit (const char *name)
{
	/* actual tracing, if activated */
	if (cobsetptr->cob_line_trace
	 && (COB_MODULE_PTR->flag_debug_trace & COB_MODULE_TRACE)) {
		char	val[60];
		if (cob_trace_prep ()
		 || name == NULL) {
			return;
		}
		snprintf (val, sizeof (val), "     Exit: %s", name);
		cob_trace_print (val);
		return;
	}
}

static void
do_trace_statement (const enum cob_statement stmt)
{
	char	val[60];
	if (cob_trace_prep ()) {
		return;
	}
	snprintf (val, sizeof (val), "           %s",
		stmt != STMT_UNKNOWN ? cob_statement_name[stmt] : _("unknown"));
	cob_trace_print (val);
}


/* this function is altogether a compat-only function for pre 3.2,
   later versions use cob_trace_statement(enum cob_statement) */
void
cob_trace_stmt (const char *stmt_name)
{
	const enum cob_statement stmt = get_stmt_from_name (stmt_name);

	/* compat: add to module structure */
	COB_MODULE_PTR->statement = stmt;

	/* actual tracing, if activated */
	if (cobsetptr->cob_line_trace
	 && (COB_MODULE_PTR->flag_debug_trace & COB_MODULE_TRACEALL)) {
		do_trace_statement (stmt);
	}
}

void
cob_trace_statement (const enum cob_statement stmt)
{
	/* actual tracing, if activated */
	if (cobsetptr->cob_line_trace
	 && (COB_MODULE_PTR->flag_debug_trace & COB_MODULE_TRACEALL)) {
		do_trace_statement (stmt);
	}
}

void
cob_nop (void)
{
	/* this is only an empty function, a call to it may be inserted by cobc
	   to force some optimizations in the C compiler to not be triggered in
	   a quite portable way */
	;
}

void
cob_ready_trace (void)
{
	cobsetptr->cob_line_trace = 1;
}

void
cob_reset_trace (void)
{
	cobsetptr->cob_line_trace = 0;
}

unsigned char *
cob_get_pointer (const void *srcptr)
{
	void	*tmptr;

	memcpy (&tmptr, srcptr, sizeof (void *));
	return (cob_u8_ptr)tmptr;
}

static void
call_exit_handlers_and_terminate (void)
{
	if (exit_hdlrs != NULL) {
		struct exit_handlerlist *h = exit_hdlrs;
		while (h != NULL) {
			/* ensure that exit handlers set their own locations */
			cob_source_file = NULL;
			cob_source_line = 0;
			/* tell 'em they are not called with any parameters */
			cobglobptr->cob_call_params = 0;

			/* actual call and starting next iteration */
			h->proc ();
			h = h->next;
		}
	}
	cob_terminate_routines ();
}

void
cob_stop_run (const int status)
{
	if (!cob_initialized) {
		cob_hard_failure ();
	}
	call_exit_handlers_and_terminate ();
	exit_code = status;
#ifndef COB_WITHOUT_JMP
	if (return_jmp_buffer_set) {
		longjmp (return_jmp_buf, 1);
	}
#endif
	exit (status);
}

void
cob_stop_error (void)
{
	cob_runtime_error ("STOP ERROR");
	cob_hard_failure ();
}

static int
handle_core_on_error ()
{
	unsigned int core_on_error = 0;
	if (cob_initialized) {
		core_on_error = cobsetptr->cob_core_on_error;
	} else {
		char *env_val = cob_getenv_direct ("COB_CORE_ON_ERROR");
		if (env_val && env_val[0] && env_val [1] == 0
		 && env_val[0] >= '0' && env_val[0] <= '3') {
			core_on_error = COB_D2I (env_val[0]);
		}
	}
	/* explicit create a coredump file */
	if (core_on_error == 3) {
		int ret = create_dumpfile ();
		if (ret) {
			/* creation did not work, set to "internally 4" */
			if (cob_initialized) {
				cobsetptr->cob_core_on_error = 4;
			}
			core_on_error = 4;
		}
	}
	return core_on_error;
}

void
cob_hard_failure ()
{
	unsigned int core_on_error = handle_core_on_error ();
	if (core_on_error != 4) {
		if (core_on_error == 2 && cob_initialized) {
			/* prevent unloading modules */
			cobsetptr->cob_physical_cancel = -1;
		}
		call_exit_handlers_and_terminate ();
	}
	exit_code = -1;
#ifndef COB_WITHOUT_JMP
	if (return_jmp_buffer_set) {
		longjmp (return_jmp_buf, -1);
	}
#endif
	/* if explicit requested for errors or
	   an explicit manual coredump creation did
	   not work raise an abort here */
	if (core_on_error == 2
	 || core_on_error == 4) {
		cob_raise (SIGABRT);
	}
	exit (EXIT_FAILURE);
}

void
cob_hard_failure_internal (const char *prefix)
{
	unsigned int core_on_error;
	if (prefix) {
		fprintf (stderr, "\n%s: ", prefix);
	} else {
		fprintf (stderr, "\n");
	}
	fprintf (stderr, _("Please report this!"));
	fprintf (stderr, "\n");
	core_on_error = handle_core_on_error ();
	if (core_on_error != 4) {
		if (core_on_error == 2 && cob_initialized) {
			/* prevent unloading modules */
			cobsetptr->cob_physical_cancel = -1;
		}
		call_exit_handlers_and_terminate ();
	}
	exit_code = -2;
#ifndef COB_WITHOUT_JMP
	if (return_jmp_buffer_set) {
		longjmp (return_jmp_buf, -2);
	}
#endif
	/* if explicit requested for errors or
	   an explicit manual coredump creation did
	   not work raise an abort here */
	if (core_on_error == 2
	 || core_on_error == 4) {
		cob_raise (SIGABRT);
	}
	exit (EXIT_FAILURE);
}

/* get internal exit code, which is set on
  STOP RUN / last GOBACK / runtime error / cob_tidy */
int
cob_last_exit_code ()
{
	return exit_code;
}

/* get pointer to last runtime error (empty if no runtime error raised) */
const char *
cob_last_runtime_error ()
{
	return runtime_err_str;
}

int
cob_is_initialized (void)
{
	return (cobglobptr != NULL);
}

cob_global *
cob_get_global_ptr (void)
{
	/* LCOV_EXCL_START */
	if (unlikely (!cob_initialized)) {
		cob_fatal_error (COB_FERROR_INITIALIZED);
	}
	/* LCOV_EXCL_STOP */
	return cobglobptr;
}

int
cob_module_global_enter (cob_module **module, cob_global **mglobal,
		  const int auto_init, const int entry, const unsigned int *name_hash)
{
	/* Check initialized */
	if (unlikely (!cob_initialized)) {
		if (auto_init) {
			cob_init (0, NULL);
		} else {
			cob_fatal_error (COB_FERROR_INITIALIZED);
		}
	}

	/* Set global pointer */
	*mglobal = cobglobptr;

#if 0 /* cob_call_name_hash and cob_call_from_c are rw-branch only features
         for now - TODO: activate on merge of r1547 */
	/* Was caller a COBOL module */
	if (name_hash != NULL
	 && cobglobptr->cob_call_name_hash != 0) {
		cobglobptr->cob_call_from_c = 1;
		while (*name_hash != 0) {	/* Scan table of values */
			if (cobglobptr->cob_call_name_hash == *name_hash) {
				cobglobptr->cob_call_from_c = 0;
				break;
			}
			name_hash++;
		}
	}
#else
	/* LCOV_EXCL_LINE */
	COB_UNUSED (name_hash);
#endif

	/* Check module pointer */
	if (!*module) {
		struct cob_alloc_module *mod_ptr;

		*module = cob_cache_malloc (sizeof (cob_module));
		/* Add to list of all modules activated */
		mod_ptr = cob_malloc (sizeof (struct cob_alloc_module));
		mod_ptr->cob_pointer = *module;
		mod_ptr->next = cob_module_list;
		cob_module_list = mod_ptr;
#if 0 /* cob_call_name_hash and cob_call_from_c are rw-branch only features
         for now - TODO: activate on merge of r1547 */
	} else if (entry == 0
		&& !cobglobptr->cob_call_from_c) {
#else
	} else if (entry == 0) {
#endif
		register int		k = 0;
		register cob_module	*mod;
		for (mod = COB_MODULE_PTR; mod; mod = mod->next) {
			if (*module == mod) {
				/* CHECKME: can we move this in 4.x to the generated program
				   to be done _before_ executing cob_module_global_enter using
				   a _static_ variable ? */
				if (cobglobptr->cob_stmt_exception) {
					/* CALL has ON EXCEPTION so return to caller */
					cob_set_exception (COB_EC_PROGRAM_RECURSIVE_CALL);
					cobglobptr->cob_stmt_exception = 0;
					return 1;
				}
				cob_module_err = mod;
				cob_fatal_error (COB_FERROR_RECURSIVE);
			}
			/* LCOV_EXCL_START */
			if (k++ == MAX_MODULE_ITERS) {	/* prevent endless loop in case of broken list */
				/* not translated as highly unexpected */
				cob_runtime_warning ("max module iterations exceeded, possible broken chain");
				break;
			}
			/* LCOV_EXCL_STOP */
		}
	}

	/* Save parameter count, get number from argc if main program */
	if (!COB_MODULE_PTR) {
		cobglobptr->cob_call_params = cob_argc - 1;
	}

	(*module)->module_num_params = cobglobptr->cob_call_params;

	/* Push module pointer */
	(*module)->next = COB_MODULE_PTR;
	COB_MODULE_PTR = *module;
	COB_MODULE_PTR->module_stmt = 0;
	COB_MODULE_PTR->statement = STMT_UNKNOWN;

	cobglobptr->cob_stmt_exception = 0;
	return 0;
}

void
cob_module_enter (cob_module **module, cob_global **mglobal,
		  const int auto_init)
{
	(void)cob_module_global_enter (module, mglobal, auto_init, 0, 0);
}

void
cob_module_leave (cob_module *module)
{
	COB_UNUSED (module);
	/* Pop module pointer */
	COB_MODULE_PTR = COB_MODULE_PTR->next;
}

void
cob_module_free (cob_module **module)
{
	struct cob_alloc_module	*ptr, *prv;
	if (*module == NULL) {
		return;
	}

	/* TODO: consider storing the last entry and a prev pointer
	   to optimize for the likely case of "program added last is removed"
	   instead of checking _all_ previous entries */
	prv = NULL;
	/* Remove from list of all modules activated */
	for (ptr = cob_module_list; ptr; ptr = ptr->next) {
		if (ptr->cob_pointer == *module) {
			if (prv == NULL) {
				cob_module_list = ptr->next;
			} else {
				prv->next = ptr->next;
			}
			cob_free (ptr);
			break;
		}
		prv = ptr;
	}

#if 0 /* cob_module->param_buf and cob_module->param_field are rw-branch only features
         for now - TODO: activate on merge of r1547 */
	&& !cobglobptr->cob_call_from_c
	if ((*module)->param_buf != NULL)
		cob_cache_free ((*module)->param_buf);
	if ((*module)->param_field != NULL)
		cob_cache_free ((*module)->param_field);
#endif
	cob_cache_free (*module);
	*module = NULL;
}

/* save module environment - returns an allocated cob_func_loc (free at cob_restore_func)
   and the intermediate return field (must be freed by caller) */
struct cob_func_loc *
cob_save_func (cob_field **savefld, const int params,
	       const int eparams, ...)
{
	struct cob_func_loc	*fl;
	int			numparams;

	if (unlikely (params > eparams)) {
		numparams = eparams;
	} else {
		numparams = params;
	}

	/* Allocate return field */
	*savefld = cob_malloc (sizeof (cob_field));

	/* Allocate save area */
	fl = cob_malloc (sizeof (struct cob_func_loc));
	fl->func_params = cob_malloc (sizeof (void *) * ((size_t)numparams + 1U));
	fl->data = cob_malloc (sizeof (void *) * ((size_t)numparams + 1U));

	/* Save values */
	fl->save_module = COB_MODULE_PTR->next;
	fl->save_call_params = cobglobptr->cob_call_params;
	fl->save_proc_parms = COB_MODULE_PTR->cob_procedure_params;
	fl->save_num_params = COB_MODULE_PTR->module_num_params;

	/* Set current values */
	COB_MODULE_PTR->cob_procedure_params = fl->func_params;
	cobglobptr->cob_call_params = numparams;
	if (numparams) {
		va_list			args;
		int			n;
		va_start (args, eparams);
		for (n = 0; n < numparams; ++n) {
			fl->func_params[n] = va_arg (args, cob_field *);
			if (fl->func_params[n]) {
				fl->data[n] = fl->func_params[n]->data;
			}
		}
		va_end (args);
	}
	return fl;
}

/* restores module environment - frees the passed cob_func_loc */
void
cob_restore_func (struct cob_func_loc *fl)
{
	/* Restore calling environment */
	cobglobptr->cob_call_params = fl->save_call_params;
#if	0	/* RXWRXW - MODNEXT */
	COB_MODULE_PTR->next = fl->save_module;
#endif
	COB_MODULE_PTR->cob_procedure_params = fl->save_proc_parms;
	COB_MODULE_PTR->module_num_params = fl->save_num_params;
	cob_free (fl->data);
	cob_free (fl->func_params);
	cob_free (fl);
}

struct ver_t {
	int major, minor, point;
	unsigned int version;
};

/*
 * Convert version components to an integer value for comparison.
 */
static COB_INLINE unsigned int
version_bitstring( const struct ver_t module )
{
	unsigned int version =
		((unsigned int)module.major << 24) |
		((unsigned int)module.minor << 16) |
		((unsigned int)module.point <<  8);
	return version;
}

void
cob_check_version (const char *prog,
		   const char *packver_prog, const int patchlev_prog)
{
	int nparts;
	struct ver_t lib = { 9, 9, 9 };
	struct ver_t app = { 0 };

	nparts = sscanf (PACKAGE_VERSION, "%d.%d.%d",
			 &lib.major, &lib.minor, &lib.point);

	if (nparts >= 2) {
		lib.version = version_bitstring(lib);

		(void)sscanf (packver_prog, "%d.%d.%d",
			 &app.major, &app.minor, &app.point);
		app.version = version_bitstring(app);

		if (app.major == 2 && app.minor < 2) {
			goto err;
		}
		/* COB_MODULE_PTR is expected to be set, because cob_module_global_enter
		   was called _directly_ before this function, also with 2.2,
		   still checking at least for the case of "misuse" of this function as
		   undocumented ABI call [as in our testsuite ...] */
		if (cobglobptr && COB_MODULE_PTR && !COB_MODULE_PTR->gc_version) {
			COB_MODULE_PTR->gc_version = packver_prog;
		}
		if (app.version == lib.version
		 && patchlev_prog <= PATCH_LEVEL) {
			return;
		}
		if (app.version < lib.version) {
			/* we only claim compatibility to 2.2+ */
			struct ver_t minimal = { 2, 2 }; 
			if (app.version <= version_bitstring (minimal)) {
				cannot_check_subscript = 1;
			}
			minimal.minor = 1;
			if (app.version >= version_bitstring (minimal)) {
				return;
			}
		}
	}

err:
	/* TODO: when CALLed - raise exception so program can go ON EXCEPTION */
	cob_runtime_error (_("version mismatch"));
	cob_runtime_hint (_("%s has version %s.%d"), prog,
			   packver_prog, patchlev_prog);
	cob_runtime_hint (_("%s has version %s.%d"), "libcob",
			   PACKAGE_VERSION, PATCH_LEVEL);
	cob_hard_failure ();
}

void
cob_parameter_check (const char *func_name, const int num_arguments)
{
	if (cobglobptr->cob_call_params < num_arguments) {
		/* TODO: raise exception */
		cob_runtime_error (_("CALL to %s requires %d arguments"),
				   func_name, num_arguments);
		cob_hard_failure ();
	}
}

void
cob_correct_numeric (cob_field *f)
{
	register unsigned char *p = f->data;
	unsigned char	*end = p + f->size;
	unsigned char	*c;

	if (!COB_FIELD_IS_NUMDISP (f)) {
		return;
	}

	if (COB_FIELD_HAVE_SIGN (f)) {
		/* Adjust for sign byte */
		if (unlikely (COB_FIELD_SIGN_LEADING (f))) {
			c = p++;
		} else {
			c = --end;
		}
		if (unlikely (COB_FIELD_SIGN_SEPARATE (f))) {
			if (*c != '+' && *c != '-') {
				*c = '+';
			}
		} else if (unlikely (COB_MODULE_PTR->ebcdic_sign)) {
			switch (*c) {
			case '{':
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case '}':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
				break;
			case '0':
				*c = '{';
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				*c = 'A' + (*c - '1');
				break;
			case 0:
			case ' ':
				*c = '{';
				break;
			default:
				break;
			}
		} else {
			if (!*c || *c == ' ') {
				*c = '0';
			}
		}
	} else {
		c = end - 1;
		if (unlikely (COB_MODULE_PTR->ebcdic_sign)) {
			switch (*c) {
			case 0:
			case ' ':
			case '{':
			case '}':
				*c = '0';
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
				*c = '1' + (*c - 'A');
				break;
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
				*c = '1' + (*c - 'J');
				break;
			default:
				break;
			}
		} else {
			switch (*c) {
			case 0:
			case ' ':
			case 'p':
				*c = '0';
				break;
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
				*c = *c - 'p';
				break;
			default:
				break;
			}
		}
	}
	while (p < end) {
		switch (*p) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		case 0:
		case ' ':
			*p = '0';
			break;
		default:
			if ((*p & 0x0F) <= 9) {
				*p = COB_I2D (*p & 0x0F);
			}
			break;
		}
		p++;
	}
}

static int
cob_check_numdisp (const cob_field *f)
{
	register const unsigned char	*p = f->data;
	const unsigned char		*end = p + f->size;

	if (COB_FIELD_HAVE_SIGN (f)) {
		/* Adjust for sign byte */
		unsigned char c;
		if (unlikely (COB_FIELD_SIGN_LEADING (f))) {
			c = *p++;
		} else {
			c = *(--end);
		}
		if (unlikely (COB_FIELD_SIGN_SEPARATE (f))) {
			if (c != '+' && c != '-') {
				return 0;
			}
		} else if (IS_INVALID_DIGIT_DATA (c)) {
			if (COB_MODULE_PTR->ebcdic_sign) {
				switch (c) {
				case '{':
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
				case 'G':
				case 'H':
				case 'I':
				case '}':
				case 'J':
				case 'K':
				case 'L':
				case 'M':
				case 'N':
				case 'O':
				case 'P':
				case 'Q':
				case 'R':
					break;
				default:
					return 0;
				}
			} else {
				switch (c) {
				case 'p':
				case 'q':
				case 'r':
				case 's':
				case 't':
				case 'u':
				case 'v':
				case 'w':
				case 'x':
				case 'y':
					break;
				default:
					return 0;
				}
			}
		}
	}

	while (p < end) {
		if (IS_INVALID_DIGIT_DATA (*p)) {
			return 0;
		}
		p++;
	}
	return 1;
}

/* Sign */

static COB_INLINE COB_A_INLINE unsigned char *
locate_sign (cob_field *f)
{
	if (COB_FIELD_SIGN_LEADING (f)) {
		return f->data;
	}
	return f->data + f->size - 1;
}

/* get sign from DISPLAY/PACKED field 'f';

   if 'adjust_ebcdic' is set then original DISPLAY data is "unpunched"
   for `ebcdic_sign` and return adjusted;
   that allows conversion without handling that afterwards
   
   returns one of: 1 = positive (non-negative); -1 = negative;
                   2 = positive (non-negative), adjusted;
				  -2 = negative, adjusted;
                   0 = neither DISPLAY nor PACKED */
int
cob_real_get_sign (cob_field *f, const int adjust_ebcdic)
{
	unsigned char	*p;

	switch (COB_FIELD_TYPE (f)) {
	case COB_TYPE_NUMERIC_DISPLAY:
		p = locate_sign (f);

		/* Get sign */
		if (unlikely (COB_FIELD_SIGN_SEPARATE (f))) {
			return (*p == '-') ? -1 : 1;
		}
		if (IS_VALID_DIGIT_DATA (*p)) {
			return 1;
		}
		if (*p == ' ') {
#if	0	/* RXWRXW - Space sign */
			*p = '0';
#endif
			return 1;
		}
		if (adjust_ebcdic) {
#ifdef	COB_EBCDIC_MACHINE
			if (COB_MODULE_PTR->ebcdic_sign) {
				return cob_get_sign_ebcdic (p);
			}
			return cob_get_sign_ascii (p) < 0 ? -2 : 2;
#else
			if (COB_MODULE_PTR->ebcdic_sign) {
				return cob_get_sign_ebcdic (p) < 0 ? -2 : 2;
			}
			return ((*p & 0xF0) == 0x70) ? -1 : 1;
#endif
		} else {
			if (COB_MODULE_PTR->ebcdic_sign) {
				return cob_get_sign_ebcdic (p);
			}
			return cob_get_sign_ascii (p);
		}
	case COB_TYPE_NUMERIC_PACKED:
		if (COB_FIELD_NO_SIGN_NIBBLE (f)) {
			return 1;
		}
		p = f->data + f->size - 1;
		return ((*p & 0x0F) == 0x0D) ? -1 : 1;
	}
	return 0;
}

/* store sign to DISPLAY/PACKED fields */
void
cob_real_put_sign (cob_field *f, const int sign)
{
	unsigned char	*p;

	switch (COB_FIELD_TYPE (f)) {
	case COB_TYPE_NUMERIC_DISPLAY:
		/* Note: we only locate the sign if needed,
		   as the common case will be "nothing to do" */
		if (unlikely (COB_FIELD_SIGN_SEPARATE (f))) {
			const unsigned char	c = (sign == -1) ? '-' : '+';
			p = locate_sign (f);
			if (*p != c) {
				*p = c;
			}
		} else if (unlikely (COB_MODULE_PTR->ebcdic_sign)) {
			p = locate_sign (f);
			cob_put_sign_ebcdic (p, sign);
		} else if (sign == -1) {
			p = locate_sign (f);
			cob_put_sign_ascii (p);
		}
		return;
	case COB_TYPE_NUMERIC_PACKED:
		if (COB_FIELD_NO_SIGN_NIBBLE (f)) {
			return;
		}
		p = f->data + f->size - 1;
		if (sign == -1) {
			*p = (*p & 0xF0) | 0x0D;
		} else {
			*p = (*p & 0xF0) | 0x0C;
		}
		return;
	}
}

/* Registration of external handlers */
void
cob_reg_sighnd (void (*sighnd) (int))
{
	if (!cob_initialized) {
		cob_set_signal ();
	}
	cob_ext_sighdl = sighnd;
}

/* Switch */

int
cob_get_switch (const int n)
{
	if (n < 0 || n > COB_SWITCH_MAX) {
		return 0;
	}
	return cob_switch[n];
}

void
cob_set_switch (const int n, const int flag)
{
	if (n < 0 || n > COB_SWITCH_MAX) {
		return;
	}
	if (flag == 0) {
		cob_switch[n] = 0;
	} else if (flag == 1) {
		cob_switch[n] = 1;
	}
}

int
cob_cmp (cob_field *f1, cob_field *f2)
{
	unsigned short		f1_type = COB_FIELD_TYPE (f1);
	unsigned short		f2_type = COB_FIELD_TYPE (f2);

	const int	f1_is_numeric = f1_type & COB_TYPE_NUMERIC;
	const int	f2_is_numeric = f2_type & COB_TYPE_NUMERIC;

	/* both numeric -> direct numeric compare */
	if (f1_is_numeric && f2_is_numeric) {
		return cob_numeric_cmp (f1, f2);
	}

	/* one is an internal ALL field (ZERO,LOW-VALUE, ...) */
	if (f2_type == COB_TYPE_ALPHANUMERIC_ALL) {
		if (f2->size == 1 && f2->data[0] == '0'
		 && f1_is_numeric) {
			return cob_cmp_int (f1, 0);
		}
		return cob_cmp_all (f1, f2);
	}
	if (f1_type == COB_TYPE_ALPHANUMERIC_ALL) {
		if (f1->size == 1 && f1->data[0] == '0'
		 && f2_is_numeric) {
			return -cob_cmp_int (f2, 0);
		}
		return -cob_cmp_all (f2, f1);
	}

#if 0	/* FIXME later: must cater for national fields, too,
		   at least if that is numeric NATIONAL	*/
	if (COB_FIELD_IS_NATIONAL (f1)) {
		...
	}
#endif

	/* all else -> alphanumeric comparision */

	/* if one is numeric (cannot be "both", as checked above), then
	   convert that to alphanumeric for final test;
	   note: this is _very_ seldom the case, during "make checkall"
	   only in test "Alphanumeric and binary numeric" */

	if (f1_is_numeric || f2_is_numeric) {
		/* Note: the standard explicit defines how to handle that:
		   intermediate MOVE to a NUMERIC_DISPLAY with same amount
		   of digits (= drop sign and implied decimal point), then
		   compare that */
		cob_field	field;
		cob_field_attr	attr;
		unsigned char	buff[COB_MAX_DIGITS + 10];

		/* note: the standard explicit forbits floating-point numeric
		   in this scenario */
		if (f1_is_numeric
		 && f1_type != COB_TYPE_NUMERIC_DISPLAY) {
			COB_FIELD_INIT (COB_FIELD_DIGITS (f1), buff, &attr);
			attr = *f1->attr;
			attr.type = COB_TYPE_NUMERIC_DISPLAY;
			attr.flags &= ~COB_FLAG_HAVE_SIGN;
			cob_move (f1, &field);
			f1 = &field;
		}
		if (f2_is_numeric
		 && f2_type != COB_TYPE_NUMERIC_DISPLAY) {
			COB_FIELD_INIT (COB_FIELD_DIGITS (f2), buff, &attr);
			attr = *f2->attr;
			attr.type = COB_TYPE_NUMERIC_DISPLAY;
			attr.flags &= ~COB_FLAG_HAVE_SIGN;
			cob_move (f2, &field);
			f2 = &field;
		}

		if (COB_FIELD_HAVE_SIGN (f1)) {
			/* Note: if field is numeric then it is always
			   USAGE DISPLAY here */
			if (f1 != &field) {
				const int	sign = COB_GET_SIGN (f1);
				int		ret = cob_cmp_alnum (f1, f2);
				COB_PUT_SIGN (f1, sign);
				return ret;
			} else {
				/* we operate on a buffer already, just go on */
				(void)cob_real_get_sign (f1, 0);
				return cob_cmp_alnum (f1, f2);
			}
		}

		if (COB_FIELD_HAVE_SIGN (f2)) {
			/* Note: if field is numeric then it is always
			   USAGE DISPLAY here */
			if (f2 != &field) {
				const int	sign = COB_GET_SIGN (f2);
				int		ret = cob_cmp_alnum (f1, f2);
				COB_PUT_SIGN (f2, sign);
				return ret;
			} else {
				/* we operate on a buffer already, just go on */
				(void)cob_real_get_sign (f2, 0);
				return cob_cmp_alnum (f1, f2);
			}
		}
		/* done here to have the data for non-signed numeric vs. non-numeric in scope */
		return cob_cmp_alnum (f1, f2);
	}

	/* both data not numeric: compare as string */
	return cob_cmp_alnum (f1, f2);
}

/* Class check */

int
cob_is_omitted (const cob_field *f)
{
	return f->data == NULL;
}

int
cob_is_numeric (const cob_field *f)
{

	switch (COB_FIELD_TYPE (f)) {
	case COB_TYPE_NUMERIC_BINARY:
		return 1;
	case COB_TYPE_NUMERIC_FLOAT:
		{
			float		fval;
			memcpy (&fval, f->data, sizeof (float));
			return !ISFINITE ((double)fval);
		}
	case COB_TYPE_NUMERIC_DOUBLE:
		{
			double		dval;
			memcpy (&dval, f->data, sizeof (double));
			return !ISFINITE (dval);
		}
	case COB_TYPE_NUMERIC_L_DOUBLE:
		{
			long double lval;
			memcpy (&lval, f->data, sizeof (long double));
			return !ISFINITE ((double)lval);
		}
	case COB_TYPE_NUMERIC_PACKED:
		{
			register const unsigned char *p = f->data;
			const unsigned char *end = p + f->size - 1;

			/* Check sign */			
			{
				const char sign = *end & 0x0F;
				if (COB_FIELD_NO_SIGN_NIBBLE (f)) {
					/* COMP-6 - Check last nibble */
					if (sign > 0x09) {
						return 0;
					}
				} else if (COB_FIELD_HAVE_SIGN (f)) {
					if (COB_MODULE_PTR->flag_host_sign
					 && sign == 0x0F) {
						/* all fine, go on */
					} else
					if (sign != 0x0C
					 && sign != 0x0D) {
						return 0;
					}
				} else if (sign != 0x0F) {
					return 0;
				}
			}

			/* Check high nibble of last byte */
			if ((*end & 0xF0) > 0x90) {
				return 0;
			}

			/* Check digits */
			while (p < end) {
				if (IS_INVALID_BCD_DATA (*p)) {
					return 0;
				}
				p++;
			}

			return 1;
		}
	case COB_TYPE_NUMERIC_DISPLAY:
		return cob_check_numdisp (f);
	case COB_TYPE_NUMERIC_FP_DEC64:
#ifdef	WORDS_BIGENDIAN
		return (f->data[0] & 0x78U) != 0x78U;
#else
		return (f->data[7] & 0x78U) != 0x78U;
#endif
	case COB_TYPE_NUMERIC_FP_DEC128:
#ifdef	WORDS_BIGENDIAN
		return (f->data[0] & 0x78U) != 0x78U;
#else
		return (f->data[15] & 0x78U) != 0x78U;
#endif
	default:
		{
			register const unsigned char *p = f->data;
			const unsigned char *end = p + f->size;

			while (p < end) {
				if (IS_INVALID_DIGIT_DATA (*p)) {
					return 0;
				}
				p++;
			}
			return 1;
		}
	}
}

int
cob_is_alpha (const cob_field *f)
{
	register const unsigned char *p = f->data;
	const unsigned char *end = p + f->size;

	while (p < end) {
		if (*p == (unsigned char)' '
		 || isalpha (*p)) {
			p++;
		} else {
			return 0;
		}
	}
	return 1;
}

int
cob_is_upper (const cob_field *f)
{
	register const unsigned char *p = f->data;
	const unsigned char *end = p + f->size;

	while (p < end) {
		if (*p == (unsigned char)' '
		 || isupper (*p)) {
			p++;
		} else {
			return 0;
		}
	}
	return 1;
}

int
cob_is_lower (const cob_field *f)
{
	register const unsigned char *p = f->data;
	const unsigned char *end = p + f->size;

	while (p < end) {
		if (*p == (unsigned char)' '
		 || islower (*p)) {
			p++;
		} else {
			return 0;
		}
	}
	return 1;
}

/* Table sort */

void
cob_table_sort_init (const size_t nkeys, const unsigned char *collating_sequence)
{
	sort_nkeys = 0;
	sort_keys = cob_malloc (nkeys * sizeof (cob_file_key));
	if (collating_sequence) {
		sort_collate = collating_sequence;
	} else {
		sort_collate = COB_MODULE_PTR->collating_sequence;
	}
}

void
cob_table_sort_init_key (cob_field *field, const int flag,
			 const unsigned int offset)
{
	sort_keys[sort_nkeys].field = field;
	sort_keys[sort_nkeys].flag = flag;
	sort_keys[sort_nkeys].offset = offset;
	sort_nkeys++;
}

void
cob_table_sort (cob_field *f, const int n)
{
	if (sort_collate) {
		qsort (f->data, (size_t) n, f->size, sort_compare_collate);
	} else {
		qsort (f->data, (size_t) n, f->size, sort_compare);
	}
	cob_free (sort_keys);
}

/* Run-time error checking */

void
cob_check_beyond_exit (const char *name)
{
	/* possibly allow to lower this to a runtime warning later */
	cob_runtime_error (_("code execution leaving %s"), name);
	cob_hard_failure ();
}

void
cob_check_based (const unsigned char *x, const char *name)
{
	if (!x) {
		/* name includes '' already and can be ... 'x' (addressed by 'y') */
		/* TODO: raise exception */
		cob_runtime_error (_("BASED/LINKAGE item %s has NULL address"), name);
		cob_hard_failure ();
	}
}

/* internal test for writing outside of storage iduring CALL / UDF invocation,
   checking 'fence_pre'/'fence_post' to contain 0xFFFEFDFC00 / 0xFAFBFCFD00;
   'statement' specifies the place where that happened, which may
   include psuedo-statements "INIT CALL" and "INIT UDF",
   'name' (optional) specifies the variable where this was recognized */
void
cob_check_fence (const char *fence_pre, const char *fence_post,
		const enum cob_statement stmt, const char *name)
{
	if (memcmp (fence_pre, "\xFF\xFE\xFD\xFC\xFB\xFA\xFF", 8)
	 || memcmp (fence_post, "\xFA\xFB\xFC\xFD\xFE\xFF\xFA", 8)) {
		if (name) {
			cob_runtime_error (_("memory violation detected for '%s' after %s"),
				name, cob_statement_name[stmt]);
		} else {
			cob_runtime_error (_("memory violation detected after %s"),
				cob_statement_name[stmt]);
		}
		cob_hard_failure ();
	}
}

/* raise argument mismatch after pushing a temporary static "current module"
   as COB_MODULE_PTR; caller needs to restore pop it afterwards! */
static int
raise_arg_mismatch (const char *entry_name,
		const char **module_sources, unsigned int module_stmt)
{
	static cob_module mod_temp;

	cob_module *mod = &mod_temp;

	memset (mod, 0, sizeof (cob_module));
	mod->next = COB_MODULE_PTR;
	mod->module_name = entry_name;	/* not correct, but enough */
	mod->module_sources = module_sources;
	mod->statement = STMT_ENTRY;
	mod->module_stmt = module_stmt;
	COB_MODULE_PTR = mod;

	cob_set_exception (COB_EC_PROGRAM_ARG_MISMATCH);

	if (cobglobptr->cob_stmt_exception) {
		/* CALL has ON EXCEPTION so return to caller */
		cobglobptr->cob_stmt_exception = 0;
		return 0;
	}
	return 1;
}

/* validates that the data item 'name' was passed by the caller
   and has at least as much size as used in the callee,
   used during CALL in the entry points of the callee to check
   for COB_EC_PROGRAM_ARG_MISMATCH */
int
cob_check_linkage_size (const char *entry_name,
		const char *name, const unsigned int ordinal_pos,
		const int optional, const unsigned long size,
		const char **module_sources, unsigned int module_stmt)
{
	/* name includes '' already and can be ... 'x' of 'y' */

	if (!cobglobptr || !COB_MODULE_PTR) {
		/* unlikely case: runtime not initialized, or we have no module
		   so caller _must_ be something other than a GnuCOBOL module
		   (while ENTRY-CONVENTION is COBOL) -> skip these checks */
		/* possibly raise (an optional) runtime warning */
		return 0;
	} else if (cobglobptr->cob_call_params < ordinal_pos) {
		if (optional) {
			return 0;
		} else {
			if (raise_arg_mismatch (entry_name, module_sources, module_stmt)) {
				cob_runtime_error (_("LINKAGE item %s not passed by caller"), name);
				cob_hard_failure ();
			}
			COB_MODULE_PTR = COB_MODULE_PTR->next;
		}
		return -1;
	} else {
		/* note: the current module points to the caller, as we
		         are early in the called function (its entry point) */
		const cob_field		*parameter = COB_MODULE_PTR->cob_procedure_params[ordinal_pos - 1];
		if (!parameter || !parameter->data) {
			if (optional) {
				return 0;
			} else {
				if (raise_arg_mismatch (entry_name, module_sources, module_stmt)) {
					cob_runtime_error (_("LINKAGE item %s not passed by caller"), name);
					cob_hard_failure ();
				}
				COB_MODULE_PTR = COB_MODULE_PTR->next;
			}
			return -1;
		} else {
			if (parameter->size < size) {
				if (raise_arg_mismatch (entry_name, module_sources, module_stmt)) {
					cob_runtime_error (_("LINKAGE item %s (size %lu) too small in the caller (size %lu)"),
						name, size, (unsigned long) parameter->size);
					cob_hard_failure ();
				}
				COB_MODULE_PTR = COB_MODULE_PTR->next;
				return -1;
			} else if ((unsigned long)parameter->size != size) {
				/* possible warning that can additionally be activated */
			}
		}
	}
	return 0;
}

/* validates that the data item 'name' has a non-null data 'x',
   used for both CALL (COB_EC_PROGRAM_ARG_MISMATCH) and
   for actual use of the argument (COB_EC_PROGRAM_ARG_OMITTED) */
void
cob_check_linkage (const unsigned char *x, const char *name, const int check_type)
{
	if (!x) {
		/* name includes '' already and can be ... 'x' of 'y' */
		switch (check_type) {
		case 0: /* check for passed items on module entry */
			cob_set_exception (COB_EC_PROGRAM_ARG_MISMATCH);
			if (cobglobptr->cob_stmt_exception) {
				/* CALL has ON EXCEPTION so return to caller */
				cobglobptr->cob_stmt_exception = 0;
				return;
			}
			cob_runtime_error (_("LINKAGE item %s not passed by caller"), name);
			break;
		case 1: /* check for passed OPTIONAL items on item use */
			cob_set_exception (COB_EC_PROGRAM_ARG_OMITTED);
			cob_runtime_error (_("LINKAGE item %s not passed by caller"), name);
			break;
		}
		cob_hard_failure ();
	}
}

const char *
explain_field_type (const cob_field *f)
{
	switch (COB_FIELD_TYPE (f)) {
	case COB_TYPE_GROUP:
		return "GROUP";
	case COB_TYPE_BOOLEAN:
		return "BOOLEAN";
	case COB_TYPE_NUMERIC_DISPLAY:
		return "NUMERIC DISPLAY";
	case COB_TYPE_NUMERIC_BINARY:
		return "BINARY";
	case COB_TYPE_NUMERIC_PACKED:
		if (COB_FIELD_NO_SIGN_NIBBLE (f)) {
			return "COMP-6";
		}
		if (!COB_FIELD_HAVE_SIGN (f)) {
			return "PACKED-DECIMAL (unsigned)";
		}
		return "PACKED-DECIMAL";
	case COB_TYPE_NUMERIC_FLOAT:
		return "FLOAT";
	case COB_TYPE_NUMERIC_DOUBLE:
		return "DOUBLE";	/* FLOAT-LONG */
	case COB_TYPE_NUMERIC_L_DOUBLE:
		return "LONG DOUBLE";	/* FLOAT-EXTENDED */
	case COB_TYPE_NUMERIC_FP_DEC64:
		return "FP DECIMAL 64";
	case COB_TYPE_NUMERIC_FP_DEC128:
		return "FP DECIMAL 128";
	case COB_TYPE_NUMERIC_FP_BIN32:
		return "FP BINARY 32";
	case COB_TYPE_NUMERIC_FP_BIN64:
		return "FP BINARY 64";
	case COB_TYPE_NUMERIC_FP_BIN128:
		return "FP BINARY 128";
	/* note: may be not reached depending on endianness */
	case COB_TYPE_NUMERIC_COMP5:
		return "COMP-5";
	case COB_TYPE_NUMERIC_EDITED:
		return "NUMERIC EDITED";
	case COB_TYPE_ALPHANUMERIC:
		return "ALPHANUMERIC";
	case COB_TYPE_ALPHANUMERIC_ALL:
		return "ALPHANUMERIC ALL";
	case COB_TYPE_ALPHANUMERIC_EDITED:
		return "ALPHANUMERIC EDITED";
	case COB_TYPE_NATIONAL:
		return "NATIONAL";
	case COB_TYPE_NATIONAL_EDITED:
		return "NATIONAL EDITED";
	default:
		break;
	}
	return "UNKNOWN";
}

void
cob_check_numeric (const cob_field *f, const char *name)
{
	if (!cob_is_numeric (f)) {
		register unsigned char	*data = f->data;
		unsigned char *end = data + f->size;
		char		*p;
		char		*buff;

		cob_set_exception (COB_EC_DATA_INCOMPATIBLE);
		buff = cob_fast_malloc ((size_t)COB_SMALL_BUFF);
		p = buff;
		if (COB_FIELD_IS_NUMDISP(f) || COB_FIELD_IS_ANY_ALNUM(f)) {
			while (data < end) {
				if (isprint (*data)) {
					*p++ = *data++;
				} else {
					p += sprintf (p, "\\%03o", *data++);
				}
			}
		} else {
			p += sprintf (p, "0x");
			while (data < end) {
				p += sprintf (p, "%02x", *data++);
			}
		}
		*p = '\0';
		cob_runtime_error (_("'%s' (Type: %s) not numeric: '%s'"),
			name, explain_field_type(f), buff);
		cob_free (buff);
		cob_hard_failure ();
	}
}

void
cob_check_odo (const int i, const int min, const int max,
			const char *name, const char *dep_name)
{
	/* Check OCCURS DEPENDING ON item */
	if (i < min || i > max) {
		cob_set_exception (COB_EC_BOUND_ODO);

		/* Hack for call from 2.0 modules as the module signature was changed :-(
		   Note: depending on the actual C runtime this may work or directly break
		*/
		/* LCOV_EXCL_START */
		if (dep_name == NULL) {
			dep_name = name;
			name = "unknown field";
		}
		/* LCOV_EXCL_STOP */

		cob_runtime_error (_("OCCURS DEPENDING ON '%s' out of bounds: %d"),
					dep_name, i);
		if (i > max) {
			cob_runtime_hint (_("maximum subscript for '%s': %d"), name, max);
		} else {
			cob_runtime_hint (_("minimum subscript for '%s': %d"), name, min);
		}
		cob_hard_failure ();
	}
}

void
cob_check_subscript (const int i, const int max,
			const char *name, const int odo_item)
{
#if 1
	/* Hack for call from 2.0 modules as the module signature was changed :-(
	   Note: depending on the actual C runtime this may work or directly break
	*/
	/* LCOV_EXCL_START */
	if (cannot_check_subscript) {
		/* Check zero subscript */
		if (i == 0) {
			cob_set_exception (COB_EC_BOUND_SUBSCRIPT);
			cob_runtime_error (_("subscript of '%s' out of bounds: %d"), "unknown field", i);
			cob_hard_failure ();
		}
		return;
	}
	/* LCOV_EXCL_STOP */
#endif

	/* Check subscript */
	if (i < 1 || i > max) {
		cob_set_exception (COB_EC_BOUND_SUBSCRIPT);
		cob_runtime_error (_("subscript of '%s' out of bounds: %d"), name, i);
		if (i >= 1) {
			if (odo_item) {
				cob_runtime_hint (_("current maximum subscript for '%s': %d"),
							name, max);
			} else {
				cob_runtime_hint (_("maximum subscript for '%s': %d"),
							name, max);
			}
		}
		cob_hard_failure ();
	}
}

void
cob_check_ref_mod_detailed (const char *name, const int abend, const int zero_allowed,
	const int size, const int offset, const int length)
{
	const int minimal_length = zero_allowed ? 0 : 1;

	/* Check offset */
	if (offset < 1 || offset > size) {
		cob_set_exception (COB_EC_BOUND_REF_MOD);
		if (offset < 1) {
			cob_runtime_error (_("offset of '%s' out of bounds: %d"),
			   name, offset);
		} else {
			cob_runtime_error (_("offset of '%s' out of bounds: %d, maximum: %d"),
			   name, offset, size);
		}
		if (abend) {
			cob_hard_failure ();
		}
	}

	/* Check plain length */
	if (length < minimal_length || length > size) {
		cob_set_exception (COB_EC_BOUND_REF_MOD);
		if (length < minimal_length) {
			cob_runtime_error (_("length of '%s' out of bounds: %d"),
			   name, length);
		} else {
			cob_runtime_error (_("length of '%s' out of bounds: %d, maximum: %d"),
			   name, length, size);
		}
		if (abend) {
			cob_hard_failure ();
		}
	}

	/* Check length with offset */
	if (offset + length - 1 > size) {
		cob_set_exception (COB_EC_BOUND_REF_MOD);
		cob_runtime_error (_("length of '%s' out of bounds: %d, starting at: %d, maximum: %d"),
			name, length, offset, size);
		if (abend) {
			cob_hard_failure ();
		}
	}
}

/* kept for 2.2-3.1-rc1 compat only */
void
cob_check_ref_mod (const int offset, const int length,
	const int size, const char* name)
{
	cob_check_ref_mod_detailed (name, 1, 0, size, offset, length);
}

void
cob_check_ref_mod_minimal (const char* name, const int offset, const int length)
{
	/* Check offset */
	if (offset < 1) {
		cob_set_exception (COB_EC_BOUND_REF_MOD);
		cob_runtime_error (_("offset of '%s' out of bounds: %d"),
			name, offset);
		cob_hard_failure ();
	}

	/* Check length */
	if (length < 1) {
		cob_set_exception (COB_EC_BOUND_REF_MOD);
		cob_runtime_error (_("length of '%s' out of bounds: %d"),
			name, length);
		cob_hard_failure ();
	}
}

/* check if already allocated, if yes returns its address and sets exlength */
static void *
cob_external_addr_lookup (const char *exname, int *exlength)
{
	struct cob_external *eptr;

	for (eptr = basext; eptr; eptr = eptr->next) {
		if (!strcmp (exname, eptr->ename)) {
			if (exlength) {
				*exlength = eptr->esize;
			}
			return eptr->ext_alloc;
		}
	}
	return NULL;
}

/* allocate new external entry;
   returns allocated pointer with requested size */
static void *
cob_external_addr_create (const char *exname, int exlength)
{
	struct cob_external *eptr;

	eptr = cob_malloc (sizeof (struct cob_external));
	eptr->next = basext;
	eptr->esize = exlength;
	eptr->ename = cob_strdup (exname);
	eptr->ext_alloc = cob_malloc ((size_t)exlength);
	basext = eptr;

	return eptr->ext_alloc;
}

/* lookup external item, if already created before check given length;
   returns allocated pointer with at least requested size */
void *
cob_external_addr (const char *exname, const int exlength)
{
	int		stored_length;
	void	*ret;

	/* special external "C" registers */
	if (exlength == sizeof (int)
	 && !strcmp (exname, "ERRNO")) {
		return &errno;
	}

	/* Locate or allocate EXTERNAL item */
	ret = cob_external_addr_lookup (exname, &stored_length);
	if (ret != NULL) {
		if (exlength > stored_length) {
			cob_runtime_error (_ ("EXTERNAL item '%s' previously allocated with size %d, requested size is %d"),
				exname, stored_length, exlength);
			cob_hard_failure ();
		}
		if (exlength < stored_length) {
			cob_runtime_warning (_ ("EXTERNAL item '%s' previously allocated with size %d, requested size is %d"),
				exname, stored_length, exlength);
		}
		cobglobptr->cob_initial_external = 0;
	} else {
		ret = cob_external_addr_create (exname, exlength);
		cobglobptr->cob_initial_external = 1;
	}
	return ret;
}

#if defined (_MSC_VER) && COB_USE_VC2008_OR_GREATER

/* Get function pointer for most precise time function
   GetSystemTimePreciseAsFileTime is available since OS-version Windows 2000
   GetSystemTimeAsFileTime        is available since OS-version Windows 8 / Server 2012
*/
static void
get_function_ptr_for_precise_time (void)
{
	HMODULE		kernel32_handle;

	kernel32_handle = GetModuleHandle (TEXT ("kernel32.dll"));
	if (kernel32_handle != NULL) {
		time_as_filetime_func = (VOID (WINAPI *) (LPFILETIME))
			GetProcAddress (kernel32_handle, "GetSystemTimePreciseAsFileTime");
	}
	if (time_as_filetime_func == NULL) {
		time_as_filetime_func = GetSystemTimeAsFileTime;
	}
}
#endif

/* split the timep to cob_time and set the offset from UTC */
void
static set_cob_time_from_localtime (time_t curtime,
		struct cob_time *cb_time) {

	struct tm	*tmptr = NULL;
#if !defined (_BSD_SOURCE) && !defined (HAVE_TIMEZONE)
	time_t		utctime, lcltime, difftime;
#endif

	static time_t last_time = 0;
	static struct cob_time last_cobtime;
	
	/* FIXME: on setting related locale set last_time = 0 */
	if (curtime == last_time) {
		memcpy (cb_time, &last_cobtime, sizeof (struct cob_time));
		return;
	}
	if (last_time != 0) {
		const time_t sec_passed = curtime - last_time;
		/* most likely we are in the same hour as before - in which case
		   there is no need to lookup the tz-database via localtime -
		   instead directly adjust the last value */
		if (sec_passed > 0 && sec_passed < 3600) {
			memcpy (cb_time, &last_cobtime, sizeof (struct cob_time));
			cb_time->second += (int)sec_passed;
			if (cb_time->second < 60) {
				memcpy (&last_cobtime, cb_time, sizeof (struct cob_time));
				last_time = curtime;
				return;
			} else {
				cb_time->minute += cb_time->second / 60;
				/* note: if the minute is >= 60 then we need to adjust the hour
				   and may end up with DST issues (or simple day switch),
				   we may also get into minute == 60 for leap second cases;
				   do a full recalculation in this case */
				if (cb_time->minute < 60) {
					cb_time->second = cb_time->second % 60;
					memcpy (&last_cobtime, cb_time, sizeof (struct cob_time));
					last_time = curtime;
					return;
				}
			}
		}
	}
	last_time = curtime;

	localtime (curtime, tmptr);

	cb_time->year = tmptr->tm_year + 1900;
	cb_time->month = tmptr->tm_mon + 1;
	cb_time->day_of_month = tmptr->tm_mday;
	cb_time->day_of_week = one_indexed_day_of_week_from_monday (tmptr->tm_wday);
	cb_time->day_of_year = tmptr->tm_yday + 1;
	cb_time->hour = tmptr->tm_hour;
	cb_time->minute = tmptr->tm_min;
	/* LCOV_EXCL_START */
	/* Leap seconds ? */
	if (tmptr->tm_sec >= 60) {
		tmptr->tm_sec = 59;
	}
	/* LCOV_EXCL_STOP */
	cb_time->second = tmptr->tm_sec;
	cb_time->nanosecond = 0;
	cb_time->is_daylight_saving_time = tmptr->tm_isdst;

#if defined (_BSD_SOURCE)
	cb_time->offset_known = 1;
	cb_time->utc_offset = tmptr->tm_gmtoff / 60;
#elif defined (HAVE_TIMEZONE)
	cb_time->offset_known = 1;
	cb_time->utc_offset = timezone / -60;
	/* LCOV_EXCL_START */
	if (tmptr->tm_isdst) {
		cb_time->utc_offset += 60;
	}
	/* LCOV_EXCL_STOP */
#else
	lcltime = mktime (tmptr);

	tmptr = gmtime (&curtime);
	utctime = mktime (tmptr);

	if (utctime != -1 && lcltime != -1) { /* LCOV_EXCL_BR_LINE */
		difftime = utctime - lcltime;
		/* LCOV_EXCL_START */
		if (tmptr->tm_isdst) {
			difftime -= 3600;
		}
		/* LCOV_EXCL_STOP */
		cb_time->utc_offset = difftime / 60;
		cb_time->offset_known = 1;
	/* LCOV_EXCL_START */
	} else {
		cb_time->offset_known = 0;
		cb_time->utc_offset = 0;
	}
	/* LCOV_EXCL_STOP */
#endif

	/* keep backup for next call */
	memcpy (&last_cobtime, cb_time, sizeof (struct cob_time));
}

#if defined (_WIN32) /* cygwin does not define _WIN32 */
static struct cob_time
cob_get_current_date_and_time_from_os (const enum cob_datetime_res res)
{
	SYSTEMTIME	local_time;
#if defined (_MSC_VER) && COB_USE_VC2008_OR_GREATER
	FILETIME	filetime;
	SYSTEMTIME	utc_time;
#endif

	time_t		curtime;
	struct cob_time	cb_time;

	curtime = time (NULL);
	set_cob_time_from_localtime (curtime, &cb_time);

	if (res <= DTR_TIME_NO_NANO) {
		cb_time.nanosecond = 0;
		return cb_time;
	}

	/* Get nanoseconds with highest precision possible */
#if defined (_MSC_VER) && COB_USE_VC2008_OR_GREATER
	if (!time_as_filetime_func) {
		get_function_ptr_for_precise_time ();
	}
#pragma warning(suppress: 6011) /* the function pointer is always set by get_function_ptr_for_precise_time */
	(time_as_filetime_func) (&filetime);
	/* fallback to GetLocalTime if one of the following does not work */
	if (FileTimeToSystemTime (&filetime, &utc_time) &&
		SystemTimeToTzSpecificLocalTime (NULL, &utc_time, &local_time)) {
		set_cob_time_ns_from_filetime (filetime, &cb_time);
		return cb_time;
	}
#endif
	GetLocalTime (&local_time);
	cb_time.nanosecond = local_time.wMilliseconds * 1000000;
	return cb_time;
}
#else
static struct cob_time
cob_get_current_date_and_time_from_os (const enum cob_datetime_res res)
{
#if defined (HAVE_CLOCK_GETTIME)
	struct timespec	time_spec;
#elif defined (HAVE_SYS_TIME_H) && defined (HAVE_GETTIMEOFDAY)
	struct timeval	tmv;
#endif
	time_t		curtime;
	struct cob_time	cb_time;

	/* Get the current time */
#if defined (HAVE_CLOCK_GETTIME)
	clock_gettime (CLOCK_REALTIME, &time_spec);
	curtime = time_spec.tv_sec;
#elif defined (HAVE_SYS_TIME_H) && defined (HAVE_GETTIMEOFDAY)
	gettimeofday (&tmv, NULL);
	curtime = tmv.tv_sec;
#else
	time (&curtime);
#endif

	set_cob_time_from_localtime (curtime, &cb_time);

	/* Get nanoseconds or microseconds, if possible */
#if defined (HAVE_CLOCK_GETTIME)
	cb_time.nanosecond = (int) time_spec.tv_nsec;
#elif defined (HAVE_SYS_TIME_H) && defined (HAVE_GETTIMEOFDAY)
	cb_time.nanosecond = tmv.tv_usec * 1000;
#else
	cb_time.nanosecond = 0;
#endif

	return cb_time;
}
#endif

/* obsolete function, only used in cobc before 3.2 */
struct cob_time
cob_get_current_date_and_time (void)
{
	return cob_get_current_datetime (DTR_TIME_NO_NANO);
}

struct cob_time
cob_get_current_datetime (const enum cob_datetime_res res)
{
	struct cob_time	cb_time = cob_get_current_date_and_time_from_os (res);

	/* Do we have a constant time? */
	if (cobsetptr != NULL
	 && cobsetptr->cob_time_constant.year != 0) {
		if (cobsetptr->cob_time_constant.hour != -1) {
			cb_time.hour = cobsetptr->cob_time_constant.hour;
		}
		if (cobsetptr->cob_time_constant.minute != -1) {
			cb_time.minute = cobsetptr->cob_time_constant.minute;
		}
		if (cobsetptr->cob_time_constant.second != -1) {
			cb_time.second = cobsetptr->cob_time_constant.second;
		}
		if (cobsetptr->cob_time_constant.nanosecond != -1) {
			cb_time.nanosecond = cobsetptr->cob_time_constant.nanosecond;
		}
		if (cobsetptr->cob_time_constant.offset_known) {
			cb_time.offset_known = cobsetptr->cob_time_constant.offset_known;
			cb_time.utc_offset = cobsetptr->cob_time_constant.utc_offset;
		}

		if (cobsetptr->cob_time_constant_is_calculated) {
			cb_time.year = cobsetptr->cob_time_constant.year;
			cb_time.month = cobsetptr->cob_time_constant.month;
			cb_time.day_of_month = cobsetptr->cob_time_constant.day_of_month;
			cb_time.day_of_week = cobsetptr->cob_time_constant.day_of_week;
			cb_time.day_of_year = cobsetptr->cob_time_constant.day_of_year;
			cb_time.is_daylight_saving_time = cobsetptr->cob_time_constant.is_daylight_saving_time;
		} else {
			int		needs_calculation = 0;
			/* Note: constant time but X not part of constant --> -1 */
			if (cobsetptr->cob_time_constant.year != -1) {
				cb_time.year = cobsetptr->cob_time_constant.year;
				needs_calculation = 1;
			}
			if (cobsetptr->cob_time_constant.month != -1) {
				cb_time.month = cobsetptr->cob_time_constant.month;
				needs_calculation = 1;
			}
			if (cobsetptr->cob_time_constant.day_of_month != -1) {
				cb_time.day_of_month = cobsetptr->cob_time_constant.day_of_month;
				needs_calculation = 1;
			}
			/* set day_of_week, day_of_year, is_daylight_saving_time, if necessary */
			if (needs_calculation) {
				time_t		t;
				struct tm 	*tmptr = NULL;
				/* allocate tmptr (needs a correct time) */
				time (&t);
				localtime (t, tmptr);
				tmptr->tm_isdst = -1;
				tmptr->tm_sec	= cb_time.second;
				tmptr->tm_min	= cb_time.minute;
				tmptr->tm_hour	= cb_time.hour;
				tmptr->tm_year	= cb_time.year - 1900;
				tmptr->tm_mon	= cb_time.month - 1;
				tmptr->tm_mday	= cb_time.day_of_month;
				tmptr->tm_wday	= -1;
				tmptr->tm_yday	= -1;
				(void)mktime(tmptr);
				cb_time.day_of_week = one_indexed_day_of_week_from_monday (tmptr->tm_wday);
				cb_time.day_of_year = tmptr->tm_yday + 1;
				cb_time.is_daylight_saving_time = tmptr->tm_isdst;
			}
		}
	}

	/* Leap seconds ? */
	if (cb_time.second >= 60) {
		cb_time.second = 59;
	}

	return cb_time;
}

int
cob_set_date_from_epoch (struct cob_time *cb_time, const unsigned char *p)
{
	struct tm	*tmptr = NULL;
	time_t		t = 0;
	long long	seconds = 0;

	while (IS_VALID_DIGIT_DATA (*p)) {
		seconds = seconds * 10 + COB_D2I (*p++);
	}
	if (*p != 0 || seconds > 253402300799) {
		/* The value (as a unix timestamp) corresponds to date
		   "Dec 31 9999 23:59:59 UTC", which is the latest date that __DATE__
		   and __TIME__ can store.  */
		return 1;
	}

	/* allocate tmptr for epoch */
	localtime (t, tmptr);
	/* set seconds, minutes, hours and big days */
	tmptr->tm_sec = seconds % 60;
	seconds /= 60;
	tmptr->tm_min = seconds % 60;
	seconds /= 60;
	tmptr->tm_hour = seconds % 24;
	seconds /= 24;
	tmptr->tm_mday = (int)seconds;
	tmptr->tm_isdst = -1;

	/* normalize if needed (definitely for epoch, but also for example 30 Feb
		to be changed to correct march date),
		set tm_wday, tm_yday and tm_isdst */
	if (mktime (tmptr) == -1) {
		return 1;
	}

	cb_time->year = tmptr->tm_year + 1900;
	cb_time->month = tmptr->tm_mon + 1;
	cb_time->day_of_month = tmptr->tm_mday;
	cb_time->hour = tmptr->tm_hour;
	cb_time->minute = tmptr->tm_min;
	cb_time->second = tmptr->tm_sec;
	cb_time->nanosecond = -1;

	cb_time->day_of_week = tmptr->tm_wday + 1;
	cb_time->day_of_year = tmptr->tm_yday + 1;
	cb_time->is_daylight_saving_time = tmptr->tm_isdst;
	return 0;
}

static void
check_current_date ()
{
	int		yr, mm, dd, hh, mi, ss, ns;
	int		offset = 9999;
	int		i, ret;
	time_t		t;
	struct tm	*tmptr = NULL;
	char	iso_timezone[7] = { 0 };
	unsigned char	*p = (unsigned char*)cobsetptr->cob_date;

	if (p == NULL) {
		return;
	}

	/* skip quotes and space-characters */
	while (*p == '\''
	    || *p == '"'
	    || isspace (*p)) {
		p++;
	}

	/* extract epoch, if specified */
	if (*p == '@') {
		/* @sssssssss   seconds since epoch */
		ret = cob_set_date_from_epoch (&cobsetptr->cob_time_constant, ++p);
		if (ret) {
			cob_runtime_warning (_("COB_CURRENT_DATE '%s' is invalid"), cobsetptr->cob_date);
		}
		return;
	}

	yr = mm = dd = hh = mi = ss = ns = -1;
	ret = 0;

	/* extract date */
	if (*p) {
		yr = 0;
		for (i = 0; *p; p++) {
			if (IS_INVALID_DIGIT_DATA (*p)) {
				break;
			}
			yr = yr * 10 + COB_D2I (*p);
			if (++i == 4) {
				p++;
				break;
			}
		}
		if (i != 2 && i != 4) {
			/* possible template with partial system lookup */
			if (*p == 'Y') {
				while (*p == 'Y') p++;
			} else {
				ret = 1;
			}
			yr = -1;
		} else if (yr < 100) {
			yr += 2000;
		}
		if (*p == '/'
		 || *p == '-') {
			p++;
		}
	}
	if (*p) {
		mm = 0;
		for (i = 0; *p; p++) {
			if (IS_INVALID_DIGIT_DATA (*p)) {
				break;
			}
			mm = mm * 10 + COB_D2I (*p);
			if (++i == 2) {
				p++;
				break;
			}
		}
		if (i != 2) {
			/* possible template with partial system lookup */
			if (*p == 'M') {
				while (*p == 'M') p++;
			} else {
				ret = 1;
			}
			mm = -1;
		} else if (mm < 1 || mm > 12) {
			ret = 1;
		}
		if (*p == '/'
		 || *p == '-') {
			p++;
		}
	}
	if (*p) {
		dd = 0;
		for (i = 0; *p; p++) {
			if (IS_INVALID_DIGIT_DATA (*p)) {
				break;
			}
			dd = dd * 10 + COB_D2I (*p);
			if (++i == 2) {
				p++;
				break;
			}
		}
		if (i != 2) {
			/* possible template with partial system lookup */
			if (*p == 'D') {
				while (*p == 'D') p++;
			} else {
				ret = 1;
			}
			dd = -1;
		} else if (dd < 1 || dd > 31) {
			ret = 1;
		}
	}

	/* extract time */
	if (*p) {
		hh = 0;
		while (isspace (*p)) p++;
		for (i = 0; *p; p++) {
			if (IS_INVALID_DIGIT_DATA (*p)) {
				break;
			}
			hh = hh * 10 + COB_D2I (*p);
			if (++i == 2) {
				p++;
				break;
			}
		}

		if (i != 2) {
			/* possible template with partial system lookup */
			if (*p == 'H') {
				while (*p == 'H') p++;
			} else {
				ret = 1;
			}
			hh = -1;
		} else if (hh > 23) {
			ret = 1;
		}
		if (*p == ':'
		 || *p == '-')
			p++;
	}
	if (*p) {
		mi = 0;
		for (i = 0; *p; p++) {
			if (IS_INVALID_DIGIT_DATA (*p)) {
				break;
			}
			mi = mi * 10 + COB_D2I (*p);
			if (++i == 2) {
				p++;
				break;
			}
		}
		if (i != 2) {
			/* possible template with partial system lookup */
			if (*p == 'M') {
				while (*p == 'M') p++;
			} else {
				ret = 1;
			}
			mi = -1;
		} else if (mi > 59) {
			ret = 1;
		}
		if (*p == ':'
		 || *p == '-') {
			p++;
		}
	}

	if (*p != 0
	 && *p != 'Z'
	 && *p != '+'
	 && *p != '-') {
		ss = 0;
		for (i = 0; *p != 0; p++) {
			if (IS_INVALID_DIGIT_DATA (*p)) {
				break;
			}
			ss = ss * 10 + COB_D2I (*p);
			if (++i == 2) {
				p++;
				break;
			}
		}
		if (i != 2) {
			/* possible template with partial system lookup */
			if (*p == 'S') {
				while (*p == 'S') p++;
			} else {
				ret = 1;
			}
			ss = -1;
		/* leap second would be 60 */
		} else if (ss > 60) {
			ret = 1;
		}
	}

	/* extract nanoseconds */
	if (*p != 0
	 && *p != 'Z'
	 && *p != '+'
	 && *p != '-') {
		ns = 0;
		if (*p == '.'
		 || *p == ':') {
			p++;
		}
		for (i = 0; *p; p++) {
			if (IS_INVALID_DIGIT_DATA (*p)) {
				break;
			}
			ns = ns * 10 + COB_D2I (*p);
			if (++i == 9) {
				p++;
				break;
			}
		}
	}

	/* extract UTC offset */
	if (*p == 'Z') {
		offset = 0;
		iso_timezone[0] = 'Z';
	} else
	if (*p == '+'
	 || *p == '-') {
		/* we operate on a buffer here to drop the included ":" */
		int len = snprintf (&iso_timezone[0], 7, "%s", p);
		if (len == 3) {
			memcpy (iso_timezone + 3, "00", 3);
		} else
		if (len >= 5 && iso_timezone[3] == ':') {
			snprintf (&iso_timezone[3], 3, "%s", p + 4);
			len--;
		}
		if (len > 5) {
			ret = 1;
		}
		for (i = 1; i < 5 && iso_timezone[i] != 0; i++) {
			if (IS_INVALID_DIGIT_DATA (iso_timezone[i])) {
				break;
			}
		}
		i--;
		if (i == 4) {
			offset = COB_D2I (iso_timezone[1]) * 60 * 10
			       + COB_D2I (iso_timezone[2]) * 60
			       + COB_D2I (iso_timezone[3]) * 10
			       + COB_D2I (iso_timezone[4]);
			if (iso_timezone[0] == '-') {
				offset *= -1;
			}
		} else {
			ret = 1;
			iso_timezone[0] = '\0';
		}
	}

	if (ret != 0) {
		cob_runtime_warning (_("COB_CURRENT_DATE '%s' is invalid"), cobsetptr->cob_date);
		return;
	}

	/* get local time, allocate tmptr */
	time (&t);
	localtime (t, tmptr);
	/* override given parts in time */
	if (ss != -1) {
		tmptr->tm_sec	= ss;
	}
	if (mi != -1) {
		tmptr->tm_min	= mi;
	}
	if (hh != -1) {
		tmptr->tm_hour	= hh;
	}
	if (yr != -1) {
		tmptr->tm_year	= yr - 1900;
	}
	if (mm != -1) {
		tmptr->tm_mon	= mm - 1;
	}
	if (dd != -1) {
		tmptr->tm_mday	= dd;
	}

	tmptr->tm_isdst = -1;

	/* normalize if needed (for example 30 Feb to be changed to
	   correct march date), set tm_wday, tm_yday and tm_isdst */
	(void) mktime (tmptr);

	/* set datetime constant */
	if (yr != -1) {
		cobsetptr->cob_time_constant.year = tmptr->tm_year + 1900;
	} else {
		cobsetptr->cob_time_constant.year = -1;
	}
	if (mm != -1) {
		cobsetptr->cob_time_constant.month = tmptr->tm_mon + 1;
	} else {
		cobsetptr->cob_time_constant.month = -1;
	}
	if (dd != -1) {
		cobsetptr->cob_time_constant.day_of_month = tmptr->tm_mday;
	} else {
		cobsetptr->cob_time_constant.day_of_month = -1;
	}

	if (hh != -1) {
		cobsetptr->cob_time_constant.hour	= tmptr->tm_hour;
	} else {
		cobsetptr->cob_time_constant.hour	= -1;
	}
	if (mi != -1) {
		cobsetptr->cob_time_constant.minute	= tmptr->tm_min;
	} else {
		cobsetptr->cob_time_constant.minute	= -1;
	}
	if (ss != -1) {
		cobsetptr->cob_time_constant.second	= tmptr->tm_sec;
	} else {
		cobsetptr->cob_time_constant.second	= -1;
	}
	cobsetptr->cob_time_constant.nanosecond	= ns;

	/* the following are only set in the constant, if the complete date is set,
	   otherwise in the "current" instances */
	if (yr != -1 && mm != -1 && dd != -1) {
		cobsetptr->cob_time_constant_is_calculated = 1;
		cobsetptr->cob_time_constant.day_of_week = one_indexed_day_of_week_from_monday (tmptr->tm_wday);
		cobsetptr->cob_time_constant.day_of_year = tmptr->tm_yday + 1;
		cobsetptr->cob_time_constant.is_daylight_saving_time = tmptr->tm_isdst;
	} else {
		cobsetptr->cob_time_constant_is_calculated = 0;
		cobsetptr->cob_time_constant.day_of_week = -1;
		cobsetptr->cob_time_constant.day_of_year = -1;
		cobsetptr->cob_time_constant.is_daylight_saving_time = -1;
	}

	if (iso_timezone[0] != '\0') {
		cobsetptr->cob_time_constant.offset_known = 1;
		cobsetptr->cob_time_constant.utc_offset = offset;
	} else {
		cobsetptr->cob_time_constant.offset_known = 0;
		cobsetptr->cob_time_constant.utc_offset = 0;
	}
}

/* ACCEPT FROM system-name / DISPLAY UPON system-name  */

/* get date as YYMMDD */
void
cob_accept_date (cob_field *f)
{
	const struct cob_time	time = cob_get_current_datetime (DTR_DATE);
	const cob_u32_t	val = time.day_of_month
		+ time.month * 100
		+ (time.year % 100) * 10000;
	cob_field	field;
	cob_field_attr	attr;
	const unsigned short	digits = 6;

	COB_FIELD_INIT (sizeof (cob_u32_t), (unsigned char *)&val, &attr);
	COB_ATTR_INIT (COB_TYPE_NUMERIC_BINARY, digits, 0, 0, NULL);

	if (COB_FIELD_TYPE (f) != COB_TYPE_GROUP) {
		cob_move (&field, f);
	} else {
		cob_move_to_group_as_alnum (&field, f);
	}
}

/* get date as YYYYMMDD */
void
cob_accept_date_yyyymmdd (cob_field *f)
{
	const struct cob_time	time = cob_get_current_datetime (DTR_DATE);
	const cob_u32_t	val = time.day_of_month
		+ time.month * 100
		+ time.year  * 10000;
	cob_field	field;
	cob_field_attr	attr;
	const unsigned short	digits = 8;

	COB_FIELD_INIT (sizeof (cob_u32_t), (unsigned char *)&val, &attr);
	COB_ATTR_INIT (COB_TYPE_NUMERIC_BINARY, digits, 0, 0, NULL);

	if (COB_FIELD_TYPE (f) != COB_TYPE_GROUP) {
		cob_move (&field, f);
	} else {
		cob_move_to_group_as_alnum (&field, f);
	}
}

/* get day as YYDDD */
void
cob_accept_day (cob_field *f)
{
	const struct cob_time	time = cob_get_current_datetime (DTR_DATE);
	const cob_u32_t	val = time.day_of_year + (time.year % 100) * 1000;
	cob_field	field;
	cob_field_attr	attr;
	const unsigned short	digits = 5;

	COB_FIELD_INIT (sizeof (cob_u32_t), (unsigned char *)&val, &attr);
	COB_ATTR_INIT (COB_TYPE_NUMERIC_BINARY, digits, 0, 0, NULL);

	if (COB_FIELD_TYPE (f) != COB_TYPE_GROUP) {
		cob_move (&field, f);
	} else {
		cob_move_to_group_as_alnum (&field, f);
	}
}

/* get day as YYYYDDD */
void
cob_accept_day_yyyyddd (cob_field *f)
{
	const struct cob_time	time = cob_get_current_datetime (DTR_DATE);
	const cob_u32_t	val = time.day_of_year + time.year * 1000;
	cob_field	field;
	cob_field_attr	attr;
	const unsigned short	digits = 7;

	COB_FIELD_INIT (sizeof (cob_u32_t), (unsigned char *)&val, &attr);
	COB_ATTR_INIT (COB_TYPE_NUMERIC_BINARY, digits, 0, 0, NULL);

	if (COB_FIELD_TYPE (f) != COB_TYPE_GROUP) {
		cob_move (&field, f);
	} else {
		cob_move_to_group_as_alnum (&field, f);
	}
}

/* get day of week as 1 (monday) - 7 (sunday) */
void
cob_accept_day_of_week (cob_field *f)
{
	const struct cob_time		time = cob_get_current_datetime (DTR_DATE);
	const unsigned char		day = (unsigned char)(time.day_of_week + '0');
	const unsigned short		digits = 1;
	cob_move_intermediate (f, &day, digits);
}

/* get time as HHMMSS[ss] */
void
cob_accept_time (cob_field *f)
{
	const struct cob_time	time = f->size > 6
		? cob_get_current_datetime (DTR_FULL)
		: cob_get_current_datetime (DTR_TIME_NO_NANO);
	const cob_u32_t	val = (time.nanosecond / 10000000)
		+ time.second * 100
		+ time.minute * 10000
		+ time.hour   * 1000000;
	cob_field	field;
	cob_field_attr	attr;
	const unsigned short	digits = 8;

	COB_FIELD_INIT (sizeof (cob_u32_t), (unsigned char *)&val, &attr);
	COB_ATTR_INIT (COB_TYPE_NUMERIC_BINARY, digits, 0, 0, NULL);

	if (COB_FIELD_TYPE (f) != COB_TYPE_GROUP) {
		cob_move (&field, f);
	} else {
		cob_move_to_group_as_alnum (&field, f);
	}
}

/* get time as HHMMSSssssss */
void
cob_accept_microsecond_time (cob_field *f)
{
	const struct cob_time	time = cob_get_current_datetime (DTR_FULL);
	const cob_u64_t	val = (cob_u64_t)(time.nanosecond / 1000)
		+ (cob_u64_t)time.second * 1000000
		+ (cob_u64_t)time.minute * 100000000
		+ (cob_u64_t)time.hour   * 10000000000;
	cob_field	field;
	cob_field_attr	attr;
	const unsigned short	digits = 12;

	COB_FIELD_INIT (sizeof (cob_u64_t), (unsigned char *)&val, &attr);
	COB_ATTR_INIT (COB_TYPE_NUMERIC_BINARY, digits, 0, 0, NULL);

	if (COB_FIELD_TYPE (f) != COB_TYPE_GROUP) {
		cob_move (&field, f);
	} else {
		cob_move_to_group_as_alnum (&field, f);
	}
}

void
cob_display_command_line (cob_field *f)
{
	if (commlnptr) {
		cob_free (commlnptr);
	}
	commlnptr = cob_malloc (f->size + 1U);
	commlncnt = f->size;
	memcpy (commlnptr, f->data, commlncnt);
}

void
cob_accept_command_line (cob_field *f)
{
	char	*buff;
	size_t	i;
	size_t	size;
	size_t	len;

	if (commlncnt) {
		cob_move_intermediate (f, commlnptr, commlncnt);
		return;
	}

	if (cob_argc <= 1) {
		cob_move_intermediate (f, " ", (size_t)1);
		return;
	}

	size = 0;
	for (i = 1; i < (size_t)cob_argc; ++i) {
		size += (strlen (cob_argv[i]) + 1);
		if (size > f->size) {
			break;
		}
	}
	buff = cob_malloc (size);
	buff[0] = ' ';
	size = 0;
	for (i = 1; i < (size_t)cob_argc; ++i) {
		len = strlen (cob_argv[i]);
		memcpy (buff + size, cob_argv[i], len);
		size += len;
		if (i != (size_t)cob_argc - 1U) {
			buff[size++] = ' ';
		}
		if (size > f->size) {
			break;
		}
	}
	cob_move_intermediate (f, buff, size);
	cob_free (buff);
}

/* Argument number */

void
cob_display_arg_number (cob_field *f)
{
	int		n;
	cob_field	field;
	cob_field_attr	attr;
	const unsigned short	digits = 9;

	COB_FIELD_INIT (4, (unsigned char *)&n, &attr);
	COB_ATTR_INIT (COB_TYPE_NUMERIC_BINARY, digits, 0, 0, NULL);
	cob_move (f, &field);
	if (n < 0 || n >= cob_argc) {
		cob_set_exception (COB_EC_IMP_DISPLAY);
		return;
	}
	current_arg = n;
}

void
cob_accept_arg_number (cob_field *f)
{
	const cob_u32_t		n = cob_argc - 1;
	cob_field	field;
	cob_field_attr	attr;
	const unsigned short	digits = 9;

	COB_FIELD_INIT (sizeof (cob_u32_t), (unsigned char *)&n, &attr);
	COB_ATTR_INIT (COB_TYPE_NUMERIC_BINARY, digits, 0, 0, NULL);
	cob_move (&field, f);
}

void
cob_accept_arg_value (cob_field *f)
{
	if (current_arg >= cob_argc) {
		cob_set_exception (COB_EC_IMP_ACCEPT);
		return;
	}
	cob_move_intermediate (f, cob_argv[current_arg],
		    strlen (cob_argv[current_arg]));
	current_arg++;
}

/* Environment variable handling */

#ifdef	_MSC_VER
/* _MSC does *NOT* have `setenv` (!)
   But as the handling of the fallback `putenv` is different in POSIX and _MSC
   (POSIX stores no duplicate of `putenv`, where _MSC does), we pretend to
   have support for `setenv` and define it here with the same behaviour: */

static COB_INLINE COB_A_INLINE int
setenv (const char *name, const char *value, int overwrite) {
	/* remark: _putenv_s does always overwrite, add a check for overwrite = 1 if necessary later */
	COB_UNUSED (overwrite);
	return _putenv_s (name, value);
}
static COB_INLINE COB_A_INLINE int
unsetenv (const char *name) {
	return _putenv_s (name, "");
}
#endif

/* set entry into environment, with/without overwriting existing values */
int
cob_setenv (const char *name, const char *value, int overwrite) {
#if defined (HAVE_SETENV) && HAVE_SETENV
	return setenvOW (name, value, overwrite);
#else
	char	*env;
	size_t	len;

	COB_UNUSED (overwrite);
	len = strlen (name) + strlen (value) + 2U;
	env = cob_fast_malloc (len);
	sprintf (env, "%s=%s", name, value);
	return putenv (env);
#endif
}

/* remove entry from environment */
int
cob_unsetenv (const char *name) {
#if defined(HAVE_SETENV) && HAVE_SETENV
	return unsetenv (name);
#else
	char	*env;

	env = cob_fast_malloc (strlen (name) + 2U);
	sprintf (env, "%s=", name);
	return putenv (env);
#endif
}

/* resolve entry from environment */
char *
cob_getenv_direct (const char *name) {
	return getenv (name);
}

/* resolve entry from environment and return an allocated string copy
   --> call cob_free after use! */
char *
cob_getenv (const char *name)
{

	if (name) {
		char	*p = getenv (name);
		if (p) {
			return cob_strdup (p);
		}
	}
	return NULL;
}

int
cob_putenv (char *name)
{
	int	ret;

	if (name && strchr (name, '=')) {
		ret = putenv (cob_strdup (name));
		if (!ret) {
			cob_rescan_env_vals ();
		}
		return ret;
	}
	return -1;
}

void
cob_display_environment (const cob_field *f)
{
	int	ret;

	if (cob_local_env_size < f->size) {
		cob_local_env_size = f->size;
		if (cob_local_env) {
			cob_free (cob_local_env);
		}
		cob_local_env = cob_malloc (cob_local_env_size + 1U);
	}
	ret = cob_field_to_string (f, cob_local_env, cob_local_env_size, CCM_NONE);
	if (ret < 0) {
		return;
	}
	if (unlikely (cobsetptr->cob_env_mangle)) {
		const size_t len = ret;
		size_t i;
		for (i = 0; i < len; ++i) {
			if (!isalnum ((int)cob_local_env[i])) {
				cob_local_env[i] = '_';
			}
		}
	}
}

/* DISPLAY ... UPON ENVIRONMENT VALUE */
void
cob_display_env_value (const cob_field *f)
{
	int		ret;

	if (!cob_local_env
	 || !cob_local_env[0]) {
		cob_set_exception (COB_EC_IMP_DISPLAY);
		return;
	}
	{
		char	buff[COB_MEDIUM_BUFF];
		int 	flen = cob_field_to_string (f, buff,
					COB_MEDIUM_MAX, CCM_NONE);
		if (flen < 0) {
			cob_set_exception (COB_EC_IMP_DISPLAY);
			return;
		}
		ret = cob_setenv (cob_local_env, buff, 1);
	}
	if (ret != 0) {
		cob_set_exception (COB_EC_IMP_DISPLAY);
		return;
	}
	/* Rescan term/screen variables */
	cob_rescan_env_vals ();
}

void
cob_set_environment (const cob_field *f1, const cob_field *f2)
{
	cob_display_environment (f1);
	cob_display_env_value (f2);
}

void
cob_get_environment (const cob_field *envname, cob_field *envval)
{
	char	buff[COB_MEDIUM_BUFF];
	char	*p;
	int 	flen;

	if (envname->size == 0 || envval->size == 0) {
		cob_set_exception (COB_EC_IMP_ACCEPT);
		return;
	}

	flen = cob_field_to_string (envname, buff,
				COB_MEDIUM_MAX, CCM_NONE);
	if (flen < 1) {
		cob_set_exception (COB_EC_IMP_ACCEPT);
		return;
	}

	if (unlikely (cobsetptr->cob_env_mangle)) {
		const char *p_end = buff + flen;
		for (p = buff; p < p_end; ++p) {
			if (!isalnum ((int)*p)) {
				*p = '_';
			}
		}
	}
	p = getenv (buff);
	if (p) {
		cob_move_intermediate (envval, p, strlen (p));
	} else {
		cob_set_exception (COB_EC_IMP_ACCEPT);
		cob_move_intermediate (envval, " ", 1);
	}
}

void
cob_accept_environment (cob_field *f)
{
	const char *p = NULL;

	if (cob_local_env) {
		p = getenv (cob_local_env);
	}
	if (p) {
		cob_move_intermediate (f, p, strlen (p));
	} else {
		cob_set_exception (COB_EC_IMP_ACCEPT);
		cob_move_intermediate (f, " ", 1);
	}
}

void
cob_chain_setup (void *data, const size_t parm, const size_t size)
{
	/* only set if given on command-line, otherwise use normal
	   program internal initialization */
	if (parm <= (size_t)cob_argc - 1) {
		const size_t	len = strlen (cob_argv[parm]);
		memset (data, ' ', size);
		if (len <= size) {
			memcpy (data, cob_argv[parm], len);
		} else {
			memcpy (data, cob_argv[parm], size);
		}
	}
}

void
cob_continue_after (cob_field *decimal_seconds)
{
	cob_s64_t	nanoseconds = get_sleep_nanoseconds_from_seconds (decimal_seconds);

	if (nanoseconds < 0) {
		cob_set_exception (COB_EC_CONTINUE_LESS_THAN_ZERO);
		return;
	}
	internal_nanosleep (nanoseconds);
}

/* ALLOCATE statement
   dataptr    -> used for ALLOCATE identifier only, NULL otherwise
   retptr     -> RETURNING ret, may be NULL
   initialize -> used for ALLOCATE CHARACTERS only, may be NULL
*/
void
cob_allocate (unsigned char **dataptr, cob_field *retptr,
	      cob_field *sizefld, cob_field *initialize)
{
	const cob_s64_t		fsize = cob_get_llint (sizefld);
	void			*mptr = NULL;

	cobglobptr->cob_exception_code = 0;
	if (fsize > COB_MAX_ALLOC_SIZE) {
		cob_set_exception (COB_EC_STORAGE_IMP);
	} else if (fsize > 0) {
		const size_t memsize = (size_t)fsize;
		if (initialize
		 && initialize->data[0] == 0
		 && COB_FIELD_TYPE (initialize) == COB_TYPE_ALPHANUMERIC_ALL) {
			mptr = calloc (1, memsize, &libcobPage);
		} else {
			mptr = malloc (memsize, &libcobPage);
		}
		if (!mptr) {
			cob_set_exception (COB_EC_STORAGE_NOT_AVAIL);
		} else {
			struct cob_alloc_cache	*cache_ptr;
			if (initialize) {
				cob_field	temp;
				temp.size = memsize;
				temp.data = mptr;
				temp.attr = &const_alpha_attr;
				cob_move (initialize, &temp);
			}
#if 0
			else {
				memset (mptr, 0, memsize);
			}
#endif
			cache_ptr = cob_malloc (sizeof (struct cob_alloc_cache));
			cache_ptr->cob_pointer = mptr;
			cache_ptr->size = memsize;
			cache_ptr->next = cob_alloc_base;
			cob_alloc_base = cache_ptr;
		}
	}
	if (dataptr) {
		*dataptr = mptr;
	}
	if (retptr) {
		*(void **)(retptr->data) = mptr;
	}
}

/* FREE statement */
void
cob_free_alloc (unsigned char **ptr1, unsigned char *ptr2)
{
	struct cob_alloc_cache	*cache_ptr = cob_alloc_base;
	struct cob_alloc_cache	*prev_ptr = cob_alloc_base;

	cobglobptr->cob_exception_code = 0;
	if (ptr1 && *ptr1) {
		void	*vptr1;
		vptr1 = *ptr1;
		for (; cache_ptr; cache_ptr = cache_ptr->next) {
			if (vptr1 == cache_ptr->cob_pointer) {
				cob_free (cache_ptr->cob_pointer);
				if (cache_ptr == cob_alloc_base) {
					cob_alloc_base = cache_ptr->next;
				} else {
					prev_ptr->next = cache_ptr->next;
				}
				cob_free (cache_ptr);
				*ptr1 = NULL;
				return;
			}
			prev_ptr = cache_ptr;
		}
		cob_set_exception (COB_EC_STORAGE_NOT_ALLOC);
		return;
	}
	if (ptr2 && *(void **)ptr2) {
		for (; cache_ptr; cache_ptr = cache_ptr->next) {
			if (*(void **)ptr2 == cache_ptr->cob_pointer) {
				cob_free (cache_ptr->cob_pointer);
				if (cache_ptr == cob_alloc_base) {
					cob_alloc_base = cache_ptr->next;
				} else {
					prev_ptr->next = cache_ptr->next;
				}
				cob_free (cache_ptr);
				*(void **)ptr2 = NULL;
				return;
			}
			prev_ptr = cache_ptr;
		}
		cob_set_exception (COB_EC_STORAGE_NOT_ALLOC);
		return;
	}
}

#if 0 /* debug only */
void print_stat (const char *filename, struct stat sb)
{
	printf("File name:                ");
	if (filename) {
		printf ("%s\n", filename);
	} else {
		printf("- unknown -\n");
	}
	printf("File type:                ");

	switch (sb.st_mode & S_IFMT) {
#ifdef S_IFBLK
	case S_IFBLK:  printf("block device\n");            break;
#endif
#ifdef S_IFCHR
	case S_IFCHR:  printf("character device\n");        break;
#endif
	case S_IFDIR:  printf("directory\n");               break;
#ifdef S_IFIFO
	case S_IFIFO:  printf("FIFO/pipe\n");               break;
#endif
#ifdef S_IFLNK
	case S_IFLNK:  printf("symlink\n");                 break;
#endif
	case S_IFREG:  printf("regular file\n");            break;
#ifdef S_IFSOCK
	case S_IFSOCK: printf("socket\n");                  break;
#endif
	default:       printf("unknown?\n");                break;
	}

	printf("I-node number:            %ld\n", (long)sb.st_ino);

	printf("Mode:                     %lo (octal)\n",
		(unsigned long)sb.st_mode);

	printf("Link count:               %ld\n", (long)sb.st_nlink);
	printf("Ownership:                UID=%ld   GID=%ld\n",
		(long)sb.st_uid, (long)sb.st_gid);
	printf("File size:                %lld bytes\n",
		(long long)sb.st_size);
#if 0
	printf("Preferred I/O block size: %ld bytes\n",
		(long)sb.st_blksize);
	printf("Blocks allocated:         %lld\n",
		(long long)sb.st_blocks);
#endif

	printf("Last status change:       %s", ctime(&sb.st_ctime));
	printf("Last file access:         %s", ctime(&sb.st_atime));
	printf("Last file modification:   %s", ctime(&sb.st_mtime));
}
#endif

static COB_INLINE int
check_valid_dir (const char *dir)
{
	struct stat		sb;
	if (strlen (dir) > COB_NORMAL_MAX) return 1;
	if (stat (dir, &sb) || !(S_ISDIR (sb.st_mode))) return 1;

#if 0
	print_stat (dir, sb);
#endif
	
	return 0;
}

static const char *
check_valid_env_tmpdir (const char *envname)
{
	const char *dir;

	dir = getenv (envname);
	if (!dir || !dir[0]) {
		return NULL;
	}
	if (check_valid_dir (dir)) {
		cob_runtime_warning ("Temporary directory %s is invalid, adjust TMPDIR!", envname);
		(void)cob_unsetenv (envname);
		return NULL;
	}
	return dir;
}


/* return pointer to TMPDIR without trailing slash */
static const char *
cob_gettmpdir (void)
{
	static const char	*tmpdir = NULL;
	char	*tmp;

	if (tmpdir != NULL) {
		return tmpdir;
	}

	tmp = NULL;
	/* target directory, also used by most called external tools*/
	if ((tmpdir = check_valid_env_tmpdir ("TMPDIR")) == NULL
	/* OS specific temporary paths */
#ifdef	_WIN32
	 && (tmpdir = check_valid_env_tmpdir ("TEMP")) == NULL
	 && (tmpdir = check_valid_env_tmpdir ("TMP")) == NULL
	 && (tmpdir = check_valid_env_tmpdir ("USERPROFILE")) == NULL) {
#else
	 && (tmpdir = check_valid_env_tmpdir ("TMP")) == NULL
	 && (tmpdir = check_valid_env_tmpdir ("TEMP")) == NULL) {
		if (!check_valid_dir ("/tmp")) {
			tmp = cob_fast_malloc (5U);
			strcpy (tmp, "/tmp");
			tmpdir = tmp;
		}
#endif
	}
	/* fallback if still not valid: current directory */
	if (!tmpdir) {
		tmp = cob_fast_malloc (2U);
		tmp[0] = '.';
		tmp[1] = 0;
		tmpdir = tmp;
	} else {
		/* if we have a valid path: ensure there's no trailing slash */
		size_t size = strlen (tmpdir) - 1;
		if (tmpdir[size] == SLASH_CHAR) {
			tmp = (char*)cob_fast_malloc (size + 1);
			memcpy (tmp, tmpdir, size);
			tmp[size] = 0;
			tmpdir = tmp;
		}
	}
	/* ensure TMPDIR is set for called tools (which partially break hard otherwise) */
	(void)cob_setenv ("TMPDIR", tmpdir, 1);

	if (tmp) {
		cob_free ((void *)tmp);
	}

	/* get the pointer to the environment copy - as this may point to a different place -
	   store it for subsequent calls and finally return it to the caller */
	tmpdir = getenv ("TMPDIR");
	return tmpdir;
}

/* Set temporary file name */
void
cob_temp_name (char *filename, const char *ext)
{
	int pid = cob_sys_getpid ();
#ifndef HAVE_8DOT3_FILENAMES
#define TEMP_EXT_SCHEMA	"%s%ccob%d_%d%s"
#define TEMP_SORT_SCHEMA	"%s%ccobsort%d_%d"
#else
/* 8.3 allows only short names... */
#define TEMP_EXT_SCHEMA	"%s%cc%d_%d%s"
#define TEMP_SORT_SCHEMA	"%s%cs%d_%d"
	pid = pid % 9999;
#endif
	if (ext) {
		snprintf (filename, (size_t)COB_FILE_MAX, TEMP_EXT_SCHEMA,
			cob_gettmpdir (), SLASH_CHAR, pid, cob_temp_iteration, ext);
	} else {
		snprintf (filename, (size_t)COB_FILE_MAX, TEMP_SORT_SCHEMA,
			cob_gettmpdir (), SLASH_CHAR, pid, cob_temp_iteration);
	}
#undef TEMP_EXT_SCHEMA
#undef TEMP_SORT_SCHEMA
}

void
cob_incr_temp_iteration (void)
{
	cob_temp_iteration++;
}

int
cob_extern_init (void)
{
	/* can be called multiple times (MF docs say: should be done in all threads) */
	if (!cob_initialized) {
		cob_init (0, NULL);
	}
	return 0;
}

char *
cob_command_line (int flags, int *pargc, char ***pargv,
		  char ***penvp, char **pname)
{
#if	0	/* RXWRXW cob_command_line */
	char		**spenvp;
	char		*spname;
#else
	COB_UNUSED (penvp);
	COB_UNUSED (pname);
#endif

	COB_UNUSED (flags);

	if (!cob_initialized) {
		cob_fatal_error (COB_FERROR_INITIALIZED);
	}
	if (pargc && pargv) {
		cob_argc = *pargc;
		cob_argv = *pargv;
	}

#if	0	/* RXWRXW cob_command_line */
	if (penvp) {
		spenvp = *penvp;
	}
	if (pname) {
		spname = *pname;
	}
#endif

	/* What are we supposed to return here? */
	return NULL;
}

int
cob_tidy (void)
{
	if (!cob_initialized) {
		exit_code = -1;
		return 1;
	}
	exit_code = 0;
	call_exit_handlers_and_terminate ();
	return 0;
}

/* System routines */

/* CBL_EXIT_PROC - register exit handlers that will be called
   before teardown (after posible error procedures) without
   any parameters passed
   'dispo': intallation flag (add/remove/priority)
   'pptr':  function / ENTRY point to be called */
int
cob_sys_exit_proc (const void *dispo, const void *pptr)
{
	struct exit_handlerlist *hp, *h;
	unsigned char	install_flag;
	/* only initialized to silence -Wmaybe-uninitialized */
	unsigned char	priority = 0;
	int			(**p)(void);

	COB_CHK_PARMS (CBL_EXIT_PROC, 2);

#if 0	/* TODO: take care of ACU variant:
	   pptr is not an already resolved entry point
	   but a name which is to be cob_resolve'd (at use-time);
	   maybe resolve here and return -1 if not possible;
	   furthermore the second parameter is a mixed priority + install_flag */
	if (something) {
		const char *name = (char *)pptr;
		pptr = cob_resolve_cobol (name, 0, 0);

		if (!p) {
			return -1;
		}

		priority = *(unsigned char *)dispo;
		if (priority == 254) {
			install_flag = 1;
		} else if (priority == 255) {
			install_flag = 2;
		} else {
			install_flag = 3;
		}

	} else {
#endif
		memcpy (&p, &pptr, sizeof (void *));

		if (!p || !*p) {
			return -1;
		}

		install_flag = *(unsigned char*)dispo;
		switch (install_flag) {
		case 0:
			priority = 64;
			break;
		case 1:
		case 2:
			/* remove / query - no need to check priority */
			break;
		case 3:
			/* set with explicit priority */
			priority = *((unsigned char*)pptr + sizeof (void*));
			if (priority > 127) {
				priority = 64;
			}
			break;
		default:
			return -1;
		}
#if 0
	}
#endif

	hp = NULL;
	h = exit_hdlrs;
	/* Search handler, remove if not function 2  */
	while (h != NULL) {
		if (h->proc == *p) {
			switch (install_flag) {
			case 2:
			/* Return priority of installed handler */
#if 0	/* TODO: take care of ACU variant: priority in return */
				if (something) {
					return priority;
				}
#endif
				*((unsigned char*)pptr + sizeof (void*)) = h->priority;
				return 0;
			case 0:
			case 3:
				if (priority == h->priority) {
					return -1;
				}
				break;
			default:
				break;
			}
			if (hp != NULL) {
				hp->next = h->next;
			} else {
				exit_hdlrs = h->next;
			}
			cob_free (h);
			/* Remove handler --> done */
			if (install_flag == 1) {
				return 0;
			}
			break;
		}
		hp = h;
		h = h->next;
	}
	if (install_flag == 1
	 || install_flag == 2) {
		/* deete or query priority with not available */
#if 0	/* TODO: take care of ACU variant: priority 255 = not available */
		if (something) {
			return 255;
		}
#endif
		return -1;
	}
	h = cob_malloc (sizeof (struct exit_handlerlist));
	h->next = exit_hdlrs;
	h->proc = *p;
	h->priority = priority;
	exit_hdlrs = h;
	return 0;
}

/* CBL_ERROR_PROC - register error handlers that will be called
   on runtime errors and may early-stop, those are called with a single
   parameter containing the error message
   'dispo': intallation flag (add/remove/priority)
   'pptr':  function / ENTRY point to be called */
int
cob_sys_error_proc (const void *dispo, const void *pptr)
{
	struct handlerlist	*hp, *h;
	const unsigned char	*x;
	int			(**p) (char *s);

	COB_CHK_PARMS (CBL_ERROR_PROC, 2);

#if 0	/* TODO: take care of ACU variant:
	   pptr is not an already resolved entry point
	   but a name which is to be cob_resolve'd (at use-time);
	   maybe resolve here and return -1 if not possible */
	if (something) {
		const char *name = (char *)pptr;
		pptr = cob_resolve_cobol (name, 0, 0);
		if (!p) {
			return -1;
		}
	} else {
#endif
		memcpy (&p, &pptr, sizeof (void *));
		if (!p || !*p) {
			return -1;
		}
#if 0
}
#endif

	hp = NULL;
	h = hdlrs;
	/* Search for existing handler */
	while (h != NULL) {
		if (h->proc == *p) {
			break;
		}
		hp = h;
		h = h->next;
	}
	x = dispo;
	if (*x != 0) {
		/* Remove handler */
		if (h != NULL) {
			if (hp != NULL) {
				hp->next = h->next;
			} else {
				hdlrs = h->next;
			}
			cob_free (h);
		}
	} else {
		if (h == NULL) {
			/* insert handler */
			h = cob_malloc (sizeof (struct handlerlist));
			h->next = hdlrs;
			h->proc = *p;
			hdlrs = h;
		} else {
#if 0	/* TODO: take care of ACU variant: placing it first */
			if (something) {
				if (hp != NULL) {
					hp->next = h->next;
				}
				h->next = hdlrs;
				hdlrs = h;
			} else {
				/* MF-Variant: when already existing: do nothing */
				return 0;
			}
#else
			/* MF-Variant: when already existing: do nothing */
			return 0;
#endif
		}
	}
	return 0;
}

int cob_sys_runtime_error_proc (const void *err_flags, const void *err_msg)
{
	const char *msg = (const char*)err_msg;

	COB_CHK_PARMS (CBL_RUNTIME_ERROR, 2);
	COB_UNUSED (err_flags);

	if (msg && msg[0]) {
		cob_runtime_error ("%s: %s",
			_("Program abandoned at user request"), msg);
	} else {
		cob_runtime_error ("%s",
			_("Program abandoned at user request"));
	}
	cob_hard_failure ();
	return 0;
}

int
cob_sys_system (const void *cmdline)
{
	COB_CHK_PARMS (SYSTEM, 1);

	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		const char *cmd = cmdline;
		size_t		i = COB_MODULE_PTR->cob_procedure_params[0]->size;
		/* FIXME: if caller wasn't COBOL then size must be evaluated by strlen */
		i--;
		do {
			if (cmd[i] != ' ' && cmd[i] != 0) {
				break;
			}
		} while (--i != 0);
		if (i > 0) {
			char	*command;
			/* LCOV_EXCL_START */
			if (unlikely (i > COB_MEDIUM_MAX)) {
				cob_runtime_warning (_("parameter to SYSTEM call is larger than %d characters"),
					COB_MEDIUM_MAX);
				return 1;
			}
			/* LCOV_EXCL_STOP */
#ifdef _WIN32
			/* All known _WIN32 implementations use MSVCRT's system()
			   which passes the given commandline as parameter to "cmd /k".
			   Because "of compatibility" this checks if you have a
			   leading and trailing " and if yes simply removes them (!).
			   Check if this is the case and if it is handled already
			   by an *extra* pair of quotes, otherwise add these...
			   This fixes CALL 'SYSTEM' USING '"someprog" "opt"' being
			   executed as 'someprog" "opt'.
			*/
			if (i > 2 && cmd[0] == '"' && cmd[i] == '"'
			&& (cmd[1] != '"' || cmd[i - 1] != '"')) {
				command = cob_malloc (i + 4);
				command[0] = '"';
				memcpy (command + 1, cmd, i + 1);
				command[i + 1] = '"';
			} else {
#endif /* _WIN32 */
				command = cob_malloc (i + 2);
				memcpy (command, cmd, i + 1);
#ifdef _WIN32
			}
#endif
			{
				int status;
				if (cobglobptr->cob_screen_initialized) {
					cob_screen_set_mode (0);
				}
				/* note: if the command cannot be executed _WIN32 always returns 1
				   while GNU/Linux returns -1 */
				status = system (command);
				if (cobglobptr->cob_screen_initialized) {
					cob_screen_set_mode (1U);
				}
#ifdef	WIFSIGNALED
				if (WIFSIGNALED (status)) {
					int sig = WTERMSIG (status);
					const char *signal_name = cob_get_sig_name (sig);
					cob_runtime_warning (_("external process \"%s\" ended with signal %s (%d)"),
						command, signal_name, sig);
				}
#endif
				cob_free (command);
#if 0	/* possibly do this, but only if explicit asked for via a new runtime configuration
		   as at least MicroFocus always returns all bytes here;
		   from its docs it _looks_ like ACU only returns the lower bytes ... */
#ifdef WEXITSTATUS
				if (WIFEXITED (status)) {
					status = WEXITSTATUS (status);
				}
#endif
#endif
				return status;
			}
		}
	}
	return 1;
}

/**
* Return some hosted C variables, argc, argv, stdin, stdout, stderr.
*/
int cob_sys_hosted (void *p, const void *var)
{
	const char		*name = var;
	cob_u8_ptr		data = p;
	size_t			i;

	COB_CHK_PARMS (CBL_GC_HOSTED, 2);

	if (!data) {
		return 1;
	}

	if (COB_MODULE_PTR->cob_procedure_params[1]) {
		i = (int)COB_MODULE_PTR->cob_procedure_params[1]->size;
		if ((i == 4) && !memcmp (name, "argc", 4)) {
			*((int *)data) = cob_argc;
			return 0;
		}
		if ((i == 4) && !memcmp (name, "argv", 4)) {
			*((char ***)data) = cob_argv;
			return 0;
		}
		if ((i == 5) && !memcmp (name, "stdin", 5)) {
			*((fd_t *)data) = stdin;
			return 0;
		}
		if ((i == 6) && !memcmp (name, "stdout", 6)) {
			*((fd_t *)data) = stdout;
			return 0;
		}
		if ((i == 6) && !memcmp (name, "stderr", 6)) {
			*((fd_t *)data) = stderr;
			return 0;
		}
		if ((i == 5) && !memcmp (name, "errno", 5)) {
			*((int **)data) = &errno;
			return 0;
		}
#if defined (HAVE_TIMEZONE)
		if ((i == 6) && !memcmp (name, "tzname", 6)) {
			/* Recheck: bcc raises "suspicious pointer conversion */
			*((char ***)data) = tzname;
			return 0;
		}
		if ((i == 8) && !memcmp (name, "timezone", 8)) {
			*((long *)data) = timezone;
			return 0;
		}
		if ((i == 8) && !memcmp (name, "daylight", 8)) {
			*((int *)data) = daylight;
			return 0;
		}
#endif /* HAVE_TIMEZONE */
	}
	return 1;
}

int
cob_sys_and (const void *p1, void *p2, const int length)
{
	const cob_u8_ptr	data_1 = p1;
	cob_u8_ptr		data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_AND, 3);

	if (length <= 0) {
		return 0;
	}
	for (n = 0; n < (size_t)length; ++n) {
		data_2[n] &= data_1[n];
	}
	return 0;
}

int
cob_sys_or (const void *p1, void *p2, const int length)
{
	const cob_u8_ptr	data_1 = p1;
	cob_u8_ptr		data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_OR, 3);

	if (length <= 0) {
		return 0;
	}
	for (n = 0; n < (size_t)length; ++n) {
		data_2[n] |= data_1[n];
	}
	return 0;
}

int
cob_sys_nor (const void *p1, void *p2, const int length)
{
	const cob_u8_ptr	data_1 = p1;
	cob_u8_ptr		data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_NOR, 3);

	if (length <= 0) {
		return 0;
	}
	for (n = 0; n < (size_t)length; ++n) {
		data_2[n] = ~(data_1[n] | data_2[n]);
	}
	return 0;
}

int
cob_sys_xor (const void *p1, void *p2, const int length)
{
	const cob_u8_ptr	data_1 = p1;
	cob_u8_ptr		data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_XOR, 3);

	if (length <= 0) {
		return 0;
	}
	for (n = 0; n < (size_t)length; ++n) {
		data_2[n] ^= data_1[n];
	}
	return 0;
}

/* COBOL routine to perform for logical IMPLIES between the bits in two fields,
   storing the result in the second field */
int
cob_sys_imp (const void *p1, void *p2, const int length)
{
	const cob_u8_ptr	data_1 = p1;
	cob_u8_ptr		data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_IMP, 3);

	if (length <= 0) {
		return 0;
	}
	for (n = 0; n < (size_t)length; ++n) {
		data_2[n] = (~data_1[n]) | data_2[n];
	}
	return 0;
}


/* COBOL routine to perform for logical NOT IMPLIES between the bits in two fields,
   storing the result in the second field */
int
cob_sys_nimp (const void *p1, void *p2, const int length)
{
	const cob_u8_ptr	data_1 = p1;
	cob_u8_ptr		data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_NIMP, 3);

	if (length <= 0) {
		return 0;
	}
	for (n = 0; n < (size_t)length; ++n) {
		data_2[n] = data_1[n] & (~data_2[n]);
	}
	return 0;
}

/* COBOL routine to check for logical EQUIVALENCE between the bits in two fields,
   storing the result in the second field */
int
cob_sys_eq (const void *p1, void *p2, const int length)
{
	const cob_u8_ptr	data_1 = p1;
	cob_u8_ptr		data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_EQ, 3);

	if (length <= 0) {
		return 0;
	}
	for (n = 0; n < (size_t)length; ++n) {
		data_2[n] = ~(data_1[n] ^ data_2[n]);
	}
	return 0;
}

/* COBOL routine to perform a logical NOT on the bits of a field */
int
cob_sys_not (void *p1, const int length)
{
	cob_u8_ptr	data_1 = p1;
	size_t		n;

	COB_CHK_PARMS (CBL_NOT, 2);

	if (length <= 0) {
		return 0;
	}
	for (n = 0; n < (size_t)length; ++n) {
		data_1[n] = ~data_1[n];
	}
	return 0;
}

/* COBOL routine to pack the least significant bits in eight bytes into a single byte */
int
cob_sys_xf4 (void *p1, const void *p2)
{
	cob_u8_ptr		data_1 = p1;
	const cob_u8_ptr	data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_XF4, 2);

	*data_1 = 0;
	for (n = 0; n < 8; ++n) {
		*data_1 |= (data_2[n] & 1) << (7 - n);
	}
	return 0;
}

/* COBOL routine to unpack the bits in a byte into eight bytes */
int
cob_sys_xf5 (const void *p1, void *p2)
{
	const cob_u8_ptr	data_1 = p1;
	cob_u8_ptr		data_2 = p2;
	size_t			n;

	COB_CHK_PARMS (CBL_XF5, 2);

	for (n = 0; n < 8; ++n) {
		data_2[n] = (*data_1 & (1 << (7 - n))) ? 1 : 0;
	}
	return 0;
}

/* COBOL (only) routine for different functions, including functions for
   the programmable COBOL SWITCHES:
   11: set  COBOL switches 0-7 and debug switch
   12: read COBOL switches 0-7 and debug switch
   16: return number of CALL USING parameters
*/
int
cob_sys_x91 (void *p1, const void *p2, void *p3)
{
	cob_u8_ptr		result = p1;
	const cob_u8_ptr	func = p2;
	cob_u8_ptr		parm = p3;
	unsigned char		*p;
	size_t			i;

	switch (*func) {

	/* Set switches (0-7) + DEBUG module */
	case 11:
		p = parm;
		for (i = 0; i < 8; ++i, ++p) {
			if (*p == 0) {
				cob_switch[i] = 0;
			} else if (*p == 1) {
				cob_switch[i] = 1;
			}
		}
		/* MF additionally sets the ANSI DEBUG module switch */
		if (COB_MODULE_PTR->cob_procedure_params[0]->size >= 9) {
			p++;
			cobsetptr->cob_debugging_mode = (*p == 1);
		}
		*result = 0;
		break;

	/* Get switches (0-7) */
	case 12:
		p = parm;
		for (i = 0; i < 8; ++i, ++p) {
			*p = (unsigned char)cob_switch[i];
		}
		/* MF additionally passes the ANSI DEBUG module switch */
		if (COB_MODULE_PTR->cob_procedure_params[0]->size >= 9) {
			p++;
			*p = (unsigned char)cobsetptr->cob_debugging_mode;
		}
		*result = 0;
		break;

	/* Set switches (A-Z -> 11-36) */
	case 13:
		p = parm;
		for (i = 11; i < 36; ++i, ++p) {
			if (*p == 0) {
				cob_switch[i] = 0;
			} else if (*p == 1) {
				cob_switch[i] = 1;
			}

			if (i == 'D' - 'A' + 11) {
				cobsetptr->cob_debugging_mode = cob_switch[i];
			} else if (i == 'N' - 'A' + 11) {
				cobsetptr->cob_ls_nulls = cob_switch[i];
#if 0	/* TODO add in trunk*/
			} else if (i == 'T' - 'A' + 11) {
				cobsetptr->cob_ls_tabs = cob_switch[i];
#endif
			}
		}
		*result = 0;
		break;

	/* Get switches (A-Z -> 11-36) */
	case 14:
		p = parm;
		for (i = 1; i < 27; ++i, ++p) {
			*p = (unsigned char)cob_switch[i];
		}
		*result = 0;
		break;

#if 0	/* program lookup 
		   may be implemented as soon as some legacy code
		   shows its exact use and a test case */
	case 15:
		p = parm + 1;
		{
			char name[256];
			strncpy (name, p, *parm);
			void * func = cob_resolve (name);
			/* TODO: the full name should be copied back into p */
			return (func != NULL);
		}
		break;
#endif

	/* Return number of call parameters
		according to the docs this is only set for programs CALLed from COBOL
		NOT for main programs in contrast to C$NARG (cob_sys_return_args)
	   MF deprecated it in favor of CBL_GET_PROGRAM_INFO function 8 */
	case 16:
		*parm = (unsigned char)COB_MODULE_PTR->module_num_params;
		*result = 0;
		break;

#if 1	/* EXEC call "like DOS 4B call"
		   working prototype, may be finalized as soon as some legacy code
		   shows its exact use and a test case; CHECKME: what is the return
		   code with MF on UNIX where this is "not supported"? */
	case 35:
		p = parm + 1;
		/* zero = just [re-]execute */
		if (*parm != 0) {
			/* note: we can't check for existence
			   as "pause" and similar inbuilts must also work;
			   CHECKME: possibly start via cmd.exe wrapper ? */
			/* put on command line here */
			{
				cob_field field;
				COB_FIELD_INIT (*parm, p, NULL);
				cob_display_command_line (&field);
			}
		}
		{
			/* execute the command line */
			int ret = system ((const char *)commlnptr);
			*result = (unsigned char)ret;
		}
		break;
#endif


#if 0	/* note: 46-49 should be implemented in 4.x with file-specific settings */
	/* enable/disable LS_NULLs for a specific FD */
	case 46:
	case 47:
	/* enable/disable LS_TABs for a specific FD */
	case 48:
	case 49:
		{
			*result = 0;
			cob_file *f = get_file (p3);
			if (f == NULL
			 || f->open_mode == COB_OPEN_CLOSED
			 || f->open_mode == COB_OPEN_LOCKED) {
				*result = 1;
			} else if (*func == 46) {
				f->ls_nulls = 1;
			} else if (*func == 47) {
				f->ls_nulls = 0;
			} else if (*func == 48) {
				f->ls_tabss = 1;
			} else if (*func == 49) {
				f->ls_tabs = 0;
			}
		}
		break;
#endif

#if 0	/* directory search
		   may be implemented when CBL_DIR_SCAN / C$LISTDIR is added and
		   likely only finalized as soon as some legacy code
		   shows its exact use and a test case
	   MF deprecated it in favor of CBL_DIR_SCAN */
	case 69:
		*result = 1;
		break;
#endif

	/* unimplemented function */
	default:
		*result = 1;
		break;
	}
	return 0;
}

int
cob_sys_toupper (void *p1, const int length)
{
	cob_u8_ptr	data = p1;
	size_t		n;

	COB_CHK_PARMS (CBL_TOUPPER, 2);

	if (length > 0) {
		for (n = 0; n < (size_t)length; ++n) {
			data[n] = (cob_u8_t)toupper (data[n]);
		}
	}
	return 0;
}

int
cob_sys_tolower (void *p1, const int length)
{
	cob_u8_ptr	data = p1;
	size_t		n;

	COB_CHK_PARMS (CBL_TOLOWER, 2);

	if (length > 0) {
		for (n = 0; n < (size_t)length; ++n) {
			data[n] = (cob_u8_t)tolower (data[n]);
		}
	}
	return 0;
}

/* maximum sleep time in seconds, currently 7 days */
#define MAX_SLEEP_TIME 3600*24*7

static cob_s64_t
get_sleep_nanoseconds (cob_field *nano_seconds) {

	cob_s64_t	nanoseconds = cob_get_llint (nano_seconds);

	if (nanoseconds < 0) {
		return -1;
	}
	if (nanoseconds >= ((cob_s64_t)MAX_SLEEP_TIME * 1000000000)) {
		return (cob_s64_t)MAX_SLEEP_TIME * 1000000000;
	} else {;
		return nanoseconds;
	}
}

static cob_s64_t
get_sleep_nanoseconds_from_seconds (cob_field *decimal_seconds) {

#define MAX_SLEEP_TIME 3600*24*7
	cob_s64_t	seconds = cob_get_llint (decimal_seconds);

	if (seconds < 0) {
		return -1;
	}
	if (seconds >= MAX_SLEEP_TIME) {
		return (cob_s64_t)MAX_SLEEP_TIME * 1000000000;
} else {
		cob_s64_t	nanoseconds;
		cob_field	temp;
		temp.size = 8;
		temp.data = (unsigned char *)&nanoseconds;
		temp.attr = &const_bin_nano_attr;
		cob_move (decimal_seconds, &temp);
		return nanoseconds;
	}
}

static void
internal_nanosleep (cob_s64_t nsecs)
{
#ifdef	HAVE_NANO_SLEEP
	struct timespec	tsec;

	tsec.tv_sec = nsecs / 1000000000;
	tsec.tv_nsec = nsecs % 1000000000;
	nanosleep (&tsec, NULL);

#else

	unsigned int	msecs;
#if	defined (_WIN32)
	msecs = (unsigned int)(nsecs / 1000000);
	if (msecs > 0) {
		Sleep (msecs);
	}
#else /* includes "defined (__370__) || defined (__OS400__)" */
	msecs = (unsigned int)(nsecs / 100000000);
	if (msecs % 10 > 4) {
		msecs = (msecs / 10) + 1;
	} else {
		msecs = msecs / 10;
	}
	if (msecs > 0) {
		sleep (msecs);
	}
#endif
#endif
}

/* CBL_GC_NANOSLEEP / CBL_OC_NANOSLEEP, origin: OpenCOBOL */
int
cob_sys_oc_nanosleep (const void *data)
{
	COB_UNUSED (data);
	COB_CHK_PARMS (CBL_GC_NANOSLEEP, 1);

	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		cob_s64_t nsecs
			= get_sleep_nanoseconds (COB_MODULE_PTR->cob_procedure_params[0]);
		if (nsecs > 0) {
			internal_nanosleep (nsecs);
		}
		return 0;
	}
	return -1;
}

/* C$SLEEP, origin: ACUCOBOL */
int
cob_sys_sleep (const void *data)
{
	COB_UNUSED (data);
	COB_CHK_PARMS (C$SLEEP, 1);

	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		cob_s64_t	nanoseconds
			= get_sleep_nanoseconds_from_seconds (COB_MODULE_PTR->cob_procedure_params[0]);
		if (nanoseconds < 0) {
			/* ACUCOBOL specifies a runtime error here... */
			return -1;
		}
		internal_nanosleep (nanoseconds);
		return 0;
	}
	return 0;	/* CHECKME */
}

int cob_sys_getpid (void)
{
    #ifdef HAVE_GETPID
	if (!cob_process_id) {
		cob_process_id = (int)getpid ();
	}
	return cob_process_id;
    #else
    return -1;	/* not supported */
    #endif
}

int cob_sys_fork (void)
{
 /* cygwin does not define _WIN32, but implements [slow] fork() and provides unistd.h
    MSYS defines _WIN32, provides unistd.h and not implements fork()
 */
#if defined (HAVE_FORK) && defined (HAVE_UNISTD_H) && !(defined (_WIN32))
	int	pid;
	if ((pid = fork ()) == 0 ) {
		cob_process_id = 0;	/* reset cached value */
		return 0;		/* child process just returns */
	}
	if (pid < 0) {			/* Some error happened */
		cob_runtime_warning (_("error '%s' during CBL_GC_FORK"), cob_get_strerror ());
		return -2;
	}
	return pid;			/* parent gets process id of child */
#else
	cob_runtime_warning (_("'%s' is not supported on this platform"), "CBL_GC_FORK");
	return -1;
#endif
}


/* wait for a pid to end and return its exit code
   error codes are returned as negative value
*/
int cob_sys_waitpid (const void *p_id)
{
#ifdef	HAVE_SYS_WAIT_H
	int	pid, status, wait_sts;

	COB_UNUSED (p_id);

	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		pid = cob_get_int (COB_MODULE_PTR->cob_procedure_params[0]);
		if (pid == cob_sys_getpid ()) {
			status = 0 - EINVAL;
			return status;
		}
		wait_sts = waitpid (pid, &status, 0);
		if (wait_sts < 0) {			/* Some error happened */
			status = 0 - errno;
			cob_runtime_warning (_("error '%s' for P%d during CBL_GC_WAITPID"),
				cob_get_strerror (), pid);
			return status;
		}
		status = WEXITSTATUS (status);
	} else {
		status = 0 - EINVAL;
	}
	return status;
#elif defined (_WIN32)
	int	pid, status;
	HANDLE process = NULL;
	DWORD ret;

	COB_UNUSED (p_id);

	status = 0;
	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		pid = cob_get_int (COB_MODULE_PTR->cob_procedure_params[0]);
		if (pid == cob_sys_getpid ()) {
			status = 0 - ERROR_INVALID_DATA;
			return status;
		}
		/* get process handle with least necessary rights
		   PROCESS_QUERY_LIMITED_INFORMATION is available since OS-version Vista / Server 2008
		                                     and always leads to ERROR_ACCESS_DENIED on older systems
		   PROCESS_QUERY_INFORMATION         needs more rights
		   SYNCHRONIZE                       necessary for WaitForSingleObject
		*/
#if defined (PROCESS_QUERY_LIMITED_INFORMATION)
		process = OpenProcess (SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
#if !defined (_MSC_VER) || COB_USE_VC2012_OR_GREATER /* only try a higher level if we possibly compile on XP/2003 */
		/* TODO: check what happens on WinXP / 2003 as PROCESS_QUERY_LIMITED_INFORMATION isn't available there */
		if (!process && GetLastError () == ERROR_ACCESS_DENIED) {
			process = OpenProcess (SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, pid);
		}
#endif
#else
		process = OpenProcess (SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, pid);
#endif
		/* if we don't get access to query the process' exit status try to get at least
			access to the process end (needed for WaitForSingleObject)
		*/
		if (!process && GetLastError () == ERROR_ACCESS_DENIED) {
			process = OpenProcess (SYNCHRONIZE, FALSE, pid);
			status = -2;
		}
		if (process) {
			/* wait until process exit */
			ret = WaitForSingleObject (process, INFINITE);
			if (ret == WAIT_FAILED) {
				status = 0 - GetLastError ();
			/* get exit code, if possible */
			} else if (status != -2) {
				if (!GetExitCodeProcess (process, &ret)) {
					status = 0 - GetLastError ();
				} else {
					status = (int) ret;
				}
			}
			CloseHandle (process);
		} else {
			status = 0 - GetLastError ();
		}
	} else {
		status = 0 - ERROR_INVALID_DATA;
	}
	return status;
#else
	COB_UNUSED (p_id);

	cob_runtime_warning (_("'%s' is not supported on this platform"), "CBL_GC_WAITPID");
	return -1;
#endif
}

/* set the number of arguments passed to the current program;
   works both for main programs and called sub programs
   Implemented according to ACUCOBOL-GT -> returns the number of arguments that were passed,
   not like in MF implementation the number of arguments that were received */
int
cob_sys_return_args (void *data)
{
	COB_UNUSED (data);

	COB_CHK_PARMS (C$NARG, 1);

	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		cob_set_int (COB_MODULE_PTR->cob_procedure_params[0],
			COB_MODULE_PTR->module_num_params);
	}
	return 0;
}

int
cob_sys_calledby (void *data)
{
	size_t		size;
	size_t		msize;

	COB_CHK_PARMS (C$CALLEDBY, 1);

	if (!COB_MODULE_PTR->cob_procedure_params[0]) {
		/* TO-DO: check what ACU ccbl/runcbl returns,
		   the documentation doesn't say anything about this */
		return -1;
	}
	size = COB_MODULE_PTR->cob_procedure_params[0]->size;
	memset (data, ' ', size);
	if (!COB_MODULE_PTR->next) {
		return 0;
	}
	msize = strlen (COB_MODULE_PTR->next->module_name);
	if (msize > size) {
		msize = size;
	}
	memcpy (data, COB_MODULE_PTR->next->module_name, msize);
	return 1;
}

int
cob_sys_parameter_size (void *data)
{
	int	n;

	COB_UNUSED (data);

	COB_CHK_PARMS (C$PARAMSIZE, 1);

	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		n = cob_get_int (COB_MODULE_PTR->cob_procedure_params[0]);
		if (n > 0 && n <= COB_MODULE_PTR->module_num_params) {
			n--;
			if (COB_MODULE_PTR->next
			 && COB_MODULE_PTR->next->cob_procedure_params[n]) {
				return (int)COB_MODULE_PTR->next->cob_procedure_params[n]->size;
			}
		}
	}
	return 0;
}

int
cob_sys_getopt_long_long (void *so, void *lo, void *idx, const int long_only, void *return_char, void *opt_val)
{
	/*
	 * cob_argc is a static int containing argc from runtime
	 * cob_argv is a static char ** containing argv from runtime
	 */

	size_t opt_val_size = 0;
	size_t so_size = 0;
	size_t lo_size = 0;

	unsigned int lo_amount;
	int exit_status;

	char *shortoptions;
	char *temp;

	struct option *longoptions, *longoptions_root;
	longoption_def *l = NULL;

	int longind = 0;
	unsigned int i;
	int j;

	int return_value;

	COB_UNUSED (idx);
	COB_UNUSED (lo);
	COB_UNUSED (so);

	COB_CHK_PARMS (CBL_GC_GETOPT, 6);

	/* Read in sizes of some parameters */
	if (COB_MODULE_PTR->cob_procedure_params[1]) {
		lo_size = COB_MODULE_PTR->cob_procedure_params[1]->size;
	}
	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		so_size = COB_MODULE_PTR->cob_procedure_params[0]->size;
	}
	if (COB_MODULE_PTR->cob_procedure_params[5]) {
		opt_val_size = COB_MODULE_PTR->cob_procedure_params[5]->size;
	}

	/* buffering longoptions (COBOL), target format (struct option) */
	if (lo_size % sizeof (longoption_def) == 0) {
		lo_amount = (int)lo_size / sizeof (longoption_def);
		longoptions_root = (struct option*) cob_malloc (sizeof (struct option) * ((size_t)lo_amount + 1U));
	} else {
		cob_runtime_error (_("call to CBL_GC_GETOPT with wrong longoption size"));
		cob_hard_failure ();
	}

	if (!COB_MODULE_PTR->cob_procedure_params[2]) {
		cob_runtime_error (_("call to CBL_GC_GETOPT with missing longind"));
		cob_hard_failure ();
	}
	longind = cob_get_int (COB_MODULE_PTR->cob_procedure_params[2]);

	/* add 0-termination to strings */
	shortoptions = cob_malloc (so_size + 1U);
	if (COB_MODULE_PTR->cob_procedure_params[0]) {
		cob_field_to_string (COB_MODULE_PTR->cob_procedure_params[0],
				shortoptions, so_size, CCM_NONE);
	}

	if (COB_MODULE_PTR->cob_procedure_params[1]) {
		l = (struct __longoption_def*) (COB_MODULE_PTR->cob_procedure_params[1]->data);
	}

	longoptions = longoptions_root;
	for (i = 0; i < lo_amount; i++) {
		j = sizeof (l->name) - 1;
		while (j >= 0 && l->name[j] == ' ') {
			l->name[j] = 0;
			j--;
		}
		longoptions->name = l->name;
		longoptions->has_arg = COB_D2I (l->has_option);
		memcpy (&longoptions->flag, l->return_value_pointer, sizeof (l->return_value_pointer));
		memcpy (&longoptions->val, &l->return_value, 4);

		l = l + 1; /* +1 means pointer + 1*sizeof (longoption_def) */
		longoptions = longoptions + 1;
	}

	/* appending final record, so getopt can spot the end of longoptions */
	longoptions->name = NULL;
	longoptions->has_arg = 0;
	longoptions->flag = NULL;
	longoptions->val = 0;


	l -= lo_amount; /* Set pointer back to begin of longoptions */
	longoptions -= lo_amount;

	return_value = cob_getopt_long_long (cob_argc, cob_argv, shortoptions, longoptions, &longind, long_only);
	temp = (char *) &return_value;

	/* Write data back to COBOL */
#ifdef	WORDS_BIGENDIAN
	if (temp[3] == '?'
	 || temp[3] == ':'
	 || temp[3] == 'W'
	 || temp[3] == 0) {
		exit_status = temp[3] & 0xFF;
	} else if (return_value == -1) {
		exit_status = -1;
	} else {
		exit_status = 3;
	}
	 /* cob_getopt_long_long sometimes returns and 'int' value and sometimes a 'x   ' in the int */
	if (temp[0] == 0
	 && temp[1] == 0
	 && temp[2] == 0) {
		/* Move option value to 1st byte and SPACE fill the 'int' */
		temp[0] = temp[3];
		temp[1] = temp[2] = temp[3] = ' ';
	}
#else
	if (temp[0] == '?'
	 || temp[0] == ':'
	 || temp[0] == 'W'
	 || temp[0] == -1
	 || temp[0] == 0) {
		exit_status = return_value;
	} else {
		exit_status = 3;
	}

	for (i = 3; i > 0; i--) {
		if (temp[i] == 0) temp[i] = ' ';
		else break;
	}
#endif

	cob_set_int (COB_MODULE_PTR->cob_procedure_params[2], longind);
	memcpy (return_char, &return_value, 4);

	if (cob_optarg != NULL) {
		size_t optlen;
		memset (opt_val, 0, opt_val_size);

		optlen = strlen (cob_optarg);
		if (optlen > opt_val_size) {
			/* Return code 2 for "Option value too long => cut" */
			optlen = opt_val_size;
			exit_status = 2;
		}
		memcpy (opt_val, cob_optarg, optlen);
	}

	cob_free (shortoptions);
	cob_free (longoptions_root);

	return exit_status;
}

int
cob_sys_printable (void *p1, ...)
{
	size_t			datalen, n;
	unsigned char		*dotptr;
	unsigned char		dotrep;
	va_list			args;
	cob_u8_ptr		data;
	char		*previous_locale = NULL;

	COB_CHK_PARMS (CBL_GC_PRINTABLE, 1);

	if (!COB_MODULE_PTR->cob_procedure_params[0]) {
		return 0;
	}
	datalen = COB_MODULE_PTR->cob_procedure_params[0]->size;
	if (datalen == 0) {
		return 0;
	}
	if (cobglobptr->cob_call_params > 1) {
		va_start (args, p1);
		dotptr = va_arg (args, unsigned char *);
		va_end (args);
		dotrep = *dotptr;
	} else {
		dotrep = (unsigned char)'.';
	}
#ifdef	HAVE_SETLOCALE
	if (cobglobptr->cob_locale_ctype) {
		previous_locale = setlocale (LC_CTYPE, NULL);
		setlocale (LC_CTYPE, cobglobptr->cob_locale_ctype);
	}
#endif
	data = p1;
	for (n = 0; n < datalen; ++n) {
		if (!isprint (data[n])) {
			data[n] = dotrep;
		}
	}
#ifdef	HAVE_SETLOCALE
	if (previous_locale) {
		setlocale (LC_CTYPE, previous_locale);
	}
#endif
	return 0;
}

int
cob_sys_justify (void *p1, ...)
{
	cob_u8_ptr	data;
	size_t		datalen;
	size_t		left;
	size_t		right;
	size_t		movelen;
	size_t		centrelen;
	size_t		n;
	size_t		shifting;

	COB_CHK_PARMS (C$JUSTIFY, 1);

	if (!COB_MODULE_PTR->cob_procedure_params[0]) {
		return 0;
	}
	data = p1;
	datalen = COB_MODULE_PTR->cob_procedure_params[0]->size;
	if (datalen < 2) {
		return 0;
	}
	if (data[0] != ' ' && data[datalen - 1] != ' ') {
		return 0;
	}
	for (left = 0; left < datalen; ++left) {
		if (data[left] != ' ') {
			break;
		}
	}
	if (left == datalen) {
		return 0;
	}
	right = 0;
	for (n = datalen - 1; ; --n, ++right) {
		if (data[n] != ' ') {
			break;
		}
		if (n == 0) {
			break;
		}
	}
	movelen = datalen - left - right;
	shifting = 0;
	if (cobglobptr->cob_call_params > 1) {
		unsigned char	*direction;
		va_list		args;
		va_start (args, p1);
		direction = va_arg (args, unsigned char *);
		va_end (args);
		if (*direction == 'L') {
			shifting = 1;
		} else if (*direction == 'C') {
			shifting = 2;
		}
	}
	switch (shifting) {
	case 1:
		memmove (data, &data[left], movelen);
		memset (&data[movelen], ' ', datalen - movelen);
		break;
	case 2:
		centrelen = (left + right) / 2;
		memmove (&data[centrelen], &data[left], movelen);
		memset (data, ' ', centrelen);
		if ((left + right) % 2) {
			memset (&data[centrelen + movelen], ' ', centrelen + 1);
		} else {
			memset (&data[centrelen + movelen], ' ', centrelen);
		}
		break;
	default:
		memmove (&data[left + right], &data[left], movelen);
		memset (data, ' ', datalen - movelen);
		break;
	}
	return 0;
}

void
cob_set_locale (cob_field *locale, const int category)
{
#ifdef	HAVE_SETLOCALE
	char	buff[COB_MINI_BUFF];
	char	*p;

	if (locale) {
		int 	flen = cob_field_to_string (locale, buff,
					COB_MINI_MAX, CCM_NONE);
		if (flen < 1) {
			return;
		}
		p = buff;
	} else {
		p = NULL;
	}

	switch (category) {
	case COB_LC_COLLATE:
		p = setlocale (LC_COLLATE, p);
		break;
	case COB_LC_CTYPE:
		p = setlocale (LC_CTYPE, p);
		break;
#ifdef	LC_MESSAGES
	case COB_LC_MESSAGES:
		p = setlocale (LC_MESSAGES, p);
		break;
#endif
	case COB_LC_MONETARY:
		p = setlocale (LC_MONETARY, p);
		break;
	case COB_LC_NUMERIC:
		p = setlocale (LC_NUMERIC, p);
		break;
	case COB_LC_TIME:
		p = setlocale (LC_TIME, p);
		break;
	case COB_LC_ALL:
		p = setlocale (LC_ALL, p);
		break;
	case COB_LC_USER:
		if (cobglobptr->cob_locale_orig) {
			p = setlocale (LC_ALL, cobglobptr->cob_locale_orig);
			(void)setlocale (LC_NUMERIC, "C");
		}
		break;
	case COB_LC_CLASS:
		if (cobglobptr->cob_locale_ctype) {
			p = setlocale (LC_CTYPE, cobglobptr->cob_locale_ctype);
		}
		break;
	}
	if (!p) {
		cob_set_exception (COB_EC_LOCALE_MISSING);
		return;
	}
	p = setlocale (LC_ALL, NULL);
	if (p) {
		if (cobglobptr->cob_locale) {
			cob_free (cobglobptr->cob_locale);
		}
		cobglobptr->cob_locale = cob_strdup (p);
	}
#else
	cob_set_exception (COB_EC_LOCALE_MISSING);
#endif
}


#if 0 /* currently not used */
char *
cob_int_to_string (int i, char *number)
{
	if (!number) return NULL;
	sprintf (number, "%i", i);
	return number;
}

char *
cob_int_to_formatted_bytestring (int i, char *number)
{
	double		d;
	char		*byte_unit;

	if (!number) return NULL;

	byte_unit = (char *) cob_fast_malloc (3);

	if (i > (1024 * 1024)) {
		d = i / 1024.0 / 1024.0;
		byte_unit = (char *) "MB";
	} else if (i > 1024) {
		d = i / 1024.0;
		byte_unit = (char *) "kB";
	} else {
		d = 0;
		byte_unit = (char *) "B";
	}
	sprintf (number, "%3.2f %s", d, byte_unit);
	return number;
}
#endif

/* concatenate two strings allocating a new one
   and optionally free one of the strings
   set str_to_free if the result is assigned to
   one of the two original strings
*/
char *
cob_strcat (char *str1, char *str2, int str_to_free)
{
	size_t		l;
	char		*temp1, *temp2;

	l = strlen (str1) + strlen (str2) + 1;

	/*
	 * If one of the parameter is the buffer itself,
	 * we copy the buffer before continuing.
	 */
	if (str1 == strbuff) {
		temp1 = cob_strdup (str1);
	} else {
		temp1 = str1;
	}
	if (str2 == strbuff) {
		temp2 = cob_strdup (str2);
	} else {
		temp2 = str2;
	}

	if (strbuff) {
		cob_free (strbuff);
	}
	strbuff = (char *) cob_fast_malloc (l);

	sprintf (strbuff, "%s%s", temp1, temp2);
	switch (str_to_free) {
		case 1: cob_free (temp1);
		        break;
		case 2: cob_free (temp2);
		        break;
		default: break;
	}
	return strbuff;
}

char *
cob_strjoin (char **strarray, int size, char *separator)
{
	char	*result;
	int	i;

	if (!strarray || size <= 0 || !separator) return NULL;

	result = cob_strdup (strarray[0]);
	for (i = 1; i < size; i++) {
		result = cob_strcat (result, separator, 1);
		result = cob_strcat (result, strarray[i], 1);
	}

	return result;
}

static void
var_print (const char *msg, const char *val, const char *default_val,
		const unsigned int format)
{
#if 0 /* currently only format 0/1 used */
	switch (format) {
	case 0:
		printf ("%-*.*s : ", CB_IMSG_SIZE, CB_IMSG_SIZE, msg);
		break;
	case 1: {
			int	lablen;
			printf ("  %s: ", _("env"));
			lablen = CB_IMSG_SIZE - 2 - (int)strlen (_("env")) - 2;
			printf ("%-*.*s : ", lablen, lablen, msg);
			break;
		}
	case 2:
		printf ("    %-*.*s     : ", CB_IMSG_SIZE, CB_IMSG_SIZE, msg);
		break;
	case 3:
		printf ("        %-*.*s : ", CB_IMSG_SIZE, CB_IMSG_SIZE, msg);
		break;
	default:
		printf ("%-*.*s : ", CB_IMSG_SIZE, CB_IMSG_SIZE, msg);
		break;
	}

	if (!val && (!default_val || default_val[0] == 0)) {
		putchar ('\n');
		return;
	} else if (format != 0 && val && default_val &&
		((format != 2 && val[0] == '0') || strcmp (val, default_val) == 0)) {
		char dflt[40];
		snprintf (dflt, 39, " %s", _("(default)"));
		val = cob_strcat ((char *) default_val, dflt, 0);
	} else if (!val && default_val) {
		val = default_val;
	}
#else
	if (format == 0) {
		printf ("%-*.*s : ", CB_IMSG_SIZE, CB_IMSG_SIZE, msg);
	} else {
		int	lablen;
		printf ("  %s: ", _("env"));
		lablen = CB_IMSG_SIZE - 2 - (int)strlen (_("env")) - 2;
		printf ("%-*.*s : ", lablen, lablen, msg);
	}

	if (!val && (!default_val || default_val[0] == 0)) {
		putchar ('\n');
		return;
	} else if (format == 1 && val && default_val &&
		(val[0] == '0' || strcmp (val, default_val) == 0)) {
		char dflt[40];
		snprintf (dflt, 39, " %s", _("(default)"));
		val = cob_strcat ((char *) default_val, dflt, 0);
	} else if (!val && default_val) {
		val = default_val;
	}
#endif

	if (!val && (!default_val || default_val[0] == 0)) {
		putchar ('\n');
		return;
	} else if (format != 0 && val && default_val &&
		((format != 2 && val[0] == '0') || strcmp (val, default_val) == 0)) {
		char dflt[40];
		snprintf (dflt, 39, " %s", _("(default)"));
		val = cob_strcat ((char *) default_val, dflt, 0);
	} else if (!val && default_val) {
		val = default_val;
	}

	if (val && strlen (val) <= CB_IVAL_SIZE) {
		printf ("%s", val);
		putchar ('\n');

		return;
	}


	{
		char	*p;
		char	*token;
		size_t	n;

		p = cob_strdup (val);

		n = 0;
		token = strtok (p, " ");
		for (; token; token = strtok (NULL, " ")) {
			int toklen = (int)strlen (token) + 1;
			if ((n + toklen) > CB_IVAL_SIZE) {
				if (n) {
					if (format == 2 || format == 3)
						printf ("\n        %*.*s", CB_IMSG_SIZE + 3,
						CB_IMSG_SIZE + 3, " ");
					else
						printf ("\n%*.*s", CB_IMSG_SIZE + 3, CB_IMSG_SIZE + 3, " ");
				}
				n = 0;
			}
			printf ("%s%s", (n ? " " : ""), token);
			n += toklen;
		}
		putchar ('\n');
		cob_free (p);
	}

}

/*
 Expand a string with environment variable in it.
 Return malloced string.
*/
char *
cob_expand_env_string (char *strval)
{
	unsigned int	i;
	unsigned int	j = 0;
	unsigned int	k = 0;
	size_t	envlen = 1280;
	char		*env;
	char		*str;
	char		ename[128] = { '\0' };
	char		*penv;

	env = cob_malloc (envlen);
	for (k = 0; strval[k] != 0; k++) {
		/* String almost full?; Expand it */
		if (j >= envlen - 128) {
			env = cob_realloc (env, envlen, envlen + 256);
			envlen += 256;
		}

		/* ${envname:default} */
		if (strval[k] == '$' && strval[k + 1] == '{') {
			k += 2;
			for (i = 0; strval[k] != '}'
				     && strval[k] != 0
				     && strval[k] != ':'; k++) {
				ename[i++] = strval[k];
			}
			ename[i++] = 0;
			penv = getenv (ename);
			if (penv == NULL) {
				/* Copy 'default' value */
				if (strval[k] == ':') {
					k++;
					/* ${name:-default} */
					if (strval[k] == '-') {
						k++;
					}
					while (strval[k] != '}' && strval[k] != 0) {
						if (j >= envlen - 50) {
							env = cob_realloc (env, envlen, envlen + 128);
							envlen += 128;
						}
						env[j++] = strval[k++];
					}
				} else if (strcmp (ename, "COB_CONFIG_DIR") == 0) {
					penv = (char *)COB_CONFIG_DIR;
				} else if (strcmp (ename, "COB_COPY_DIR") == 0) {
					penv = (char *)COB_COPY_DIR;
				}
			}
			if (penv != NULL) {
				if ((strlen (penv) + j) > (envlen - 128)) {
					env = cob_realloc (env, envlen, strlen (penv) + 256);
					envlen = strlen (penv) + 256;
				}
				j += sprintf (&env[j], "%s", penv);
				penv = NULL;
			}
			while (strval[k] != '}' && strval[k] != 0) {
				k++;
			}
			if (strval[k] == '}') {
				k++;
			}
			k--;
		} else if (strval[k] == '$'
		        && strval[k+1] == '$') {	/* Replace $$ with process-id */
			j += sprintf (&env[j], "%d", cob_sys_getpid());
			k++;
		/* CHECME: possibly add $f /$b as basename of executable [or, when passed to cob_init the first name] 
		           along with $d date as yyyymmdd and $t as hhmmss */
		} else if (!isspace ((unsigned char)strval[k])) {
			env[j++] = strval[k];
		} else {
			env[j++] = ' ';
		}
	}

	env[j] = '\0';
	str = cob_strdup (env);
	cob_free (env);

	return str;
}

/* Store 'integer' value in field for correct length (computed with sizeof (fieldtype)) */
static void
set_value (char *data, int len, cob_s64_t val)
{
	/* keep in order of occurrence in data types, last nanoseconds for startup... */
	if (len == sizeof (int)) {
		*(int *)data = (int)val;
	} else if (len == sizeof (short)) {
		*(short *)data = (short)val;
	} else if (len == sizeof (cob_s64_t)) {
		*(cob_s64_t *)data = val;
	} else {
		*data = (char)val;
	}
}

/* Get 'integer' value from field */
static cob_s64_t
get_value (char *data, int len)
{
	if (len == sizeof (int)) {
		return *(int *)data;
	} else if (len == sizeof (short)) {
		return *(short *)data;
	} else if (len == sizeof (cob_s64_t)) {
		return *(cob_s64_t *)data;
	} else {
		return *data;
	}
}

static int
translate_boolean_to_int (const char* ptr)
{
	if (ptr == NULL || *ptr == 0) {
		return 2;
	}

	if (*(ptr + 1) == 0
	 && (*ptr == '0' || *ptr == '1')) {
		return COB_D2I (*ptr);		/* 0 or 1 */
	} else
	/* pre-translated boolean "never" - not set" */
	if (strcmp (ptr, "!") == 0) {
		return -1;
	} else
	if (strcasecmp (ptr, "true") == 0
	 || strcasecmp (ptr, "t") == 0
	 || strcasecmp (ptr, "on") == 0
	 || strcasecmp (ptr, "yes") == 0
	 || strcasecmp (ptr, "y") == 0) {
		return 1;			/* True value */
	} else
	if (strcasecmp (ptr, "false") == 0
	 || strcasecmp (ptr, "f") == 0
	 || strcasecmp (ptr, "off") == 0
	 || strcasecmp (ptr, "no") == 0
	 || strcasecmp (ptr, "n") == 0) {
		return 0;			/* False value */
	}
	return 2;
}

/* Set runtime setting with given value */
static int					/* returns 1 if any error, else 0 */
set_config_val (char *value, int pos)
{
	register char	*ptr = value;
	char	*str;
	cob_s64_t	numval = 0;
	int 	i, slen;

	const int 	data_type = gc_conf[pos].data_type;
	const size_t	data_loc = gc_conf[pos].data_loc;
	const int 	data_len = gc_conf[pos].data_len;

	char 	*data = ((char *)cobsetptr) + data_loc;

	if (gc_conf[pos].enums) {		/* Translate 'word' into alternate 'value' */

		for (i = 0; gc_conf[pos].enums[i].match != NULL; i++) {
			if (strcasecmp (value, gc_conf[pos].enums[i].match) == 0) {
				ptr = value = (char *)gc_conf[pos].enums[i].value;
				break;
			}
			if ((data_type & ENV_ENUMVAL) && strcasecmp (value, gc_conf[pos].enums[i].value) == 0) {
				break;
			}
		}
		if ((data_type & ENV_ENUM || data_type & ENV_ENUMVAL)	/* Must be one of the 'enum' values */
		 && gc_conf[pos].enums[i].match == NULL
		 && (!(data_type & ENV_BOOL))) {
			conf_runtime_error_value (ptr, pos);
			fprintf (stderr, _("should be one of the following values: %s"), "");
			for (i = 0; gc_conf[pos].enums[i].match != NULL; i++) {
				if (i != 0) {
					fputc (',', stderr);
					fputc (' ', stderr);
				}
				fprintf (stderr, "%s", (char *)gc_conf[pos].enums[i].match);
				if (data_type & ENV_ENUMVAL) {
					fprintf (stderr, "(%s)", (char *)gc_conf[pos].enums[i].value);
				}
			}
			fputc ('\n', stderr);
			fflush (stderr);
			return 1;
		}
	}

	if ((data_type & ENV_BOOL)) {	/* Boolean: Yes/No, True/False,... */
		numval = translate_boolean_to_int (ptr);

		if (numval != -1
		 && numval != 1
		 && numval != 0) {
			conf_runtime_error_value (ptr, pos);
			conf_runtime_error (1, _("should be one of the following values: %s"), "true, false");
			return 1;
		}
		if ((data_type & ENV_NOT)) {	/* Negate logic for actual setting */
			numval = !numval;
		}
		set_value (data, data_len, numval);

		/* call internal routines that do post-processing */
		if (data == (char *)&cobsetptr->cob_debugging_mode) {
			/* Copy variables from settings (internal) to global structure, each time */
			cobglobptr->cob_debugging_mode = cobsetptr->cob_debugging_mode;
		} else if (data == (char *)&cobsetptr->cob_insert_mode) {
			cob_settings_screenio ();
		} else if (data == (char *)&cobsetptr->cob_debugging_mode) {
			cob_switch[11 + 'D' - 'A'] = (int)numval;
		} else if (data == (char *)&cobsetptr->cob_ls_nulls) {
			cob_switch[11 + 'N' - 'A'] = (int)numval;
#if 0	/* TODO add in trunk */
		} else if (data == (char *)&cobsetptr->cob_ls_tabs) {
			cob_switch[11 + 'T' - 'A'] = (int)numval;
#endif
		}

	} else if ((data_type & ENV_UINT) 				/* Integer data, unsigned */
	        || (data_type & ENV_SINT) 				/* Integer data, signed */
	        || (data_type & ENV_SIZE) ) {			/* Size: integer with K, M, G */
		char sign = 0;
		for (; *ptr == ' '; ptr++);	/* skip leading space */
		if (*ptr == '-'
		 || *ptr == '+') {
			if ((data_type & ENV_SINT) == 0) {
				conf_runtime_error_value (ptr, pos);
				/* CHECKME: likely cob_runtime_warning would be more reasonable */
				conf_runtime_error (1, _("should be unsigned"));
				return 1;
			}
			sign = *ptr;
			ptr++;
		}
		if (IS_INVALID_DIGIT_DATA (*ptr)) {
			conf_runtime_error_value (ptr, pos);
			conf_runtime_error (1, _("should be numeric"));
			return 1;
		}
		while (IS_VALID_DIGIT_DATA (*ptr)) {
			numval = (numval * 10) + COB_D2I (*ptr++);
		}
		if (sign != 0
		 && ( *ptr == '-'
		   || *ptr == '+')) {
			if ((data_type & ENV_SINT) == 0) {
				conf_runtime_error_value (ptr, pos);
				conf_runtime_error (1, _("should be unsigned"));
				return 1;
			}
			sign = *ptr;
			ptr++;
		}
		if ((data_type & ENV_SIZE)			/* Size: any K, M, G */
		 && *ptr != 0) {
			switch (toupper ((unsigned char)*ptr)) {
			case 'K':
				numval = numval * 1024;
				ptr++;
				break;
			case 'M':
				if (numval < 4001) {
					numval = numval * 1024 * 1024;
				} else {
					/* use max. guaranteed value for unsigned long
					   to raise a warning as max value is limit to one less */
					numval = 4294967295UL;
				}
				ptr++;
				break;
			case 'G':
				if (numval < 4) {
					numval = numval * 1024 * 1024 * 1024;
				} else {
					/* use max. guaranteed value for unsigned long
					   to raise a warning as max value is limit to one less */
					numval = 4294967295UL;
				}
				ptr++;
				break;
			}
		}
		for (; *ptr == ' '; ptr++);	/* skip trailing space */
		if (*ptr != 0) {
			conf_runtime_error_value (ptr, pos);
			conf_runtime_error (1, _("should be numeric"));
			return 1;
		}
		if (sign == '-') {
			numval = -numval;
		}
		if (gc_conf[pos].min_value > 0
		 && numval < gc_conf[pos].min_value) {
			conf_runtime_error_value (value, pos);
			conf_runtime_error (1, _("minimum value: %lu"), gc_conf[pos].min_value);
			return 1;
		}
		if (gc_conf[pos].max_value > 0
		 && numval > gc_conf[pos].max_value) {
			conf_runtime_error_value (value, pos);
			conf_runtime_error (1, _("maximum value: %lu"), gc_conf[pos].max_value);
			return 1;
		}
		set_value (data, data_len, numval);

		/* call internal routines that do post-processing */
		if (data == (char *)&cobsetptr->cob_mouse_flags
#ifdef HAVE_MOUSEINTERVAL	/* possibly add an internal option for mouse support, too */
		 || data == (char *)&cobsetptr->cob_mouse_interval
#endif
		    ) {
			cob_settings_screenio ();
		}

	} else if ((data_type & ENV_FILE)
	        || (data_type & ENV_PATH)) {	/* Path (environment expanded) to be stored as a string */
		memcpy (&str, data, sizeof (char *));
		if (str != NULL) {
			cob_free ((void *)str);
		}
		str = cob_expand_env_string (value);
		if ((data_type & ENV_FILE)
		 && strchr (str, PATHSEP_CHAR) != NULL) {
			conf_runtime_error_value (value, pos);
			conf_runtime_error (1, _("should not contain '%c'"), PATHSEP_CHAR);
			cob_free (str);
			return 1;
		}
		memcpy (data, &str, sizeof (char *));
		if (data_loc == offsetof (cob_settings, cob_preload_str)) {
			cobsetptr->cob_preload_str_set = cob_strdup(str);
		}

		/* call internal routines that do post-processing */
		if (data == (char *)cobsetptr->cob_trace_filename
		 && cobsetptr->cob_trace_file != VFS_INVALID_FD) {
			cob_new_trace_file ();
		}

	} else if (data_type & ENV_STR) {	/* String (environment expanded) */
		memcpy (&str, data, sizeof (char *));
		if (str != NULL) {
			cob_free ((void *)str);
		}
		str = cob_expand_env_string (value);
		memcpy (data, &str, sizeof (char *));
		if (data_loc == offsetof (cob_settings, cob_preload_str)) {
			cobsetptr->cob_preload_str_set = cob_strdup(str);
		}

		/* call internal routines that do post-processing */
		if (data == (void *)cobsetptr->cob_date) {
			check_current_date ();
		}

	} else if ((data_type & ENV_CHAR)) {	/* 'char' field inline */
		memset (data, 0, data_len);
		slen = (int)strlen (value);
		if (slen > data_len) {
			slen = data_len;
		}
		memcpy (data, value, slen);
	}
	return 0;
}

/* Set runtime setting by name with given value */
static int					/* returns 1 if any error, else 0 */
set_config_val_by_name (char *value, const char *name, const char *func)
{
	int	i;
	int ret = 1;

	for (i = 0; i < NUM_CONFIG; i++) {
		if (!strcmp (gc_conf[i].conf_name, name)) {
			ret = set_config_val (value, i);
			if (func) {
				gc_conf[i].data_type |= STS_FNCSET;
				gc_conf[i].set_by = FUNC_NAME_IN_DEFAULT;
				gc_conf[i].default_val = func;
			}
			break;
		}
	}
	return ret;
}

/* Return setting value as a 'string' */
static char *
get_config_val (char *value, int pos, char *orgvalue)
{
	char 	*data;
	char	*str;
	double	dval;
	cob_s64_t	numval;
	int	i, data_type, data_len;
	size_t	data_loc;

	data_type	= gc_conf[pos].data_type;
	data_loc	= gc_conf[pos].data_loc;
	data_len	= gc_conf[pos].data_len;

	data = ((char *)cobsetptr) + data_loc;

	if (min_conf_length == 0) {
		not_set = _("not set");
		min_conf_length = (char) strlen (not_set) + 1;
		if (min_conf_length < 6) {
			min_conf_length = 6;
		} else if (min_conf_length > 15) {
			min_conf_length = 15;
		}
	}

	strcpy (value, _("unknown"));
	orgvalue[0] = 0;

	if ((data_type & ENV_BOOL)) {	/* Boolean: Yes/No, True/False,... */
		numval = get_value (data, data_len);
		if (numval == -1) {
#if 0		/* boolean "not set" - used for file specific settings (4.x feature) */
			if (gc_conf[pos].enums == never) {
				strcpy (value, "never");
			} else {
				strcpy (value, _("not set"));
			}
#else
			strcpy (value, "never");
#endif
		} else {
			if (data_type & ENV_NOT) {
				numval = !numval;
			}
			if (numval) {
				strcpy (value, _("yes"));
			} else {
				strcpy (value, _("no"));
			}
		}

	} else if (data_type & ENV_UINT) {				/* Integer data, unsigned */
		numval = get_value (data, data_len);
		sprintf (value, CB_FMT_LLU, numval);

	} else if (data_type & ENV_SINT) {				/* Integer data, signed */
		numval = get_value (data, data_len);
		sprintf (value, CB_FMT_LLD, numval);

	} else if ((data_type & ENV_SIZE)) {			/* Size: integer with K, M, G */
		numval = get_value (data, data_len);
		dval = (double) numval;
		if (numval > (1024 * 1024 * 1024)) {
			if ((numval % (1024 * 1024 * 1024)) == 0) {
				sprintf (value, CB_FMT_LLD" GB", numval / (1024 * 1024 * 1024));
			} else {
				sprintf (value, "%.2f GB", dval / (1024.0 * 1024.0 * 1024.0));
			}
		} else if (numval > (1024 * 1024)) {
			if ((numval % (1024 * 1024)) == 0) {
				sprintf (value, CB_FMT_LLD" MB", numval / (1024 * 1024));
			} else {
				sprintf (value, "%.2f MB", dval / (1024.0 * 1024.0));
			}
		} else if (numval > 1024) {
			if ((numval % 1024) == 0) {
				sprintf (value, CB_FMT_LLD" KB", numval / 1024);
			} else {
				sprintf (value, "%.2f KB", dval / 1024.0);
			}
		} else {
			sprintf (value, CB_FMT_LLD, numval);
		}

	/* TO-DO: Consolidate copy-and-pasted code! */
	} else if (data_type & ENV_STR) {	/* String stored as a string */
		memcpy (&str, data, sizeof (char *));
		if (data_loc == offsetof (cob_settings, cob_display_print_filename)
		 && cobsetptr->cob_display_print_file) {
			snprintf (value, COB_MEDIUM_MAX, _("set by %s"), "cob_set_runtime_option");
		} else if (data_loc == offsetof (cob_settings, cob_display_punch_filename)
		 && cobsetptr->cob_display_punch_file) {
			snprintf (value, COB_MEDIUM_MAX, _("set by %s"), "cob_set_runtime_option");
		} else if(data_loc == offsetof (cob_settings, cob_trace_filename)
		      && cobsetptr->external_trace_file) {
			snprintf (value, COB_MEDIUM_MAX, _("set by %s"), "cob_set_runtime_option");
		} else if (str == NULL) {
			snprintf (value, COB_MEDIUM_MAX, _("not set"));
		} else {
			snprintf (value, COB_MEDIUM_MAX, "'%s'", str);
		}

	} else if ((data_type & ENV_FILE)) {	/* File/path stored as a string */
		memcpy (&str, data, sizeof (char *));
		/* TODO: add special cases here on merging rw-branch */
		if (str == NULL)  {
			snprintf (value, COB_MEDIUM_MAX, _("not set"));
		} else {
			snprintf (value, COB_MEDIUM_MAX, "%s", str);
		}

	} else if ((data_type & ENV_PATH)) {	/* Path stored as a string */
		memcpy (&str, data, sizeof (char *));
		if (str == NULL)  {
			snprintf (value, COB_MEDIUM_MAX, _("not set"));
		} else {
			snprintf (value, COB_MEDIUM_MAX, "%s", str);
		}

	} else if ((data_type & ENV_CHAR)) {	/* 'char' field inline */
		if (*(char *)data == 0) {
			strcpy (value, "Nul");
		} else if (isprint (*(unsigned char *)data)) {
			sprintf (value, "'%s'", (char *)data);
		} else {
			sprintf (value, "0x%02X", *(char *)data);
		}
	}
	value[COB_MEDIUM_MAX] = 0;	/* fix warning */

	if (gc_conf[pos].enums) {		/* Translate 'word' into alternate 'value' */
		for (i = 0; gc_conf[pos].enums[i].match != NULL; i++) {
			if (strcasecmp (value, gc_conf[pos].enums[i].value) == 0) {
				if (strcmp (value, "0") != 0
				 && strcmp (value, gc_conf[pos].default_val) != 0) {
					strcpy (orgvalue, value);
				}
				/* insert either value or translated "not set" */
				if (strcmp (gc_conf[pos].enums[i].match, "not set") == 0) {
					snprintf (value, COB_MEDIUM_MAX, _("not set"));
					value[COB_MEDIUM_MAX] = 0;	/* fix warning */
				} else {
					strcpy (value, gc_conf[pos].enums[i].match);
				}
				break;
			}
		}
		if (gc_conf[pos].enums[i].match == NULL
		 && gc_conf[pos].default_val != NULL
		 && strcmp (value, gc_conf[pos].default_val) != 0) {
			strcpy (orgvalue, value);
		}
	} else
	if (!(gc_conf[pos].data_type & STS_ENVSET)
	 && !(gc_conf[pos].data_type & STS_CNFSET)
	 && !(gc_conf[pos].data_type & ENV_BOOL)
	 && gc_conf[pos].default_val != NULL) {
		strcpy (value, gc_conf[pos].default_val);
		orgvalue[0] = 0;
	}

	if (gc_conf[pos].default_val != NULL
	 && strcmp (orgvalue, gc_conf[pos].default_val) != 0) {
		orgvalue[0] = 0;
	} else if (strcmp (value, orgvalue) == 0) {
		orgvalue[0] = 0;
	}
	return value;
}

static int
cb_lookup_config (char *keyword)
{
	int	i;
	for (i = 0; i < NUM_CONFIG; i++) {		/* Set value from config file */
		if (gc_conf[i].conf_name
		 && strcasecmp (keyword, gc_conf[i].conf_name) == 0) {	/* Look for config file name */
			break;
		}
		if (gc_conf[i].env_name
		 && strcasecmp (keyword, gc_conf[i].env_name) == 0) {	/* Catch using env var name */
			break;
		}
	}
	return i;
}

static int
cb_config_entry (char *buf, int line)
{
	int	i, j, k, old_type;
	void	*data;
	char	*str, qt;
	char	keyword[COB_MINI_BUFF], value[COB_SMALL_BUFF];

	cob_source_line = line;

	for (j = (int)strlen (buf); buf[j-1] == '\r' || buf[j-1] == '\n'; )	/* Remove CR LF */
		buf[--j] = 0;

	for (i = 0; isspace ((unsigned char)buf[i]); i++); /* drop leading spaces */

	for (j = 0; buf[i] != 0 && buf[i] != ':' && !isspace ((unsigned char)buf[i]) && buf[i] != '=' && buf[i] != '#'; )
		keyword[j++] = buf[i++];
	keyword[j] = 0;

	while (buf[i] != 0 && (isspace ((unsigned char)buf[i]) || buf[i] == ':' || buf[i] == '=')) i++;
	if (buf[i] == '"'
	||  buf[i] == '\'') {
		qt = buf[i++];
		for (j = 0; buf[i] != qt && buf[i] != 0; )
			value[j++] = buf[i++];
	} else {
		for (j = 0; !isspace ((unsigned char)buf[i]) && buf[i] != '#' && buf[i] != 0; )
			value[j++] = buf[i++];
	}

	value[j] = 0;
	if (strcasecmp (keyword, "reset") != 0
	 && strcasecmp (keyword, "include") != 0
	 && strcasecmp (keyword, "includeif") != 0
	 && strcasecmp (keyword, "setenv") != 0
	 && strcasecmp (keyword, "unsetenv") != 0) {
		i = cb_lookup_config (keyword);

		if (i >= NUM_CONFIG) {
			conf_runtime_error (1, _("unknown configuration tag '%s'"), keyword);
			return -1;
		}
	}
	if (strcmp (value, "") == 0) {
		if (strcasecmp (keyword, "include") != 0
		&&  strcasecmp (keyword, "includeif")) {
			conf_runtime_error(1, _("WARNING - '%s' without a value - ignored!"), keyword);
			return 2;
		} else {
			conf_runtime_error (1, _("'%s' without a value!"), keyword);
			return -1;
		}
	}

	if (strcasecmp (keyword, "setenv") == 0 ) {
		char	value2[COB_SMALL_BUFF];
		/* collect additional value and push into environment */
		strcpy (value2, "");
		/* check for := in value 2 and split, if necessary */
		k = 0; while (value[k] != '=' && value[k] != ':' && value[k] != '"' && value[k] != '\'' && value[k] != 0) k++;
		if (value[k] == '=' || value[k] == ':') {
			i = i - (int)strlen (value + k);
			value[k] = 0;
		}
		while (isspace ((unsigned char)buf[i]) || buf[i] == ':' || buf[i] == '=') i++;
		if (buf[i] == '"'
		|| buf[i] == '\'') {
			qt = buf[i++];
			for (j = 0; buf[i] != qt && buf[i] != 0; )
				value2[j++] = buf[i++];
		} else {
			for (j = 0; !isspace ((unsigned char)buf[i]) && buf[i] != '#' && buf[i] != 0; )
				value2[j++] = buf[i++];
		}
		value2[j] = 0;
		if (strcmp (value2, "") == 0) {
			conf_runtime_error (1, _("WARNING - '%s %s' without a value - ignored!"), keyword, value);
			return 2;
		}
		/* check additional value for inline env vars ${varname:-default} */
		str = cob_expand_env_string (value2);

		(void)cob_setenv (value, str, 1);
		cob_free (str);
		for (i = 0; i < NUM_CONFIG; i++) {		/* Set value from config file */
			if (gc_conf[i].env_name
			&& strcasecmp (value, gc_conf[i].env_name) == 0) {/* no longer cleared by runtime.cfg */
				gc_conf[i].data_type &= ~STS_ENVCLR;
				break;
			}
		}
		return 0;
	}

	if (strcasecmp (keyword, "unsetenv") == 0) {
		if ((getenv (value)) != NULL ) {
			for (i = 0; i < NUM_CONFIG; i++) {		/* Set value from config file */
				if (gc_conf[i].env_name
				&& strcasecmp (value, gc_conf[i].env_name) == 0) {	/* Catch using env var name */
					gc_conf[i].data_type |= STS_ENVCLR;
					break;
				}
			}
			(void)cob_unsetenv (value);
		}
		return 0;
	}

	if (strcasecmp (keyword, "include") == 0 ||
		strcasecmp (keyword, "includeif") == 0) {
		str = cob_expand_env_string (value);
		strcpy (buf, str);
		cob_free (str);
		if (strcasecmp (keyword, "include") == 0) {
			return 1;
		} else {
			return 3;
		}
	}

	if (strcasecmp (keyword, "reset") == 0) {
		i = cb_lookup_config (value);
		if (i >= NUM_CONFIG) {
			conf_runtime_error (1, _("unknown configuration tag '%s'"), value);
			return -1;
		}
		gc_conf[i].data_type &= ~(STS_ENVSET | STS_CNFSET | STS_ENVCLR);	/* Clear status */
		gc_conf[i].data_type |= STS_RESET;
		gc_conf[i].set_by = 0;
		gc_conf[i].config_num = cobsetptr->cob_config_cur - 1;
		if (gc_conf[i].default_val) {
			set_config_val ((char *)gc_conf[i].default_val, i);
		} else if ((gc_conf[i].data_type & ENV_STR)
			|| (gc_conf[i].data_type & ENV_FILE)
			|| (gc_conf[i].data_type & ENV_PATH)) {	/* String/Path stored as a string */
			data = (void *) ((char *)cobsetptr + gc_conf[i].data_loc);
			memcpy (&str, data, sizeof (char *));
			if (str != NULL) {
				cob_free ((void *)str);
			}
			str = NULL;
			memcpy (data, &str, sizeof (char *));	/* Reset pointer to NULL */
		} else {
			set_config_val ((char *)"0", i);
		}
		return 0;
	}

	i = cb_lookup_config (keyword);

	if (i >= NUM_CONFIG) {
		conf_runtime_error (1, _("unknown configuration tag '%s'"), keyword);
		return -1;
	}

	old_type = gc_conf[i].data_type;
	gc_conf[i].data_type |= STS_CNFSET;
	if (!set_config_val (value, i)) {
		gc_conf[i].data_type &= ~STS_RESET;
		gc_conf[i].config_num = cobsetptr->cob_config_cur - 1;

		if (gc_conf[i].env_group == GRP_HIDE) {
			for (j = 0; j < NUM_CONFIG; j++) {		/* Any alias present? */
				if (j != i
				 && gc_conf[i].data_loc == gc_conf[j].data_loc) {
					gc_conf[j].data_type |= STS_CNFSET;
					gc_conf[j].data_type &= ~STS_RESET;
					gc_conf[j].config_num = gc_conf[i].config_num;
					gc_conf[j].set_by = i;
				}
			}
		}
	} else {
		gc_conf[i].data_type = old_type;
	}
	return 0;
}

static int
cob_load_config_file (const char *config_file, int isoptional)
{
	char			buff[COB_FILE_BUFF-10], filename[COB_FILE_BUFF];
	char			*penv;
	int			sub_ret, ret;
	unsigned int	i;
	int			line;
	fd_t conf_fd;

	for (i = 0; config_file[i] != 0 && config_file[i] != SLASH_CHAR; i++);
	if (config_file[i] == 0) {			/* Just a name, No directory */
		if (access (config_file, F_OK) != 0) {	/* and file does not exist */
			/* check for path of previous configuration file (for includes) */
			filename[0] = 0;
			if (cobsetptr->cob_config_cur != 0) {
				size_t size;
				strncpy (buff,
					cobsetptr->cob_config_file[cobsetptr->cob_config_cur - 1],
					(size_t)COB_FILE_MAX-10);
				size = strlen (buff);
				if (size != 0 && buff[size] == SLASH_CHAR) buff[--size] = 0;
				if (size != 0) {
					snprintf (filename, (size_t)COB_FILE_MAX, "%s%c%s", buff, SLASH_CHAR,
						config_file);
					if (access (filename, F_OK) == 0) {	/* and prefixed file exist */
						config_file = filename;		/* Prefix last directory */
					} else {
						filename[0] = 0;
					}
				}
			}
			if (filename[0] == 0) {
				/* check for COB_CONFIG_DIR (use default if not in environment) */
				penv = getenv ("COB_CONFIG_DIR");
				if (penv != NULL) {
					snprintf (filename, (size_t)COB_FILE_MAX, "%s%c%s",
						penv, SLASH_CHAR, config_file);
				} else {
					snprintf (filename, (size_t)COB_FILE_MAX, "%s%c%s",
						COB_CONFIG_DIR, SLASH_CHAR, config_file);
				}
				if (access (filename, F_OK) == 0) {	/* and prefixed file exist */
					config_file = filename;		/* Prefix COB_CONFIG_DIR */
				}
			}
		}
	}

	cob_source_file = config_file;

	/* check for recursion */
	for (i = 0; i < cobsetptr->cob_config_num; i++) {
		if (strcmp (cobsetptr->cob_config_file[i], config_file) == 0) {
			cob_source_line = 0;
			conf_runtime_error (1, _("recursive inclusion"));
			return -2;
		}
	}

	/* Open the configuration file */
	conf_fd = fopen (config_file, "r");
	if (conf_fd == VFS_INVALID_FD && !isoptional) {
		cob_source_line = 0;
		conf_runtime_error (1, cob_get_strerror ());
		if (cobsetptr->cob_config_file) {
			cob_source_file = cobsetptr->cob_config_file[cobsetptr->cob_config_num-1];
		}
		return -1;
	}
	if (conf_fd != VFS_INVALID_FD) {
		if (cobsetptr->cob_config_file == NULL) {
			cobsetptr->cob_config_file = cob_malloc (sizeof (char *));
		} else {
			const size_t old_size = sizeof (char *) * cobsetptr->cob_config_num;
			const size_t new_size = sizeof (char *) * (cobsetptr->cob_config_num + 1);
			cobsetptr->cob_config_file = cob_realloc (cobsetptr->cob_config_file, old_size, new_size);
		}
		cobsetptr->cob_config_file[cobsetptr->cob_config_num++] = cob_strdup (config_file);	/* Save config file name */
		cobsetptr->cob_config_cur = cobsetptr->cob_config_num;
	}


	/* Read the configuration file */
	ret = 0;
	line = 0;
	while ((conf_fd != VFS_INVALID_FD)
	   &&  (fgets (buff, COB_SMALL_BUFF, conf_fd) != NULL) ) {
		line++;
		for (i = 0; isspace ((unsigned char)buff[i]); i++);
		if (buff[i] == 0
		 || buff[i] == '#'
		 || buff[i] == '\r'
		 || buff[i] == '\n')
			continue;	/* Skip comments and blank lines */

		/* Evaluate config line */
		sub_ret = cb_config_entry (buff, line);

		/* Include another configuration file */
		if (sub_ret == 1 || sub_ret == 3) {
			cob_source_line = line;
			sub_ret = cob_load_config_file (buff, sub_ret == 3);
			cob_source_file = config_file;
			if (sub_ret < 0) {
				ret = -1;
				cob_source_line = line;
				conf_runtime_error (1, _("configuration file was included here"));
				break;
			}
		}
		if (sub_ret < ret) ret = sub_ret;
	}
	if (conf_fd) {
		fclose (conf_fd);
		cobsetptr->cob_config_cur--;
	}
	cob_source_file = NULL;
	conf_fd = VFS_INVALID_FD;

	return ret;
}

/*
 * Load the GnuCOBOL runtime configuration information
 */
int
cob_load_config (void)
{
	char		*env;
	char		conf_file[COB_MEDIUM_BUFF];
	int		is_optional = 1, sts, i, j;


	/* Get the name for the configuration file */
	if ((env = getenv ("COB_RUNTIME_CONFIG")) != NULL && env[0]) {
		strncpy (conf_file, env, (size_t)COB_MEDIUM_MAX);
		conf_file[COB_MEDIUM_MAX] = 0;
		is_optional = 0;			/* If declared then it is NOT optional */
		if (strchr (conf_file, PATHSEP_CHAR) != NULL) {
			conf_runtime_error (0, _("invalid value '%s' for configuration tag '%s'"), conf_file, "COB_RUNTIME_CONFIG");
			conf_runtime_error (1, _("should not contain '%c'"), PATHSEP_CHAR);
			return -1;
		}
	} else {
		/* check for COB_CONFIG_DIR (use default if not in environment) */
		if ((env = getenv ("COB_CONFIG_DIR")) != NULL && env[0]) {
			snprintf (conf_file, (size_t)COB_MEDIUM_MAX, "%s%c%s", env, SLASH_CHAR, "runtime.cfg");
		} else {
			snprintf (conf_file, (size_t)COB_MEDIUM_MAX, "%s%c%s", COB_CONFIG_DIR, SLASH_CHAR, "runtime.cfg");
		}
		conf_file[COB_MEDIUM_MAX] = 0; /* fixing code analyser warning */
		is_optional = 1;			/* If not present, then just use env vars */
		if (strchr (conf_file, PATHSEP_CHAR) != NULL) {
			conf_runtime_error (0, _("invalid value '%s' for configuration tag '%s'"), conf_file, "COB_CONFIG_DIR");
			conf_runtime_error (1, _("should not contain '%c'"), PATHSEP_CHAR);
			return -1;
		}
	}

	sprintf (varseq_dflt, "%d", WITH_VARSEQ);		/* Default comes from config.h */
	for (i = 0; i < NUM_CONFIG; i++) {
		gc_conf[i].data_type &= ~(STS_ENVSET | STS_CNFSET | STS_ENVCLR);	/* Clear status */
	}

	sts = cob_load_config_file (conf_file, is_optional);
	if (sts < 0) {
		return sts;
	}
	cob_rescan_env_vals (); 			/* Check for possible environment variables */

	/* Set with default value if present and not set otherwise */
	for (i = 0; i < NUM_CONFIG; i++) {
		if (gc_conf[i].default_val
		&& !(gc_conf[i].data_type & STS_CNFSET)
		&& !(gc_conf[i].data_type & STS_ENVSET)) {
			for (j = 0; j < NUM_CONFIG; j++) {	/* Any alias present? */
				if (j != i
				&& gc_conf[i].data_loc == gc_conf[j].data_loc)
					break;
			}
			if (j < NUM_CONFIG) {
				if (!(gc_conf[j].data_type & STS_CNFSET)
				&& !(gc_conf[j].data_type & STS_ENVSET)) {	/* alias not defined? */
					set_config_val ((char *)gc_conf[i].default_val, i);
				}
			} else {
				set_config_val ((char *)gc_conf[i].default_val, i); /* Set default value */
			}
		}
	}
	check_current_date ();

	return 0;
}

/* output runtime warning for issues produced by external API functions */
void
cob_runtime_warning_external (const char *caller_name, const int cob_reference, const char *fmt, ...)
{
	va_list args;

	if (!cobsetptr->cob_display_warn) {
		return;
	}
	if (!(caller_name && *caller_name)) caller_name = "unknown caller";

	/* Prefix */
	if (cob_reference) {
		fflush (stderr); /* necessary as we write afterwards */
		write_to_stderr_or_return_arr ("libcob: ");
		cob_get_source_line ();
		output_source_location ();
	} else {
		fprintf (stderr, "libcob: ");
	}
	fprintf (stderr, _("warning: "));
	fprintf (stderr, "%s: ", caller_name);

	/* Body */
	va_start (args, fmt);
	vfprintf (stderr, fmt, args);
	va_end (args);

	/* Postfix */
	fputc ('\n', stderr);
	fflush (stderr);
}

void
cob_runtime_warning_ss (const char *msg, const char *addition)
{
	if (cobsetptr && !cobsetptr->cob_display_warn) {
		return;
	}

	/* Prefix */
	write_to_stderr_or_return_arr ("libcob: ");
	output_source_location ();
	write_to_stderr_or_return_str (warning_msgid);

	/* Body */
	write_to_stderr_or_return_str (msg);
	if (addition) {
		write_to_stderr_or_return_str (addition);
	}

	/* Postfix */
	write_to_stderr_or_return_arr ("\n");
}

void
cob_runtime_warning (const char *fmt, ...)
{
	va_list args;

	if (cobsetptr && !cobsetptr->cob_display_warn) {
		return;
	}

	fflush (stderr);	/* necessary as we write afterwards */

	/* Prefix */
	write_to_stderr_or_return_arr ("libcob: ");
	cob_get_source_line ();
	output_source_location ();

	fprintf (stderr, _("warning: "));	/* back to stdio */

	/* Body */
	va_start (args, fmt);
	vfprintf (stderr, fmt, args);
	va_end (args);

	/* Postfix */
	fputc ('\n', stderr);
	fflush (stderr);
}

void
cob_runtime_hint (const char *fmt, ...)
{
	va_list args;

	/* Prefix */
	fprintf (stderr, "%s", _("note: "));

	/* Body */
	va_start (args, fmt);
	vfprintf (stderr, fmt, args);
	va_end (args);

	/* Postfix */
	fputc ('\n', stderr);
	fflush (stderr);
}

/* extra function for direct interaction with the debugger
   when a new runtime error string is constructed */
static void COB_NOINLINE 
cob_setup_runtime_error_str (const char *fmt, va_list ap)
{
	const char		*source_file;
	unsigned int	 source_line;
	char *p = runtime_err_str;

	set_source_location (&source_file, &source_line);
	if (source_file) {
		if (source_line) {
			sprintf (runtime_err_str, "%s:%u: ",
				source_file, source_line);
		} else {
			sprintf (runtime_err_str, "%s: ",
				source_file);
		}
		p = runtime_err_str + strlen (runtime_err_str);
	}
	vsprintf (p, fmt, ap);
}

void
cob_runtime_error (const char *fmt, ...)
{
	struct handlerlist	*h;
	va_list			ap;

	int	more_error_procedures = 1;
	cob_get_source_line ();

	va_start (ap, fmt);
	cob_setup_runtime_error_str (fmt, ap);
	va_end (ap);

#if	1	/* RXWRXW - Exit screen */
	/* Exit screen mode early */
	cob_exit_screen ();
#endif

	if (hdlrs != NULL && !active_error_handler && cobglobptr) {

		const char		*err_source_file;
		unsigned int	err_source_line, err_module_statement = 0;
		cob_module_ptr	err_module_pointer = NULL;
		cob_field *err_module_param0 = NULL;
		cob_field err_field = {COB_ERRBUF_SIZE, NULL, &const_alpha_attr };
		int call_params = cobglobptr->cob_call_params;

		/* save error location */
		set_source_location (&err_source_file, &err_source_line);
		if (COB_MODULE_PTR) {
			err_module_pointer = COB_MODULE_PTR;
			err_module_statement = COB_MODULE_PTR->module_stmt;
			err_module_param0 = COB_MODULE_PTR->cob_procedure_params[0];
			COB_MODULE_PTR->cob_procedure_params[0] = &err_field;
		}

		/* run registered error handlers */
		active_error_handler = 1;
		h = hdlrs;
		while (h != NULL) {
			int			(*current_handler)(char *) = h->proc;
			struct handlerlist	*hp = h;

			h = h->next;
			cob_free (hp);

			if (more_error_procedures) {
				/* fresh error buffer with guaranteed size */
				char local_err_str[COB_ERRBUF_SIZE];
				memcpy (local_err_str, runtime_err_str, COB_ERRBUF_SIZE);
				err_field.data = (unsigned char *)local_err_str;

				/* ensure that error handlers set their own locations */
				cob_source_file = NULL;
				cob_source_line = 0;
				cobglobptr->cob_call_params = 1;

				more_error_procedures = current_handler (local_err_str);
			}
		}
		hdlrs = NULL;
		active_error_handler = 0;

		/* restore error location */
		cob_source_file = err_source_file;
		cob_source_line = err_source_line;
		COB_MODULE_PTR = err_module_pointer;
		if (COB_MODULE_PTR) {
			COB_MODULE_PTR->module_stmt = err_module_statement;
			COB_MODULE_PTR->cob_procedure_params[0] = err_module_param0;
		}
		cobglobptr->cob_call_params = call_params;
	}

	/* Prefix */
	if (more_error_procedures) {
		const char		*source_file;
		unsigned int	 source_line;
		set_source_location (&source_file, &source_line);
		fputs ("libcob: ", stderr);
		if (source_file) {
			fprintf (stderr, "%s:", source_file);
			if (source_line) {
				fprintf (stderr, "%u:", source_line);
			}
			fputc (' ', stderr);
		}
		fprintf (stderr, "%s: ", _("error"));

		/* Body */
		va_start (ap, fmt);
		vfprintf (stderr, fmt, ap);
		va_end (ap);

		/* Postfix */
		fputc ('\n', stderr);
		fflush (stderr);
	}

	/* setup reason for optional module dump */
	if (cob_initialized && abort_reason[0] == 0) {
#if 0	/* Is there a use in this message ?*/
		fputc ('\n', stderr);
		fprintf (stderr, _("abnormal termination - file contents may be incorrect"));
#endif
		va_start (ap, fmt);
		vsnprintf (abort_reason, COB_MINI_BUFF, fmt, ap);
		va_end (ap);
	}
}

void
cob_fatal_error (const enum cob_fatal_error fatal_error)
{
	const char	*msg;
	unsigned char	*file_status;
	char		*err_cause;
	int		status;
#ifdef	_WIN32
	char		*p;
#endif

	switch (fatal_error) {
#if 0 /* Currently not in use, should enter unknown error */
	case COB_FERROR_NONE:
		break;
#endif
	/* Note: can be simply tested; therefore no exclusion */
	case COB_FERROR_CANCEL:
		cob_runtime_error (_("attempt to CANCEL active program"));
		break;
	/* Note: can be simply tested; therefore no exclusion */
	case COB_FERROR_INITIALIZED:
#ifdef	_WIN32
		/* cob_unix_lf needs to be set before any error message is thrown,
		as they would have wrong line endings otherwise */
		p = getenv ("COB_UNIX_LF");
		if (p && (*p == 'Y' || *p == 'y' ||
			*p == 'T' || *p == 't' ||
			*p == '1')) {
			(void)_setmode (_fileno (stdin), _O_BINARY);
			(void)_setmode (_fileno (stdout), _O_BINARY);
			(void)_setmode (_fileno (stderr), _O_BINARY);
		}
#endif
		/* note: same message in call.c */
		cob_runtime_error (_("cob_init() has not been called"));
		break;
	/* LCOV_EXCL_START */
	case COB_FERROR_CODEGEN:
		cob_runtime_error ("codegen error");	/* not translated by intent */
		cob_runtime_error (_("Please report this!"));
		break;
	/* LCOV_EXCL_STOP */
	/* Note: can be simply tested; therefore no exclusion */
	case COB_FERROR_CHAINING:
		cob_runtime_error (_("CALL of program with CHAINING clause"));
		break;
	/* LCOV_EXCL_START */
	case COB_FERROR_STACK:
		cob_runtime_error (_("stack overflow, possible PERFORM depth exceeded"));
		break;
	/* LCOV_EXCL_STOP */
	/* Note: can be simply tested (GO TO DECLARATIVES); therefore no exclusion */
	case COB_FERROR_GLOBAL:
		cob_runtime_error (_("invalid entry/exit in GLOBAL USE procedure"));
		break;
	/* LCOV_EXCL_START */
	case COB_FERROR_MEMORY:
		cob_runtime_error (_("unable to allocate memory"));
		break;
	/* LCOV_EXCL_STOP */
	/* LCOV_EXCL_START */
	case COB_FERROR_MODULE:
		cob_runtime_error (_("invalid entry into module"));
		break;
	/* LCOV_EXCL_STOP */
	/* Note: can be simply tested; therefore no exclusion */
	case COB_FERROR_RECURSIVE:
		/* LCOV_EXCL_LINE */
		if (cob_module_err) {
			cob_runtime_error (_("recursive CALL from '%s' to '%s' which is NOT RECURSIVE"),
					COB_MODULE_PTR->module_name, cob_module_err->module_name);
		/* LCOV_EXCL_START */
		/* Note: only in for old modules - not active with current generation */
		} else {
			cob_runtime_error (_("invalid recursive COBOL CALL to '%s'"),
					   COB_MODULE_PTR->module_name);
		}
		/* LCOV_EXCL_STOP */
		break;
	/* LCOV_EXCL_START */
	case COB_FERROR_FREE:
		cob_runtime_error (_("call to %s with NULL pointer"), "cob_free");
		break;
	/* LCOV_EXCL_STOP */
	case COB_FERROR_FILE:
		file_status = cobglobptr->cob_error_file->file_status;
		status = COB_D2I (file_status[0]) * 10 + COB_D2I (file_status[1]);
		switch (status) {
		case COB_STATUS_10_END_OF_FILE:
			msg = _("end of file");
			break;
		case COB_STATUS_14_OUT_OF_KEY_RANGE:
			msg = _("key out of range");
			break;
		case COB_STATUS_21_KEY_INVALID:
			msg = _("key order not ascending");
			break;
		case COB_STATUS_22_KEY_EXISTS:
			msg = _("record key already exists");
			break;
		case COB_STATUS_23_KEY_NOT_EXISTS:
			msg = _("record key does not exist");
			break;
		case COB_STATUS_30_PERMANENT_ERROR:
			msg = _("permanent file error");
			break;
		case COB_STATUS_31_INCONSISTENT_FILENAME:
			msg = _("inconsistent file name");
			break;
		case COB_STATUS_35_NOT_EXISTS:
			msg = _("file does not exist");
			break;
		case COB_STATUS_37_PERMISSION_DENIED:
			msg = _("permission denied");
			break;
		case COB_STATUS_39_CONFLICT_ATTRIBUTE:
			msg = _("mismatch of fixed file attributes");
			break;
		case COB_STATUS_41_ALREADY_OPEN:
			msg = _("file already open");
			break;
		case COB_STATUS_42_NOT_OPEN:
			msg = _("file not open");
			break;
		case COB_STATUS_43_READ_NOT_DONE:
			msg = _("READ must be executed first");
			break;
		case COB_STATUS_44_RECORD_OVERFLOW:
			msg = _("record overflow");
			break;
		case COB_STATUS_46_READ_ERROR:
			msg = _("READ after unsuccessful READ/START");
			break;
		case COB_STATUS_47_INPUT_DENIED:
			msg = _("READ/START not allowed, file not open for input");
			break;
		case COB_STATUS_48_OUTPUT_DENIED:
			msg = _("WRITE not allowed, file not open for output");
			break;
		case COB_STATUS_49_I_O_DENIED:
			msg = _("DELETE/REWRITE not allowed, file not open for I-O");
			break;
		case COB_STATUS_51_RECORD_LOCKED:
			msg = _("record locked by another file connector");
			break;
		case COB_STATUS_57_I_O_LINAGE:
			msg = _("LINAGE values invalid");
			break;
		case COB_STATUS_61_FILE_SHARING:
			msg = _("file sharing conflict");
			break;
		case COB_STATUS_71_BAD_CHAR:
			msg = _("invalid data in LINE SEQUENTIAL file");
			break;
		/* LCOV_EXCL_START */
		case COB_STATUS_91_NOT_AVAILABLE:
			msg = _("runtime library is not configured for this operation");
			break;
		/* LCOV_EXCL_STOP */
		/* LCOV_EXCL_START */
		default:
			msg = _("unknown file error");
			break;
		/* LCOV_EXCL_STOP */
		}
		err_cause = cob_get_filename_print (cobglobptr->cob_error_file, 1);
		/* FIXME: additional check if referenced program has active code location */
		if (cobglobptr->last_exception_statement == STMT_UNKNOWN) {
			cob_runtime_error (_("%s (status = %02d) for file %s"),
				msg, status, err_cause);
		} else {
			cob_runtime_error (_("%s (status = %02d) for file %s on %s"),
				msg, status, err_cause,
				cob_statement_name[cobglobptr->last_exception_statement]);
		}
		break;
	/* LCOV_EXCL_START */
	case COB_FERROR_FUNCTION:
		cob_runtime_error (_("attempt to use non-implemented function"));
		break;
	case COB_FERROR_XML:
		cob_runtime_error (_("attempt to use non-implemented XML I/O"));
		break;
	case COB_FERROR_JSON:
		cob_runtime_error (_("attempt to use non-implemented JSON I/O"));
		break;		
	default:
		/* internal rare error, no need for translation */
		cob_runtime_error ("unknown failure: %d", fatal_error);
		break;
	/* LCOV_EXCL_STOP */
	}
	cob_hard_failure ();
}

static void
conf_runtime_error_value (const char *value, const int pos)
{
	const char *name = NULL;

	if (gc_conf[pos].data_type & STS_CNFSET) {
		name = gc_conf[pos].conf_name;
	} else {
		name = gc_conf[pos].env_name;
	}
	conf_runtime_error (0, _("invalid value '%s' for configuration tag '%s'"), value, name);
}

static void
conf_runtime_error (const int finish_error, const char *fmt, ...)
{
	va_list args;

	if (!conf_runtime_error_displayed) {
		conf_runtime_error_displayed = 1;
		fputs (_("configuration error:"), stderr);
		fputc ('\n', stderr);
	}

	/* Prefix; note: no need to strcmp as we check against
	           the value passed last time */
	if (cob_source_file != last_runtime_error_file
	 || cob_source_line != last_runtime_error_line) {
		last_runtime_error_file = cob_source_file;
		last_runtime_error_line = cob_source_line;
		if (cob_source_file) {
			fprintf (stderr, "%s", cob_source_file);
			if (cob_source_line) {
				fprintf (stderr, ":%u", cob_source_line);
			}
		} else {
			fprintf (stderr, "%s", _("environment variables"));
		}
		fputc(':', stderr);
		fputc(' ', stderr);
	}

	/* Body */
	va_start (args, fmt);
	vfprintf (stderr, fmt, args);
	va_end (args);

	/* Postfix */
	if (!finish_error) {
		fputc (';', stderr);
		fputc ('\n', stderr);
		fputc ('\t', stderr);
	} else {
		fputc ('\n', stderr);
		fflush (stderr);
	}
}

#if defined (COB_GEN_SCREENIO)
/* resolve curses library related version information
   stores the information in the version_buffer parameter
   returns the mouse info */
static const char *
get_screenio_and_mouse_info (char *version_buffer, size_t size, const int verbose)
{
	const char	*mouse_support = _("unknown");
	int	major, minor, patch;
#if defined (__PDCURSES__)
	int	opt1, opt2, opt3;
#if defined (PDC_FORCE_UTF8)
	const int utf8 = 1;
#else
	const int utf8 = 0;
#endif
#endif
#if defined (__PDCURSES__) || defined (NCURSES_VERSION)
#if defined (PDC_WIDE) || defined (NCURSES_WIDECHAR)
	const int wide = 1;
#else
	const int wide = 0;
#endif
#endif
	char	buff[56] = {'\0'};

	memset (version_buffer, 0, size--);

	if (verbose) {
		initscr ();
	}
#ifdef HAVE_HAS_MOUSE
	if (verbose) {
		int mouse_available = 0;
		mousemask (ALL_MOUSE_EVENTS, NULL);
		if (has_mouse () == TRUE) mouse_available = 1;
		if (mouse_available) {
			mouse_support = _("yes");
		} else {
			mouse_support = _("no");
		}
	}
#elif defined (HAVE_MOUSEMASK)
#if defined (__PDCURSES__)
	/* CHECKME: that looks wrong - can't we test as above?
	   Double check with older PDCurses! */
	mouse_support = _("yes");
#endif
#else
	mouse_support = _("disabled");
#endif

#if defined (__PDCURSES__) || defined (NCURSES_VERSION)
#if defined (__PDCURSES__)
#if defined (PDC_VER_MAJOR)
#define CURSES_CMP_MAJOR	PDC_VER_MAJOR
#define CURSES_CMP_MINOR	PDC_VER_MINOR
#if PDC_VER_MAJOR > 3 || (PDC_VER_MAJOR == 3 && PDC_BUILD >= 3703)
#define RESOLVED_PDC_VER
	{
		PDC_VERSION ver;
		PDC_get_version (&ver);
		major = ver.major;
		minor = ver.minor;
#ifdef __PDCURSESMOD__
		patch = ver.change;		/* note: PDCursesMod has an extra field */
#else
		patch = ver.build % 100;	/* note: PDCurses has it only "embedded" here */
#endif
		opt1 = ver.csize * 8;
		opt2 = ver.flags & PDC_VFLAG_WIDE;
		opt3 = ver.flags & PDC_VFLAG_UTF8;
	}
#else
	COB_UNUSED (opt1);
	COB_UNUSED (opt2);
	COB_UNUSED (opt3);
#endif
#else
#define CURSES_CMP_MAJOR	(PDC_BUILD / 1000)
#define CURSES_CMP_MINOR	(PDC_BUILD - CURSES_CMP_MAJOR * 1000) / 100
	COB_UNUSED (opt1);
	COB_UNUSED (opt2);
	COB_UNUSED (opt3);
#endif
#elif defined (NCURSES_VERSION)
#define CURSES_CMP_MAJOR	NCURSES_VERSION_MAJOR
#define CURSES_CMP_MINOR	NCURSES_VERSION_MINOR
#endif
#if !defined (RESOLVED_PDC_VER)
	snprintf (version_buffer, size, "%s", curses_version ());
	major = 0, minor = 0, patch = 0;
	if ((sscanf (version_buffer, "%55s %55s %d.%d.%d", (char *)&buff, (char *)&buff, &major, &minor, &patch) < 4)
	 && (sscanf (version_buffer, "%55s %d.%d.%d", (char *)&buff, &major, &minor, &patch) < 3)
	 && (sscanf (version_buffer, "%d.%d.%d", &major, &minor, &patch) < 2)) {
		major = 0, minor = 0;
	}
#endif
	if (major == CURSES_CMP_MAJOR && minor == CURSES_CMP_MINOR) {
		snprintf (buff, 55, _("%s, version %d.%d.%d"), WITH_CURSES, major, minor, patch);
	} else if (major != 0) {
		snprintf (buff, 55, _("%s, version %d.%d.%d (compiled with %d.%d)"),
			WITH_CURSES, major, minor, patch, CURSES_CMP_MAJOR, CURSES_CMP_MINOR);
	} else {
		snprintf (buff, 55, _("%s, version %s"), WITH_CURSES, version_buffer);
	}
#if defined (RESOLVED_PDC_VER) 
	{
		const int	chtype_val = (int)sizeof (chtype) * 8;
		char	chtype_def[10] = { '\0' };
		char	wide_def[6] = {'\0'};
		char	utf8_def[6] = {'\0'};
		const char	*match;
		if (chtype_val != opt1) {
			match = "!";
		} else {
			match = "";
		}
		snprintf (chtype_def, 9, "%d[%d%s]", chtype_val, opt1, match);
		if (wide != opt2) {
			match = "!";
		} else {
			match = "";
		}
		snprintf (wide_def, 5, "%d[%d%s]", wide, opt2, match);
		if (wide != opt2) {
			match = "!";
		} else {
			match = "";
		}
		snprintf (utf8_def, 5, "%d[%d%s]", utf8, opt3, match);
		snprintf (version_buffer, size, "%s (CHTYPE=%s, WIDE=%s, UTF8=%s)",
			buff, chtype_def, wide_def, utf8_def);
	}
#undef RESOLVED_PDC_VER
#elif defined (__PDCURSES__)
	snprintf (version_buffer, size, "%s (CHTYPE=%d, WIDE=%d, UTF8=%d)", buff,
		(int)sizeof (chtype) * 8, wide, utf8);
#else
	snprintf (version_buffer, size, "%s (CHTYPE=%d, WIDE=%d)", buff,
		(int)sizeof (chtype) * 8, wide);
#endif

#else /* defined (__PDCURSES__) || defined (NCURSES_VERSION) */
	snprintf (version_buffer, size, "%s (CHTYPE=%d)", WITH_CURSES,
		(int)sizeof (chtype) * 8);
#endif

	if (verbose) {
		size_t curr_size = strlen (version_buffer);
		snprintf (version_buffer + curr_size, size - curr_size, " %s",
			longname ());
		endwin ();
	}

	return mouse_support;
}
#endif

/* resolve math library related version information
   stores the information in the version_buffer parameter */
static void
get_math_info (char *version_buffer, size_t size, const int verbose)
{
	int	major, minor, patch;
	size_t	curr_size;
	COB_UNUSED (verbose);

	memset (version_buffer, 0, size--);
	major = 0, minor = 0, patch = 0;
	(void)sscanf (gmp_version, "%d.%d.%d", &major, &minor, &patch);
	if (major == __GNU_MP_VERSION && minor == __GNU_MP_VERSION_MINOR) {
		curr_size = snprintf (version_buffer, size, _("%s, version %d.%d.%d"), "GMP", major, minor, patch);
	} else {
		curr_size = snprintf (version_buffer, size, _("%s, version %d.%d.%d (compiled with %d.%d)"),
			"GMP", major, minor, patch, __GNU_MP_VERSION, __GNU_MP_VERSION_MINOR);
	}
#if defined (mpir_version)
	major = 0, minor = 0, patch = 0;
	(void)sscanf (mpir_version, "%d.%d.%d", &major, &minor, &patch);
	{
		const char *deli = " - ";
		curr_size += snprintf (version_buffer + curr_size, size - curr_size, "%s", deli);
	}

	if (major == __MPIR_VERSION && minor == __MPIR_VERSION_MINOR) {
		snprintf (version_buffer + curr_size, size - curr_size,
			_("%s, version %d.%d.%d"),
			"MPIR", major, minor, patch);
	} else {
		snprintf (version_buffer + curr_size, size - curr_size,
			_("%s, version %d.%d.%d (compiled with %d.%d)"),
			"MPIR", major, minor, patch, __MPIR_VERSION, __MPIR_VERSION_MINOR);
	}
#else
	COB_UNUSED (curr_size);
#endif
}

/* internal library version as string,
   note: the patchlevel may differ from the package one */
const char* libcob_version () {

/* FIXME: replace this define by a general one (COB_TREE_DEBUG) _was_ for debugging
          the parse tree only ... */
#if defined (COB_TREE_DEBUG) || defined (_DEBUG)
	{
		int	major, minor;
		major = 0, minor = 0;
		(void)sscanf (PACKAGE_VERSION, "%d.%d", &major, &minor);
		/* LCOV_EXCL_START */
		if (major != __LIBCOB_VERSION || minor != __LIBCOB_VERSION_MINOR) {
			const char* version = CB_XSTRINGIFY (__LIBCOB_VERSION) "."
				CB_XSTRINGIFY (__LIBCOB_VERSION_MINOR);
			cob_runtime_error (_("version mismatch"));
			cob_runtime_hint (_("%s has version %s.%d"), "libcob internally",
						version, __LIBCOB_VERSION_PATCHLEVEL);
			cob_runtime_hint (_("%s has version %s.%d"), "libcob package",
						PACKAGE_VERSION, PATCH_LEVEL);
			cob_hard_failure ();
		}
		/* LCOV_EXCL_STOP */
		{
			int check, patch;
			patch = 0;
			check = set_libcob_version (&major, &minor, &patch);
			/* LCOV_EXCL_START */
			if (check != 0 && check != 3) {
				cob_runtime_error (_("version mismatch"));
				/* untranslated as it is very unlikely to happen */
				cob_runtime_hint ("internal version check differs at %d\n", check);
				cob_hard_failure ();
			}
			/* LCOV_EXCL_STOP */
		}
	}
#endif
	return CB_XSTRINGIFY (__LIBCOB_VERSION) "."
		CB_XSTRINGIFY (__LIBCOB_VERSION_MINOR) "."
		CB_XSTRINGIFY (__LIBCOB_VERSION_PATCHLEVEL);
}

/* internal library version set/compare,
   if 'mayor' is not 0 on entry compares against the given
   values, returns the parameter that is not matching
   given parameters will be set to the internal values on exit
   note: the patchlevel may differ from the package one */
int set_libcob_version (int *mayor, int *minor, int *patch) {
	int ret = 0;
	if (*mayor != 0) {
		if (*mayor != __LIBCOB_VERSION) {
			ret = 1;
		} else if (*minor != __LIBCOB_VERSION_MINOR) {
			ret = 2;
		} else if (*patch != __LIBCOB_VERSION_PATCHLEVEL) {
			ret = 3;
		}
	}
	*mayor = __LIBCOB_VERSION;
	*minor = __LIBCOB_VERSION_MINOR;
	*patch = __LIBCOB_VERSION_PATCHLEVEL;
	return ret;
}

static void set_cob_build_stamp (char *cob_build_stamp)
{
	int		status, day, year;
	char	month[64];

	/* Set up build time stamp */
	memset (cob_build_stamp, 0, (size_t)COB_MINI_BUFF);
	memset (month, 0, sizeof (month));
	day = 0;
	year = 0;
	status = sscanf (__DATE__, "%63s %d %d", month, &day, &year);
	if (status == 3) {
		snprintf (cob_build_stamp, (size_t)COB_MINI_MAX,
			"%s %2.2d %4.4d %s", month, day, year, __TIME__);
	} else {
		snprintf (cob_build_stamp, (size_t)COB_MINI_MAX,
			"%s %s", __DATE__, __TIME__);
	}
}

/* provides a two line output for GnuCOBOL + C compiler and used libraries */
void
print_version_summary (void)
{
	char	cob_build_stamp[COB_MINI_BUFF];

	set_cob_build_stamp (cob_build_stamp);
	
	printf ("%s %s (%s), ",
		PACKAGE_NAME, libcob_version(), cob_build_stamp);

	/* note: some compilers use a very long identifier */
	printf ("%s\n", GC_C_VERSION_PRF GC_C_VERSION);

	printf ("%s %d.%d.%d",
#ifdef __MPIR_VERSION
		"MPIR", __MPIR_VERSION, __MPIR_VERSION_MINOR, __MPIR_VERSION_PATCHLEVEL
#else
		"GMP", __GNU_MP_VERSION, __GNU_MP_VERSION_MINOR, __GNU_MP_VERSION_PATCHLEVEL
#endif
	);

#if defined (LIBXML_VERSION)
	printf (", libxml2 %d.%d.%d",
		LIBXML_VERSION / 10000,
		(LIBXML_VERSION - (int)(LIBXML_VERSION / 10000) * 10000) / 100,
		LIBXML_VERSION % 100);
#endif

#if defined (CJSON_VERSION_MAJOR)
	printf (", cJSON %d.%d.%d",
		CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH);
#endif
#if defined (JSON_C_MAJOR_VERSION)
	printf (", JSON-c %d.%d.%d",
		JSON_C_MAJOR_VERSION, JSON_C_MINOR_VERSION, JSON_C_MICRO_VERSION);
#endif
#if defined (__PDCURSES__)
	printf (", %s %d.%d",
#ifdef __PDCURSESMOD__
		"PDCursesMod",
#else
		"PDCurses",
#endif
		CURSES_CMP_MAJOR, CURSES_CMP_MINOR);
#ifdef PDC_VER_CHANGE
	printf (".%d", PDC_VER_CHANGE);
#elif defined (PDC_BUILD)
	printf (" (%d)", PDC_BUILD);
#endif
#endif
#if defined (NCURSES_VERSION_MAJOR)
	printf (", %s %d.%d.%d",
#ifdef NCURSES_WIDECHAR
		"ncursesw",
#else
		"ncurses",
#endif
		NCURSES_VERSION_MAJOR, NCURSES_VERSION_MINOR, NCURSES_VERSION_PATCH);
#endif

#if defined	(WITH_DB)
	printf (", BDB %d.%d.%d",
		DB_VERSION_MAJOR, DB_VERSION_MINOR, DB_VERSION_PATCH);
#endif
#if defined	(WITH_CISAM)
	printf (", C-ISAM");
#endif
#if defined	(WITH_DISAM)
	printf (", D-ISAM");
#endif
#if defined	(WITH_VBISAM)
	printf (", VB-ISAM");
#endif
	putchar ('\n');

}

void
print_version (void)
{
	char	cob_build_stamp[COB_MINI_BUFF];

	set_cob_build_stamp (cob_build_stamp);

	printf ("libcob (%s) %s.%d\n",
		PACKAGE_NAME, PACKAGE_VERSION, PATCH_LEVEL);
	puts ("Copyright (C) 2023 Free Software Foundation, Inc.");
	printf (_("License LGPLv3+: GNU LGPL version 3 or later <%s>"), "https://gnu.org/licenses/lgpl.html");
	putchar ('\n');
	puts (_("This is free software; see the source for copying conditions.  There is NO\n"
	        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."));
	printf (_("Written by %s"), "Keisuke Nishida, Roger While, Ron Norman, Simon Sobisch, Edward Hart");
	putchar ('\n');

	/* TRANSLATORS: This msgid is intended as the "Packaged" msgid, %s expands to date and time */
	printf (_("Built     %s"), cob_build_stamp);
	putchar ('\n');
	/* TRANSLATORS: This msgid is intended as the "Built" msgid, %s expands to date and time */
	printf (_("Packaged  %s"), COB_TAR_DATE);
	putchar ('\n');
}

void
print_info (void)
{
	print_info_detailed (0);
}

void
print_info_detailed (const int verbose)
{
	char	screenio_info[150];
	const char *mouse_support;

	char	buff[56] = { '\0' };
	char	*s;

	/* resolving screenio related information before anything else as this
	   function will possibly run initscr + endwin and therefore
	   may interfer with other output */
#if defined (COB_GEN_SCREENIO)
	mouse_support = get_screenio_and_mouse_info
		((char*)&screenio_info, sizeof (screenio_info), verbose);
#else
	snprintf ((char *)&screenio_info, sizeof(screenio_info) - 1,
		"%s", _("disabled"));
	mouse_support = _("disabled");
#endif

	print_version ();
	putchar ('\n');
	puts (_("build information"));
	var_print (_("build environment"), 	COB_BLD_BUILD, "", 0);
	var_print ("CC", COB_BLD_CC, "", 0);
	/* Note: newline because most compilers define a long version string (> 30 characters) */
	var_print (_("C version"), GC_C_VERSION_PRF GC_C_VERSION, "", 0);
	var_print ("CPPFLAGS", COB_BLD_CPPFLAGS, "", 0);
	var_print ("CFLAGS", COB_BLD_CFLAGS, "", 0);
	var_print ("LD", COB_BLD_LD, "", 0);
	var_print ("LDFLAGS", COB_BLD_LDFLAGS, "", 0);
	putchar ('\n');

	puts (_("GnuCOBOL information"));

	var_print ("COB_MODULE_EXT", COB_MODULE_EXT, "", 0);
#if 0 /* only relevant for cobc */
	var_print ("COB_OBJECT_EXT", COB_OBJECT_EXT, "", 0);
	var_print ("COB_EXE_EXT", COB_EXE_EXT, "", 0);
#endif

#if	defined (USE_LIBDL) || defined (_WIN32)
	var_print (_("dynamic loading"), 	"system", "", 0);
#else
	var_print (_("dynamic loading"), 	"libtool", "", 0);
#endif

	if (verbose) {
#ifdef	COB_PARAM_CHECK
		var_print ("\"CBL_\" param check", 	_("enabled"), "", 0);
#else
		var_print ("\"CBL_\" param check", 	_("disabled"), "", 0);
#endif
	}
#ifdef COB_64_BIT_POINTER
	var_print ("64bit-mode", 	_("yes"), "", 0);
#else
	var_print ("64bit-mode", 	_("no"), "", 0);
#endif

#ifdef	COB_LI_IS_LL
	var_print ("BINARY-C-LONG", 	_("8 bytes"), "", 0);
#else
	var_print ("BINARY-C-LONG", 	_("4 bytes"), "", 0);
#endif

#ifdef WORDS_BIGENDIAN
	var_print (_("endianness"),		_("big-endian"), "", 0);
#else
	var_print (_("endianness"),		_("little-endian"), "", 0);
#endif

#ifdef COB_EBCDIC_MACHINE
	var_print (_("native EBCDIC"),		_("yes"), "", 0);
#else
	var_print (_("native EBCDIC"),		_("no"), "", 0);
#endif

	snprintf (buff, sizeof (buff), "%d", WITH_VARSEQ);
	var_print (_("variable file format"), buff, "", 0);
	if ((s = getenv ("COB_VARSEQ_FORMAT")) != NULL) {
		var_print ("COB_VARSEQ_FORMAT", s, "", 1);
	}

#ifdef	WITH_SEQRA_EXTFH
	var_print (_("sequential file handler"),	"EXTFH", "", 0);
#else
	var_print (_("sequential file handler"),	_("built-in"), "", 0);
#endif

#if defined	(WITH_INDEX_EXTFH)
	var_print (_("indexed file handler"), 		"EXTFH", "", 0);
#elif defined	(WITH_DB)
	{
		int	major, minor, patch;
		major = 0, minor = 0, patch = 0;
		db_version (&major, &minor, &patch);
		if (major == DB_VERSION_MAJOR && minor == DB_VERSION_MINOR) {
			snprintf (buff, 55, _("%s, version %d.%d.%d"),
				"BDB", major, minor, patch);
		} else {
			snprintf (buff, 55, _("%s, version %d.%d.%d (compiled with %d.%d)"),
				"BDB", major, minor, patch, DB_VERSION_MAJOR, DB_VERSION_MINOR);
		}
	}
	var_print (_("indexed file handler"), 		buff, "", 0);
#elif defined	(WITH_CISAM)
	var_print (_("indexed file handler"), 		"C-ISAM", "", 0);
#elif defined	(WITH_DISAM)
	var_print (_("indexed file handler"), 		"D-ISAM", "", 0);
#elif defined	(WITH_VBISAM)
#if defined	(VB_RTD)
	var_print (_("indexed file handler"), 		"VBISAM (RTD)", "", 0);
#else
	var_print (_("indexed file handler"), 		"VBISAM", "", 0);
#endif
#else
	var_print (_("indexed file handler"), 		_("disabled"), "", 0);
#endif

	{
		char	math_info[115];
		get_math_info ((char*)&math_info, sizeof (math_info), verbose);
		var_print (_("mathematical library"), 	(char *)&math_info, "", 0);
	}

#ifdef WITH_XML2
	{
		int	major, minor, patch;
		major = LIBXML_VERSION / 10000;
		minor = (LIBXML_VERSION - major * 10000) / 100 ;
		patch = LIBXML_VERSION - major * 10000 - minor * 100;
		snprintf (buff, 55, _("%s, version %d.%d.%d"),
			"libxml2", major, minor, patch);
		var_print (_("XML library"), 		buff, "", 0);
		LIBXML_TEST_VERSION
		xmlCleanupParser ();
	}
#else
	var_print (_("XML library"), 		_("disabled"), "", 0);
#endif


#if defined (WITH_CJSON)
	{
		int	major, minor, patch;
		major = 0, minor = 0, patch = 0;
		(void)sscanf (cJSON_Version(), "%d.%d.%d", &major, &minor, &patch);
		if (major == CJSON_VERSION_MAJOR && minor == CJSON_VERSION_MINOR) {
			snprintf (buff, 55, _("%s, version %d.%d.%d"),
				"cJSON", major, minor, patch);
		} else {
			snprintf (buff, 55, _("%s, version %d.%d.%d (compiled with %d.%d)"),
				"cJSON", major, minor, patch, CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR);
		}
	}
	var_print (_("JSON library"), 		buff, "", 0);

#elif defined (WITH_JSON_C)
	{
		int	major, minor, patch;
		major = 0, minor = 0, patch = 0;
		(void)sscanf (json_c_version (), "%d.%d.%d", &major, &minor, &patch);
		if (major == JSON_C_MAJOR_VERSION && minor == JSON_C_MINOR_VERSION) {
			snprintf (buff, 55, _("%s, version %d.%d.%d"),
				"json-c", major, minor, patch);
		} else {
			snprintf (buff, 55, _("%s, version %d.%d.%d (compiled with %d.%d)"),
				"json-c", major, minor, patch, JSON_C_MAJOR_VERSION, JSON_C_MINOR_VERSION);
		}
	}
	var_print (_("JSON library"), 		buff, "", 0);
#else
	var_print (_("JSON library"), 		_("disabled"), "", 0);
#endif

	var_print (_("extended screen I/O"),	(char*)&screenio_info, "", 0);
	var_print (_("mouse support"),		mouse_support, "", 0);

#ifdef COB_DEBUG_LOG
	var_print ("DEBUG_LOG",		_("enabled"), "", 0);
#endif
}

void
print_runtime_conf ()
{
	unsigned int 	i, j, k, vl, dohdg, hdlen, plen, plen2;
	char	value[COB_MEDIUM_BUFF], orgvalue[COB_MINI_BUFF];

#ifdef ENABLE_NLS	/* note: translated version of definition values */
#ifdef	HAVE_SETLOCALE
	const char	*s;
#endif
	setting_group[1] = _("CALL configuration");
	setting_group[2] = _("File I/O configuration");
	setting_group[3] = _("Screen I/O configuration");
	setting_group[4] = _("Miscellaneous");
	setting_group[5] = _("System configuration");
#endif

	printf ("%s %s.%d ", PACKAGE_NAME, PACKAGE_VERSION, PATCH_LEVEL);
	puts (_("runtime configuration"));
	if (cobsetptr->cob_config_file) {
		strncpy (value, _("via"), (size_t)COB_MEDIUM_MAX);
		value[COB_MEDIUM_MAX] = 0;
		hdlen = (unsigned int)strlen (value) + 3;

		/* output path of main configuration file */
		printf (" %s  ", value);
		plen = 80 - hdlen;
		strncpy (value, cobsetptr->cob_config_file[0], (size_t)COB_MEDIUM_MAX);
		value[COB_MEDIUM_MAX] = 0;
		vl = (unsigned int)strlen (value);
		for (k = 0; vl > plen; vl -= plen, k += plen) {
			printf ("%.*s\n%-*s", plen, &value[k], hdlen, "");
		}
		printf ("%s\n", &value[k]);

		/* output path of additional configuration files */
		for (i = 1; i < cobsetptr->cob_config_num; i++) {
			printf ("%*d  ", hdlen - 2, i);
			strncpy (value, cobsetptr->cob_config_file[i], (size_t)COB_MEDIUM_MAX);
			value[COB_MEDIUM_MAX] = 0;
			vl = (unsigned int)strlen (value);
			for (k = 0; vl > plen; vl -= plen, k += plen) {
				printf ("%.*s\n%-*s", plen, &value[k], hdlen, "");
			}
			printf ("%s\n", &value[k]);
		}

	}
	putchar ('\n');
	strcpy (value, "todo");
	hdlen = 15;
	for (i = 0; i < NUM_CONFIG; i++) {
		j = (unsigned int)strlen (gc_conf[i].env_name);
		if (j > hdlen)
			hdlen = j;
		j = (unsigned int)strlen (gc_conf[i].conf_name);
		if (j > hdlen)
			hdlen = j;
	}

	for (j = 1; j < GRP_MAX; j++) {
		dohdg = 1;
		for (i = 0; i < NUM_CONFIG; i++) {
			if (gc_conf[i].env_group == j) {
				if (dohdg) {
					dohdg = 0;
					if (j > 1) {
						putchar ('\n');
					}
					printf (" %s\n", setting_group[j]);
				}
				/* Convert value back into string and display it */
				get_config_val (value, i, orgvalue);
				if ((gc_conf[i].data_type & STS_ENVSET)
				 || (gc_conf[i].data_type & STS_FNCSET)) {
					putchar (' ');
					if (gc_conf[i].data_type & STS_FNCSET) {
						printf ("   ");
					} else
					if (gc_conf[i].data_type & STS_CNFSET) {
						printf ("Ovr");
					} else {
						printf ("env");
						if (gc_conf[i].data_loc == offsetof(cob_settings,cob_preload_str)
						 && cobsetptr->cob_preload_str_set != NULL) {
							printf (": %-*s : ", hdlen, gc_conf[i].env_name);
							printf ("%s\n", cobsetptr->cob_preload_str_set);
							printf ("eval");
						}
					}
					printf (": %-*s : ", hdlen, gc_conf[i].env_name);
				} else
				if (gc_conf[i].data_type & STS_CNFSET) {
					if ((gc_conf[i].data_type & STS_ENVCLR)) {
						printf ("    : %-*s : ", hdlen, gc_conf[i].env_name);
						puts (_("... removed from environment"));
					}
					if (gc_conf[i].config_num > 0) {
						printf ("  %d ", gc_conf[i].config_num);
					} else {
						printf ("    ");
					}
					if (gc_conf[i].data_loc == offsetof(cob_settings,cob_preload_str)
					 && cobsetptr->cob_preload_str_set != NULL) {
						printf (": %-*s : ",hdlen,
							gc_conf[i].set_by > 0 ? gc_conf[i].env_name
							: gc_conf[i].conf_name);
						printf ("%s\n",cobsetptr->cob_preload_str_set);
						printf ("eval");
					}
					if (gc_conf[i].set_by > 0) {
						printf (": %-*s : ", hdlen, gc_conf[i].env_name);
					} else {
						printf (": %-*s : ", hdlen, gc_conf[i].conf_name);
					}
				} else if (gc_conf[i].env_name) {
					if (gc_conf[i].config_num > 0){
						printf ("  %d ", gc_conf[i].config_num);
					} else {
						printf ("    ");
					}
					printf (": %-*s : ", hdlen, gc_conf[i].env_name);
					if ((gc_conf[i].data_type & STS_ENVCLR)) {
						puts (_("... removed from environment"));
						continue;
					}
				} else {
					printf ("    : %-*s : ", hdlen, gc_conf[i].conf_name);
				}
				vl = (unsigned int)strlen (value);
				plen = 71 - hdlen;
				if (vl < min_conf_length) {
					plen2 = min_conf_length - vl;
				} else if (vl == min_conf_length) {
					plen2 = 1;
				} else {
					plen2 = 0;
				}
				for (k = 0; vl > plen; vl -= plen, k += plen) {
					printf ("%.*s\n      %-*s : ", plen, &value[k], hdlen, "");
				}
				printf ("%s", &value[k]);
				printf ("%.*s", plen2, "               ");
				if (orgvalue[0]) {
					printf (" (%s)", orgvalue);
				}
				if (gc_conf[i].set_by != 0) {
					putchar (' ');
					if (gc_conf[i].set_by != FUNC_NAME_IN_DEFAULT) {
						printf (_("(set by %s)"), gc_conf[gc_conf[i].set_by].env_name);
					} else {
						printf (_("(set by %s)"), gc_conf[i].default_val);
					}
				}
				if (!(gc_conf[i].data_type & STS_ENVSET)
				 && !(gc_conf[i].data_type & STS_CNFSET)
				 && !(gc_conf[i].data_type & STS_FNCSET)) {
					putchar (' ');
					if ((gc_conf[i].data_type & STS_RESET)) {
						printf (_("(reset)"));
					} else
					if (strcmp (value, not_set) != 0) {
						printf (_("(default)"));
					} else
					if (gc_conf[i].default_val
					 && strcmp (gc_conf[i].default_val, not_set) == 0) {
						printf (_("(default)"));
					}
				}
				putchar ('\n');
			}
		}
	}


#ifdef	HAVE_SETLOCALE
#ifdef	ENABLE_NLS
	s = getenv ("LOCALEDIR");
	printf ("    : %-*s : %s\n", hdlen, "LOCALEDIR", s ? s : LOCALEDIR);
#endif
	printf ("    : %-*s : %s\n", hdlen, "LC_CTYPE", setlocale (LC_CTYPE, NULL));
	printf ("    : %-*s : %s\n", hdlen, "LC_NUMERIC", setlocale (LC_NUMERIC, NULL));
	printf ("    : %-*s : %s\n", hdlen, "LC_COLLATE", setlocale (LC_COLLATE, NULL));
#ifdef	LC_MESSAGES
	printf ("    : %-*s : %s\n", hdlen, "LC_MESSAGES", setlocale (LC_MESSAGES, NULL));
#endif
	printf ("    : %-*s : %s\n", hdlen, "LC_MONETARY", setlocale (LC_MONETARY, NULL));
	printf ("    : %-*s : %s\n", hdlen, "LC_TIME", setlocale (LC_TIME, NULL));
#endif
}

cob_settings *
cob_get_settings_ptr ()
{
	return cobsetptr;
}

void
cob_init_nomain (const int argc, char **argv)
{
	check_mainhandle = 0;
	cob_init (argc, argv);
}

void
cob_common_init (void *setptr)
{
#ifdef	ENABLE_NLS
	{
		struct stat	localest;
		const char * localedir;

		localedir = getenv ("LOCALEDIR");
		if (localedir != NULL
		 && !stat (localedir, &localest)
		 && (S_ISDIR (localest.st_mode))) {
			bindtextdomain (PACKAGE, localedir);
		} else {
			bindtextdomain (PACKAGE, LOCALEDIR);
		}
		textdomain (PACKAGE);
	}
#endif

#ifdef	_WIN32
	/* Allows running tests under Win */
	{
		int use_unix_lf = 0;
		char *s = getenv ("COB_UNIX_LF");

		if (s != NULL) {
			if (setptr) {
				set_config_val_by_name (s, "unix_lf", NULL);
				use_unix_lf = cobsetptr->cob_unix_lf;
			} else
			if (*s == 'Y' || *s == 'y' ||
			    *s == 'O' || *s == 'o' ||
			    *s == 'T' || *s == 't' ||
			    *s == '1') {
				use_unix_lf = 1;
			}
		}
		if (use_unix_lf) {
			(void)_setmode (_fileno (stdin), _O_BINARY);
			(void)_setmode (_fileno (stdout), _O_BINARY);
			(void)_setmode (_fileno (stderr), _O_BINARY);
		}
	}
#endif
}

/* normal call, but longjmp back on exit (1),
   errors (-1), hard errors (-2) or signals (-3) */
int
cob_call_with_exception_check (const char *name, const int argc, void **argv)
{	
#ifndef COB_WITHOUT_JMP
	int ret;
	return_jmp_buffer_set = 1;
	ret = setjmp (return_jmp_buf);
	if (ret) {
		return_jmp_buffer_set = 0;
		return ret;
	}
#endif
	exit_code = cob_call (name, argc, argv);
	return 0;
}

void
cob_init (const int argc, char **argv)
{
	char		*s;
#if	defined (HAVE_READLINK) || defined (HAVE_GETEXECNAME)
	const char	*path;
#endif
	int		i;

	/* Ensure initialization is only done once. Within generated modules and
	   libcob this is already ensured, but an external caller may call this
	   function again */
	if (cob_initialized) {
#if 0	/* Simon: We may raise a runtime warning/error in the future here */
		cob_runtime_warning ("%s called more than once", "cob_init");
#endif
		return;
	}

#ifdef __GLIBC__
	{
		/* 
		 * GNU libc may write a stack trace to /dev/tty when malloc
		 * detects corruption.  If LIBC_FATAL_STDERR_ is set to any
		 * nonempty string, it writes to stderr instead. See:
		 *https://code.woboq.org/userspace/glibc/sysdeps/posix/libc_fatal.c.html
		 */
		if (getenv ((const char*)"LIBC_FATAL_STDERR_") == NULL ) {
			(void)putenv ((char*)"LIBC_FATAL_STDERR_=keep_off_the_grass");
		}
	}
#endif

	cob_set_signal ();

	cob_alloc_base = NULL;
	cob_local_env = NULL;
	cob_last_sfile = NULL;
	commlnptr = NULL;
	basext = NULL;
	sort_keys = NULL;
	sort_collate = NULL;
	cob_source_file = NULL;
	exit_hdlrs = NULL;
	hdlrs = NULL;
	commlncnt = 0;
	sort_nkeys = 0;
	cob_source_line = 0;
	cob_local_env_size = 0;

	current_arg = 1;

	cob_argc = argc;
	cob_argv = argv;

	/* Get global structure */
	cobglobptr = cob_malloc (sizeof (cob_global));

	/* Get settings structure */
	cobsetptr = cob_malloc (sizeof (cob_settings));

	cob_initialized = 1;

#ifdef	HAVE_SETLOCALE
	/* Prime the locale from user settings */
	s = setlocale (LC_ALL, "");
	if (s) {
		/* Save initial values */
		cobglobptr->cob_locale_orig = cob_strdup (s);
		s = setlocale (LC_CTYPE, NULL);
		if (s) {
			cobglobptr->cob_locale_ctype = cob_strdup (s);
		}
		s = setlocale (LC_COLLATE, NULL);
		if (s) {
			cobglobptr->cob_locale_collate = cob_strdup (s);
		}
#ifdef	LC_MESSAGES
		s = setlocale (LC_MESSAGES, NULL);
		if (s) {
			cobglobptr->cob_locale_messages = cob_strdup (s);
		}
#endif
		s = setlocale (LC_MONETARY, NULL);
		if (s) {
			cobglobptr->cob_locale_monetary = cob_strdup (s);
		}
		s = setlocale (LC_NUMERIC, NULL);
		if (s) {
			cobglobptr->cob_locale_numeric = cob_strdup (s);
		}
		s = setlocale (LC_TIME, NULL);
		if (s) {
			cobglobptr->cob_locale_time = cob_strdup (s);
		}
		/* Set to standard "C" locale for COBOL */
		setlocale (LC_NUMERIC, "C");
		setlocale (LC_CTYPE, "C");
		/* Save changed locale */
		s = setlocale (LC_ALL, NULL);
		if (s) {
			cobglobptr->cob_locale = cob_strdup (s);
		}
	}
#endif
	cob_init_sig_descriptions ();

	cob_common_init (cobsetptr);

	/* Load runtime configuration file */
	if (unlikely (cob_load_config () < 0)) {
		cob_hard_failure ();
	}

	/* Copy COB_PHYSICAL_CANCEL from settings (internal) to global structure */
	cobglobptr->cob_physical_cancel = cobsetptr->cob_physical_cancel;

	/* Internal Debug Log */
	if (cobsetptr->cob_debug_log) {
#ifndef COB_DEBUG_LOG
		cob_runtime_warning (_("compiler was not built with --enable-debug-log; COB_DEBUG_LOG ignored"));
#else
		cob_debug_open ();
#endif
	}

	/* Call inits with cobsetptr to get the addresses of all */
	/* Screen-IO might be needed for error outputs */
	cob_init_screenio (cobglobptr, cobsetptr);
	cob_init_cconv (cobglobptr);
	cob_init_numeric (cobglobptr);
	cob_init_strings (cobglobptr);
	cob_init_move (cobglobptr, cobsetptr);
	cob_init_intrinsic (cobglobptr);
	cob_init_fileio (cobglobptr, cobsetptr);
	cob_init_call (cobglobptr, cobsetptr, check_mainhandle);
	cob_init_termio (cobglobptr, cobsetptr);
	cob_init_reportio (cobglobptr, cobsetptr);
	cob_init_mlio (cobglobptr);

	/* Set up library routine stuff */
	cobglobptr->cob_term_buff = cob_malloc ((size_t)COB_MEDIUM_BUFF);

	/* Set switches */
	for (i = 0; i <= COB_SWITCH_MAX; ++i) {
		char	switch_name[16];
		sprintf (switch_name, "COB_SWITCH_%d", i);
		s = getenv (switch_name);
		if (s && (*s == '1' || strcasecmp (s, "ON") == 0)) {
			cob_switch[i] = 1;
		} else {
			cob_switch[i] = 0;
		}
	}

#ifndef	HAVE_DESIGNATED_INITS
	init_statement_list ();
#endif

	/* Get user name if not set via environment already */
	if (cobsetptr->cob_user_name == NULL) {
#if defined (_WIN32)
	/* note: only defined manual (needs additional link to advapi32): */
#if defined (HAVE_GETUSERNAME)
		unsigned long bsiz = COB_MINI_MAX;
		char user_name[COB_MINI_BUFF];
		if (GetUserName (user_name, &bsiz)) {
			set_config_val_by_name (user_name, "username", "GetUserName()");
		}
#endif
#elif !defined(__OS400__)
		s = getlogin ();
		if (s) {
			set_config_val_by_name (s, "username", "getlogin()");
		}
#endif
#if 0	/* likely not needed, if unset then empty */
		if (cobsetptr->cob_user_name == NULL) {
			set_config_val_by_name (_("unknown"), "username", "cob_init()");
		}
#endif
	}

	/* This must be last in this function as we do early return */
	/* from certain ifdef's */

#ifdef	_WIN32
	s = cob_malloc ((size_t)COB_LARGE_BUFF);
	i = GetModuleFileNameA (NULL, s, COB_LARGE_MAX);
	if (i > 0 && i < COB_LARGE_BUFF) {
		cobglobptr->cob_main_argv0 = cob_strdup (s);
		cob_free (s);
		return;
	}
	cob_free (s);
#elif	defined (HAVE_READLINK)
	path = NULL;
	if (!access ("/proc/self/exe", R_OK)) {
		path = "/proc/self/exe";
	} else if (!access ("/proc/curproc/file", R_OK)) {
		path = "/proc/curproc/file";
	} else if (!access ("/proc/self/path/a.out", R_OK)) {
		path = "/proc/self/path/a.out";
	}
	if (path) {
		s = cob_malloc ((size_t)COB_LARGE_BUFF);
		i = (int)readlink (path, s, (size_t)COB_LARGE_MAX);
		if (i > 0 && i < COB_LARGE_BUFF) {
			s[i] = 0;
			cobglobptr->cob_main_argv0 = cob_strdup (s);
			cob_free (s);
			return;
		}
		cob_free (s);
	}
#endif

#ifdef	HAVE_GETEXECNAME
	path = getexecname ();
	if (path) {
#ifdef	HAVE_REALPATH
		s = cob_malloc ((size_t)COB_LARGE_BUFF);
		if (realpath (path, s) != NULL) {
			cobglobptr->cob_main_argv0 = cob_strdup (s);
		} else {
			cobglobptr->cob_main_argv0 = cob_strdup (path);
		}
		cob_free (s);
#else
		cobglobptr->cob_main_argv0 = cob_strdup (path);
#endif
		return;
	}
#endif

	if (argc && argv && argv[0]) {
#if	defined (HAVE_CANONICALIZE_FILE_NAME)
		/* Returns malloced path or NULL */
		cobglobptr->cob_main_argv0 = canonicalize_file_name (argv[0]);
#elif	defined (HAVE_REALPATH)
		s = cob_malloc ((size_t)COB_LARGE_BUFF);
		if (realpath (argv[0], s) != NULL) {
			cobglobptr->cob_main_argv0 = cob_strdup (s);
		}
		cob_free (s);
#elif	defined	(_WIN32)
		/* Returns malloced path or NULL */
		cobglobptr->cob_main_argv0 = _fullpath (NULL, argv[0], 1);
#endif
		if (!cobglobptr->cob_main_argv0) {
			cobglobptr->cob_main_argv0 = cob_strdup (argv[0]);
		}
	} else {
		cobglobptr->cob_main_argv0 = cob_strdup (_("unknown"));
	}
	/* The above must be last in this function as we do early return */
	/* from certain ifdef's */
}

/*
 * Set special runtime options:
 * Currently this is only FILE * for trace and printer output
 * or to reload the runtime configuration after changing environment
 */
void
cob_set_runtime_option (enum cob_runtime_option_switch opt, void *p)
{
	switch (opt) {
	case COB_SET_RUNTIME_TRACE_FILE:
		cobsetptr->cob_trace_file = *(fd_t*)p;
		if (p) {
			cobsetptr->external_trace_file = 1;
		} else {
			cobsetptr->external_trace_file = 0;
		}
		break;
	case COB_SET_RUNTIME_DISPLAY_PRINTER_FILE:
		/* note: if set cob_display_print_file is always external */
		cobsetptr->cob_display_print_file = *(fd_t*)p;
		break;
	case COB_SET_RUNTIME_DISPLAY_PUNCH_FILE:
		/* note: if set cob_display_punch_file is always external */
		if (cobsetptr->cob_display_punch_filename != NULL) {
			/* if previously opened by libcob: close and free pointer to filename */
			if (cobsetptr->cob_display_punch_file != VFS_INVALID_FD) {
				fclose (cobsetptr->cob_display_punch_file);
			}
			cob_free (cobsetptr->cob_display_punch_filename);
			cobsetptr->cob_display_punch_filename = NULL;
		}
		cobsetptr->cob_display_punch_file = *(fd_t*)p;
		break;
	case COB_SET_RUNTIME_DUMP_FILE:
		/* note: if set cob_dump_file is always external (libcob only opens it on abort)
		         therefore we don't need to close the old one */
		cobsetptr->cob_dump_file = *(fd_t*)p;
		if (!cobsetptr->cob_dump_file) {
			if (cobsetptr->cob_dump_filename) {
				cob_free (cobsetptr->cob_dump_filename);
			}
			cobsetptr->cob_dump_filename = cob_strdup ("NONE");
		}
		break;
	case COB_SET_RUNTIME_RESCAN_ENV:
		cob_rescan_env_vals ();
		break;
	default:
		cob_runtime_warning (_("%s called with unknown option: %d"),
			"cob_set_runtime_option", opt);
	}
	return;
}

/*
 * Return current value of special runtime options
 */
void *
cob_get_runtime_option (enum cob_runtime_option_switch opt)
{
	switch (opt) {
	case COB_SET_RUNTIME_TRACE_FILE:
		return (void*)cobsetptr->cob_trace_file;
	case COB_SET_RUNTIME_DISPLAY_PRINTER_FILE:
		return (void*)cobsetptr->cob_display_print_file;
	case COB_SET_RUNTIME_DISPLAY_PUNCH_FILE:
		/* only externalize if not aquired by libcob */
		if (cobsetptr->cob_display_punch_filename != NULL) {
			return NULL;
		}
		return (void*)cobsetptr->cob_display_punch_file;
	case COB_SET_RUNTIME_DUMP_FILE:
		return (void*)cobsetptr->cob_dump_file;
	default:
		cob_runtime_error (_("%s called with unknown option: %d"),
			"cob_get_runtime_option", opt);
	}
	return NULL;
}

/* output the COBOL-view of the stacktrace to the given target,
   does an early exit if 'target' is NULL, 
   'target' is FILE *  and should be flushed before */
void
cob_stack_trace (void *target)
{
	if (target == NULL || !cobglobptr || !COB_MODULE_PTR) {
		return;
	}
	dump_trace_started |= DUMP_TRACE_ACTIVE_TRACE;
	cob_stack_trace_internal (*(fd_t *)target, 1, 0);
	dump_trace_started ^= DUMP_TRACE_ACTIVE_TRACE;
}

static void flush_target (fd_t target)
{
	if (target == stderr
	 || target == stdout) {
		fflush (stdout);
		fflush (stderr);
	} else {
		fflush (target);
	}
}

/* output the COBOL-view of the stacktrace to the given target,
   does an early exit if 'target' is NULL,
   'target' is FILE *, output similar to GDBs backtrace command,
   "count" to limit to the first / last entries,
   REMARK: other than in GDB 0 means "full output" */
void
cob_backtrace (void *target, int count)
{
	if (target == NULL) {
		return;
	}
	if (!cobglobptr || !COB_MODULE_PTR) {
		fd_t fd = *(fd_t*)target;
		flush_target (fd);
		fputc (' ', fd);
		/* TRANSLATORS: This msgid is shown for a requested but empty stack trace. */
		fputs (_("No COBOL runtime elements on stack."), fd);
		fputc ('\n', fd);
		return;
	}
	dump_trace_started |= DUMP_TRACE_ACTIVE_TRACE;
	cob_stack_trace_internal (*(fd_t*)target, 0, count);
	dump_trace_started ^= DUMP_TRACE_ACTIVE_TRACE;
}

/* internal output the procedure stack entry to the given target */
static void
output_procedure_stack_entry (const int file_no,
		const char *section, const char *paragraph,
		const char *source_file, const unsigned int source_line)
{
	if (!section && !paragraph) {
		return;
	}
	write_or_return_arr (file_no, "\n\t");
	if (section && paragraph) {
		write_or_return_str (file_no, paragraph);
		write_or_return_arr (file_no, " OF ");
		write_or_return_str (file_no, section);
	} else {
		if (section) {
			write_or_return_str (file_no, section);
		} else {
			write_or_return_str (file_no, paragraph);
		}
	}
	write_or_return_arr (file_no, " at ");
	write_or_return_str (file_no, source_file);
	write_or_return_arr (file_no, ":");
	write_or_return_int (file_no, (int)source_line);
}

/* internal output the COBOL-view of the stacktrace to the given target */
void cob_stack_trace_internal (fd_t target, int verbose, int count)
{
	cob_module	*mod;
	int	first_entry = 0;
	int i, k;
	int file_no;

	/* exit early in the case of no module loaded at all,
	   possible to happen for example when aborted from cob_check_version of first module */
	if (!COB_MODULE_PTR
	 || (   COB_MODULE_PTR->module_stmt == 0
	     && COB_MODULE_PTR->next == NULL)) {
		return;
	}

	if (target == stderr) {
		file_no = stderr;
	} else {
		flush_target (target);
		file_no = fileno (target);
	}

	k = 0;
	if (count < 0) {
		for (mod = COB_MODULE_PTR, i = 0; mod; mod = mod->next, i++) {
			if (mod->next == mod
			 || k++ == MAX_MODULE_ITERS) {
				break;	/* messages in same checks below */
			}
		}
		first_entry = i + count;
	}

	if (verbose) {
		write_or_return_arr (file_no, "\n");
	}
	k = 0;
	for (mod = COB_MODULE_PTR, i = 0; mod; mod = mod->next, i++) {
		if (i < first_entry) {
			continue;
		}
		if (count > 0 && count == i) {
			break;
		}
		write_or_return_arr (file_no, " ");
		if (mod->module_stmt != 0
		 && mod->module_sources) {
			const unsigned int source_file_num = COB_GET_FILE_NUM (mod->module_stmt);
			const unsigned int source_line = COB_GET_LINE_NUM (mod->module_stmt);
			const char *source_file = mod->module_sources[source_file_num];
			if (!verbose) {
				write_or_return_str (file_no, mod->module_name);
				write_or_return_arr (file_no, " at ");
				write_or_return_str (file_no, source_file);
				write_or_return_arr (file_no, ":");
				write_or_return_int (file_no, (int)source_line);
			} else
			if (mod->statement == STMT_UNKNOWN
			 && !mod->section_name
			 && !mod->paragraph_name) {
				/* GC 3.1 output, now used for "no source location / no trace" case */
				write_or_return_arr (file_no, "Last statement of ");
				if (mod->module_type == COB_MODULE_TYPE_FUNCTION) {
					write_or_return_arr (file_no, "FUNCTION ");
				}
				write_or_return_arr (file_no, "\"");
				write_or_return_str (file_no, mod->module_name);
				write_or_return_arr (file_no, "\" was at line ");
				write_or_return_int (file_no, (int)source_line);
				write_or_return_arr (file_no, " of ");
				write_or_return_str (file_no, source_file);
			} else
			if (!mod->section_name && !mod->paragraph_name) {
				/* special case: there _would_ be data,
				   but there's no procedure defined in the program */
				write_or_return_arr (file_no, "Last statement of ");
				if (mod->module_type == COB_MODULE_TYPE_FUNCTION) {
					write_or_return_arr (file_no, "FUNCTION ");
				}
				write_or_return_arr (file_no, "\"");
				write_or_return_str (file_no, mod->module_name);
				write_or_return_arr (file_no, "\" was ");
				write_or_return_str (file_no, cob_statement_name[mod->statement]);
				write_or_return_arr (file_no, " at line ");
				write_or_return_int (file_no, (int)source_line);
				write_or_return_arr (file_no, " of ");
				write_or_return_str (file_no, source_file);
			} else {
				/* common case when compiled with runtime checks enabled: statement and
				   procedure known - the later is printed from the stack entry with the
				   source location by the following call */
				write_or_return_arr (file_no, "Last statement of ");
				if (mod->module_type == COB_MODULE_TYPE_FUNCTION) {
					write_or_return_arr (file_no, "FUNCTION ");
				}
				write_or_return_arr (file_no, "\"");
				write_or_return_str (file_no, mod->module_name);
				write_or_return_arr (file_no, "\" was ");
				write_or_return_str (file_no, cob_statement_name[mod->statement]);
			}
			output_procedure_stack_entry (file_no, mod->section_name, mod->paragraph_name,
					source_file, source_line);
			if (mod->frame_ptr) {
				struct cob_frame_ext *perform_ptr = mod->frame_ptr;
				int frame_max = 512; /* max from -fstack-size */
				while (frame_max--) {
					const unsigned int ffile_num = COB_GET_FILE_NUM (perform_ptr->module_stmt);
					const unsigned int fline = COB_GET_LINE_NUM (perform_ptr->module_stmt);
					const char *ffile = mod->module_sources[ffile_num];
					if (perform_ptr->section_name) {
						/* marker for "root frame" - at ENTRY */
						if (perform_ptr->section_name[0] == 0) {
							write_or_return_arr (file_no, "\n\tENTRY ");
							write_or_return_str (file_no, perform_ptr->paragraph_name);
							write_or_return_arr (file_no, " at ");
							write_or_return_str (file_no, ffile);
							write_or_return_arr (file_no, ":");
							write_or_return_int (file_no, (int)fline);
							break;
						}
					}
					output_procedure_stack_entry (file_no,
						perform_ptr->section_name, perform_ptr->paragraph_name,
						ffile, fline);
					perform_ptr--;
				}
			}
		} else {
			if (verbose) {
				write_or_return_arr (file_no, "Last statement of ");
				if (mod->module_type == COB_MODULE_TYPE_FUNCTION) {
					write_or_return_arr (file_no, "FUNCTION ");
				}
				write_or_return_arr (file_no, "\"");
				write_or_return_str (file_no, mod->module_name);
				if (mod->statement != STMT_UNKNOWN) {
					write_or_return_arr (file_no, "\" was ");
					write_or_return_str (file_no, cob_statement_name[mod->statement]);
				} else {
					write_or_return_arr (file_no, "\" unknown");
				}
			} else {
				write_or_return_str (file_no, mod->module_name);
				write_or_return_arr (file_no, " at unknown");
			}
		}
		write_or_return_arr (file_no, "\n");
		if (mod->next == mod) {
			/* not translated as highly unexpected */
			write_or_return_arr (file_no, "FIXME: recursive mod (stack trace)\n");
			break;
		}
		if (k++ == MAX_MODULE_ITERS) {
			/* not translated as highly unexpected */
			write_or_return_arr (file_no,
				"max module iterations exceeded, possible broken chain\n");
			break;
		}
			
	}
	if (mod) {
		write_or_return_arr (file_no, " ");
		write_or_return_str (file_no, more_stack_frames_msgid);
		write_or_return_arr (file_no, "\n");
	}

	if (verbose && cob_argc != 0) {
		size_t ia;
		write_or_return_arr (file_no, " Started by ");
		write_or_return_str (file_no, cob_argv[0]);
		write_or_return_arr (file_no, "\n");
		for (ia = 1; ia < (size_t)cob_argc; ++ia) {
			write_or_return_arr (file_no, "\t");
			write_or_return_str (file_no, cob_argv[ia]);
			write_or_return_arr (file_no, "\n");
		}
	}
}

fd_t cob_get_dump_file (void)
{
#if 1 /* new version as currently only COB_DUMP_TO_FILE is used */
	if (cobsetptr->cob_dump_file != VFS_INVALID_FD) {	/* If DUMP active, use that */
		return cobsetptr->cob_dump_file;
	} else if (cobsetptr->cob_dump_filename != NULL) {	/* DUMP file defined */
		if (cob_check_env_false(cobsetptr->cob_dump_filename)) {
			return VFS_INVALID_FD;
		}
		cobsetptr->cob_dump_file = cob_open_logfile (cobsetptr->cob_dump_filename);
		if (cobsetptr->cob_dump_file != VFS_INVALID_FD) {
			return cobsetptr->cob_dump_file;
		}
		/* could not open the file
		   unset the filename for not referencing it later */
		cob_free (cobsetptr->cob_dump_filename);
		cobsetptr->cob_dump_filename = NULL;
		/* Fall-through */
	}
	if (cobsetptr->cob_trace_file != VFS_INVALID_FD) {	/* If TRACE active, use that */
		return cobsetptr->cob_trace_file;
	} else {
		return stderr;
	}
#else /* currently only COB_DUMP_TO_FILE used */
	FILE    *fp;
	if (where == COB_DUMP_TO_FILE) {
		fp = cobsetptr->cob_dump_file;
		if (fp == NULL) {
			if(cobsetptr->cob_trace_file != NULL) {	/* If TRACE active, use that */
				fp = cobsetptr->cob_trace_file;
			} else if(cobsetptr->cob_dump_filename != NULL) {	/* Dump file defined */
				if (cob_check_env_false(cobsetptr->cob_dump_filename)) {
					return NULL;
				}
				fp = fopen(cobsetptr->cob_dump_filename, "a");
				if(fp == NULL)
					fp = stderr;
				cobsetptr->cob_dump_file = fp;
			} else {
				fp = stderr;
			}
		}
	} else if (where == COB_DUMP_TO_PRINT) {
		fp = cobsetptr->cob_display_print_file;
		if (fp == NULL) {
			if(cobsetptr->cob_trace_file != NULL) {	/* If TRACE active, use that */
				fp = cobsetptr->cob_trace_file;
			} else {
				fp = stdout;
			}
		}
	} else {
		fp = stderr;
	}
	return fp;
#endif
}

static void
cob_dump_module (char *reason)
{
	cob_module	*mod;
	int		wants_dump = 0;
	int k;

	/* Was any module compiled with -fdump? */
	k = 0;
	for (mod = COB_MODULE_PTR; mod; mod = mod->next) {
		if (mod->flag_dump_ready) {
			wants_dump = 1;
		}
		if (mod->next == mod) {
			/* not translated as highly unexpected */
			fputs ("FIXME: recursive mod (module dump)\n", stderr);
			break;
		}
		if (k++ == MAX_MODULE_ITERS) {
			/* not translated as highly unexpected */
			fputs ("max module iterations exceeded, possible broken chain\n", stderr);
			break;
		}
		if (mod->flag_dump_ready) {
			break;
		}
	}

	if (wants_dump) {
		fd_t fp;
		char		*previous_locale = NULL;
#if 1 /* new version as currently only COB_DUMP_TO_FILE is used */
		fp = cob_get_dump_file ();
#else
		fp = cob_get_dump_file (COB_DUMP_TO_FILE);
#endif
		/* explicit disabled dump */
		if (fp == VFS_INVALID_FD) {
			return;
		}
		if (fp != stderr) {
			if (reason) {
				if (reason[0] == 0) {
					reason = (char *)_ ("unknown");
				}
				fputc ('\n', fp);
				fprintf (fp, _("Module dump due to %s"), reason);
				fputc ('\n', fp);
			}
			if (fp != stdout) {
				/* was already sent to stderr before this function was called,
				   so skip here for stdout/stderr ... */
				if (!(dump_trace_started & DUMP_TRACE_ACTIVE_TRACE)) {
					dump_trace_started |= DUMP_TRACE_ACTIVE_TRACE;
					cob_stack_trace_internal (fp, 1, 0);
					dump_trace_started ^= DUMP_TRACE_ACTIVE_TRACE;
				}
			}
			fflush (stdout);
		} else {
			fflush (stderr);
		}

		fputc ('\n', fp);
		if (cobglobptr->cob_locale_ctype) {
			previous_locale = setlocale (LC_CTYPE, NULL);
			setlocale (LC_CTYPE, cobglobptr->cob_locale_ctype);
		}
		k = 0;
		for (mod = COB_MODULE_PTR; mod; mod = mod->next) {
			if (mod->module_cancel.funcint) {
				int (*cancel_func)(const int);
				cancel_func = mod->module_cancel.funcint;

				fprintf (fp, _("Dump Program-Id %s from %s compiled %s"),
					mod->module_name, mod->module_source, mod->module_formatted_date);
				fputc ('\n', fp);
				(void)cancel_func (-10);
				fputc ('\n', fp);
			}
			if (mod->next == mod
			 || k++ == MAX_MODULE_ITERS) {
				break;
			}
		}
		if (previous_locale) {
			setlocale (LC_CTYPE, previous_locale);
		}
		if (fp != stdout && fp != stderr) {
			char * fname = NULL;
			if (cobsetptr->cob_dump_filename) {
				fname = cobsetptr->cob_dump_filename;
			} else
			if (cobsetptr->cob_trace_file == fp
			 && cobsetptr->cob_trace_filename != NULL
			 && !cobsetptr->external_trace_file) {
				fname = cobsetptr->cob_trace_filename;
			}
			if (fname != NULL) {
				fputc ('\n', stderr);
				fprintf (stderr, _("dump written to %s"), fname);
				fputc ('\n', stderr);
				fflush (stderr);
			}
		}
	}
}

#ifdef COB_DEBUG_LOG
/******************************/
/* Routines for COB_DEBUG_LOG */
/******************************/

/* Check env var value and open log file */
/*
 * Env var is  COB_DEBUG_LOG
 * Env Var string is a series of keyword=value parameters where keywords:
 * L=x  - options: T for trace level, W for warnings, N for normal, A for ALL
 * M=yy - module:  RW for report writer, the 2 char code is tabled and compared
 *        with the value coded on DEBUG_LOG("yy",("format",args));
 * O=path/file - file name to write log data to, default is: cob_debug_log.$$
 *        note:  replacements already done in common setting handling
 */
void
cob_debug_open (void)
{
	char	*debug_env = cobsetptr->cob_debug_log;
	int		i, j;
	char	module_name[4];
	char	log_opt;
	char	logfile[COB_SMALL_BUFF];

	logfile[0] = 0;

	for (i=0; debug_env[i] != 0; i++) {
		/* skip separator */
		if (debug_env[i] == ','
		 || debug_env[i] == ';')
			continue;

		/* debugging flags (not include in file name) */
		if (debug_env[i + 1] == '=') {
			log_opt = debug_env[i];
			i += 2;

			switch (log_opt) {

			case 'M':	/* module to debug */
			case 'm':
				for (j = 0; j < 4; i++) {
					if (debug_env[i] == ','
					 || debug_env[i] == ';'
					 || debug_env[i] == 0) {
						break;
					}
					module_name[j++] = debug_env[i];
				}
				module_name[j] = 0;
				/* note: special module ALL is checked later */
				for (j = 0; j < 12 && cob_debug_modules[j][0] > ' '; j++) {
					if (strcasecmp (cob_debug_modules[j], module_name) == 0) {
						break;
					}
				}
				if (j < 12 && cob_debug_modules[j][0] == ' ') {
					strcpy (cob_debug_modules[j], module_name);
				}
				if (debug_env[i] == 0) i--;
				break;

			case 'L':	/* logging options */
			case 'l':
				log_opt = debug_env[i];
				switch (log_opt) {
				case 'T':	/* trace */
				case 't':
					cob_debug_log_time = cob_debug_level = 3;
					break;
				case 'W':	/* warnings */
				case 'w':
					cob_debug_level = 2;
					break;
				case 'N':	/* normal */
				case 'n':
					cob_debug_level = 0;
					break;
				case 'A':	/* all */
				case 'a':
					cob_debug_level = 9;
					break;
				default:	/* Unknown log option, just ignored for now */
					i--;
					break;
				}
				break;

			case 'O':	/* output name for logfile */
			case 'o':
				for (j = 0; j < COB_SMALL_MAX; i++) {
					if (debug_env[i] == ','
					 || debug_env[i] == ';'
					 || debug_env[i] == 0) {
						break;
					}
					logfile[j++] = debug_env[i];
				}
				logfile[j] = 0;
				if (debug_env[i] == 0) i--;
				break;

			default:	/* Unknown x=, just ignored for now */
				break;
			}
		} else {
			/* invalid character, just ignored for now */
			/* note: this allows for L=WARNING (but also for L=WUMPUS) */
		}
	}

	/* set default logfile if not given */
	if (logfile[0] == 0) {
		sprintf (logfile, "cob_debug_log.%d", cob_sys_getpid());
	}
	/* store filename for possible unlink (empty log file) */
	cob_debug_file_name = cob_strdup (logfile);

	/* ensure trace file is open if we use this as debug log and exit */
	if (cobsetptr->cob_trace_filename &&
		strcmp (cobsetptr->cob_trace_filename, cob_debug_file_name) == 0) {
		cob_check_trace_file ();
		cob_debug_file = cobsetptr->cob_trace_file;
		return;
	}

	/* open logfile */
	cob_debug_file = cob_open_logfile (cob_debug_file_name);
	if (cob_debug_file == NULL) {
		/* developer-only msg - not translated */
		cob_runtime_error ("error '%s' opening COB_DEBUG_LOG '%s', resolved from '%s'",
			cob_get_strerror (), cob_debug_file_name, cobsetptr->cob_debug_log);
		return;
	}
}

/* Determine if DEBUGLOG is to be allowed */
int
cob_debug_logit (int level, char *module)
{
	int	i;
	if (cob_debug_file == NULL) {
		return 1;
	}
	if (level > cob_debug_level) {
		return 1;
	}
	for (i=0; i < 12 && cob_debug_modules[i][0] > ' '; i++) {
		if (strcasecmp ("ALL", cob_debug_modules[i]) == 0) {
			cob_debug_mod = (char*)module;
			return 0;						/* Logging is allowed */
		}
		if (strcasecmp (module,cob_debug_modules[i]) == 0) {
			cob_debug_mod = (char*)&cob_debug_modules[i];
			return 0;						/* Logging is allowed */
		}
	}
	return 1;
}

/* Write logging line */
static int cob_debug_hdr = 1;
static unsigned int cob_debug_prv_line = 0;
int
cob_debug_logger (const char *fmt, ...)
{
	va_list		ap;
	int		ln;
	struct cob_time time;

	if (cob_debug_file == NULL) {
		return 0;
	}
	if (*fmt == '~') {			/* Force line# out again to log file */
		fmt++;
		cob_debug_prv_line = -1;
		cob_debug_hdr = 1;
	}
	if (cob_debug_hdr) {
		cob_get_source_line ();
		if (cob_debug_log_time) {
			time = cob_get_current_datetime (DTR_FULL);
			fprintf (cob_debug_file, "%02d:%02d:%02d.%02d ", time.hour, time.minute,
							time.second, time.nanosecond / 10000000);
		}
		if (cob_debug_mod) {
			fprintf (cob_debug_file, "%-3s:", cob_debug_mod);
		}
		if (cob_source_file) {
			fprintf (cob_debug_file, " %s :", cob_source_file);
		}
		if (cob_source_line && cob_source_line != cob_debug_prv_line) {
			fprintf (cob_debug_file, "%5d : ", cob_source_line);
			cob_debug_prv_line = cob_source_line;
		} else {
			fprintf (cob_debug_file, "%5s : ", " ");
		}
		cob_debug_hdr = 0;
	}
	va_start (ap, fmt);
	vfprintf (cob_debug_file, fmt, ap);
	va_end (ap);
	ln = strlen(fmt);
	if (fmt[ln-1] == '\n') {
		cob_debug_hdr = 1;
		fflush (cob_debug_file);
	}
	return 0;
}

static int			/* Return TRUE if word is repeated 16 times */
repeatWord(
	char	*match,	/* 4 bytes to match */
	char	*mem)	/* Memory area to match repeated value */
{
	if(memcmp(match, &mem[0], 4) == 0
	&& memcmp(match, &mem[4], 4) == 0
	&& memcmp(match, &mem[8], 4) == 0
	&& memcmp(match, &mem[12], 4) == 0)
		return 1;
	return 0;
}

/* Hexdump of memory */
int
cob_debug_dump (void *pMem, int len)
{
#define dMaxPerLine	24
#define dMaxHex ((dMaxPerLine*2)+(dMaxPerLine/4-1))
	register int i, j, k;
	register char	c, *mem = pMem;
	char	lastWord[4];
	char	hex[dMaxHex+4],chr[dMaxPerLine+4];
	int		adrs = 0;

	if (cob_debug_file == NULL)
		return 0;
	memset (lastWord,0xFD, 4);
	for (i=0; i < len; ) {
		for (j=k=0; j < dMaxPerLine && (i+j) < len; j++) {
			k += sprintf(&hex[k],"%02X",mem[i+j]&0xFF);
			if ((j % 4) == 3 )
				hex[k++] = ' ';
		}
		if (k && hex[k-1] == ' ')
			hex[k-1] = 0;
		hex[k] = 0;

		k = 0;
		for (j=0; j<dMaxPerLine && (i+j)<len; j++) {
			c = mem[i+j];
			chr[k++] =  c >= ' ' && c < 0x7f ? c : '.';
		}
		chr[k++] = 0;

		fprintf (cob_debug_file," %6.6X : %-*s '%s'\n",adrs+i,dMaxHex,hex,chr);
		if ((i + dMaxPerLine) < len )
			memcpy( (char *)lastWord, (char *)&mem[i+dMaxPerLine-4], j<4?j:4);
		i += dMaxPerLine;
		if( (i + (16*2)) < len
		&& repeatWord (lastWord, &mem[i])
		&& repeatWord (lastWord, &mem[i+dMaxPerLine])) {
			fprintf (cob_debug_file," %6.6X : ",adrs+i);
			while (i < len - 16
			&& repeatWord(lastWord,&mem[i]))
				i += 16;
			fprintf (cob_debug_file," thru %6.6X same as last word\n",adrs+i-1);
		}
	}
	fflush (cob_debug_file);

	return 0;
}
#endif	/* end of 'COB_DEBUG_LOG' */


#ifndef	HAVE_DESIGNATED_INITS
void
init_statement_list (void)
{
	cob_statement_name[STMT_UNKNOWN] = "UNKNOWN";
#define COB_STATEMENT(ename,str) \
	cob_statement_name[ename] = str;
#include "statement.def"	/* located and installed next to common.h */
#undef COB_STATEMENT
}
#endif
