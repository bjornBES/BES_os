#include "vga_graphics.h"
#include "vga_modes.h"
#include "vga.h"

#include "debug.h"

#define MODULE "VGA_GRAPHICS"

static const uint8_t sequencer[5] = {
    0x03, 0x01, 0x0F, 0x00, 0x0E
};

static const uint8_t crtc[25] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54,
    0x80, 0xBF, 0x1F, 0x00, 0x41,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x40,
    0x96, 0xB9, 0xA3, 0xFF, 0x00
};

static const uint8_t graphics_ctrl[9] = {
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x05, 0x0F, 0xFF
};

static const uint8_t attribute_ctrl[21] = {
    0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x41, 0x00, 0x0F, 0x00,
    0x00
};

vga_mode_t *vga_mode;

uint32_t getPixelIndex(int x, int y)
{
    return (y * ScreenWidth) + x;
}
uint32_t getPixelOffset(int x, int y)
{
    return (y * vga_mode->pitch + x) * (vga_mode->bpp / 8);
}

void VGAGrap_putBpp1(int x, int y, uint8_t color)
{
    uint32_t byte_offset = getPixelOffset(x, y) / 8;
    uint8_t bit_offset = 7 - (x % 8);
    if (color)
    {
        VGA_Framebuffer[byte_offset] |= (1 << bit_offset);
    }
    else
    {
        VGA_Framebuffer[byte_offset] &= ~(1 << bit_offset);
    }
}
void VGAGrap_putBpp4(int x, int y, uint8_t color)
{
    uint32_t byte_offset = getPixelOffset(x, y) / 2;
    uint8_t old_byte = VGA_Framebuffer[byte_offset];
    if (x % 2 == 0)
    {
        // Even pixel: store in low nibble (bits 0–3)
        VGA_Framebuffer[byte_offset] = (old_byte & 0xF0) | (color & 0x0F);
    }
    else
    {
        // Odd pixel: store in high nibble (bits 4–7)
        VGA_Framebuffer[byte_offset] = (old_byte & 0x0F) | ((color & 0x0F) << 4);
    }
}
void VGAGrap_putBpp8(int x, int y, uint8_t color)
{
    uint8_t *buffer = VGA_Framebuffer + getPixelOffset(x, y);
    *buffer = color; // Assume color is an 8-bit palette index
}
void VGAGrap_putBpp15(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
{
    uint16_t pixelColor = ((red & 0x1F) << vga_mode->red_position) | ((green & 0x1F) << vga_mode->green_position) | (blue & vga_mode->blue_position);
    uint16_t *buffer = (uint16_t*)VGA_Framebuffer + getPixelOffset(x, y);
    *buffer = pixelColor;
}
void VGAGrap_putBpp16(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
{
    uint16_t pixelColor = ((red & 0x1F) << vga_mode->red_position) | ((green & 0x3F) << vga_mode->green_position) | (blue & vga_mode->blue_position);
    uint16_t *buffer = (uint16_t*)VGA_Framebuffer + getPixelOffset(x, y);
    *buffer = pixelColor;
}
void VGAGrap_putBpp24(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t *buffer = VGA_Framebuffer + getPixelOffset(x, y);
    buffer[0] = blue;
    buffer[1] = green;
    buffer[2] = red;
}
void VGAGrap_putBpp32(int x, int y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    uint32_t *buffer = (uint32_t*)VGA_Framebuffer + getPixelOffset(x, y);
    *buffer = (alpha << 24) | (red << 16) | (green << 8) | blue;
}

void VGAGrap_put(int x, int y, uint32_t color)
{
    if (x < 0 || x >= ScreenWidth || y < 0 || y >= ScreenHeight)
    {
        return;
    }
    uint8_t blue = color & 0xFF;
    uint8_t green = (color >> 8) & 0xFF;
    uint8_t red = (color >> 16) & 0xFF;
    uint8_t alpha = (color >> 24) & 0xFF;

    // log_debug(MODULE, "VGA_Grap_put: x=%d, y=%d, color=0x%X, red=0x%X, green=0x%X, blue=0x%X, alpha=0x%X", x, y, color, red, green, blue, alpha);

    // Handle different bits-per-pixel modes
    switch (VGA_ModeBPP)
    {
    case GRAP_1BPP:
    {
        // 1-bit-per-pixel (monochrome)
        VGAGrap_putBpp1(x, y, color);
        break;
    }
    case GRAP_4BPP:
    {

        // 4-bit-per-pixel (16 colors)
        VGAGrap_putBpp4(x, y, color);
        break;
    }
    case GRAP_8BPP:
    {
        // 8-bit-per-pixel (256 colors)
        VGAGrap_putBpp8(x, y, color);
        break;
    }
    case GRAP_15BPP:
    {
        // 15-bit-per-pixel (5 bits each for R, G, B)
        VGAGrap_putBpp15(x, y, red, green, blue);
        break;
    }
    case GRAP_16BPP:
    {
        // 16-bit-per-pixel (RGB565: 5 bits R, 6 bits G, 5 bits B)
        VGAGrap_putBpp16(x, y, red, green, blue);
        break;
    }
    case GRAP_24BPP:
    {
        // 24-bit-per-pixel (RGB888)
        VGAGrap_putBpp24(x, y, red, green, blue);
        break;
    }
    case GRAP_32BPP:
    {
        // 32-bit-per-pixel (ARGB8888 or BGRA8888)
        VGAGrap_putBpp32(x, y, red, green, blue, alpha);
        break;
    }
    default:
        // Unsupported BPP
        fprintf(VFS_FD_DEBUG, "Unsupported BPP: %u", VGA_ModeBPP);
        break;
    }
    // log_debug(MODULE, "VGA_Grap_put: pixel_index=%u, index=%u, VGA_ModeBPP=%u, buffer=%p", pixel_index, index, VGA_ModeBPP, buffer);
    // log_debug(MODULE, "VGA_Grap_put: VGA_Framebuffer=%p", VGA_Framebuffer);
    // fprintf(VFS_FD_DEBUG, "\n");
}

void VGAGrap_clear(uint32_t color)
{
    for (int y = 0; y < ScreenHeight; y++)
    {
        for (int x = 0; x < ScreenWidth; x++)
        {
            VGAGrap_put(x, y, color);
        }
    }
}

void VGAGrap_set_pixel(int x, int y, uint8_t color)
{
    if (x >= 0 && x < VGA_GRAPHICS_WIDTH && y >= 0 && y < VGA_GRAPHICS_HEIGHT)
    {
        VGA_Framebuffer[y * VGA_GRAPHICS_WIDTH + x] = color;
    }
}


void VGAGrap_init(vga_mode_t *mode)
{
    OUTB(VGA_MISC_PORT, 0x63);
    int i;
    for (i = 0; i < 5; i++)
    {
        WriteRegister(Sequencer_Registers, i, sequencer[i]);
    }

    WriteRegister(CRT_Controller_Registers, CRTC_End_Horizontal_Blanking_Register, ReadRegister(CRT_Controller_Registers, CRTC_Vertical_Display_End_Register) | 0x80);
    WriteRegister(CRT_Controller_Registers, CRTC_Vertical_Display_End_Register, ReadRegister(CRT_Controller_Registers, CRTC_Vertical_Display_End_Register) & 0x7F);
    
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

    // Enable video output
    i686_inb(VGA_ATTR_RESET);
    i686_outb(VGA_ATTR_INDEX, 0x20);

    vga_mode = mode;
}
