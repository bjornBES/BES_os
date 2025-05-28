#include "curspriv.h"
#include "stdlib.h"
#include "malloc.h"

int move(int y, int x)
{
    PDC_LOG("move() - called: y=%d x=%d\n", y, x);

    if (!stdscr || x < 0 || y < 0 || x >= stdscr->_maxx || y >= stdscr->_maxy)
        return ERR;

    stdscr->_curx = x;
    stdscr->_cury = y;

    return OK;
}

int mvcur(int oldrow, int oldcol, int newrow, int newcol)
{
    PDC_LOG("mvcur() - called: oldrow %d oldcol %d newrow %d newcol %d\n",
             oldrow, oldcol, newrow, newcol);

    if (!SP || newrow < 0 || newrow >= LINES || newcol < 0 || newcol >= COLS)
        return ERR;

    PDC_gotoyx(newrow, newcol);
    SP->cursrow = newrow;
    SP->curscol = newcol;

    return OK;
}

int wmove(WINDOW *win, int y, int x)
{
    PDC_LOG("wmove() - called: y=%d x=%d\n", y, x);

    if (!win || x < 0 || y < 0 || x >= win->_maxx || y >= win->_maxy)
        return ERR;

    win->_curx = x;
    win->_cury = y;

    return OK;
}
