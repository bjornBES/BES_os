#include "curspriv.h"
#include <stdlib.h>
#include "malloc.h"

WINDOW *PDC_makenew(int nlines, int ncols, int begy, int begx)
{
    WINDOW *win;

    PDC_LOG("PDC_makenew() - called: lines %d cols %d begy %d begx %d\n",
             nlines, ncols, begy, begx);

    /* allocate the window structure itself */

    win = calloc(1, sizeof(WINDOW), &cursesPage);
    if (!win)
        return win;

    /* allocate the line pointer array */

    win->_y = malloc(nlines * sizeof(chtype *), &cursesPage);
    if (!win->_y)
    {
        free(win, &cursesPage);
        return (WINDOW *)NULL;
    }

    /* allocate the minchng and maxchng arrays */

    win->_firstch = malloc(nlines * sizeof(int), &cursesPage);
    if (!win->_firstch)
    {
        free(win->_y, &cursesPage);
        free(win, &cursesPage);
        return (WINDOW *)NULL;
    }

    win->_lastch = malloc(nlines * sizeof(int), &cursesPage);
    if (!win->_lastch)
    {
        free(win->_firstch, &cursesPage);
        free(win->_y, &cursesPage);
        free(win, &cursesPage);
        return (WINDOW *)NULL;
    }

    /* initialize window variables */

    win->_maxy = nlines;  /* real max screen size */
    win->_maxx = ncols;   /* real max screen size */
    win->_begy = begy;
    win->_begx = begx;
    win->_bkgd = ' ';     /* wrs 4/10/93 -- initialize background to blank */
    win->_clear = (bool) ((nlines == LINES) && (ncols == COLS));
    win->_bmarg = nlines - 1;
    win->_parx = win->_pary = -1;

    /* initialize pad variables*/

    win->_pad._pad_y = -1;
    win->_pad._pad_x = -1;
    win->_pad._pad_top = -1;
    win->_pad._pad_left = -1;
    win->_pad._pad_bottom = -1;
    win->_pad._pad_right = -1;

    /* init to say window all changed */

    touchwin(win);

    return win;
}

WINDOW *PDC_makelines(WINDOW *win)
{
    int i, j, nlines, ncols;

    PDC_LOG("PDC_makelines() - called\n");

    if (!win)
        return (WINDOW *)NULL;

    nlines = win->_maxy;
    ncols = win->_maxx;

    for (i = 0; i < nlines; i++)
    {
        win->_y[i] = malloc(ncols * sizeof(chtype), &cursesPage);
        if (!win->_y[i])
        {
            /* if error, free all the data */

            for (j = 0; j < i; j++)
                free(win->_y[j], &cursesPage);

            free(win->_firstch, &cursesPage);
            free(win->_lastch, &cursesPage);
            free(win->_y, &cursesPage);
            free(win, &cursesPage);

            return (WINDOW *)NULL;
        }
    }

    return win;
}

void PDC_sync(WINDOW *win)
{
    PDC_LOG("PDC_sync() - called:\n");

    if (win->_immed)
        wrefresh(win);
    if (win->_sync)
        wsyncup(win);
}

WINDOW *newwin(int nlines, int ncols, int begy, int begx)
{
    WINDOW *win;

    PDC_LOG("newwin() - called:lines=%d cols=%d begy=%d begx=%d\n", nlines, ncols, begy, begx);

    if (!nlines)
        nlines = LINES - begy;
    if (!ncols)
        ncols  = COLS  - begx;

    if (!SP || begy + nlines > SP->lines || begx + ncols > SP->cols)
        return (WINDOW *)NULL;

    win = PDC_makenew(nlines, ncols, begy, begx);
    if (win)
        win = PDC_makelines(win);

    if (win)
        werase(win);

    return win;
}

int delwin(WINDOW *win)
{
    int i;

    PDC_LOG("delwin() - called\n");

    if (!win)
        return ERR;

    /* subwindows use parents' lines */

    if (!(win->_flags & (_SUBWIN|_SUBPAD)))
        for (i = 0; i < win->_maxy && win->_y[i]; i++)
            if (win->_y[i])
                free(win->_y[i], &cursesPage);

    free(win->_firstch, &cursesPage);
    free(win->_lastch, &cursesPage);
    free(win->_y, &cursesPage);
    free(win, &cursesPage);

    return OK;
}

