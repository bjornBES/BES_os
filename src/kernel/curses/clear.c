#include "curspriv.h"
#include "stdlib.h"
#include "memory.h"

int wclrtoeol(WINDOW *win)
{
    int x, y, minx;
    chtype blank, *ptr;

    PDC_LOG("wclrtoeol() - called: Row: %d Col: %d\n",
             win->_cury, win->_curx);

    if (!win)
        return ERR;

    y = win->_cury;
    x = win->_curx;

    /* wrs (4/10/93) account for window background */

    blank = win->_bkgd;

    for (minx = x, ptr = &win->_y[y][x]; minx < win->_maxx; minx++, ptr++)
        *ptr = blank;

    if (x < win->_firstch[y] || win->_firstch[y] == _NO_CHANGE)
        win->_firstch[y] = x;

    win->_lastch[y] = win->_maxx - 1;

    PDC_sync(win);
    return OK;
}

int clrtoeol(void)
{
    PDC_LOG("clrtoeol() - called\n");

    return wclrtoeol(stdscr);
}

int wclrtobot(WINDOW *win)
{
    int savey, savex;

    PDC_LOG("wclrtobot() - called\n");

    if (!win)
        return ERR;

    savey = win->_cury;
    savex = win->_curx;

    /* should this involve scrolling region somehow ? */

    if (win->_cury + 1 < win->_maxy)
    {
        win->_curx = 0;
        win->_cury++;
        for (; win->_maxy > win->_cury; win->_cury++)
            wclrtoeol(win);
        win->_cury = savey;
        win->_curx = savex;
    }
    wclrtoeol(win);

    PDC_sync(win);
    return OK;
}

int clrtobot(void)
{
    PDC_LOG("clrtobot() - called\n");

    return wclrtobot(stdscr);
}

int werase(WINDOW *win)
{
    PDC_LOG("werase() - called\n");

    if (wmove(win, 0, 0) == ERR)
        return ERR;

    return wclrtobot(win);
}