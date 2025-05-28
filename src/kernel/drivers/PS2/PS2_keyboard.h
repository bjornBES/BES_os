#pragma once

#include <stdint.h>
#include "drivers/Keyboard/keyboard.h"

extern keyboardLEDs keyboard_leds;
extern keyboardKeys keyboard_keys;

void initalize_ps2_keyboard();
void ps2_keyboard_set_leds();
void ps2_keyboard_cheak_leds();
void ps2_keyboard_process_key_value(uint32_t key_value);
