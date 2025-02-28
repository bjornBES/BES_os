#pragma once

#include <stdint.h>
#include <stdbool.h>

extern bool VGA_mono;

#define VGA_GraphicsRegOffset               0x3CE
#define VGA_SequencerRegOffset              0x3C4
#define VGA_AttributeControllerRegOffset    0x3C0
#define VGA_CRTControllerRegOffset          (VGA_mono) ? 0x3B0 : 0x3D0 
#define VGA_MiscOffset                      0x3C1

enum RegisterSelect
{

    Graphics_Registers,
    Sequencer_Registers,
    Attribute_Controller,
    CRT_Controller_Registers,
    Color_Registers,
    External_Registers,
};

enum ExternalRegisterSelect
{
    ER_OutputRegister,
    ER_FeatureControlRegister,
    ER_InputStatus0Regiser,
    ER_InputStatus1Regiser
};

enum GraphicsRegisters
{

    GR_Set_Reset_Register,
    GR_Enable_Set_Reset_Register,
    GR_Color_Compare_Register,
    GR_Data_Rotate_Register,
    GR_Read_Map_Select_Register,
    GR_Graphics_Mode_Register,
    GR_Miscellaneous_Graphics_Register,
    GR_Color_Don_Care_Register,
    GR_Bit_Mask_Register
};

enum CRTController
{
    CRTC_Horizontal_Total_Register              = 0x0,
    CRTC_End_Horizontal_Display_Register        = 0x1,
    CRTC_Start_Horizontal_Blanking_Register     = 0x2,
    CRTC_End_Horizontal_Blanking_Register       = 0x3,
    CRTC_Start_Horizontal_Retrace_Register      = 0x4,
    CRTC_End_Horizontal_Retrace_Register        = 0x5,
    CRTC_Vertical_Total_Register                = 0x6,
    CRTC_Overflow_Register                      = 0x7,
    CRTC_Preset_Row_Scan_Register               = 0x8,
    CRTC_Maximum_Scan_Line_Register             = 0x9,
    CRTC_Cursor_Start_Register                  = 0xA,
    CRTC_Cursor_End_Register                    = 0xB,
    CRTC_Start_Address_High_Register            = 0xC,
    CRTC_Start_Address_Low_Register             = 0xD,
    CRTC_Cursor_Location_High_Register          = 0xE,
    CRTC_Cursor_Location_Low_Register           = 0xF,
    CRTC_Vertical_Retrace_Start_Register        = 0x10,
    CRTC_Vertical_Retrace_End_Register          = 0x11,
    CRTC_Vertical_Display_End_Register          = 0x12,
    CRTC_Offset_Register                        = 0x13,
    CRTC_Underline_Location_Register            = 0x14,
    CRTC_Start_Vertical_Blanking_Register       = 0x15,
    CRTC_End_Vertical_Blanking                  = 0x16,
    CRTC_Mode_Control_Register                  = 0x17,
    CRTC_Line_Compare_Register                  = 0x18
};

#define VGA_INDEX 0x00
#define VGA_DATA 0x01

/* VGA Mode Types */
#define VGA_MODE_TEXT  0x01
#define VGA_MODE_GRAPH 0x02

/* VGA Mode Information Structure */
typedef struct {
    uint8_t mode;       /* VGA mode number */
    uint8_t type;       /* Text or graphics */
    uint32_t base;      /* Framebuffer base address */
    uint16_t width;     /* Columns (text) or pixels (graphics) */
    uint16_t height;    /* Rows (text) or pixels (graphics) */
    uint8_t bpp;        /* Bits per pixel (for graphics) */
} vga_mode_t;

void VGA_clrscr();
void VGA_putc(char c);
void VGA_init();
void VGA_setcursor(int x, int y);
