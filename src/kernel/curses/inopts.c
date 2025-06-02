/* PDCurses */

#include "curspriv.h"

int cbreak(void)
{
    PDC_LOG("cbreak() - called\n");

    if (!SP)
        return ERR;

    SP->cbreak = TRUE;

    return OK;
}

int nocbreak(void)
{
    PDC_LOG("nocbreak() - called\n");

    if (!SP)
        return ERR;

    SP->cbreak = FALSE;
    SP->delaytenths = 0;

    return OK;
}

int echo(void)
{
    PDC_LOG("echo() - called\n");

    if (!SP)
        return ERR;

    SP->echo = TRUE;

    return OK;
}

int noecho(void)
{
    PDC_LOG("noecho() - called\n");

    if (!SP)
        return ERR;

    SP->echo = FALSE;

    return OK;
}

int halfdelay(int tenths)
{
    PDC_LOG("halfdelay() - called\n");

    if (!SP || tenths < 1 || tenths > 255)
        return ERR;

    SP->delaytenths = tenths;

    return OK;
}

int intrflush(WINDOW *win, bool bf)
{
    PDC_LOG("intrflush() - called\n");

    return OK;
}

int keypad(WINDOW *win, bool bf)
{
    PDC_LOG("keypad() - called\n");

    if (!win)
        return ERR;

    win->_use_keypad = bf;

    return OK;
}

int meta(WINDOW *win, bool bf)
{
    PDC_LOG("meta() - called\n");

    if (!SP)
        return ERR;

    SP->raw_inp = bf;

    return OK;
}

int nl(void)
{
    PDC_LOG("nl() - called\n");

    if (!SP)
        return ERR;

    SP->autocr = TRUE;

    return OK;
}

int nonl(void)
{
    PDC_LOG("nonl() - called\n");

    if (!SP)
        return ERR;

    SP->autocr = FALSE;

    return OK;
}

int nodelay(WINDOW *win, bool flag)
{
    PDC_LOG("nodelay() - called\n");

    if (!win)
        return ERR;

    win->_nodelay = flag;

    return OK;
}

int notimeout(WINDOW *win, bool flag)
{
    PDC_LOG("notimeout() - called\n");

    return OK;
}

int raw(void)
{
    PDC_LOG("raw() - called\n");

    if (!SP)
        return ERR;

    PDC_set_keyboard_binary(TRUE);
    SP->raw_inp = TRUE;

    return OK;
}

int noraw(void)
{
    PDC_LOG("noraw() - called\n");

    if (!SP)
        return ERR;

    PDC_set_keyboard_binary(FALSE);
    SP->raw_inp = FALSE;

    return OK;
}

void noqiflush(void)
{
    PDC_LOG("noqiflush() - called\n");
}

void qiflush(void)
{
    PDC_LOG("qiflush() - called\n");
}

void timeout(int delay)
{
    PDC_LOG("timeout() - called\n");

    wtimeout(stdscr, delay);
}

void wtimeout(WINDOW *win, int delay)
{
    PDC_LOG("wtimeout() - called\n");

    if (!win)
        return;

    if (delay < 0)
    {
        /* This causes a blocking read on the window, so turn on delay
           mode */

        win->_nodelay = FALSE;
        win->_delayms = 0;
    }
    else if (!delay)
    {
        /* This causes a non-blocking read on the window, so turn off
           delay mode */

        win->_nodelay = TRUE;
        win->_delayms = 0;
    }
    else
    {
        /* This causes the read on the window to delay for the number of
           milliseconds. Also forces the window into non-blocking read
           mode */

        /*win->_nodelay = TRUE;*/
        win->_delayms = delay;
    }
}

int wgetdelay(const WINDOW *win)
{
    PDC_LOG("wgetdelay() - called\n");

    if (!win)
        return 0;

    return win->_delayms;
}

int typeahead(int fildes)
{
    PDC_LOG("typeahead() - called\n");

    return OK;
}

int crmode(void)
{
    PDC_LOG("crmode() - called\n");

    return cbreak();
}

int nocrmode(void)
{
    PDC_LOG("nocrmode() - called\n");

    return nocbreak();
}

bool is_keypad(const WINDOW *win)
{
    PDC_LOG("is_keypad() - called\n");

    if (!win)
        return FALSE;

    return win->_use_keypad;
}

bool is_nodelay(const WINDOW *win)
{
    PDC_LOG("is_nodelay() - called\n");

    if (!win)
        return FALSE;

    return win->_nodelay;
}

bool is_notimeout(const WINDOW *win)
{
    (void) win;

    PDC_LOG("is_notimeout() - called - returning FALSE...\n");

    return FALSE;
}
