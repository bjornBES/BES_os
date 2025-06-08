/* PDCurses */

#include "curspriv.h"
#include "string.h"
#include "limits.h"

int baudrate(void)
{
    PDC_LOG("baudrate() - called\n");

    return INT_MAX;
}

char erasechar(void)
{
    PDC_LOG("erasechar() - called\n");

    return _ECHAR;      /* character delete char (^H) */
}

bool has_ic(void)
{
    PDC_LOG("has_ic() - called\n");

    return TRUE;
}

bool has_il(void)
{
    PDC_LOG("has_il() - called\n");

    return TRUE;
}

char killchar(void)
{
    PDC_LOG("killchar() - called\n");

    return _DLCHAR;     /* line delete char (^U) */
}

char *longname(void)
{
    PDC_LOG("longname() - called\n");

    return ttytype + 9; /* skip "pdcurses|" */
}

chtype termattrs(void)
{
    PDC_LOG("termattrs() - called\n");

    return SP ? SP->termattrs : (chtype)0;
}

attr_t term_attrs(void)
{
    PDC_LOG("term_attrs() - called\n");

    return SP ? SP->termattrs : (attr_t)0;
}

char *termname(void)
{
    static char _termname[14] = "pdcurses";

    PDC_LOG("termname() - called\n");

    return _termname;
}

char wordchar(void)
{
    PDC_LOG("wordchar() - called\n");

    return _DWCHAR;         /* word delete char */
}

#ifdef PDC_WIDE
int erasewchar(wchar_t *ch)
{
    PDC_LOG("erasewchar() - called\n");

    if (!ch)
        return ERR;

    *ch = (wchar_t)_ECHAR;

    return OK;
}

int killwchar(wchar_t *ch)
{
    PDC_LOG("killwchar() - called\n");

    if (!ch)
        return ERR;

    *ch = (wchar_t)_DLCHAR;

    return OK;
}
#endif
