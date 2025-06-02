#include <stdlib.h>
#include "curspriv.h"

char ttytype[128];

const char *_curses_notice = "PDCurses " PDC_VERDOT " - " __DATE__;

SCREEN *SP = (SCREEN*)NULL;           /* curses variables */
WINDOW *curscr = (WINDOW *)NULL;      /* the current screen image */
WINDOW *stdscr = (WINDOW *)NULL;      /* the default screen window */

int LINES = 0;                        /* current terminal height */
int COLS = 0;                         /* current terminal width */
int TABSIZE = 8;

MOUSE_STATUS Mouse_status;

RIPPEDOFFLINE linesripped[5];
char linesrippedoff;

WINDOW *initscr(void)
{
    int i;

    PDC_LOG("initscr() - called\n");

    if (SP && SP->alive)
        return NULL;

    SP = calloc(1, sizeof(SCREEN), &cursesPage);
    if (!SP)
        return NULL;

    if (PDC_scr_open() == ERR)
    {
        fprintf(VFS_FD_STDERR, "initscr(): Unable to create SP\n");
        exit(8);
    }

    SP->autocr = TRUE;       /* cr -> lf by default */
    SP->raw_out = FALSE;     /* tty I/O modes */
    SP->raw_inp = FALSE;     /* tty I/O modes */
    SP->cbreak = TRUE;
    SP->key_modifiers = 0L;
    SP->return_key_modifiers = FALSE;
    SP->echo = TRUE;
    SP->visibility = 1;
    SP->resized = FALSE;
    SP->_trap_mbe = 0L;
    SP->linesrippedoff = 0;
    SP->linesrippedoffontop = 0;
    SP->delaytenths = 0;
    SP->line_color = -1;
    SP->lastscr = (WINDOW *)NULL;
    SP->dbfp = -1;
    SP->color_started = FALSE;
    SP->dirty = FALSE;
    SP->sel_start = -1;
    SP->sel_end = -1;

    SP->orig_cursor = PDC_get_cursor_mode();

    LINES = SP->lines = PDC_get_rows();
    COLS = SP->cols = PDC_get_columns();

    if (LINES < 2 || COLS < 2)
    {
        fprintf(VFS_FD_STDERR, "initscr(): LINES=%d COLS=%d: too small.\n",
                LINES, COLS);
        exit(4);
    }

    curscr = newwin(LINES, COLS, 0, 0);
    if (!curscr)
    {
        fprintf(VFS_FD_STDERR, "initscr(): Unable to create curscr.\n");
        exit(2);
    }

    SP->lastscr = newwin(LINES, COLS, 0, 0);
    if (!SP->lastscr)
    {
        fprintf(VFS_FD_STDERR, "initscr(): Unable to create SP->lastscr.\n");
        exit(2);
    }

    wattrset(SP->lastscr, (chtype)(-1));
    werase(SP->lastscr);

    PDC_slk_initialize();
    LINES -= SP->slklines;

    /* We have to sort out ripped off lines here, and reduce the height
       of stdscr by the number of lines ripped off */

    for (i = 0; i < linesrippedoff; i++)
    {
        if (linesripped[i].line < 0)
            (*linesripped[i].init)(newwin(1, COLS, LINES - 1, 0), COLS);
        else
            (*linesripped[i].init)(newwin(1, COLS,
                                   SP->linesrippedoffontop++, 0), COLS);

        SP->linesrippedoff++;
        LINES--;
    }

    linesrippedoff = 0;

    stdscr = newwin(LINES, COLS, SP->linesrippedoffontop, 0);
    if (!stdscr)
    {
        fprintf(VFS_FD_STDERR, "initscr(): Unable to create stdscr.\n");
        exit(1);
    }

    wclrtobot(stdscr);

    /* If preserving the existing screen, don't allow a screen clear */

    if (SP->_preserve)
    {
        untouchwin(curscr);
        untouchwin(stdscr);
        stdscr->_clear = FALSE;
        curscr->_clear = FALSE;
    }
    else
        curscr->_clear = TRUE;

    SP->atrtab = calloc(PDC_COLOR_PAIRS, sizeof(PDC_PAIR), &cursesPage);
    if (!SP->atrtab)
        return NULL;
    PDC_init_atrtab();  /* set up default colors */

    MOUSE_X_POS = MOUSE_Y_POS = -1;
    BUTTON_STATUS(1) = BUTTON_RELEASED;
    BUTTON_STATUS(2) = BUTTON_RELEASED;
    BUTTON_STATUS(3) = BUTTON_RELEASED;
    Mouse_status.changes = 0;

    SP->alive = TRUE;

    // def_shell_mode();

    sprintf(ttytype, "pdcurses|PDCurses for %s", PDC_sysname());

    SP->c_buffer = malloc(_INBUFSIZ * sizeof(int), &cursesPage);
    if (!SP->c_buffer)
        return NULL;
    SP->c_pindex = 0;
    SP->c_gindex = 1;

    SP->c_ungch = malloc(NUNGETCH * sizeof(int), &cursesPage);
    if (!SP->c_ungch)
        return NULL;
    SP->c_ungind = 0;
    SP->c_ungmax = NUNGETCH;

    return stdscr;
}

