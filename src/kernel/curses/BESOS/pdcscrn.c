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

static int _get_font(void)
{
    int retval = _FONT16;

    return retval;
}

/* _set_font() - Sets the current font size, if the adapter allows such a
   change. It is an error to attempt to change the font size on a
   "bogus" adapter. The reason for this is that we have a known video
   adapter identity problem. e.g. Two adapters report the same identifying
   characteristics. */

static void _set_font(int size)
{
    pdc_font = _get_font();
}

/* _set_80x25() - force a known screen state: 80x25 text mode. Forces the
   appropriate 80x25 alpha mode given the display adapter. */

static void _set_80x25(void)
{
    // TODO
}

/* _get_scrn_mode() - Return the current BIOS video mode */

static int _get_scrn_mode(void)
{
    return (int)VGA_currentMode;
}

/* _set_scrn_mode() - Sets the BIOS Video Mode Number only if it is
   different from the current video mode. */

static void _set_scrn_mode(int new_mode)
{
    if (_get_scrn_mode() != new_mode)
    {
        VGA_SetMode(new_mode);
    }

    pdc_font = _get_font();
    pdc_scrnmode = new_mode;
    LINES = PDC_get_rows();
    COLS = PDC_get_columns();
}

/* _sanity_check() - A video adapter identification sanity check. This
   routine will force sane values for various control flags. */

static int _sanity_check(int adapter)
{
    int fontsize = _get_font();
    int rows = PDC_get_rows();

    PDC_LOG("_sanity_check() - called: Adapter %d\n", adapter);

    switch (adapter)
    {
    case _EGACOLOR:
    case _EGAMONO:
        switch (rows)
        {
        case 25:
        case 43:
            break;
        default:
            pdc_bogus_adapter = TRUE;
        }

        switch (fontsize)
        {
        case _FONT8:
        case _FONT14:
            break;
        default:
            pdc_bogus_adapter = TRUE;
        }
        break;

    case _VGACOLOR:
    case _VGAMONO:
        break;

    case _CGA:
    case _MDA:
    case _MCGACOLOR:
    case _MCGAMONO:
        switch (rows)
        {
        case 25:
            break;
        default:
            pdc_bogus_adapter = TRUE;
        }
        break;

    default:
        pdc_bogus_adapter = TRUE;
    }

    if (pdc_bogus_adapter)
    {
        sizeable = FALSE;
        pdc_direct_video = FALSE;
    }

    return adapter;
}

/* _query_adapter_type() - Determine PC video adapter type. */

static int _query_adapter_type(void)
{
    int retval = _NONE;

    PDC_LOG("_query_adapter_type() - called\n");
            retval = _VGACOLOR;
            sizeable = TRUE;
    

    if ((retval == _NONE))
        pdc_direct_video = FALSE;

    if ((unsigned)pdc_video_seg == 0xb000)
        SP->mono = TRUE;
    else
        SP->mono = FALSE;

    if (!pdc_adapter)
        pdc_adapter = retval;

    return _sanity_check(retval);
}

/* close the physical screen -- may restore the screen to its state
   before PDC_scr_open(); miscellaneous cleanup */

void PDC_scr_close(void)
{
#if SMALL || MEDIUM
    struct SREGS segregs;
    int ds;
#endif
    PDC_LOG("PDC_scr_close() - called\n");

    if (getenv("PDC_RESTORE_SCREEN") && saved_screen)
    {
#ifdef __DJGPP__
        dosmemput(saved_screen, saved_lines * saved_cols * 2,
            (unsigned long)_FAR_POINTER(pdc_video_seg,
            pdc_video_ofs));
#else
# if (SMALL || MEDIUM)
        segread(&segregs);
        ds = segregs.ds;
        movedata(ds, (int)saved_screen, pdc_video_seg, pdc_video_ofs,
        (saved_lines * saved_cols * 2));
# else
        memcpy((void *)_FAR_POINTER(pdc_video_seg, pdc_video_ofs),
        (void *)saved_screen, (saved_lines * saved_cols * 2));
# endif
#endif
        free(saved_screen, &cursesPage);
        saved_screen = NULL;
    }

    // reset_shell_mode();

    if (SP->visibility != 1)
        curs_set(1);

    /* Position cursor to the bottom left of the screen. */

    PDC_gotoyx(PDC_get_rows() - 2, 0);
}

void PDC_scr_free(void)
{
}

int PDC_scr_open(void)
{
#if SMALL || MEDIUM
    struct SREGS segregs;
    int ds;
#endif
    int i;

    PDC_LOG("PDC_scr_open() - called\n");

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

/* the core of resize_term() */

int PDC_resize_screen(int nlines, int ncols)
{
    PDC_LOG("PDC_resize_screen() - called. Lines: %d Cols: %d\n",
             nlines, ncols);

    /* Trash the stored value of orig_cursor -- it's only good if the
       video mode doesn't change */

    SP->orig_cursor = 0x0607;

    switch (pdc_adapter)
    {
    case _EGACOLOR:
        if (nlines >= 43)
            _set_font(_FONT8);
        else
            _set_80x25();
        break;

    case _VGACOLOR:
        if (nlines > 28)
            _set_font(_FONT8);
        else
            if (nlines > 25)
                _set_font(_FONT14);
            else
                _set_80x25();
    }

    PDC_set_blink(COLORS == 8);

    return OK;
}

void PDC_reset_prog_mode(void)
{
        PDC_LOG("PDC_reset_prog_mode() - called.\n");
}

void PDC_reset_shell_mode(void)
{
        PDC_LOG("PDC_reset_shell_mode() - called.\n");
}

void PDC_restore_screen_mode(int i)
{
    if (i >= 0 && i <= 2)
    {
        pdc_font = _get_font();
        _set_font(saved_font[i]);

        if (_get_scrn_mode() != saved_scrnmode[i])
            _set_scrn_mode(saved_scrnmode[i]);
    }
}

void PDC_save_screen_mode(int i)
{
    if (i >= 0 && i <= 2)
    {
        saved_font[i] = pdc_font;
        saved_scrnmode[i] = pdc_scrnmode;
    }
}

/* _egapal() - Find the EGA palette value (0-63) for the color (0-15).
   On VGA, this is an index into the DAC. */

static short _egapal(short color)
{
    return pdc_curstoreal[color];
}

bool PDC_can_change_color(void)
{
    return (pdc_adapter == _VGACOLOR);
}

/* These are only valid when pdc_adapter == _VGACOLOR */

int PDC_color_content(short color, short *red, short *green, short *blue)
{
    VGA_GetColor((uint32_t)_egapal(color), (uint8_t*)red, (uint8_t*)green, (uint8_t*)blue);

    return OK;
}

int PDC_init_color(short color, short red, short green, short blue)
{
    /* Scale */

    PDC_LOG("PDC_resize_screen() - called. index: %d color: %d RGB: (%d,%d,%d)\n", _egapal(color), color, red, green, blue);
    uint32_t RGBColor = (red << 16) | (green << 8) | blue;
    VGA_SetColor(_egapal(color), RGBColor);

    return OK;
}
