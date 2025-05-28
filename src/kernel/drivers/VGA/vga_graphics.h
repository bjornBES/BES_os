#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "vga.h"

void VGAGrap_put(int x, int y, uint32_t color);
void VGAGrap_clear(uint32_t color);
void VGAGrap_init(vga_mode_t* mode);
