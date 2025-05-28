
#include "curspriv.h"
#include "stdlib.h"
#include "malloc.h"
#include "string.h"

int COLORS = 0;
int COLOR_PAIRS = PDC_COLOR_PAIRS;

static bool default_colors = FALSE;
static short first_col = 0;
static int allocnum = 0;

int start_color(void)
{
    PDC_LOG("start_color() - called\n");

    if (!SP || SP->mono)
        return ERR;

    SP->color_started = TRUE;

    PDC_set_blink(FALSE);   /* Also sets COLORS */

    if (!default_colors && SP->orig_attr && getenv("PDC_ORIGINAL_COLORS"))
        default_colors = TRUE;

    PDC_init_atrtab();

    return OK;
}

static void _normalize(short *fg, short *bg)
{
    if (*fg == -1)
        *fg = SP->orig_attr ? SP->orig_fore : COLOR_WHITE;

    if (*bg == -1)
        *bg = SP->orig_attr ? SP->orig_back : COLOR_BLACK;
}

static void _init_pair_core(short pair, short fg, short bg)
{
    PDC_PAIR *p = SP->atrtab + pair;

    _normalize(&fg, &bg);

    /* To allow the PDC_PRESERVE_SCREEN option to work, we only reset
       curscr if this call to init_pair() alters a color pair created by
       the user. */

    if (p->set)
    {
        if (p->f != fg || p->b != bg)
            curscr->_clear = TRUE;
    }

    p->f = fg;
    p->b = bg;
    p->count = allocnum++;
    p->set = TRUE;
}

int init_pair(short pair, short fg, short bg)
{
    PDC_LOG("init_pair() - called: pair %d fg %d bg %d\n", pair, fg, bg);

    if (!SP || !SP->color_started || pair < 1 || pair >= COLOR_PAIRS ||
        fg < first_col || fg >= COLORS || bg < first_col || bg >= COLORS)
        return ERR;

    _init_pair_core(pair, fg, bg);

    return OK;
}

bool has_colors(void)
{
    PDC_LOG("has_colors() - called\n");

    return SP ? !(SP->mono) : FALSE;
}

int init_color(short color, short red, short green, short blue)
{
    PDC_LOG("init_color() - called\n");

    if (!SP || color < 0 || color >= COLORS || !PDC_can_change_color() ||
        red < -1 || red > 1000 || green < -1 || green > 1000 ||
        blue < -1 || blue > 1000)
        return ERR;

    SP->dirty = TRUE;

    return PDC_init_color(color, red, green, blue);
}

int color_content(short color, short *red, short *green, short *blue)
{
    PDC_LOG("color_content() - called\n");

    if (color < 0 || color >= COLORS || !red || !green || !blue)
        return ERR;

    if (PDC_can_change_color())
        return PDC_color_content(color, red, green, blue);
    else
    {
        /* Simulated values for platforms that don't support palette
           changing */

        short maxval = (color & 8) ? 1000 : 680;

        *red = (color & COLOR_RED) ? maxval : 0;
        *green = (color & COLOR_GREEN) ? maxval : 0;
        *blue = (color & COLOR_BLUE) ? maxval : 0;

        return OK;
    }
}

bool can_change_color(void)
{
    PDC_LOG("can_change_color() - called\n");

    return PDC_can_change_color();
}

int pair_content(short pair, short *fg, short *bg)
{
    PDC_LOG("pair_content() - called\n");

    if (pair < 0 || pair >= COLOR_PAIRS || !fg || !bg)
        return ERR;

    *fg = SP->atrtab[pair].f;
    *bg = SP->atrtab[pair].b;

    return OK;
}

int assume_default_colors(int f, int b)
{
    PDC_LOG("assume_default_colors() - called: f %d b %d\n", f, b);

    if (f < -1 || f >= COLORS || b < -1 || b >= COLORS)
        return ERR;

    if (SP->color_started)
        _init_pair_core(0, f, b);

    return OK;
}

int use_default_colors(void)
{
    PDC_LOG("use_default_colors() - called\n");

    default_colors = TRUE;
    first_col = -1;

    return assume_default_colors(-1, -1);
}

int PDC_set_line_color(short color)
{
    PDC_LOG("PDC_set_line_color() - called: %d\n", color);

    if (!SP || color < -1 || color >= COLORS)
        return ERR;

    SP->line_color = color;

    return OK;
}

void PDC_init_atrtab(void)
{
    PDC_PAIR *p = SP->atrtab;
    short i, fg, bg;

    if (SP->color_started && !default_colors)
    {
        fg = COLOR_WHITE;
        bg = COLOR_BLACK;
    }
    else
        fg = bg = -1;

    _normalize(&fg, &bg);

    for (i = 0; i < PDC_COLOR_PAIRS; i++)
    {
        p[i].f = fg;
        p[i].b = bg;
        p[i].set = FALSE;
    }
}

int free_pair(int pair)
{
    if (pair < 1 || pair >= PDC_COLOR_PAIRS || !(SP->atrtab[pair].set))
        return ERR;

    SP->atrtab[pair].set = FALSE;
    return OK;
}

int find_pair(int fg, int bg)
{
    int i;
    PDC_PAIR *p = SP->atrtab;

    for (i = 0; i < PDC_COLOR_PAIRS; i++)
        if (p[i].set && p[i].f == fg && p[i].b == bg)
            return i;

    return -1;
}

static int _find_oldest()
{
    int i, lowind = 0, lowval = 0;
    PDC_PAIR *p = SP->atrtab;

    for (i = 1; i < PDC_COLOR_PAIRS; i++)
    {
        if (!p[i].set)
            return i;

        if (!lowval || (p[i].count < lowval))
        {
            lowind = i;
            lowval = p[i].count;
        }
    }

    return lowind;
}

int alloc_pair(int fg, int bg)
{
    int i = find_pair(fg, bg);

    if (-1 == i)
    {
        i = _find_oldest();

        if (ERR == init_pair(i, fg, bg))
            return -1;
    }

    return i;
}