int mvwin(WINDOW *win, int y, int x)
{
    PDC_LOG("mvwin() - called\n");

    if (!win || (y + win->_maxy > LINES || y < 0)
             || (x + win->_maxx > COLS || x < 0))
        return ERR;

    win->_begy = y;
    win->_begx = x;
    touchwin(win);

    return OK;
}
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begy, int begx)
{
    WINDOW *win;
    int i, j, k;

    PDC_LOG("subwin() - called: lines %d cols %d begy %d begx %d\n",
             nlines, ncols, begy, begx);

    /* make sure window fits inside the original one */

    if (!orig || (begy < orig->_begy) || (begx < orig->_begx) ||
        (begy + nlines) > (orig->_begy + orig->_maxy) ||
        (begx + ncols) > (orig->_begx + orig->_maxx))
        return (WINDOW *)NULL;

    j = begy - orig->_begy;
    k = begx - orig->_begx;

    if (!nlines)
        nlines = orig->_maxy - j;
    if (!ncols)
        ncols  = orig->_maxx - k;

    win = PDC_makenew(nlines, ncols, begy, begx);
    if (!win)
        return (WINDOW *)NULL;

    /* initialize window variables */

    win->_attrs = orig->_attrs;
    win->_bkgd = orig->_bkgd;
    win->_leaveit = orig->_leaveit;
    win->_scroll = orig->_scroll;
    win->_nodelay = orig->_nodelay;
    win->_delayms = orig->_delayms;
    win->_use_keypad = orig->_use_keypad;
    win->_immed = orig->_immed;
    win->_sync = orig->_sync;
    win->_pary = j;
    win->_parx = k;
    win->_parent = orig;

    for (i = 0; i < nlines; i++, j++)
        win->_y[i] = orig->_y[j] + k;

    win->_flags |= _SUBWIN;

    return win;
}

WINDOW *derwin(WINDOW *orig, int nlines, int ncols, int begy, int begx)
{
    return subwin(orig, nlines, ncols, begy + orig->_begy, begx + orig->_begx);
}

int mvderwin(WINDOW *win, int pary, int parx)
{
    int i, j;
    WINDOW *mypar;

    if (!win || !(win->_parent))
        return ERR;

    mypar = win->_parent;

    if (pary < 0 || parx < 0 || (pary + win->_maxy) > mypar->_maxy ||
                                (parx + win->_maxx) > mypar->_maxx)
        return ERR;

    j = pary;

    for (i = 0; i < win->_maxy; i++)
        win->_y[i] = (mypar->_y[j++]) + parx;

    win->_pary = pary;
    win->_parx = parx;

    return OK;
}

WINDOW *dupwin(WINDOW *win)
{
    WINDOW *new;
    chtype *ptr, *ptr1;
    int nlines, ncols, begy, begx, i;

    if (!win)
        return (WINDOW *)NULL;

    nlines = win->_maxy;
    ncols = win->_maxx;
    begy = win->_begy;
    begx = win->_begx;

    new = PDC_makenew(nlines, ncols, begy, begx);
    if (new)
        new = PDC_makelines(new);

    if (!new)
        return (WINDOW *)NULL;

    /* copy the contents of win into new */

    for (i = 0; i < nlines; i++)
    {
        for (ptr = new->_y[i], ptr1 = win->_y[i];
             ptr < new->_y[i] + ncols; ptr++, ptr1++)
            *ptr = *ptr1;

        new->_firstch[i] = 0;
        new->_lastch[i] = ncols - 1;
    }

    new->_curx = win->_curx;
    new->_cury = win->_cury;
    new->_maxy = win->_maxy;
    new->_maxx = win->_maxx;
    new->_begy = win->_begy;
    new->_begx = win->_begx;
    new->_flags = win->_flags;
    new->_attrs = win->_attrs;
    new->_clear = win->_clear;
    new->_leaveit = win->_leaveit;
    new->_scroll = win->_scroll;
    new->_nodelay = win->_nodelay;
    new->_delayms = win->_delayms;
    new->_use_keypad = win->_use_keypad;
    new->_tmarg = win->_tmarg;
    new->_bmarg = win->_bmarg;
    new->_parx = win->_parx;
    new->_pary = win->_pary;
    new->_parent = win->_parent;
    new->_bkgd = win->_bkgd;
    new->_flags = win->_flags;

    return new;
}

WINDOW *wgetparent(const WINDOW *win)
{
    PDC_LOG("wgetparent() - called\n");

    if (!win || !win->_parent)
        return NULL;

    return win->_parent;
}

