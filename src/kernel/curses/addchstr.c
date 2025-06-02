
#include "curspriv.h"
#include "string.h"

int waddchnstr(WINDOW *win, const chtype *ch, int n)
{
    int y, x, maxx, minx;
    chtype *ptr;

    PDC_LOG("waddchnstr() - called: win=%p n=%d\n", win, n);

    if (!win || !ch || !n || n < -1)
        return ERR;

    x = win->_curx;
    y = win->_cury;
    ptr = &(win->_y[y][x]);

    if (n == -1 || n > win->_maxx - x)
        n = win->_maxx - x;

    minx = win->_firstch[y];
    maxx = win->_lastch[y];

    for (; n && *ch; n--, x++, ptr++, ch++)
    {
        if (*ptr != *ch)
        {
            if (x < minx || minx == _NO_CHANGE)
                minx = x;

            if (x > maxx)
                maxx = x;

            PDC_LOG("y %d x %d minx %d maxx %d *ptr %x *ch"
                     " %x firstch: %d lastch: %d\n",
                     y, x, minx, maxx, *ptr, *ch,
                     win->_firstch[y], win->_lastch[y]);

            *ptr = *ch;
        }
    }

    win->_firstch[y] = minx;
    win->_lastch[y] = maxx;

    return OK;
}

int addchstr(const chtype *ch)
{
    PDC_LOG("addchstr() - called\n");

    return waddchnstr(stdscr, ch, -1);
}

int addchnstr(const chtype *ch, int n)
{
    PDC_LOG("addchnstr() - called\n");

    return waddchnstr(stdscr, ch, n);
}

int waddchstr(WINDOW *win, const chtype *ch)
{
    PDC_LOG("waddchstr() - called: win=%p\n", win);

    return waddchnstr(win, ch, -1);
}

int mvaddchstr(int y, int x, const chtype *ch)
{
    PDC_LOG("mvaddchstr() - called: y %d x %d\n", y, x);

    if (move(y, x) == ERR)
        return ERR;

    return waddchnstr(stdscr, ch, -1);
}

int mvaddchnstr(int y, int x, const chtype *ch, int n)
{
    PDC_LOG("mvaddchnstr() - called: y %d x %d n %d\n", y, x, n);

    if (move(y, x) == ERR)
        return ERR;

    return waddchnstr(stdscr, ch, n);
}

int mvwaddchstr(WINDOW *win, int y, int x, const chtype *ch)
{
    PDC_LOG("mvwaddchstr() - called:\n");

    if (wmove(win, y, x) == ERR)
        return ERR;

    return waddchnstr(win, ch, -1);
}

int mvwaddchnstr(WINDOW *win, int y, int x, const chtype *ch, int n)
{
    PDC_LOG("mvwaddchnstr() - called: y %d x %d n %d \n", y, x, n);

    if (wmove(win, y, x) == ERR)
        return ERR;

    return waddchnstr(win, ch, n);
}

#ifdef PDC_WIDE
int wadd_wchnstr(WINDOW *win, const cchar_t *wch, int n)
{
    PDC_LOG("wadd_wchnstr() - called: win=%p n=%d\n", win, n);

    return waddchnstr(win, wch, n);
}

int add_wchstr(const cchar_t *wch)
{
    PDC_LOG("add_wchstr() - called\n");

    return wadd_wchnstr(stdscr, wch, -1);
}

int add_wchnstr(const cchar_t *wch, int n)
{
    PDC_LOG("add_wchnstr() - called\n");

    return wadd_wchnstr(stdscr, wch, n);
}

int wadd_wchstr(WINDOW *win, const cchar_t *wch)
{
    PDC_LOG("wadd_wchstr() - called: win=%p\n", win);

    return wadd_wchnstr(win, wch, -1);
}

int mvadd_wchstr(int y, int x, const cchar_t *wch)
{
    PDC_LOG("mvadd_wchstr() - called: y %d x %d\n", y, x);

    if (move(y, x) == ERR)
        return ERR;

    return wadd_wchnstr(stdscr, wch, -1);
}

int mvadd_wchnstr(int y, int x, const cchar_t *wch, int n)
{
    PDC_LOG("mvadd_wchnstr() - called: y %d x %d n %d\n", y, x, n);

    if (move(y, x) == ERR)
        return ERR;

    return wadd_wchnstr(stdscr, wch, n);
}

int mvwadd_wchstr(WINDOW *win, int y, int x, const cchar_t *wch)
{
    PDC_LOG("mvwadd_wchstr() - called:\n");

    if (wmove(win, y, x) == ERR)
        return ERR;

    return wadd_wchnstr(win, wch, -1);
}

int mvwadd_wchnstr(WINDOW *win, int y, int x, const cchar_t *wch, int n)
{
    PDC_LOG("mvwadd_wchnstr() - called: y %d x %d n %d \n", y, x, n);

    if (wmove(win, y, x) == ERR)
        return ERR;

    return wadd_wchnstr(win, wch, n);
}
#endif
