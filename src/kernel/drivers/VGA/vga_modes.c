#include "vga_modes.h"
#include "vga_graphics.h"
#include "vga_text.h"

#include "debug.h"

#include "arch/i686/bios.h"
#include "vga.h"

#define MODULE "VGA_MODES"

uint16_t CurrentMode;

ModeBPP GetBPP(vga_mode_t *vgaMode)
{
    if (vgaMode->mode == VGA_MODE_GRAPH)
    {
        if (vgaMode->bpp == 15)
            return GRAP_4BPP;
        else if (vgaMode->bpp == 8)
            return GRAP_8BPP;
        else if (vgaMode->bpp == 15)
            return GRAP_15BPP;
        else if (vgaMode->bpp == 16)
            return GRAP_16BPP;
        else if (vgaMode->bpp == 24)
            return GRAP_24BPP;
        else if (vgaMode->bpp == 32)
            return GRAP_32BPP;
        else
            return GRAP_1BPP;
    }
    else if (vgaMode->mode == VGA_MODE_TEXT)
    {
        if (vgaMode->bpp == 1)
            return TEXT_1BPP;
    }
    return Unknown; // Default to 1BPP for unknown modes
}

void mode_SetMode(uint16_t mode)
{
    VGA_currentMode = mode;
    vga_mode_t selectedMode = vga_modes[mode];
    /*
    Registers16 regs;
    regs.ax = 0x4F01; // Get mode function
    regs.cx = mode;   // Mode number
    uint32_t addr = (uint32_t)&selectedMode;
    regs.es = (addr & 0xFFFF0000) >> 16;
    regs.di = addr & 0xFFFF; // Address to store mode information
    log_debug(MODULE, "VGA_SetMode: regs.es=%x, regs.di=%x", regs.es, regs.di);
    log_debug(MODULE, "VGA_SetMode: mode=%u, addr=%p", mode, addr);
    CallInt(0x10, &regs, &regs);
    log_debug(MODULE, "VGA_SetMode: regs.ax=%x, regs.bx=%x, regs.cx=%x, regs.dx=%x", regs.ax, regs.bx, regs.cx, regs.dx);
    log_debug(MODULE, "VGA_SetMode: selectedMode=%p", &selectedMode);
    if ((regs.ax & 0x00FF) != 0x4F) // Check if the function was successful
    {
        log_err(MODULE, "Failed to get mode information for mode %u", mode);
        return;
    }   
    
    regs.ax = 0x4F02; // Set mode function
    regs.bx = mode;   // Mode number
    CallInt(0x10, &regs, &regs);
    if ((regs.ax & 0x00FF) != 0x4F) // Check if the function was successful
    {
        log_err(MODULE, "Failed to set mode %u", mode);
        return;
    }
    */

    VGA_Framebuffer = (uint8_t *)selectedMode.framebuffer; // Set the framebuffer address for graphics mode
    if (VGA_Framebuffer == NULL)
    {
        VGA_Framebuffer = (uint8_t*)0xA0000;
    }

    ScreenWidth = selectedMode.width;
    ScreenHeight = selectedMode.height;
    VGA_ModeBPP = GetBPP(&selectedMode);
    ScreenPitch = selectedMode.pitch;

    log_debug(MODULE, "Framebuffer: %p, Width: %u, Height: %u, BPP: %u, Pitch: %u",
              VGA_Framebuffer, ScreenWidth, ScreenHeight, VGA_ModeBPP, selectedMode.pitch);

    if (selectedMode.mode == VGA_MODE_GRAPH)
    {
        CurrentMode = VGA_MODE_GRAPH;
        VGAGrap_init(&selectedMode);
    }
    else if (selectedMode.mode == VGA_MODE_TEXT)
    {
        CurrentMode = VGA_MODE_TEXT;
        VGAText_init(&selectedMode);
    }

    VGA_clrscr();
}