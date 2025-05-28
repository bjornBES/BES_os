#include "vga_text.h"
#include "vga.h"
#include "memory.h"

#include "debug.h"

#define MODULE "VGA_TEXT"

extern char default8x16Font;
uint16_t* buffer;

static const uint8_t sequencer[5] = {
    0x03, 0x01, 0x03, 0x00, 0x06
};

static const uint8_t crtc[25] = {
    0x5F, 0x4F, 0x50, 0x82, 0x55,
    0x81, 0xBF, 0x1F, 0x00, 0x4F,
    0x0D, 0x0E, 0x00, 0x00, 0x00,
    0x50, 0x9C, 0x0E, 0x8F, 0x28,
    0x40, 0x96, 0xB9, 0xA3, 0xFF
};

static const uint8_t graphics_ctrl[9] = {
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x0E, 0x00, 0xFF
};

static const uint8_t attribute_ctrl[21] = {
    0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x14, 0x07, 0x38, 0x39,
    0x3A, 0x3B, 0x3C, 0x3D, 0x3E,
    0x3F, 0x0C, 0x00, 0x0F, 0x08,
    0x00
};

extern char VGAModesAddr;
const uint8_t DEFAULT_COLOR = 0x7;

bool VGA_mono;
int ScreenX = 0, ScreenY = 0;

void VGAText_getcursor(int *x, int *y)
{
    *x = ScreenX;
    *y = ScreenY;
}

void VGAText_putchr(int x, int y, char c)
{
    buffer[2 * (y * ScreenWidth + x)] = c;
}

void VGA_putcolor(int x, int y, uint8_t color)
{
    buffer[2 * (y * ScreenWidth + x) + 1] = color;
}

void VGA_put(int x, int y, char c, uint8_t color)
{
    uint16_t* ptr = (uint16_t*)VGA_Framebuffer + y * ScreenWidth + x;
    *ptr = (c | (color << 8));
    /*
    log_debug(MODULE, "(%u, %u) buffer[%i] = %x, buffer[%i] = %x",
        x, y,
        index, priv,
        index, *ptr
    );
    */
}

char VGA_getchr(int x, int y)
{
    return buffer[2 * (y * ScreenWidth + x)];
}

uint8_t VGA_getcolor(int x, int y)
{
    return buffer[2 * (y * ScreenWidth + x) + 1];
}

void VGAText_setcursor(int x, int y)
{
    int pos = y * ScreenWidth + x;
    ScreenX = x;
    ScreenY = y;

    WriteRegister(CRT_Controller_Registers, CRTC_Cursor_Location_Low_Register, (uint8_t)(pos & 0xFF));
    WriteRegister(CRT_Controller_Registers, CRTC_Cursor_Location_High_Register, (uint8_t)((pos >> 8) & 0xFF));
}

void VGAText_LoadFont(uint8_t* font)
{
    uint8_t ExtraGlyphs = font[0] + 1;
    uint8_t fontWidth = font[6];
    uint8_t fontHeight = font[7];
    uint16_t numberOfEntries = ((uint16_t*)font)[4];
    fprintf(VFS_FD_DEBUG, "font data width %u, height %u, number of entries %u\n", fontWidth, fontHeight, numberOfEntries);
    WriteRegister(Sequencer_Registers, 0x02, 0x04);
    WriteRegister(Graphics_Registers, GR_Read_Map_Select_Register, 0x02);
    WriteRegister(Graphics_Registers, GR_Graphics_Mode_Register, 0x00);
    WriteRegister(Graphics_Registers, GR_Miscellaneous_Graphics_Register, 0x04);

    log_debug(MODULE, "font = %p, after = %p", font, font + fontHeight * ExtraGlyphs);
    font += fontHeight * ExtraGlyphs;

    uint8_t* vga = (uint8_t*)0xA0000;

    memcpy(vga, font, numberOfEntries * fontHeight);

    WriteRegister(Graphics_Registers, GR_Read_Map_Select_Register, 0x00);
    WriteRegister(Graphics_Registers, GR_Miscellaneous_Graphics_Register, 0x0E);
    WriteRegister(Sequencer_Registers, 0x02, 0x03);
}

