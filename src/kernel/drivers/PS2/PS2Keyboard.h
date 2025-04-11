#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

void ps2_init_keyboard();
void ps2_write(uint8_t port, uint8_t data);
uint8_t ps2_read(uint8_t port);
uint8_t ps2_read_data();