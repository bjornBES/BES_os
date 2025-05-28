/* PDCurses */

#include "pdcBESOS.h"
#include "malloc.h"
#include "drivers/VGA/vga.h"

#include <stdlib.h>

int pdc_adapter;         /* screen type */
int pdc_scrnmode;        /* default screen mode */
int pdc_font;            /* default font size */
bool pdc_direct_video;   /* allow direct screen memory writes */
bool pdc_bogus_adapter;  /* TRUE if adapter has insane values */
unsigned pdc_video_seg;  /* video base segment */
unsigned pdc_video_ofs;  /* video base offset */


static short realtocurs[16] =
{
    COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED,
    COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE, COLOR_BLACK + 8,
    COLOR_BLUE + 8, COLOR_GREEN + 8, COLOR_CYAN + 8, COLOR_RED + 8,
    COLOR_MAGENTA + 8, COLOR_YELLOW + 8, COLOR_WHITE + 8
};

short pdc_curstoreal[16];

static bool sizeable = FALSE;   /* TRUE if adapter is resizeable    */

static unsigned short *saved_screen = NULL;
static int saved_lines = 0;
static int saved_cols = 0;

static int saved_scrnmode[3];
static int saved_font[3];

int PDC_scr_open(void)
{
#if SMALL || MEDIUM
    struct SREGS segregs;
    int ds;
#endif
    int i;

    PDC_LOG(("PDC_scr_open() - called\n"));

    for (i = 0; i < 16; i++)
        pdc_curstoreal[realtocurs[i]] = i;

    SP->orig_attr = FALSE;

    pdc_direct_video = TRUE; /* Assume that we can */
    pdc_video_seg = 0xb000;  /* Base screen segment addr */
    pdc_video_ofs = 0x0;     /* Base screen segment ofs */

    pdc_adapter = _VGACOLOR;
    pdc_scrnmode = VGA_currentMode;
    pdc_font = 16;

    SP->mouse_wait = PDC_CLICK_PERIOD;
    SP->audible = TRUE;

    SP->termattrs = (SP->mono ? A_UNDERLINE : A_COLOR) | A_REVERSE | A_BLINK;

    /* If the environment variable PDCURSES_BIOS is set, the DOS int10()
       BIOS calls are used in place of direct video memory access. */

    if (getenv("PDCURSES_BIOS"))
        pdc_direct_video = FALSE;

    /* This code for preserving the current screen. */

    if (getenv("PDC_RESTORE_SCREEN"))
    {
        saved_lines = ScreenWidth;
        saved_cols = ScreenHeight;

        saved_screen = malloc(saved_lines * saved_cols * 2, &cursesPage);

        if (!saved_screen)
        {
            SP->_preserve = FALSE;
            return OK;
        }
#ifdef __DJGPP__
        dosmemget((unsigned long)_FAR_POINTER(pdc_video_seg, pdc_video_ofs),
                  saved_lines * saved_cols * 2, saved_screen);
#else
# if SMALL || MEDIUM
        segread(&segregs);
        ds = segregs.ds;
        movedata(pdc_video_seg, pdc_video_ofs, ds, (int)saved_screen,
                 (saved_lines * saved_cols * 2));
# else
        memcpy((void *)saved_screen,
               (void *)_FAR_POINTER(pdc_video_seg, pdc_video_ofs),
               (saved_lines * saved_cols * 2));
# endif
#endif
    }

    SP->_preserve = (getenv("PDC_PRESERVE_SCREEN") != NULL);

    return OK;
}