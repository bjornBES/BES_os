#pragma once

#define LC_CTYPE                0
#define LC_NUMERIC              1
#define LC_TIME                 2
#define LC_COLLATE              3
#define LC_MONETARY             4
#define LC_MESSAGES             5
#define	LC_ALL                  6
#define LC_PAPER                7
#define LC_NAME                 8
#define LC_ADDRESS              9
#define LC_TELEPHONE            10
#define LC_MEASUREMENT          11
#define LC_IDENTIFICATION       12

/* ADD YOURS HERE */
#define _NCAT		6	/* one more than last */
		/* type definitions */
struct lconv {
		/* controlled by LC_MONETARY */
	char *currency_symbol;
	char *int_curr_symbol;
	char *mon_decimal_point;
	char *mon_grouping;
	char *mon_thousands_sep;
	char *negative_sign;
	char *positive_sign;
	char frac_digits;
	char int_frac_digits;
	char n_cs_precedes;
	char n_sep_by_space;
	char n_sign_posn;
	char p_cs_precedes;
	char p_sep_by_space;
	char p_sign_posn;
		/* controlled by LC_NUMERIC */
	char *decimal_point;
	char *grouping;
	char *thousands_sep;
	};
		/* declarations */
struct lconv *localeconv(void);
char *setlocale(int, const char *);
extern struct lconv _Locale;
		/* macro overrides */
#define localeconv()	(&_Locale)