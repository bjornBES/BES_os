#include "vga_text.h"

#include <stdio.h>
#include "debug.h"
#include "malloc.h"
#include <arch/i686/io.h>

#include <stdarg.h>

#define MODULE "VGA"

extern char VGAModesAddr;

uint16_t SCREEN_WIDTH = 80;
uint16_t SCREEN_HEIGHT = 25;
const uint8_t DEFAULT_COLOR = 0x7;

Page *VGAModePage;
vga_mode_t *vga_modes;

bool VGA_mono;
uint16_t VGA_mode;
uint8_t *g_ScreenBuffer = (uint8_t *)0xB8000;
int g_ScreenX = 0, g_ScreenY = 0;

vga_mode_t *vga_get_mode(uint8_t mode)
{
    for (int i = 0; vga_modes[i].mode != 0xFF; i++)
    {
        if (vga_modes[i].mode == mode)
            return &vga_modes[i];
    }
    return 0; /* Mode not found */
}

uint8_t VGAExternalRegisterSelect(uint16_t regOffset, uint8_t data, bool read)
{
    switch (regOffset)
    {
    case ER_OutputRegister:
        if (read)
        {
            return i686_inb(0x3CC);
        }
        else
        {
            i686_outb(0x3C2, data);
        }
        break;
    case ER_FeatureControlRegister:
        if (read)
        {
            return i686_inb(0x3CA);
        }
        else
        {
            if (VGA_mono)
            {
                i686_outb(0x3BA, data);
            }
            else
            {
                i686_outb(0x3DA, data);
            }
        }
        break;
    case ER_InputStatus0Regiser:
        if (read)
        {
            return i686_inb(0x3C2);
        }
        return 0;

    case ER_InputStatus1Regiser:
        if (read)
        {
            if (VGA_mono)
            {
                return i686_inb(0x3BA);
            }
            else
            {
                return i686_inb(0x3DA);
            }
        }
        break;

    default:
        break;
    }
    return 0;
}

uint16_t ReadRegister(uint16_t regSelect, uint16_t regOffset)
{
    switch (regSelect)
    {
    case Graphics_Registers:
        i686_outb(VGA_GraphicsRegOffset + VGA_INDEX, regOffset);
        return i686_inb(VGA_GraphicsRegOffset + VGA_DATA);

    case CRT_Controller_Registers:
        i686_outb(VGA_CRTControllerRegOffset + VGA_INDEX, regOffset);
        return i686_inb(VGA_CRTControllerRegOffset + VGA_DATA);

    case Sequencer_Registers:
        i686_outb(VGA_SequencerRegOffset + VGA_INDEX, regOffset);
        return i686_inb(VGA_SequencerRegOffset + VGA_DATA);

    case Attribute_Controller:
        /* Attribute controller uses special access */
        i686_inb(0x3DA); // Reset flip-flop
        i686_outb(VGA_AttributeControllerRegOffset, regOffset);
        return i686_inb(VGA_AttributeControllerRegOffset + VGA_DATA);

    case External_Registers:
        return VGAExternalRegisterSelect(regOffset, 0, true);

    default:
        break;
    }

    return 0;
}
void WriteRegister(uint16_t regSelect, uint16_t regOffset, uint8_t data)
{
    switch (regSelect)
    {
    case Graphics_Registers:
        i686_outb(VGA_GraphicsRegOffset + VGA_INDEX, regOffset);
        i686_outb(VGA_GraphicsRegOffset + VGA_DATA, data);
        break;

    case CRT_Controller_Registers:
        i686_outb(VGA_CRTControllerRegOffset + VGA_INDEX, regOffset);
        i686_outb(VGA_CRTControllerRegOffset + VGA_DATA, data);
        break;

    case Sequencer_Registers:
        i686_outb(VGA_SequencerRegOffset + VGA_INDEX, regOffset);
        i686_outb(VGA_SequencerRegOffset + VGA_DATA, data);
        break;

    case Attribute_Controller:
        /* Reset flip-flop */
        i686_inb(0x3DA);
        i686_outb(VGA_AttributeControllerRegOffset, regOffset);
        i686_outb(VGA_AttributeControllerRegOffset, data);
        break;

    case External_Registers:
        VGAExternalRegisterSelect(regOffset, data, false);
        return;
    default:
        break;
    }
}

void VGA_putchr(int x, int y, char c)
{
    g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x)] = c;
}

void VGA_putcolor(int x, int y, uint8_t color)
{
    g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1] = color;
}

void VGA_put(int x, int y, char c, uint8_t color)
{
}

char VGA_getchr(int x, int y)
{
    return g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x)];
}

uint8_t VGA_getcolor(int x, int y)
{
    return g_ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1];
}

