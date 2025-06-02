/* PDCurses */

#include "curspriv.h"

int waddch(WINDOW *win, const chtype ch)
{
    int x, y;
    chtype text, attr;
    bool xlat;

    PDC_LOG("waddch() - called: win=%p ch=%x (text=%c attr=0x%x)\n",
             win, ch, ch & A_CHARTEXT, ch & A_ATTRIBUTES);

    if (!win || !SP)
        return ERR;

    x = win->_curx;
    y = win->_cury;

    if (y > win->_maxy || x > win->_maxx || y < 0 || x < 0)
        return ERR;

    xlat = !SP->raw_out && !(ch & A_ALTCHARSET);
    text = ch & A_CHARTEXT;
    attr = ch & A_ATTRIBUTES;

    if (xlat && (text < ' ' || text == 0x7f))
    {
        int x2;

        switch (text)
        {
        case '\t':
            for (x2 = ((x / TABSIZE) + 1) * TABSIZE; x < x2; x++)
            {
                if (waddch(win, attr | ' ') == ERR)
                    return ERR;

                /* if tab to next line, exit the loop */

                if (!win->_curx)
                    break;
            }
            return OK;

        case '\n':
            /* if lf -> crlf */

            if (!SP->raw_out)
                x = 0;

            wclrtoeol(win);

            if (++y > win->_bmarg)
            {
                y--;

                if (wscrl(win, 1) == ERR)
                    return ERR;
            }

            break;

        case '\b':
            /* don't back over left margin */

            if (--x < 0)
        case '\r':
                x = 0;

            break;

        case 0x7f:
            if (waddch(win, attr | '^') == ERR)
                return ERR;

            return waddch(win, attr | '?');

        default:
            /* handle control chars */

            if (waddch(win, attr | '^') == ERR)
                return ERR;

            return waddch(win, ch + '@');
        }
    }
    else
    {
        /* If the incoming character doesn't have its own attribute,
           then use the current attributes for the window. If it has
           attributes but not a color component, OR the attributes to
           the current attributes for the window. If it has a color
           component, use the attributes solely from the incoming
           character. */

        if (!(attr & A_COLOR))
            attr |= win->_attrs;

        /* wrs (4/10/93): Apply the same sort of logic for the window
           background, in that it only takes precedence if other color
           attributes are not there and that the background character
           will only print if the printing character is blank. */

        if (!(attr & A_COLOR))
            attr |= win->_bkgd & A_ATTRIBUTES;
        else
            attr |= win->_bkgd & (A_ATTRIBUTES ^ A_COLOR);

        if (text == ' ')
            text = win->_bkgd & A_CHARTEXT;

        /* Add the attribute back into the character. */

        text |= attr;

        /* Only change _firstch/_lastch if the character to be added is
           different from the character/attribute that is already in
           that position in the window. */

        if (win->_y[y][x] != text)
        {
            if (win->_firstch[y] == _NO_CHANGE)
                win->_firstch[y] = win->_lastch[y] = x;
            else
                if (x < win->_firstch[y])
                    win->_firstch[y] = x;
                else
                    if (x > win->_lastch[y])
                        win->_lastch[y] = x;

            win->_y[y][x] = text;
        }

        if (++x >= win->_maxx)
        {
            /* wrap around test */

            x = 0;

            if (++y > win->_bmarg)
            {
                y--;

                if (wscrl(win, 1) == ERR)
                {
                    PDC_sync(win);
                    return ERR;
                }
            }
        }
    }

    win->_curx = x;
    win->_cury = y;

    if (win->_immed)
        wrefresh(win);
    if (win->_sync)
        wsyncup(win);

    return OK;
}

int addch(const chtype ch)
{
    PDC_LOG("addch() - called: ch=%x\n", ch);

    return waddch(stdscr, ch);
}

int mvaddch(int y, int x, const chtype ch)
{
    PDC_LOG("mvaddch() - called: y=%d x=%d ch=%x\n", y, x, ch);

    if (move(y,x) == ERR)
        return ERR;

    return waddch(stdscr, ch);
}

int mvwaddch(WINDOW *win, int y, int x, const chtype ch)
{
    PDC_LOG("mvwaddch() - called: win=%p y=%d x=%d ch=%d\n", win, y, x, ch);

    if (wmove(win, y, x) == ERR)
        return ERR;

    return waddch(win, ch);
}

int echochar(const chtype ch)
{
    PDC_LOG("echochar() - called: ch=%x\n", ch);

    return wechochar(stdscr, ch);
}

int wechochar(WINDOW *win, const chtype ch)
{
    PDC_LOG("wechochar() - called: win=%p ch=%x\n", win, ch);

    if (waddch(win, ch) == ERR)
        return ERR;

    return wrefresh(win);
}

int waddrawch(WINDOW *win, chtype ch)
{
    PDC_LOG("waddrawch() - called: win=%p ch=%x (text=%c attr=0x%x)\n",
             win, ch, ch & A_CHARTEXT, ch & A_ATTRIBUTES);

    if ((ch & A_CHARTEXT) < ' ' || (ch & A_CHARTEXT) == 0x7f)
        ch |= A_ALTCHARSET;

    return waddch(win, ch);
}

int addrawch(chtype ch)
{
    PDC_LOG("addrawch() - called: ch=%x\n", ch);

    return waddrawch(stdscr, ch);
}

int mvaddrawch(int y, int x, chtype ch)
{
    PDC_LOG("mvaddrawch() - called: y=%d x=%d ch=%d\n", y, x, ch);

    if (move(y, x) == ERR)
        return ERR;

    return waddrawch(stdscr, ch);
}

int mvwaddrawch(WINDOW *win, int y, int x, chtype ch)
{
    PDC_LOG("mvwaddrawch() - called: win=%p y=%d x=%d ch=%d\n",
             win, y, x, ch);

    if (wmove(win, y, x) == ERR)
        return ERR;

    return waddrawch(win, ch);
}

#ifdef PDC_WIDE
int wadd_wch(WINDOW *win, const cchar_t *wch)
{
    PDC_LOG("wadd_wch() - called: win=%p wch=%x\n", win, *wch);

    return wch ? waddch(win, *wch) : ERR;
}

int add_wch(const cchar_t *wch)
{
    PDC_LOG("add_wch() - called: wch=%x\n", *wch);

    return wadd_wch(stdscr, wch);
}

int mvadd_wch(int y, int x, const cchar_t *wch)
{
    PDC_LOG("mvaddch() - called: y=%d x=%d wch=%x\n", y, x, *wch);

    if (move(y,x) == ERR)
        return ERR;

    return wadd_wch(stdscr, wch);
}

int mvwadd_wch(WINDOW *win, int y, int x, const cchar_t *wch)
{
    PDC_LOG("mvwaddch() - called: win=%p y=%d x=%d wch=%d\n",
             win, y, x, *wch);

    if (wmove(win, y, x) == ERR)
        return ERR;

    return wadd_wch(win, wch);
}

int echo_wchar(const cchar_t *wch)
{
    PDC_LOG("echo_wchar() - called: wch=%x\n", *wch);

    return wecho_wchar(stdscr, wch);
}

int wecho_wchar(WINDOW *win, const cchar_t *wch)
{
    PDC_LOG("wecho_wchar() - called: win=%p wch=%x\n", win, *wch);

    if (!wch || (wadd_wch(win, wch) == ERR))
        return ERR;

    return wrefresh(win);
}
#endif
