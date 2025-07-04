
#include "curspriv.h"
#include "string.h"

static struct cttyset
{
    bool been_set;
    SCREEN saved;
} ctty[3];

enum { PDC_SH_TTY, PDC_PR_TTY, PDC_SAVE_TTY };

static void _save_mode(int i)
{
    ctty[i].been_set = TRUE;

    memcpy(&(ctty[i].saved), SP, sizeof(SCREEN));

    PDC_save_screen_mode(i);
}

static int _restore_mode(int i)
{
    if (ctty[i].been_set == TRUE)
    {
        memcpy(SP, &(ctty[i].saved), sizeof(SCREEN));

        if (ctty[i].saved.raw_out)
            raw();

        PDC_restore_screen_mode(i);

        if ((LINES != ctty[i].saved.lines) ||
            (COLS != ctty[i].saved.cols))
            resize_term(ctty[i].saved.lines, ctty[i].saved.cols);

        PDC_curs_set(ctty[i].saved.visibility);

        PDC_gotoyx(ctty[i].saved.cursrow, ctty[i].saved.curscol);
    }

    return ctty[i].been_set ? OK : ERR;
}

int def_prog_mode(void)
{
    PDC_LOG("def_prog_mode() - called\n");

    if (!SP)
        return ERR;

    _save_mode(PDC_PR_TTY);

    return OK;
}

int def_shell_mode(void)
{
    PDC_LOG("def_shell_mode() - called\n");

    if (!SP)
        return ERR;

    _save_mode(PDC_SH_TTY);

    return OK;
}

int reset_prog_mode(void)
{
    PDC_LOG("reset_prog_mode() - called\n");

    if (!SP)
        return ERR;

    _restore_mode(PDC_PR_TTY);
    PDC_reset_prog_mode();

    return OK;
}

int reset_shell_mode(void)
{
    PDC_LOG("reset_shell_mode() - called\n");

    if (!SP)
        return ERR;

    _restore_mode(PDC_SH_TTY);
    PDC_reset_shell_mode();

    return OK;
}

int resetty(void)
{
    PDC_LOG("resetty() - called\n");

    if (!SP)
        return ERR;

    return _restore_mode(PDC_SAVE_TTY);
}

int savetty(void)
{
    PDC_LOG("savetty() - called\n");

    if (!SP)
        return ERR;

    _save_mode(PDC_SAVE_TTY);

    return OK;
}

int curs_set(int visibility)
{
    int ret_vis;

    PDC_LOG("curs_set() - called: visibility=%d\n", visibility);

    if (!SP || visibility < 0 || visibility > 2)
        return ERR;

    ret_vis = PDC_curs_set(visibility);

    /* If the cursor is changing from invisible to visible, update
       its position */

    if (visibility && !ret_vis)
        PDC_gotoyx(SP->cursrow, SP->curscol);

    return ret_vis;
}

int napms(int ms)
{
    PDC_LOG("napms() - called: ms=%d\n", ms);

    if (!SP)
        return ERR;

    if (SP->dirty)
    {
        int curs_state = SP->visibility;
        bool leave_state = is_leaveok(curscr);

        SP->dirty = FALSE;

        leaveok(curscr, TRUE);

        wrefresh(curscr);

        leaveok(curscr, leave_state);
        curs_set(curs_state);
    }

    if (ms)
        PDC_napms(ms);

    return OK;
}

/*
int ripoffline(int line, int (*init)(WINDOW *, int))
{
    PDC_LOG(("ripoffline() - called: line=%d\n", line));
    
    if (linesrippedoff < 5 && line && init)
    {
        linesripped[(int)linesrippedoff].line = line;
        linesripped[(int)linesrippedoff++].init = init;
        
        return OK;
    }
    
    return ERR;
}
*/

int draino(int ms)
{
    PDC_LOG(("draino() - called\n"));

    return napms(ms);
}

int resetterm(void)
{
    PDC_LOG(("resetterm() - called\n"));

    return reset_shell_mode();
}

int fixterm(void)
{
    PDC_LOG(("fixterm() - called\n"));

    return reset_prog_mode();
}

int saveterm(void)
{
    PDC_LOG(("saveterm() - called\n"));

    return def_prog_mode();
}
