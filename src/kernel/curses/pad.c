/* PDCurses */

#include "curspriv.h"
#include "string.h"

WINDOW *newpad(int nlines, int ncols)
{
    WINDOW *win;

    PDC_LOG("newpad() - called: lines=%d cols=%d\n", nlines, ncols);

    win = PDC_makenew(nlines, ncols, 0, 0);
    if (win)
        win = PDC_makelines(win);

    if (!win)
        return (WINDOW *)NULL;

    werase(win);

    win->_flags = _PAD;
    win->_pad._pad_y = 0;
    win->_pad._pad_x = 0;
    win->_pad._pad_top = 0;
    win->_pad._pad_left = 0;
    win->_pad._pad_bottom = min(LINES, nlines) - 1;
    win->_pad._pad_right = min(COLS, ncols) - 1;

    return win;
}

WINDOW *subpad(WINDOW *orig, int nlines, int ncols, int begy, int begx)
{
    WINDOW *win;
    int i;

    PDC_LOG("subpad() - called: lines=%d cols=%d begy=%d begx=%d\n",
             nlines, ncols, begy, begx);

    if (!orig || !(orig->_flags & _PAD))
        return (WINDOW *)NULL;

    /* make sure window fits inside the original one */

    if (begy < 0 || begx < 0 ||
        (begy + nlines) > orig->_maxy ||
        (begx + ncols)  > orig->_maxx)
        return (WINDOW *)NULL;

    if (!nlines)
        nlines = orig->_maxy - begy;

    if (!ncols)
        ncols = orig->_maxx - begx;

    win = PDC_makenew(nlines, ncols, begy, begx);
    if (!win)
        return (WINDOW *)NULL;

    /* initialize window variables */

    win->_attrs = orig->_attrs;
    win->_leaveit = orig->_leaveit;
    win->_scroll = orig->_scroll;
    win->_nodelay = orig->_nodelay;
    win->_use_keypad = orig->_use_keypad;
    win->_parent = orig;

    for (i = 0; i < nlines; i++)
        win->_y[i] = orig->_y[begy + i] + begx;

    win->_flags = _SUBPAD;
    win->_pad._pad_y = 0;
    win->_pad._pad_x = 0;
    win->_pad._pad_top = 0;
    win->_pad._pad_left = 0;
    win->_pad._pad_bottom = min(LINES, nlines) - 1;
    win->_pad._pad_right = min(COLS, ncols) - 1;

    return win;
}

int prefresh(WINDOW *win, int py, int px, int sy1, int sx1, int sy2, int sx2)
{
    PDC_LOG("prefresh() - called\n");

    if (pnoutrefresh(win, py, px, sy1, sx1, sy2, sx2) == ERR)
        return ERR;

    doupdate();
    return OK;
}

int pnoutrefresh(WINDOW *w, int py, int px, int sy1, int sx1, int sy2, int sx2)
{
    int num_cols;
    int sline;
    int pline;

    PDC_LOG("pnoutrefresh() - called\n");

    if (py < 0)
        py = 0;
    if (px < 0)
        px = 0;
    if (sy1 < 0)
        sy1 = 0;
    if (sx1 < 0)
        sx1 = 0;

    if ((!w || !(w->_flags & (_PAD|_SUBPAD)) ||
        (sy2 >= LINES) || (sx2 >= COLS)) ||
        (sy2 < sy1) || (sx2 < sx1))
        return ERR;

    sline = sy1;
    pline = py;

    num_cols = min((sx2 - sx1 + 1), (w->_maxx - px));

    while (sline <= sy2)
    {
        if (pline < w->_maxy)
        {
            memcpy(curscr->_y[sline] + sx1, w->_y[pline] + px,
                   num_cols * sizeof(chtype));

            if ((curscr->_firstch[sline] == _NO_CHANGE)
                || (curscr->_firstch[sline] > sx1))
                curscr->_firstch[sline] = sx1;

            if (sx2 > curscr->_lastch[sline])
                curscr->_lastch[sline] = sx2;

            w->_firstch[pline] = _NO_CHANGE; /* updated now */
            w->_lastch[pline] = _NO_CHANGE;  /* updated now */
        }

        sline++;
        pline++;
    }

    if (w->_clear)
    {
        w->_clear = FALSE;
        curscr->_clear = TRUE;
    }

    /* position the cursor to the pad's current position if possible --
       is the pad current position going to end up displayed? if not,
       then don't move the cursor; if so, move it to the correct place */

    if (!w->_leaveit && w->_cury >= py && w->_curx >= px &&
         w->_cury <= py + (sy2 - sy1) && w->_curx <= px + (sx2 - sx1))
    {
        curscr->_cury = (w->_cury - py) + sy1;
        curscr->_curx = (w->_curx - px) + sx1;
    }

    w->_pad._pad_y = py;
    w->_pad._pad_x = px;
    w->_pad._pad_top = sy1;
    w->_pad._pad_left = sx1;
    w->_pad._pad_bottom = sy2;
    w->_pad._pad_right = sx2;

    return OK;
}

int pechochar(WINDOW *pad, chtype ch)
{
    PDC_LOG("pechochar() - called\n");

    if (waddch(pad, ch) == ERR)
        return ERR;

    return prefresh(pad, pad->_pad._pad_y, pad->_pad._pad_x, pad->_pad._pad_top,
                    pad->_pad._pad_left, pad->_pad._pad_bottom, pad->_pad._pad_right);
}

#ifdef PDC_WIDE
int pecho_wchar(WINDOW *pad, const cchar_t *wch)
{
    PDC_LOG("pecho_wchar() - called\n");

    if (!wch || (waddch(pad, *wch) == ERR))
        return ERR;

    return prefresh(pad, pad->_pad._pad_y, pad->_pad._pad_x, pad->_pad._pad_top,
                    pad->_pad._pad_left, pad->_pad._pad_bottom, pad->_pad._pad_right);
}
#endif

bool is_pad(const WINDOW *pad)
{
    PDC_LOG("is_pad() - called\n");

    if (!pad)
        return FALSE;

    return (pad->_flags & _PAD) ? TRUE : FALSE;
}
