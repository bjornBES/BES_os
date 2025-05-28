#include "pdcBESOS.h"
#include <stdlib.h>
#include "drivers/VGA/vga.h"

int PDC_get_columns(void)
{
    int cols;

    PDC_LOG("PDC_get_columns() - called\n");

    cols = ScreenWidth;

    PDC_LOG("PDC_get_columns() - returned: cols %d\n", cols);

    return cols;
}

/* get the cursor size/shape */

int PDC_get_cursor_mode(void)
{
    PDC_LOG("PDC_get_cursor_mode() - called\n");

    return 0;
}

/* return number of screen rows */

int PDC_get_rows(void)
{
    int rows;

    PDC_LOG("PDC_get_rows() - called\n");

    rows = ScreenHeight;

    if (rows == 1 && pdc_adapter == _MDS_GENIUS)
        rows = 66;
    if (rows == 1 && pdc_adapter == _MDA)
        rows = 25;

    if (rows == 1)
    {
        rows = 25;
        pdc_direct_video = FALSE;
    }

    switch (pdc_adapter)
    {
    case _EGACOLOR:
    case _EGAMONO:
        switch (rows)
        {
        case 25:
        case 43:
            break;
        default:
            rows = 25;
        }
        break;

    case _VGACOLOR:
    case _VGAMONO:
        break;

    default:
        rows = 25;
        break;
    }

    PDC_LOG("PDC_get_rows() - returned: rows %d\n", rows);

    return rows;
}