int endwin(void)
{
    PDC_LOG("endwin() - called\n");

    /* Allow temporary exit from curses using endwin() */

    // def_prog_mode();
    PDC_scr_close();

    SP->alive = FALSE;

    return OK;
}

bool isendwin(void)
{
    PDC_LOG("isendwin() - called\n");

    return SP ? !(SP->alive) : FALSE;
}

SCREEN *newterm(const char *type, fd_t outfd, fd_t infd)
{
    PDC_LOG("newterm() - called\n");

    return initscr() ? SP : NULL;
}

SCREEN *set_term(SCREEN *new)
{
    PDC_LOG("set_term() - called\n");

    /* We only support one screen */

    return (new == SP) ? SP : NULL;
}

void delscreen(SCREEN *sp)
{
    PDC_LOG("delscreen() - called\n");

    if (!SP || sp != SP)
        return;

    free(SP->c_ungch, &cursesPage);
    free(SP->c_buffer, &cursesPage);
    free(SP->atrtab, &cursesPage);

    PDC_slk_free();     /* free the soft label keys, if needed */

    delwin(stdscr);
    delwin(curscr);
    delwin(SP->lastscr);
    stdscr = (WINDOW *)NULL;
    curscr = (WINDOW *)NULL;
    SP->lastscr = (WINDOW *)NULL;

    SP->alive = FALSE;

    PDC_scr_free();

    free(SP, &cursesPage);
    SP = (SCREEN *)NULL;
}

int resize_term(int nlines, int ncols)
{
    PDC_LOG("resize_term() - called: nlines %d\n", nlines);

    if (!stdscr || PDC_resize_screen(nlines, ncols) == ERR)
        return ERR;

    SP->resized = FALSE;

    SP->lines = PDC_get_rows();
    LINES = SP->lines - SP->linesrippedoff - SP->slklines;
    SP->cols = COLS = PDC_get_columns();

    if (SP->cursrow >= SP->lines)
        SP->cursrow = SP->lines - 1;
    if (SP->curscol >= SP->cols)
        SP->curscol = SP->cols - 1;

    if (wresize(curscr, SP->lines, SP->cols) == ERR ||
        wresize(stdscr, LINES, COLS) == ERR ||
        wresize(SP->lastscr, SP->lines, SP->cols) == ERR)
        return ERR;

    werase(SP->lastscr);
    curscr->_clear = TRUE;

    if (SP->slk_winptr)
    {
        if (wresize(SP->slk_winptr, SP->slklines, COLS) == ERR)
            return ERR;

        wmove(SP->slk_winptr, 0, 0);
        wclrtobot(SP->slk_winptr);
        PDC_slk_initialize();
        slk_noutrefresh();
    }

    touchwin(stdscr);
    wnoutrefresh(stdscr);

    return OK;
}

bool is_termresized(void)
{
    PDC_LOG("is_termresized() - called\n");

    return SP->resized;
}

const char *curses_version(void)
{
    return _curses_notice;
}

void PDC_get_version(PDC_VERSION *ver)
{
    if (!ver)
        return;

    ver->flags = 0
#ifdef PDCDEBUG
        | PDC_VFLAG_DEBUG
#endif
#ifdef PDC_WIDE
        | PDC_VFLAG_WIDE
#endif
#ifdef PDC_FORCE_UTF8
        | PDC_VFLAG_UTF8
#endif
#ifdef PDC_DLL_BUILD
        | PDC_VFLAG_DLL
#endif
#ifdef PDC_RGB
        | PDC_VFLAG_RGB
#endif
        ;

    ver->build = PDC_BUILD;
    ver->major = PDC_VER_MAJOR;
    ver->minor = PDC_VER_MINOR;
    ver->csize = sizeof(chtype);
    ver->bsize = sizeof(bool);
}

int set_tabsize(int tabsize)
{
    PDC_LOG("set_tabsize() - called: tabsize %d\n", tabsize);

    if (tabsize < 1)
        return ERR;

    TABSIZE = tabsize;

    return OK;
}

