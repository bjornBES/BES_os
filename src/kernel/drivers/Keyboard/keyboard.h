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

enum KEYCODE
{
	NULL_KEY = 0,
	KEY_ESCAPE = 0x01,
	KEY_ENTER = 0x1C,
	KEY_TAB = 0x0F,
	KEY_BACKSPACE = 0x0E,
	KEY_CAPSLOCK = 0x3A,
	KEY_LEFT_SHIFT = 0x2A,
	KEY_RIGHT_SHIFT = 0x36,
	KEY_LEFT_ALT = 0x38,
	KEY_RIGHT_ALT = 0xE038,
	KEY_LEFT_CTRL = 0x1D,
	KEY_RIGHT_CTRL = 0xE01D,
	KEY_NUMBERLOCK = 0x45,
	KEY_SCROLLLOCK = 0x46,
	KEY_HOME = 0xE047,
	KEY_PAGE_UP = 0xE049,
	KEY_PAGE_DOWN = 0xE051,
	KEY_END = 0xE04F,
	KEY_INSERT = 0xE052,
	KEY_DELETE = 0xE053,
	KEY_UP = 0xE048,
	KEY_DOWN = 0xE050,
	KEY_LEFT = 0xE04B,
	KEY_RIGHT = 0xE04D,
	KEY_F1 = 0x3B,
	KEY_F2 = 0x3C,
	KEY_F3 = 0x3D,
	KEY_F4 = 0x3E,
	KEY_F5 = 0x3F,
	KEY_F6 = 0x40,
	KEY_F7 = 0x41,
	KEY_F8 = 0x42,
	KEY_F9 = 0x43,
	KEY_F10 = 0x44,
	KEY_F11 = 0x57,
	KEY_F12 = 0x58,
	KEY_A = 0x1E,
	KEY_B = 0x30,
	KEY_C = 0x2E,
	KEY_D = 0x20,
	KEY_E = 0x12,
	KEY_F = 0x21,
	KEY_G = 0x22,
	KEY_H = 0x23,
	KEY_I = 0x17,
	KEY_J = 0x24,
	KEY_K = 0x25,
	KEY_L = 0x26,
	KEY_M = 0x32,
	KEY_N = 0x31,
	KEY_O = 0x18,
	KEY_P = 0x19,
	KEY_Q = 0x10,
	KEY_R = 0x13,
	KEY_S = 0x1F,
	KEY_T = 0x14,
	KEY_U = 0x16,
	KEY_V = 0x2F,
	KEY_W = 0x11,
	KEY_X = 0x2D,
	KEY_Y = 0x15,
	KEY_Z = 0x2C,
	KEY_SPACE = 0x39,
	KEY_1 = 0x02,
	KEY_2 = 0x03,
	KEY_3 = 0x04,
	KEY_4 = 0x05,
	KEY_5 = 0x06,
	KEY_6 = 0x07,
	KEY_7 = 0x08,
	KEY_8 = 0x09,
	KEY_9 = 0x0A,
	KEY_0 = 0x0B,
	KEY_DASH = 0x0C,
	KEY_EQUAL = 0x0D,
	KEY_LEFT_BRACKET = 0x1A,
	KEY_RIGHT_BRACKET = 0x1B,
	KEY_BACKSLASH = 0x2B,
	KEY_SEMICOLON = 0x27,
	KEY_SINGLE_QUOTE = 0x28,
	KEY_COMMA = 0x33,
	KEY_DOT = 0x34,
	KEY_FORWARD_SLASH = 0x35,
	KEY_BACK_TICK = 0x29,
	KEY_KEYPAD_ASTERISK = 0x37,
	KEY_KEYPAD_MINUS = 0x4A,
	KEY_KEYPAD_PLUS = 0x4E,
	KEY_KEYPAD_DOT = 0x53,
	KEY_KEYPAD_ENTER = 0xE01C,
	KEY_KEYPAD_FORWARD_SLASH = 0xE035,
	KEY_KEYPAD_0 = 0x52,
	KEY_KEYPAD_1 = 0x4F,
	KEY_KEYPAD_2 = 0x50,
	KEY_KEYPAD_3 = 0x51,
	KEY_KEYPAD_4 = 0x4B,
	KEY_KEYPAD_5 = 0x4C,
	KEY_KEYPAD_6 = 0x4D,
	KEY_KEYPAD_7 = 0x47,
	KEY_KEYPAD_8 = 0x48,
	KEY_KEYPAD_9 = 0x49,
	KEY_POWER_BUTTON = 0xE05E,
	KEY_PRINT_SCREEN = 0xE037,

};

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