
#include "curspriv.h"

int waddnstr(WINDOW *win, const char *str, int n)
{
    int i = 0;

    PDC_LOG("waddnstr() - called: string=\"%s\" n %d \n", str, n);

    if (!win || !str)
        return ERR;

    while (str[i] && (i < n || n < 0))
    {
#ifdef PDC_WIDE
        wchar_t wch;
        int retval = PDC_mbtowc(&wch, str + i, n >= 0 ? n - i : 6);

        if (retval <= 0)
            return OK;

        i += retval;
#else
        chtype wch = (unsigned char)(str[i++]);
#endif
        if (waddch(win, wch) == ERR)
            return ERR;
    }

    return OK;
}

int addstr(const char *str)
{
    PDC_LOG("addstr() - called: string=\"%s\"\n", str);

    return waddnstr(stdscr, str, -1);
}

int addnstr(const char *str, int n)
{
    PDC_LOG("addnstr() - called: string=\"%s\" n %d \n", str, n);

    return waddnstr(stdscr, str, n);
}

int waddstr(WINDOW *win, const char *str)
{
    PDC_LOG("waddstr() - called: string=\"%s\"\n", str);

    return waddnstr(win, str, -1);
}

int mvaddstr(int y, int x, const char *str)
{
    PDC_LOG("mvaddstr() - called: y %d x %d string=\"%s\"\n", y, x, str);

    if (move(y, x) == ERR)
        return ERR;

    return waddnstr(stdscr, str, -1);
}

int mvaddnstr(int y, int x, const char *str, int n)
{
    PDC_LOG("mvaddnstr() - called: y %d x %d string=\"%s\" n %d \n",
             y, x, str, n);

    if (move(y, x) == ERR)
        return ERR;

    return waddnstr(stdscr, str, n);
}

int mvwaddstr(WINDOW *win, int y, int x, const char *str)
{
    PDC_LOG("mvwaddstr() - called: string=\"%s\"\n", str);

    if (wmove(win, y, x) == ERR)
        return ERR;

    return waddnstr(win, str, -1);
}

int mvwaddnstr(WINDOW *win, int y, int x, const char *str, int n)
{
    PDC_LOG("mvwaddnstr() - called: y %d x %d string=\"%s\" n %d \n",
             y, x, str, n);

    if (wmove(win, y, x) == ERR)
        return ERR;

    return waddnstr(win, str, n);
}

#ifdef PDC_WIDE
int waddnwstr(WINDOW *win, const wchar_t *wstr, int n)
{
    int i = 0;

    PDC_LOG("waddnwstr() - called\n");

    if (!win || !wstr)
        return ERR;

    while (wstr[i] && (i < n || n < 0))
    {
        chtype wch = wstr[i++];

        if (waddch(win, wch) == ERR)
            return ERR;
    }

    return OK;
}

int addwstr(const wchar_t *wstr)
{
    PDC_LOG("addwstr() - called\n");

    return waddnwstr(stdscr, wstr, -1);
}

int addnwstr(const wchar_t *wstr, int n)
{
    PDC_LOG("addnwstr() - called\n");

    return waddnwstr(stdscr, wstr, n);
}

int waddwstr(WINDOW *win, const wchar_t *wstr)
{
    PDC_LOG("waddwstr() - called\n");

    return waddnwstr(win, wstr, -1);
}

int mvaddwstr(int y, int x, const wchar_t *wstr)
{
    PDC_LOG("mvaddstr() - called\n");

    if (move(y, x) == ERR)
        return ERR;

    return waddnwstr(stdscr, wstr, -1);
}

int mvaddnwstr(int y, int x, const wchar_t *wstr, int n)
{
    PDC_LOG("mvaddnstr() - called\n");

    if (move(y, x) == ERR)
        return ERR;

    return waddnwstr(stdscr, wstr, n);
}

int mvwaddwstr(WINDOW *win, int y, int x, const wchar_t *wstr)
{
    PDC_LOG("mvwaddstr() - called\n");

    if (wmove(win, y, x) == ERR)
        return ERR;

    return waddnwstr(win, wstr, -1);
}

int mvwaddnwstr(WINDOW *win, int y, int x, const wchar_t *wstr, int n)
{
    PDC_LOG("mvwaddnstr() - called\n");

    if (wmove(win, y, x) == ERR)
        return ERR;

    return waddnwstr(win, wstr, n);
}
#endif