void VGA_setcursor(int x, int y)
{
    int pos = y * SCREEN_WIDTH + x;

    WriteRegister(CRT_Controller_Registers, CRTC_Cursor_Location_Low_Register, (uint8_t)(pos & 0xFF));
    WriteRegister(CRT_Controller_Registers, CRTC_Cursor_Location_High_Register, (uint8_t)((pos >> 8) & 0xFF));
}

void VGA_clrscr()
{
    log_debug(MODULE, "enter VGA_clrscr");
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            VGA_putchr(x, y, '\0');
            VGA_putcolor(x, y, DEFAULT_COLOR);
        }

    g_ScreenX = 0;
    g_ScreenY = 0;
    VGA_setcursor(g_ScreenX, g_ScreenY);
}

void VGA_scrollback(int lines)
{
    for (int y = lines; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            VGA_putchr(x, y - lines, VGA_getchr(x, y));
            VGA_putcolor(x, y - lines, VGA_getcolor(x, y));
        }

    for (int y = SCREEN_HEIGHT - lines; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            VGA_putchr(x, y, '\0');
            VGA_putcolor(x, y, DEFAULT_COLOR);
        }

    g_ScreenY -= lines;
}

void VGA_putc(char c)
{
    switch (c)
    {
    case '\n':
        g_ScreenX = 0;
        g_ScreenY++;
        break;

    case '\t':
        for (int i = 0; i < 4 - (g_ScreenX % 4); i++)
            VGA_putc(' ');
        break;

    case '\r':
        g_ScreenX = 0;
        break;

    default:
        VGA_putchr(g_ScreenX, g_ScreenY, c);
        g_ScreenX++;
        break;
    }

    if (g_ScreenX >= SCREEN_WIDTH)
    {
        g_ScreenY++;
        g_ScreenX = 0;
    }
    if (g_ScreenY >= SCREEN_HEIGHT)
        VGA_scrollback(1);

    VGA_setcursor(g_ScreenX, g_ScreenY);
}

void SwitchMono()
{
    uint8_t data = ReadRegister(External_Registers, ER_OutputRegister);
    data &= ~0x01;
    WriteRegister(External_Registers, ER_OutputRegister, data);
    VGA_mono = true;
}
void SwitchColor()
{
    uint8_t data = ReadRegister(External_Registers, ER_OutputRegister);
    data |= 0x01;
    WriteRegister(External_Registers, ER_OutputRegister, data);
    VGA_mono = false;
}

uint8_t GetVgaMode(vesa_mode_info_t *vesaMode)
{
    if (vesaMode->attributes & 0x01 || vesaMode->memory_model == 0x00)
    {
        return VGA_MODE_TEXT;
    }
    else
    {
        return VGA_MODE_GRAPH;
    }
}

void SwitchToVGAModes(VESAInfo* VESAinfo)
{
    vga_modes = (vga_mode_t*)&VGAModesAddr;
    int size = 0;
    for (size_t i = 0; i < VESAinfo->VESACount; i++)
    {
        vesa_mode_info_t* entry = &VESAinfo->entries[i];

        log_debug(MODULE, "%u: %ux%u %u 0x%X %s, mm: %u",
                            i,
                            entry->width,
                            entry->height,
                            entry->bpp,
                            entry->framebuffer,
                            (entry->attributes & 0x80) ? "Yes" : "No",
                            entry->memory_model);

        vga_modes[i].mode = GetVgaMode(entry);
        vga_modes[i].width = entry->width;
        vga_modes[i].height = entry->height;
        vga_modes[i].bpp = entry->bpp;
        vga_modes[i].pitch = entry->pitch;
        vga_modes[i].framebuffer = entry->framebuffer;
        vga_modes[i].memory_model = entry->memory_model;
        vga_modes[i].planes = entry->planes;
        vga_modes[i].red_mask = entry->red_mask;
        vga_modes[i].red_position = entry->red_position;
        vga_modes[i].green_mask = entry->green_mask;
        vga_modes[i].green_position = entry->green_position;
        vga_modes[i].blue_mask = entry->blue_mask;
        vga_modes[i].blue_position = entry->blue_position;
        size += sizeof(vga_mode_t);
    }
}

void VGA_init(VESAInfo* VESAinfo)
{
    // log_debug(MODULE, "enter VGA_init");
    SwitchMono();
    uint8_t data = ReadRegister(External_Registers, ER_OutputRegister);
    data |= 0b10100010;
    WriteRegister(External_Registers, ER_OutputRegister, data);

    WriteRegister(CRT_Controller_Registers, CRTC_Start_Address_High_Register, 0);
    WriteRegister(CRT_Controller_Registers, CRTC_Start_Address_Low_Register, 0);

    SwitchToVGAModes(VESAinfo);
}
