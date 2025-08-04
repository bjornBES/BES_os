#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "drivers/Keyboard/keyboard.h"

#define PS2_CHANNEL_NO_DEVICE_CONNECTED 0
#define PS2_CHANNEL_KEYBOARD_CONNECTED 0xAB
#define PS2_CHANNEL_KEYBOARD_INITALIZED 0xAC
#define PS2_CHANNEL_MOUSE_CONNECTED 0x03
#define PS2_CHANNEL_MOUSE_INITALIZED 0x04

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

extern uint8_t ps2_first_channel_buffer[10];
extern uint8_t ps2_second_channel_buffer[10];

extern uint8_t ps2_controller_present;
extern uint8_t ps2_first_channel_present;
extern uint8_t ps2_second_channel_present;
extern uint8_t ps2_first_channel_device;
extern uint8_t ps2_second_channel_device;
extern uint8_t ps2_first_channel_buffer_pointer;
extern uint8_t ps2_second_channel_buffer_pointer;

void ps2_init_keyboard();
void ps2_write(uint8_t port, uint8_t data);
uint8_t ps2_read(uint8_t port);
uint8_t ps2_read_data();

uint32_t ps2_get_key();
void ps2_write_first_channel(uint8_t data);
uint8_t ps2_first_channel_wait_for_ack(void);
uint8_t ps2_first_channel_wait_for_response(void);
void ps2_write_second_channel(uint8_t data);
uint8_t ps2_second_channel_wait_for_ack(void);
uint8_t ps2_second_channel_wait_for_response(void);

void ps2_probe();