void VGAText_clrscr()
{
    log_debug(MODULE, "enter VGA_clrscr");
    for (int y = 0; y < ScreenHeight; y++)
        for (int x = 0; x < ScreenWidth; x++)
        {
            VGA_put(x, y, '\0', DEFAULT_COLOR);
        }

    ScreenX = 0;
    ScreenY = 0;
    VGAText_setcursor(ScreenX, ScreenY);
}

void VGA_scrollback(int lines)
{
    for (int y = lines; y < ScreenHeight; y++)
        for (int x = 0; x < ScreenWidth; x++)
        {
            VGA_put(x, y - lines, VGA_getchr(x, y), VGA_getcolor(x, y));
        }

    for (int y = ScreenHeight - lines; y < ScreenHeight; y++)
        for (int x = 0; x < ScreenWidth; x++)
        {
            VGA_put(x, y, '\0', DEFAULT_COLOR);
        }

    ScreenY -= lines;
}

void VGAText_putc(char c)
{
    switch (c)
    {
    case '\n':
        ScreenX = 0;
        ScreenY++;
        break;

    case '\t':
        for (int i = 0; i < 4 - (ScreenX % 4); i++)
            VGAText_putc(' ');
        break;

    case '\r':
        ScreenX = 0;
        break;

    default:
        VGA_put(ScreenX, ScreenY, c, 0x0F);
        ScreenX++;
        break;
    }

    if (ScreenX >= ScreenWidth)
    {
        ScreenY++;
        ScreenX = 0;
    }
    if (ScreenY >= ScreenHeight)
        VGA_scrollback(1);

    VGAText_setcursor(ScreenX, ScreenY);
}

void SwitchMono()
{
    uint8_t data = ReadRegister(External_Registers, ER_OutputRegister);
    data |= 0x02;
    WriteRegister(External_Registers, ER_OutputRegister, data);
    VGA_mono = true;
}
void SwitchColor()
{
    uint8_t data = ReadRegister(External_Registers, ER_OutputRegister);
    data &= ~0x02;
    WriteRegister(External_Registers, ER_OutputRegister, data);
    VGA_mono = false;
}
/*
void SwitchMode(uint8_t mode)
{
vga_mode_t *vgaMode = &vga_modes[mode];
ScreenWidth = vgaMode->width;
ScreenHeight = vgaMode->height;
VGA_Framebuffer = (uint8_t *)vgaMode->framebuffer;
VGA_ModeBPP = GetBPP(vgaMode);
ScreenX = 0;
ScreenY = 0;
if (vgaMode->Colored)
{
SwitchColor();
}
else
{
SwitchMono();
}
}
*/
void VGAText_init(vga_mode_t *mode)
{
    ScreenWidth = mode->width;
    ScreenHeight = mode->height;
    buffer = (uint16_t *)0xB8000;
    VGA_ModeBPP = mode->bpp;
    ScreenX = 0;
    ScreenY = 0;
    if (mode->Colored)
    {
        SwitchColor();
    }
    else
    {
        SwitchMono();
    }
    // log_debug(MODULE, "enter VGA_init");
    OUTB(VGA_MISC_PORT, 0x63);
    int i;
    for (i = 0; i < 5; i++)
    {
        WriteRegister(Sequencer_Registers, i, sequencer[i]);
    }

    WriteRegister(CRT_Controller_Registers, CRTC_Vertical_Retrace_End_Register, ReadRegister(CRT_Controller_Registers, CRTC_Vertical_Retrace_End_Register) & 0x7F);
    
    for (i = 0; i < 25; i++)
    {
        WriteRegister(CRT_Controller_Registers, i, crtc[i]);
    }

    // Write to Graphics Controller registers
    for (int i = 0; i < 9; i++)
    {
        WriteRegister(Graphics_Registers, i, graphics_ctrl[i]);
    }
    
    // Write to Attribute Controller registers
    for (int i = 0; i < 21; i++)
    {
        WriteRegister(Attribute_Controller, i, attribute_ctrl[i]);
    }

    OUTB(VGA_ATTR_INDEX, 0x20); // Enable attribute controller

    // VGAText_LoadFont((uint8_t*)&default8x16Font);
}
