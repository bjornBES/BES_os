
#include "curspriv.h"

int wattrset(WINDOW *win, chtype attrs)
{
    PDC_LOG(("wattrset() - called\n"));

    if (!win)
        return ERR;

    win->_attrs = attrs & A_ATTRIBUTES;

    return OK;
}