/* PDCurses */

#include "curspriv.h"

int clearok(WINDOW *win, bool bf)
{
    PDC_LOG("clearok() - called\n");

    if (!win)
        return ERR;

    win->_clear = bf;

    return OK;
}

int idlok(WINDOW *win, bool bf)
{
    PDC_LOG("idlok() - called\n");

    return OK;
}

void idcok(WINDOW *win, bool bf)
{
    PDC_LOG("idcok() - called\n");
}

void immedok(WINDOW *win, bool bf)
{
    PDC_LOG("immedok() - called\n");

    if (win)
        win->_immed = bf;
}

int leaveok(WINDOW *win, bool bf)
{
    PDC_LOG("leaveok() - called\n");

    if (!win)
        return ERR;

    win->_leaveit = bf;

    curs_set(!bf);

    return OK;
}

int setscrreg(int top, int bottom)
{
    PDC_LOG("setscrreg() - called: top %d bottom %d\n", top, bottom);

    return wsetscrreg(stdscr, top, bottom);
}

int wsetscrreg(WINDOW *win, int top, int bottom)
{
    PDC_LOG("wsetscrreg() - called: top %d bottom %d\n", top, bottom);

    if (win && 0 <= top && top <= win->_cury &&
        win->_cury <= bottom && bottom < win->_maxy)
    {
        win->_tmarg = top;
        win->_bmarg = bottom;

        return OK;
    }
    else
        return ERR;
}

int wgetscrreg(const WINDOW *win, int *top, int *bot)
{
    PDC_LOG("wgetscrreg() - called\n");

    if (!win || !top || !bot)
        return ERR;

    *top = win->_tmarg;
    *bot = win->_bmarg;

    return OK;
}

int scrollok(WINDOW *win, bool bf)
{
    PDC_LOG("scrollok() - called\n");

    if (!win)
        return ERR;

    win->_scroll = bf;

    return OK;
}

int raw_output(bool bf)
{
    PDC_LOG("raw_output() - called\n");

    if (!SP)
        return ERR;

    SP->raw_out = bf;

    return OK;
}

bool is_cleared(const WINDOW *win)
{
    PDC_LOG("is_cleared() - called\n");

    if (!win)
        return FALSE;

    return win->_clear;
}

bool is_idlok(const WINDOW *win)
{
    (void) win;

    PDC_LOG("is_idlok() - called\n");

    return FALSE;
}

bool is_idcok(const WINDOW *win)
{
    (void) win;

    PDC_LOG("is_idcok() - called\n");

    return FALSE;
}

bool is_immedok(const WINDOW *win)
{
    PDC_LOG("is_immedok() - called\n");

    if (!win)
        return FALSE;

    return win->_immed;
}

bool is_leaveok(const WINDOW *win)
{
    PDC_LOG("is_leaveok() - called\n");

    if (!win)
        return FALSE;

    return win->_leaveit;
}

bool is_scrollok(const WINDOW *win)
{
    PDC_LOG("is_scrollok() - called\n");

    if (!win)
        return FALSE;

    return win->_scroll;
}
