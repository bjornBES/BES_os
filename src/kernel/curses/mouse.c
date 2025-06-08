/* PDCurses */

#include "curspriv.h"
#include "string.h"

static bool ungot = FALSE;

int mouse_set(mmask_t mbe)
{
    PDC_LOG("mouse_set() - called: event %x\n", mbe);

    if (!SP)
        return ERR;

    SP->_trap_mbe = mbe;
    return PDC_mouse_set();
}

int mouse_on(mmask_t mbe)
{
    PDC_LOG("mouse_on() - called: event %x\n", mbe);

    if (!SP)
        return ERR;

    SP->_trap_mbe |= mbe;
    return PDC_mouse_set();
}

int mouse_off(mmask_t mbe)
{
    PDC_LOG("mouse_off() - called: event %x\n", mbe);

    if (!SP)
        return ERR;

    SP->_trap_mbe &= ~mbe;
    return PDC_mouse_set();
}

int request_mouse_pos(void)
{
    PDC_LOG("request_mouse_pos() - called\n");

    Mouse_status = SP->mouse_status;

    return OK;
}

void wmouse_position(WINDOW *win, int *y, int *x)
{
    PDC_LOG("wmouse_position() - called\n");

    if (win && wenclose(win, MOUSE_Y_POS, MOUSE_X_POS))
    {
        if (y)
            *y = MOUSE_Y_POS - win->_begy;
        if (x)
            *x = MOUSE_X_POS - win->_begx;
    }
    else
    {
        if (y)
            *y = -1;
        if (x)
            *x = -1;
    }
}

mmask_t getmouse(void)
{
    PDC_LOG("getmouse() - called\n");

    return SP ? SP->_trap_mbe : (mmask_t)0;
}

/* ncurses mouse interface */

int mouseinterval(int wait)
{
    int old_wait;

    PDC_LOG("mouseinterval() - called: %d\n", wait);

    if (!SP)
        return ERR;

    old_wait = SP->mouse_wait;

    if (wait >= 0 && wait <= 1000)
        SP->mouse_wait = wait;

    return old_wait;
}

bool wenclose(const WINDOW *win, int y, int x)
{
    PDC_LOG("wenclose() - called: %p %d %d\n", win, y, x);

    return (win && y >= win->_begy && y < win->_begy + win->_maxy
                && x >= win->_begx && x < win->_begx + win->_maxx);
}

bool wmouse_trafo(const WINDOW *win, int *y, int *x, bool to_screen)
{
    int newy, newx;

    PDC_LOG("wmouse_trafo() - called\n");

    if (!win || !y || !x)
        return FALSE;

    newy = *y;
    newx = *x;

    if (to_screen)
    {
        newy += win->_begy;
        newx += win->_begx;

        if (!wenclose(win, newy, newx))
            return FALSE;
    }
    else
    {
        if (wenclose(win, newy, newx))
        {
            newy -= win->_begy;
            newx -= win->_begx;
        }
        else
            return FALSE;
    }

    *y = newy;
    *x = newx;

    return TRUE;
}

bool mouse_trafo(int *y, int *x, bool to_screen)
{
    PDC_LOG("mouse_trafo() - called\n");

    return wmouse_trafo(stdscr, y, x, to_screen);
}

mmask_t mousemask(mmask_t mask, mmask_t *oldmask)
{
    PDC_LOG("mousemask() - called\n");

    if (!SP)
        return (mmask_t)0;

    if (oldmask)
        *oldmask = SP->_trap_mbe;

    /* The ncurses interface doesn't work with our move events, so
       filter them here */

    mask &= ~(BUTTON1_MOVED | BUTTON2_MOVED | BUTTON3_MOVED);

    mouse_set(mask);

    return SP->_trap_mbe;
}

int nc_getmouse(MEVENT *event)
{
    int i;
    mmask_t bstate = 0;

    PDC_LOG("nc_getmouse() - called\n");

    if (!event || !SP)
        return ERR;

    ungot = FALSE;

    request_mouse_pos();

    event->id = 0;

    event->x = Mouse_status.x;
    event->y = Mouse_status.y;
    event->z = 0;

    for (i = 0; i < 3; i++)
    {
        if (Mouse_status.changes & (1 << i))
        {
            int shf = i * 5;
            short button = Mouse_status.button[i] & BUTTON_ACTION_MASK;

            if (button == BUTTON_RELEASED)
                bstate |= (BUTTON1_RELEASED << shf);
            else if (button == BUTTON_PRESSED)
                bstate |= (BUTTON1_PRESSED << shf);
            else if (button == BUTTON_CLICKED)
                bstate |= (BUTTON1_CLICKED << shf);
            else if (button == BUTTON_DOUBLE_CLICKED)
                bstate |= (BUTTON1_DOUBLE_CLICKED << shf);

            button = Mouse_status.button[i] & BUTTON_MODIFIER_MASK;

            if (button & PDC_BUTTON_SHIFT)
                bstate |= BUTTON_MODIFIER_SHIFT;
            if (button & PDC_BUTTON_CONTROL)
                bstate |= BUTTON_MODIFIER_CONTROL;
            if (button & PDC_BUTTON_ALT)
                bstate |= BUTTON_MODIFIER_ALT;
        }
    }

    if (MOUSE_WHEEL_UP)
        bstate |= BUTTON4_PRESSED;
    else if (MOUSE_WHEEL_DOWN)
        bstate |= BUTTON5_PRESSED;

    /* extra filter pass -- mainly for button modifiers */

    event->bstate = bstate & SP->_trap_mbe;

    return OK;
}

int ungetmouse(MEVENT *event)
{
    int i;
    mmask_t bstate;

    PDC_LOG("ungetmouse() - called\n");

    if (!event || ungot)
        return ERR;

    ungot = TRUE;

    SP->mouse_status.x = event->x;
    SP->mouse_status.y = event->y;

    SP->mouse_status.changes = 0;
    bstate = event->bstate;

    for (i = 0; i < 3; i++)
    {
        int shf = i * 5;
        short button = 0;

        if (bstate & ((BUTTON1_RELEASED | BUTTON1_PRESSED |
            BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED) << shf))
        {
            SP->mouse_status.changes |= 1 << i;

            if (bstate & (BUTTON1_PRESSED << shf))
                button = BUTTON_PRESSED;
            if (bstate & (BUTTON1_CLICKED << shf))
                button = BUTTON_CLICKED;
            if (bstate & (BUTTON1_DOUBLE_CLICKED << shf))
                button = BUTTON_DOUBLE_CLICKED;

            if (bstate & BUTTON_MODIFIER_SHIFT)
                button |= PDC_BUTTON_SHIFT;
            if (bstate & BUTTON_MODIFIER_CONTROL)
                button |= PDC_BUTTON_CONTROL;
            if (bstate & BUTTON_MODIFIER_ALT)
                button |= PDC_BUTTON_ALT;
        }

        SP->mouse_status.button[i] = button;
    }

    if (bstate & BUTTON4_PRESSED)
        SP->mouse_status.changes |= PDC_MOUSE_WHEEL_UP;
    else if (bstate & BUTTON5_PRESSED)
        SP->mouse_status.changes |= PDC_MOUSE_WHEEL_DOWN;

    return PDC_ungetch(KEY_MOUSE);
}

bool has_mouse(void)
{
    return PDC_has_mouse();
}
