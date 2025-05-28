
#include "curspriv.h"

#include <stdlib.h>

enum { LABEL_NORMAL = 8, LABEL_EXTENDED = 10, LABEL_NCURSES_EXTENDED = 12 };

static int label_length = 0;
static int labels = 0;
static int label_fmt = 0;
static int label_line = 0;
static bool hidden = FALSE;

static struct SLK {
    chtype label[32];
    int len;
    int format;
    int start_col;
} *slk = (struct SLK *)NULL;

int slk_init(int fmt)
{
    PDC_LOG(("slk_init() - called\n"));

    if (SP)
        return ERR;

    switch (fmt)
    {
    case 0:  /* 3 - 2 - 3 */
        labels = LABEL_NORMAL;
        break;

    case 1:   /* 4 - 4 */
        labels = LABEL_NORMAL;
        break;

    case 2:   /* 4 4 4 */
        labels = LABEL_NCURSES_EXTENDED;
        break;

    case 3:   /* 4 4 4  with index */
        labels = LABEL_NCURSES_EXTENDED;
        break;

    case 55:  /* 5 - 5 */
        labels = LABEL_EXTENDED;
        break;

    default:
        return ERR;
    }

    label_fmt = fmt;

    slk = calloc(labels, sizeof(struct SLK), &cursesPage);

    if (!slk)
        labels = 0;

    return slk ? OK : ERR;
}

/* draw a single button */

static void _drawone(int num)
{
    int i, col, slen;

    if (hidden)
        return;

    slen = slk[num].len;

    switch (slk[num].format)
    {
    case 0:  /* LEFT */
        col = 0;
        break;

    case 1:  /* CENTER */
        col = (label_length - slen) / 2;

        if (col + slen > label_length)
            --col;
        break;

    default:  /* RIGHT */
        col = label_length - slen;
    }

    wmove(SP->slk_winptr, label_line, slk[num].start_col);

    for (i = 0; i < label_length; ++i)
        waddch(SP->slk_winptr, (i >= col && i < (col + slen)) ?
               slk[num].label[i - col] : ' ');
}

/* redraw each button */

static void _redraw(void)
{
    int i;

    for (i = 0; i < labels; ++i)
        _drawone(i);
}

/* slk_set() Used to set a slk label to a string.

   labnum  = 1 - 8 (or 10) (number of the label)
   label   = string (8 or 7 bytes total), or NULL
   justify = 0 : left, 1 : center, 2 : right  */

int slk_set(int labnum, const char *label, int justify)
{
#ifdef PDC_WIDE
    wchar_t wlabel[32];

    PDC_mbstowcs(wlabel, label, 31);
    return slk_wset(labnum, wlabel, justify);
#else
    PDC_LOG(("slk_set() - called\n"));

    if (labnum < 1 || labnum > labels || justify < 0 || justify > 2)
        return ERR;

    labnum--;

    if (!label || !(*label))
    {
        /* Clear the label */

        *slk[labnum].label = 0;
        slk[labnum].format = 0;
        slk[labnum].len = 0;
    }
    else
    {
        int i, j = 0;

        /* Skip leading spaces */

        while (label[j] == ' ')
            j++;

        /* Copy it */

        for (i = 0; i < label_length; i++)
        {
            chtype ch = label[i + j];

            slk[labnum].label[i] = ch;

            if (!ch)
                break;
        }

        /* Drop trailing spaces */

        while ((i + j) && (label[i + j - 1] == ' '))
            i--;

        slk[labnum].label[i] = 0;
        slk[labnum].format = justify;
        slk[labnum].len = i;
    }

    _drawone(labnum);

    return OK;
#endif
}

int slk_refresh(void)
{
    PDC_LOG(("slk_refresh() - called\n"));

    return (slk_noutrefresh() == ERR) ? ERR : doupdate();
}

int slk_noutrefresh(void)
{
    PDC_LOG(("slk_noutrefresh() - called\n"));

    if (!SP)
        return ERR;

    return wnoutrefresh(SP->slk_winptr);
}

char *slk_label(int labnum)
{
    static char temp[33];
#ifdef PDC_WIDE
    wchar_t *wtemp = slk_wlabel(labnum);

    PDC_wcstombs(temp, wtemp, 32);
#else
    chtype *p;
    int i;

    PDC_LOG(("slk_label() - called\n"));

    if (labnum < 1 || labnum > labels)
        return (char *)0;

    for (i = 0, p = slk[labnum - 1].label; *p; i++)
        temp[i] = *p++;

    temp[i] = '\0';
#endif
    return temp;
}

int slk_clear(void)
{
    PDC_LOG(("slk_clear() - called\n"));

    if (!SP)
        return ERR;

    hidden = TRUE;
    werase(SP->slk_winptr);
    return wrefresh(SP->slk_winptr);
}

