#include "curspriv.h"

int wscrl(WINDOW *win, int n)
{
    int i, l, dir, start, end;
    chtype blank, *temp;

    /* Check if window scrolls. Valid for window AND pad */

    if (!win || !win->_scroll || !n)
        return ERR;

    blank = win->_bkgd;

    if (n > 0)
    {
        start = win->_tmarg;
        end = win->_bmarg;
        dir = 1;
    }
    else
    {
        start = win->_bmarg;
        end = win->_tmarg;
        dir = -1;
    }

    for (l = 0; l < (n * dir); l++)
    {
        temp = win->_y[start];

        /* re-arrange line pointers */

        for (i = start; i != end; i += dir)
            win->_y[i] = win->_y[i + dir];

        win->_y[end] = temp;

        /* make a blank line */

        for (i = 0; i < win->_maxx; i++)
            *temp++ = blank;
    }

    touchline(win, win->_tmarg, win->_bmarg - win->_tmarg + 1);

    PDC_sync(win);
    return OK;
}

int scrl(int n)
{
    PDC_LOG("scrl() - called\n");

    return wscrl(stdscr, n);
}

int scroll(WINDOW *win)
{
    PDC_LOG("scroll() - called\n");

    return wscrl(win, 1);
}
