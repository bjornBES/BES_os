#include "curses/curspriv.h"
#include "stdlib.h"
#include "malloc.h"
#include "drivers/VGA/vga.h"

void PDC_gotoyx(int row, int col)
{
    PDC_LOG("PDC_gotoyx() - called: row %d col %d\n", row, col);

    VGA_setcursor(row, col);
}