int slk_restore(void)
{
    PDC_LOG(("slk_restore() - called\n"));

    if (!SP)
        return ERR;

    hidden = FALSE;
    _redraw();
    return wrefresh(SP->slk_winptr);
}

int slk_touch(void)
{
    PDC_LOG(("slk_touch() - called\n"));

    if (!SP)
        return ERR;

    return touchwin(SP->slk_winptr);
}

int slk_attron(const chtype attrs)
{
    int rc;

    PDC_LOG(("slk_attron() - called\n"));

    if (!SP)
        return ERR;

    rc = wattron(SP->slk_winptr, attrs);
    _redraw();

    return rc;
}

int slk_attr_on(const attr_t attrs, void *opts)
{
    PDC_LOG(("slk_attr_on() - called\n"));

    return slk_attron(attrs);
}

int slk_attroff(const chtype attrs)
{
    int rc;

    PDC_LOG(("slk_attroff() - called\n"));

    if (!SP)
        return ERR;

    rc = wattroff(SP->slk_winptr, attrs);
    _redraw();

    return rc;
}

int slk_attr_off(const attr_t attrs, void *opts)
{
    PDC_LOG(("slk_attr_off() - called\n"));

    return slk_attroff(attrs);
}

int slk_attrset(const chtype attrs)
{
    int rc;

    PDC_LOG(("slk_attrset() - called\n"));

    if (!SP)
        return ERR;

    rc = wattrset(SP->slk_winptr, attrs);
    _redraw();

    return rc;
}

int slk_color(short color_pair)
{
    int rc;

    PDC_LOG(("slk_color() - called\n"));

    if (!SP)
        return ERR;

    rc = wcolor_set(SP->slk_winptr, color_pair, NULL);
    _redraw();

    return rc;
}

int slk_attr_set(const attr_t attrs, short color_pair, void *opts)
{
    PDC_LOG(("slk_attr_set() - called\n"));

    return slk_attrset(attrs | COLOR_PAIR(color_pair));
}

static void _slk_calc(void)
{
    int i, center, col = 0;
    label_length = COLS / labels;

    if (label_length > 31)
        label_length = 31;

    switch (label_fmt)
    {
    case 0:     /* 3 - 2 - 3 F-Key layout */

        --label_length;

        slk[0].start_col = col;
        slk[1].start_col = (col += label_length);
        slk[2].start_col = (col += label_length);

        center = COLS / 2;

        slk[3].start_col = center - label_length + 1;
        slk[4].start_col = center + 1;

        col = COLS - (label_length * 3) + 1;

        slk[5].start_col = col;
        slk[6].start_col = (col += label_length);
        slk[7].start_col = (col += label_length);
        break;

    case 1:     /* 4 - 4 F-Key layout */

        for (i = 0; i < 8; i++)
        {
            slk[i].start_col = col;
            col += label_length;

            if (i == 3)
                col = COLS - (label_length * 4) + 1;
        }

        break;

    case 2:     /* 4 4 4 F-Key layout */
    case 3:     /* 4 4 4 F-Key layout with index */

        for (i = 0; i < 4; i++)
        {
            slk[i].start_col = col;
            col += label_length;
        }

        center = COLS / 2;

        slk[4].start_col = center - (label_length * 2) + 1;
        slk[5].start_col = center - label_length + 1;
        slk[6].start_col = center + 1;
        slk[7].start_col = center + label_length + 1;

        col = COLS - (label_length * 4) + 1;

        for (i = 8; i < 12; i++)
        {
            slk[i].start_col = col;
            col += label_length;
        }

        break;

    default:    /* 5 - 5 F-Key layout */

        for (i = 0; i < 10; i++)
        {
            slk[i].start_col = col;
            col += label_length;

            if (i == 4)
                col = COLS - (label_length * 5) + 1;
        }
    }

    --label_length;

    /* make sure labels are all in window */

    _redraw();
}

void PDC_slk_initialize(void)
{
    if (slk)
    {
        if (label_fmt == 3)
        {
            SP->slklines = 2;
            label_line = 1;
        }
        else
            SP->slklines = 1;

        if (!SP->slk_winptr)
        {
            SP->slk_winptr = newwin(SP->slklines, COLS,
                                    LINES - SP->slklines, 0);
            if (!SP->slk_winptr)
                return;

            wattrset(SP->slk_winptr, A_REVERSE);
        }

        _slk_calc();

        /* if we have an index line, display it now */

        if (label_fmt == 3)
        {
            chtype save_attr;
            int i;

            save_attr = SP->slk_winptr->_attrs;
            wattrset(SP->slk_winptr, A_NORMAL);
            wmove(SP->slk_winptr, 0, 0);
            whline(SP->slk_winptr, 0, COLS);

            for (i = 0; i < labels; i++)
                mvwprintw(SP->slk_winptr, 0, slk[i].start_col, "F%d", i + 1);

            SP->slk_winptr->_attrs = save_attr;
        }

        touchwin(SP->slk_winptr);
    }
}