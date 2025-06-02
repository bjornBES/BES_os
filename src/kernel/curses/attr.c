
#include "curspriv.h"

int wattroff(WINDOW *win, chtype attrs)
{
    PDC_LOG("wattroff() - called\n");

    if (!win)
        return ERR;

    win->_attrs &= (~attrs & A_ATTRIBUTES);

    return OK;
}

int attroff(chtype attrs)
{
    PDC_LOG("attroff() - called\n");

    return wattroff(stdscr, attrs);
}

int wattron(WINDOW *win, chtype attrs)
{
    chtype newcolr, oldcolr, newattr, oldattr;

    PDC_LOG("wattron() - called\n");

    if (!win)
        return ERR;

    if ((win->_attrs & A_COLOR) && (attrs & A_COLOR))
    {
        oldcolr = win->_attrs & A_COLOR;
        oldattr = win->_attrs ^ oldcolr;
        newcolr = attrs & A_COLOR;
        newattr = (attrs & A_ATTRIBUTES) ^ newcolr;
        newattr |= oldattr;
        win->_attrs = newattr | newcolr;
    }
    else
        win->_attrs |= (attrs & A_ATTRIBUTES);

    return OK;
}

int attron(chtype attrs)
{
    PDC_LOG("attron() - called\n");

    return wattron(stdscr, attrs);
}

int wattrset(WINDOW *win, chtype attrs)
{
    PDC_LOG("wattrset() - called\n");

    if (!win)
        return ERR;

    win->_attrs = attrs & A_ATTRIBUTES;

    return OK;
}

int attrset(chtype attrs)
{
    PDC_LOG("attrset() - called\n");

    return wattrset(stdscr, attrs);
}

int standend(void)
{
    PDC_LOG("standend() - called\n");

    return wattrset(stdscr, A_NORMAL);
}

int standout(void)
{
    PDC_LOG("standout() - called\n");

    return wattrset(stdscr, A_STANDOUT);
}

int wstandend(WINDOW *win)
{
    PDC_LOG("wstandend() - called\n");

    return wattrset(win, A_NORMAL);
}

int wstandout(WINDOW *win)
{
    PDC_LOG("wstandout() - called\n");

    return wattrset(win, A_STANDOUT);
}

chtype getattrs(WINDOW *win)
{
    return win ? win->_attrs : 0;
}

int wcolor_set(WINDOW *win, short color_pair, void *opts)
{
    PDC_LOG("wcolor_set() - called\n");

    if (!win)
        return ERR;

    win->_attrs = (win->_attrs & ~A_COLOR) | COLOR_PAIR(color_pair);

    return OK;
}

int color_set(short color_pair, void *opts)
{
    PDC_LOG("color_set() - called\n");

    return wcolor_set(stdscr, color_pair, opts);
}

int wattr_get(WINDOW *win, attr_t *attrs, short *color_pair, void *opts)
{
    PDC_LOG("wattr_get() - called\n");

    if (!win)
        return ERR;

    if (attrs)
        *attrs = win->_attrs & (A_ATTRIBUTES & ~A_COLOR);

    if (color_pair)
        *color_pair = PAIR_NUMBER(win->_attrs);

    return OK;
}

int attr_get(attr_t *attrs, short *color_pair, void *opts)
{
    PDC_LOG("attr_get() - called\n");

    return wattr_get(stdscr, attrs, color_pair, opts);
}

int wattr_off(WINDOW *win, attr_t attrs, void *opts)
{
    PDC_LOG("wattr_off() - called\n");

    return wattroff(win, attrs);
}

int attr_off(attr_t attrs, void *opts)
{
    PDC_LOG("attr_off() - called\n");

    return wattroff(stdscr, attrs);
}

int wattr_on(WINDOW *win, attr_t attrs, void *opts)
{
    PDC_LOG("wattr_off() - called\n");

    return wattron(win, attrs);
}

int attr_on(attr_t attrs, void *opts)
{
    PDC_LOG("attr_on() - called\n");

    return wattron(stdscr, attrs);
}

int wattr_set(WINDOW *win, attr_t attrs, short color_pair, void *opts)
{
    PDC_LOG("wattr_set() - called\n");

    if (!win)
        return ERR;

    win->_attrs = (attrs & (A_ATTRIBUTES & ~A_COLOR)) | COLOR_PAIR(color_pair);

    return OK;
}

int attr_set(attr_t attrs, short color_pair, void *opts)
{
    PDC_LOG("attr_get() - called\n");

    return wattr_set(stdscr, attrs, color_pair, opts);
}

int wchgat(WINDOW *win, int n, attr_t attr, short color, const void *opts)
{
    chtype *dest, newattr;
    int startpos, endpos;

    PDC_LOG("wchgat() - called\n");

    if (!win)
        return ERR;

    newattr = (attr & A_ATTRIBUTES) | COLOR_PAIR(color);

    startpos = win->_curx;
    endpos = ((n < 0) ? win->_maxx : min(startpos + n, win->_maxx)) - 1;
    dest = win->_y[win->_cury];

    for (n = startpos; n <= endpos; n++)
        dest[n] = (dest[n] & A_CHARTEXT) | newattr;

    n = win->_cury;

    if (startpos < win->_firstch[n] || win->_firstch[n] == _NO_CHANGE)
        win->_firstch[n] = startpos;

    if (endpos > win->_lastch[n])
        win->_lastch[n] = endpos;

    PDC_sync(win);

    return OK;
}

int chgat(int n, attr_t attr, short color, const void *opts)
{
    PDC_LOG("chgat() - called\n");

    return wchgat(stdscr, n, attr, color, opts);
}

int mvchgat(int y, int x, int n, attr_t attr, short color, const void *opts)
{
    PDC_LOG("mvchgat() - called\n");

    if (move(y, x) == ERR)
        return ERR;

    return wchgat(stdscr, n, attr, color, opts);
}

int mvwchgat(WINDOW *win, int y, int x, int n, attr_t attr, short color,
             const void *opts)
{
    PDC_LOG("mvwchgat() - called\n");

    if (wmove(win, y, x) == ERR)
        return ERR;

    return wchgat(win, n, attr, color, opts);
}

int underend(void)
{
    PDC_LOG("underend() - called\n");

    return wattroff(stdscr, A_UNDERLINE);
}

int wunderend(WINDOW *win)
{
    PDC_LOG("wunderend() - called\n");

    return wattroff(win, A_UNDERLINE);
}

int underscore(void)
{
    PDC_LOG("underscore() - called\n");

    return wattron(stdscr, A_UNDERLINE);
}

int wunderscore(WINDOW *win)
{
    PDC_LOG("wunderscore() - called\n");

    return wattron(win, A_UNDERLINE);
}
