#include "curspriv.h"
#include "stdlib.h"
#include "memory.h"


int touchwin(WINDOW *win)
{
    int i;

    PDC_LOG("touchwin() - called: Win=%x\n", win);

    if (!win)
        return ERR;

    for (i = 0; i < win->_maxy; i++)
    {
        win->_firstch[i] = 0;
        win->_lastch[i] = win->_maxx - 1;
    }

    return OK;
}

int touchline(WINDOW *win, int start, int count)
{
    int i;

    PDC_LOG("touchline() - called: win=%p start %d count %d\n",
             win, start, count);

    if (!win || start > win->_maxy || start + count > win->_maxy)
        return ERR;

    for (i = start; i < start + count; i++)
    {
        win->_firstch[i] = 0;
        win->_lastch[i] = win->_maxx - 1;
    }

    return OK;
}

int untouchwin(WINDOW *win)
{
    int i;

    PDC_LOG("untouchwin() - called: win=%p", win);

    if (!win)
        return ERR;

    for (i = 0; i < win->_maxy; i++)
    {
        win->_firstch[i] = _NO_CHANGE;
        win->_lastch[i] = _NO_CHANGE;
    }

    return OK;
}

int wtouchln(WINDOW *win, int y, int n, int changed)
{
    int i;

    PDC_LOG("wtouchln() - called: win=%p y=%d n=%d changed=%d\n",
             win, y, n, changed);

    if (!win || y > win->_maxy || y + n > win->_maxy)
        return ERR;

    for (i = y; i < y + n; i++)
    {
        if (changed)
        {
            win->_firstch[i] = 0;
            win->_lastch[i] = win->_maxx - 1;
        }
        else
        {
            win->_firstch[i] = _NO_CHANGE;
            win->_lastch[i] = _NO_CHANGE;
        }
    }

    return OK;
}

bool is_linetouched(WINDOW *win, int line)
{
    PDC_LOG("is_linetouched() - called: win=%p line=%d\n", win, line);

    if (!win || line > win->_maxy || line < 0)
        return FALSE;

    return (win->_firstch[line] != _NO_CHANGE) ? TRUE : FALSE;
}

bool is_wintouched(WINDOW *win)
{
    int i;

    PDC_LOG("is_wintouched() - called: win=%p\n", win);

    if (win)
        for (i = 0; i < win->_maxy; i++)
            if (win->_firstch[i] != _NO_CHANGE)
                return TRUE;

    return FALSE;
}

int touchoverlap(const WINDOW *win1, WINDOW *win2)
{
    int y, endy, endx, starty, startx, begy1, begx1, begy2, begx2;

    PDC_LOG("touchoverlap() - called: win1=%p win2=%p\n", win1, win2);

    if (!win1 || !win2)
        return ERR;

    begy1 = win1->_begy;
    begx1 = win1->_begx;
    begy2 = win2->_begy;
    begx2 = win2->_begy;

    starty = max(begy1, begy2);
    startx = max(begx1, begx2);
    endy = min(win1->_maxy + begy1, win2->_maxy + begy2);
    endx = min(win1->_maxx + begx1, win2->_maxx + begx2);

    if (starty >= endy || startx >= endx)
        return OK;

    starty -= begy2;
    startx -= begx2;
    endy -= begy2;
    endx -= begx2;
    endx -= 1;

    for (y = starty; y < endy; y++)
    {
        int first = win2->_firstch[y];

        if (first == _NO_CHANGE || win2->_lastch[y] < endx)
            win2->_lastch[y] = endx;
        if (first == _NO_CHANGE || first > startx)
            win2->_firstch[y] = startx;
    }

    return OK;
}