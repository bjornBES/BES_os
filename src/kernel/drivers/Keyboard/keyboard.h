#pragma once

#include <stdint.h>

#include "malloc.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
//
// Kb layouts is from the BleskOS repo
// https://github.com/VendelinSlezak/BleskOS/tree/master
//
static uint16_t english_keyboard_layout[256] = {
	0, 0,																 // zero, escape
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,	 // backspace, tab
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0xA, 0,	 // enter, left ctrl
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '`', '`', 0, '\\', // left shift
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',	 // right shift
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,										 // other control keys
	'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',	 // numpad
	0																	 // other keys
};
static uint16_t english_shift_keyboard_layout[256] = {
	0, 0,																// zero, escape
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,	// backspace, tab
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0xA, 0, // enter, left ctrl
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', // left shift
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',	// right shift
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,										// other control keys
	'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',	// numpad
	0																	// other keys
};

static uint16_t danish_keyboard_layout[256] = {
	0, 0x1B,																// zero, escape
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+', 0xB4, 0x8,		// numbers row from 1 to Backspace
	0x9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 0xE5, 0xA8, 0xA,	// tab to Enter
	0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0xE6, 0xF8, '\'',		// left ctrl to '`' DK: usually section (§) or grave accent
	0, '<', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, 			// left shift to right shift 
	'*',			// numpad *
	0,				// left alt
	' ',	 		// Spacebar
	0,				// Caps lock
	0xF001, 0xF002, 0xF003, 0xF004, 0xF005,									// function keys (F1 to F5)
	0xF006, 0xF007, 0xF008, 0xF009, 0xF001,									// function keys (F6 to F10)
	0,				// Num lock
	0,				// Scroll lock
	'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',	 	// numpad
	0, 0, 0, 0,
	0xF00B,																	// F11
	0xF00C,																	// F12
};
static uint16_t danish_shift_keyboard_layout[256] = {
	0, 0x1B,																// zero, escape
	'!', '"', '#', ' ', '%', '&', '/', '(', ')', '=', '?', 0xB4, 0x8,		// numbers row from 1 to Backspace
	0x9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0xC5, 0xA8, 0xA,	// tab to Enter
	0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0xC6, 0xD8, 0xA7, 		// left ctrl to '`' DK: usually section (§) or grave accent
	0, '>', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 0,			// left shift to right shift 
	'*',			// numpad *
	0,				// left alt
	' ',	 		// Spacebar
	0,				// Caps lock
	0xF001, 0xF002, 0xF003, 0xF004, 0xF005,									// function keys (F1 to F5)
	0xF006, 0xF007, 0xF008, 0xF009, 0xF001,									// function keys (F6 to F10)
	0,				// Num lock
	0,				// Scroll lock
	'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',	 	// numpad
	0, 0, 0, 0,
	0xF00B,																	// F11
	0xF00C,																	// F12
};

static uint16_t slovak_keyboard_layout[256] = {
	0, 0,																			   // zero, escape
	'+', 0x13E, 0x161, 0x10D, 0x165, 0x17E, 0xFD, 0xE1, 0xED, 0xE9, '=', 0x0301, 0, 0, // backspace, tab
	'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 0xFA, 0xE4, 0xA, 0,			   // enter, left ctrl
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0xF4, 0xA7, ';', 0, 0x148,			   // left shift
	'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, '\\', 0, ' ',				   // right shift
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,													   // other control keys
	'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',				   // numpad
	0																				   // other keys
};
static uint16_t slovak_shift_keyboard_layout[256] = {
	0, 0,																   // zero, escape
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '%', 0x030C, 0, 0,   // backspace, tab
	'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '/', '(', 0xA, 0,	   // enter, left ctrl
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '"', '!', 0xC2B0, 0, ')', // left shift
	'Y', 'X', 'C', 'V', 'B', 'N', 'M', '?', ':', '_', 0, '|', 0, ' ',	   // right shift
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,										   // other control keys
	'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',	   // numpad
	0																	   // other keys
};

#pragma GCC diagnostic pop

typedef struct keyboard_keys_t
{
	uint8_t shift : 1;
	uint8_t ctrl : 1;
	uint8_t alt : 1;
	uint8_t reserved : 5;
	uint32_t pressed_keys[6];
} __attribute__((packed)) keyboardKeys;

typedef struct keyboard_leds_t
{
	uint8_t capslock : 1;
	uint8_t numlock : 1;
	uint8_t scrollock : 1;
	uint8_t reserved : 5;
} __attribute__((packed)) keyboardLEDs;

extern keyboardLEDs keyboard_leds_state;
extern keyboardKeys keyboard_keys_state;
extern Page *keycachePage;

void keyboard_update_keys_state();
void keyboard_init();
void PressAnyKeyLoop();
uint16_t KeyboardGetKey();
void keyboard_process_code(uint32_t key);