WINDOW *resize_window(WINDOW *win, int nlines, int ncols)
{
    WINDOW *new;
    int i, save_cury, save_curx, new_begy, new_begx;

    PDC_LOG("resize_window() - called: nlines %d ncols %d\n",
             nlines, ncols);

    if (!win || !SP)
        return (WINDOW *)NULL;

    if (win->_flags & _SUBPAD)
    {
        new = subpad(win->_parent, nlines, ncols, win->_begy, win->_begx);
        if (!new)
            return (WINDOW *)NULL;
    }
    else if (win->_flags & _SUBWIN)
    {
        new = subwin(win->_parent, nlines, ncols, win->_begy, win->_begx);
        if (!new)
            return (WINDOW *)NULL;
    }
    else
    {
        if (win == SP->slk_winptr)
        {
            new_begy = SP->lines - SP->slklines;
            new_begx = 0;
        }
        else
        {
            new_begy = win->_begy;
            new_begx = win->_begx;
        }

        new = PDC_makenew(nlines, ncols, new_begy, new_begx);
        if (!new)
            return (WINDOW *)NULL;
    }

    save_curx = min(win->_curx, (new->_maxx - 1));
    save_cury = min(win->_cury, (new->_maxy - 1));

    if (!(win->_flags & (_SUBPAD|_SUBWIN)))
    {
        new = PDC_makelines(new);
        if (!new)
            return (WINDOW *)NULL;

        new->_bkgd = win->_bkgd;
        werase(new);

        copywin(win, new, 0, 0, 0, 0, min(win->_maxy, new->_maxy) - 1,
                min(win->_maxx, new->_maxx) - 1, FALSE);

        for (i = 0; i < win->_maxy && win->_y[i]; i++)
            if (win->_y[i])
                free(win->_y[i], &cursesPage);
    }

    new->_flags = win->_flags;
    new->_attrs = win->_attrs;
    new->_clear = win->_clear;
    new->_leaveit = win->_leaveit;
    new->_scroll = win->_scroll;
    new->_nodelay = win->_nodelay;
    new->_delayms = win->_delayms;
    new->_use_keypad = win->_use_keypad;
    new->_tmarg = (win->_tmarg > new->_maxy - 1) ? 0 : win->_tmarg;
    new->_bmarg = (win->_bmarg == win->_maxy - 1) ?
                  new->_maxy - 1 : min(win->_bmarg, (new->_maxy - 1));
    new->_parent = win->_parent;
    new->_immed = win->_immed;
    new->_sync = win->_sync;
    new->_bkgd = win->_bkgd;

    new->_curx = save_curx;
    new->_cury = save_cury;

    free(win->_firstch, &cursesPage);
    free(win->_lastch, &cursesPage);
    free(win->_y, &cursesPage);

    *win = *new;
    free(new, &cursesPage);

    return win;
}

int wresize(WINDOW *win, int nlines, int ncols)
{
    return (resize_window(win, nlines, ncols) ? OK : ERR);
}

void wsyncup(WINDOW *win)
{
    WINDOW *tmp;

    PDC_LOG("wsyncup() - called\n");

    for (tmp = win; tmp; tmp = tmp->_parent)
        touchwin(tmp);
}

int syncok(WINDOW *win, bool bf)
{
    PDC_LOG("syncok() - called\n");

    if (!win)
        return ERR;

    win->_sync = bf;

    return OK;
}

bool is_subwin(const WINDOW *win)
{
    PDC_LOG("is_subwin() - called\n");

    if (!win)
        return FALSE;

    return ((win->_flags & _SUBWIN) ? TRUE : FALSE);
}

bool is_syncok(const WINDOW *win)
{
    PDC_LOG("is_syncok() - called\n");

    if (!win)
        return FALSE;

    return win->_sync;
}

void wcursyncup(WINDOW *win)
{
    WINDOW *tmp;

    PDC_LOG("wcursyncup() - called\n");

    for (tmp = win; tmp && tmp->_parent; tmp = tmp->_parent)
        wmove(tmp->_parent, tmp->_pary + tmp->_cury, tmp->_parx + tmp->_curx);
}

void wsyncdown(WINDOW *win)
{
    WINDOW *tmp;

    PDC_LOG("wsyncdown() - called\n");

    for (tmp = win; tmp; tmp = tmp->_parent)
    {
        if (is_wintouched(tmp))
        {
            touchwin(win);
            break;
        }
    }
}
