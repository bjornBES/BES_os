#include "pdcBESOS.h"
#include <stdlib.h>
#include "drivers/VGA/vga.h"

int PDC_curs_set(int visibility)
{
    // PDCREGS regs;
    int ret_vis = 0;
    int start = 0; 
    int end = 0;

    PDC_LOG("PDC_curs_set() - called: visibility=%d\n", visibility);

    ret_vis = SP->visibility;
    SP->visibility = visibility;

    switch (visibility)
    {
    case 0: // invisible
        start = 0b00100000;
        end = 0; // was 32
        break;
    case 2:        // highly visible
        start = 0; // full-height block
        end = 0b00000111;
        break;
    default: // normal visibility
        start = (SP->orig_cursor >> 8) & 0xff;
        end = SP->orig_cursor & 0xff;
    }



    // if scrnmode is not set, some BIOSes hang

    VGA_CursorScanLine(start, end);

    // regs.h.ah = 0x01;
    // regs.h.al = (unsigned char)pdc_scrnmode;
    // regs.h.ch = (unsigned char)start;
    // regs.h.cl = (unsigned char)end;
    // PDCINT(0x10, regs);

    return ret_vis;
}

void PDC_set_title(const char *title)
{
    PDC_LOG("PDC_set_title() - called: <%s>\n", title);
}

int PDC_set_blink(bool blinkon)
{
    /*
    PDCREGS regs;

    if (!SP)
    return ERR;

    switch (pdc_adapter)
    {
        case _EGACOLOR:
        case _EGAMONO:
        case _VGACOLOR:
        case _VGAMONO:
        regs.W.ax = 0x1003;
        regs.W.bx = blinkon;

        PDCINT(0x10, regs);

        if (SP->color_started)
        COLORS = blinkon ? 8 : 16;

        break;
        default:
        COLORS = 8;
    }

    if (blinkon && (COLORS == 8))
    SP->termattrs |= A_BLINK;
    else if (!blinkon && (COLORS == 16))
    SP->termattrs &= ~A_BLINK;

    return (COLORS - (blinkon * 8) != 8) ? OK : ERR;
    */
    return OK;
}

int PDC_set_bold(bool boldon)
{
    return boldon ? ERR : OK;
}
