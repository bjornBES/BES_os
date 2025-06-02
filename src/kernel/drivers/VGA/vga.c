#include "vga.h"
#include "vga_graphics.h"
#include "vga_modes.h"
#include "vga_text.h"
#include "debug.h"

#include "malloc.h"
#include "arch/i686/bios.h"

#define MODULE "VGA"

extern char VGAModesAddr;
Page *VGAModePage;
vga_mode_t *vga_modes;
uint16_t VGA_currentMode;
uint8_t *VGA_Framebuffer;
uint16_t ScreenWidth;
uint16_t ScreenHeight;
uint16_t ScreenPitch;
ModeBPP VGA_ModeBPP;

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
        // i686_inb(VGA_ATTR_RESET);
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

uint8_t GetVgaMode(vesa_mode_info_t *vesaMode)
{

    if ((vesaMode->attributes & 0x10) == 0x10)
    {
        return VGA_MODE_GRAPH;
    }
    else
    {
        return VGA_MODE_TEXT;
    }
}

void SwitchToVGAModes(VESAInfo *VESAinfo)
{
    vga_modes = (vga_mode_t *)&VGAModesAddr;
    int size = 0;
    for (size_t i = 0; i < VESAinfo->VESACount; i++)
    {
        vesa_mode_info_t *entry = &VESAinfo->entries[i];

        vga_modes[i].mode = GetVgaMode(entry);
        vga_modes[i].width = entry->width;
        vga_modes[i].height = entry->height;
        vga_modes[i].bpp = entry->bpp;
        vga_modes[i].pitch = entry->pitch;
        vga_modes[i].framebuffer = entry->framebuffer == 0 ? (vga_modes[i].mode == VGA_MODE_TEXT ? 0xB8000 : 0xA0000) : entry->framebuffer;
        vga_modes[i].memory_model = entry->memory_model;
        vga_modes[i].planes = entry->planes;
        vga_modes[i].red_mask = entry->red_mask;
        vga_modes[i].red_position = entry->red_position;
        vga_modes[i].green_mask = entry->green_mask;
        vga_modes[i].green_position = entry->green_position;
        vga_modes[i].blue_mask = entry->blue_mask;
        vga_modes[i].blue_position = entry->blue_position;
        vga_modes[i].Colored = (entry->attributes & 0x08) ? true : false;

        log_debug(MODULE, "%2u: 0x%08X %s, %s, mm: %u %ux%ux%u Red:0x%02X & 0x%02X, Green:%02X & %02X, Blue:%02X & %02X",
                  i,
                  vga_modes[i].framebuffer,
                  (vga_modes[i].mode == VGA_MODE_GRAPH) ? "GRAP" : "TEXT",
                  (vga_modes[i].Colored == VGA_MODE_COLOR) ? "COLOR" : "MONO ",
                  vga_modes[i].memory_model,
                  vga_modes[i].width,
                  vga_modes[i].height,
                  vga_modes[i].bpp,
                  entry->red_position, entry->red_mask,
                  entry->green_position, entry->green_mask,
                  entry->blue_position, entry->blue_mask);

        size += sizeof(vga_mode_t);
    }
}

void VGA_CursorScanLine(uint8_t start, uint8_t end)
{
    uint8_t CursorStartRegister = (ReadRegister(CRT_Controller_Registers, CRTC_Cursor_Start_Register) & 0b00100000) | (start & 0b00011111);
    uint8_t CursorEndRegister = (ReadRegister(CRT_Controller_Registers, CRTC_Cursor_End_Register) & 0b01100000) | (end & 0b00011111);

    WriteRegister(CRT_Controller_Registers, CRTC_Cursor_Start_Register, CursorStartRegister);
    WriteRegister(CRT_Controller_Registers, CRTC_Cursor_End_Register, CursorEndRegister);
}

void VGA_clrscr()
{
    log_debug(MODULE, "CurrentMode = %u", CurrentMode);
    if (CurrentMode == VGA_MODE_GRAPH)
    {
        VGAGrap_clear(0);
    }
    else if (CurrentMode == VGA_MODE_TEXT)
    {
        VGAText_clrscr();
    }
}
void VGA_setcursor(int x, int y)
{
    VGAText_setcursor(x, y);
}
void VGA_getcursor(int *x, int *y)
{
    VGAText_getcursor(x, y);
}

void VGA_putc(char c)
{
    VGAText_putc(c);
}

void VGA_SetPalette(uint32_t *palette, size_t count)
{
    if (count > 256)
    {
        count = 256; // Limit to 256 colors
    }

    i686_outb(VGA_PALETTE_INDEX, 0x00); // Set palette index to 0

    for (size_t i = 0; i < count; i++)
    {
        uint32_t color = palette[i];
        VGA_SetColor(i, color);
    }
}
void VGA_SetColor(uint32_t index, uint32_t color)
{
    if (index > 256)
    {
        index = 256; // Limit to 256 colors
    }

    i686_outb(VGA_PALETTE_INDEX, index); // Set palette index to 0

    uint8_t red = (color >> 16) & 0xFF;
    uint8_t green = (color >> 8) & 0xFF;
    uint8_t blue = color & 0xFF;

    red >>= 2;
    green >>= 2;
    blue >>= 2;

    i686_outb(VGA_PALETTE_DATA, red);
    i686_outb(VGA_PALETTE_DATA, green);
    i686_outb(VGA_PALETTE_DATA, blue);
}
uint32_t VGA_GetColor(uint32_t index, uint8_t *red, uint8_t *green, uint8_t *blue)
{
    if (index > 256)
    {
        index = 256; // Limit to 256 colors
    }

    i686_outb(VGA_PALETTE_INDEX, index); // Set palette index to 0

    uint8_t _red = (i686_inb(VGA_PALETTE_DATA) * 255 + 31) / 63;
    uint8_t _green = (i686_inb(VGA_PALETTE_DATA) * 255 + 31) / 63;
    uint8_t _blue = (i686_inb(VGA_PALETTE_DATA) * 255 + 31) / 63;

    *red = _red;
    *green = _green;
    *blue = _blue;

    return (_red << 16) | (_green << 8) | _blue;
}

void VGA_SetMode(uint16_t mode)
{
    mode_SetMode(mode);
}

void vga_initialize()
{
    VGA_SetMode(2);
}

void pre_vga_initialize(BootParams *bootParams, VESAInfo *VESAinfo)
{
    SwitchToVGAModes(VESAinfo);

    VGA_SetMode(bootParams->CurrentMode);
}
