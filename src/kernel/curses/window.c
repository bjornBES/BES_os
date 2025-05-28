#include "curspriv.h"
#include <stdlib.h>
#include "malloc.h"

WINDOW *PDC_makenew(int nlines, int ncols, int begy, int begx)
{
    WINDOW *win;

    PDC_LOG("PDC_makenew() - called: lines %d cols %d begy %d begx %d\n", nlines, ncols, begy, begx);

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