#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <boot/bootparams.h>
#include "vga.h"

extern bool VGA_mono;

#define VGA_GraphicsRegOffset               0x3CE
#define VGA_SequencerRegOffset              0x3C4
#define VGA_AttributeControllerRegOffset    0x3C0
#define VGA_CRTControllerRegOffset          (VGA_mono) ? 0x3B0 : 0x3D0 
#define VGA_MiscOffset                      0x3C1

void VGAText_LoadFont(uint8_t* font);
void VGAText_clrscr();
void VGAText_putc(char c);
void VGAText_init(vga_mode_t* mode);
void VGAText_putchr(int x, int y, char c);
void VGAText_setcursor(int x, int y);
void VGAText_getcursor(int *x, int *y);