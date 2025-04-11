
#include "keyboard.h"

#include "malloc.h"
#include "memory.h"

#include "stdio.h"

#include "debug.h"

#include "arch/i686/i8259.h"
#include "arch/i686/irq.h"
#include "arch/i686/io.h"

#include "drivers/PS2/PS2Keyboard.h"

#include <printfDriver/printf.h>

enum KEYCODE
{
	NULL_KEY = 0,
	KEY_ESC = 0x01,
	KEY_ENTER = 0x1C,
	KEY_TAB = 0x0F,
	KEY_BACKSPACE = 0x0E,
	KEY_CAPSLOCK = 0x3A,
	KEY_ESCAPE = 0x01,
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

#define LED_SCROLL_LOCK 0x01
#define LED_NUM_LOCK 0x02
#define LED_CAPS_LOCK 0x04

#define SCANCODE_ENTER 0x1c
#define SCANCODE_BACK 0x0e
#define SCANCODE_SHIFT 0x2a
#define SCANCODE_CAPS 0x3a
#define SCANCODE_NUMLOCK 0x45
#define SCANCODE_SCROLLLOCK 0x46
#define SCANCODE_UP 0x48

#define KEY_RELEASED 0xF0 // 0xF0 is the prefix for released keys
#define KEY_PRESSED 0x00

#define KEY_RELEASED_MASK 0x80
#define KEY_PRESSED_MASK 0x7F

#define KB_COMMAND 0x64		/* I/O port for commands on AT */
#define KB_STATUS 0x64		/* I/O port for status on AT */
#define KB_ACK 0xFA			/* keyboard ack response */
#define KB_AUX_BYTE 0x20	/* Auxiliary Device Output Buffer Full */
#define KB_OUT_FULL 0x01	/* status bit set when keypress char pending */
#define KB_IN_FULL 0x02		/* status bit set when not ready to receive */
#define KBC_RD_RAM_CCB 0x20 /* Read controller command byte */
#define KBC_WR_RAM_CCB 0x60 /* Write controller command byte */
#define KBC_DI_AUX 0xA7		/* Disable Auxiliary Device */
#define KBC_EN_AUX 0xA8		/* Enable Auxiliary Device */
#define KBC_DI_KBD 0xAD		/* Disable Keybard Interface */
#define KBC_EN_KBD 0xAE		/* Enable Keybard Interface */
#define LED_CODE 0xED		/* command to keyboard to set LEDs */

keyboardLEDs keyboard_leds;
keyboardKeys keyboard_keys;

uint16_t *keyboard_layout_ptr;
uint16_t *keyboard_shift_layout_ptr;

uint8_t lastkey = 0;
uint8_t *keycache = 0;
uint16_t key_loc = 0;

Page *keycachePage;

void keyboard_irq(Registers *r)
{
	uint8_t key = ps2_read_data();

	if (key == NULL_KEY)
	{
		goto ExitIRQ;
	}

	if (key & KEY_RELEASED_MASK)
	{
		// Key released
		fprintf(VFS_FD_DEBUG, "Key released: 0x%x\n", key);
	}
	else
	{
		// Key pressed
		char c = KeyboardScancodeToAscii(key);
		fprintf(VFS_FD_DEBUG, "Key pressed: 0x%x/%c\n", key, c);
		if (c && key_loc < 255)
		{
			keycache[key_loc++] = c;
			keycache[key_loc] = 0;
		}
		else
		{
			fprintf(VFS_FD_DEBUG, "Key cache full\n");
			printf("KEYBOARD: Key cache full\n");
		}
	}

ExitIRQ:
	i8259_SendEndOfInterrupt(1);
}

void updateLDEs()
{
	uint8_t data = keyboard_leds.capslock << 1 | keyboard_leds.numlock << 2 | keyboard_leds.scrollock << 3;
	ps2_write(PS2_COMMAND, 0xED); // Set LED
	ps2_write(PS2_DATA, data); // Set LED
	return;
}

static char c = 0;
char KeyboardGetKey()
{
	c = 0;

	while (key_loc == 0)
	{
		;
	}
	
	c = keycache[0];
	for (int i = 1; i < key_loc; i++)
	{
		keycache[i - 1] = keycache[i];
	}
	key_loc--;

	switch (c)
	{
	case SCANCODE_CAPS:
		keyboard_leds.capslock = !keyboard_leds.capslock;
		break;
	case SCANCODE_NUMLOCK:
		keyboard_leds.numlock = !keyboard_leds.numlock;
		break;
	case SCANCODE_SCROLLLOCK:
		keyboard_leds.scrollock = !keyboard_leds.scrollock;
		break;
	}

	updateLDEs();

	return c;
}

void PressAnyKeyLoop()
{
	printf("Press any key...");
	while (true)
	{
		if (KeyboardGetKey() != 0)
		{
			break;
		}
		continue;
	}
}

static char *_qwertzuiop = "qwertzuiop"; // 0x10-0x1c
static char *_asdfghjkl = "asdfghjkl";
static char *_yxcvbnm = "yxcvbnm";
static char *_num = "123456789";
uint8_t KeyboardScancodeToAscii(uint8_t key)
{
	// kprintf("key=0x%x\n", key);
	if (key == KEY_BACKSPACE)
	{
		return '\b';
	}
	if (key == KEY_ENTER || (lastkey == 0xE0 && key == KEY_ENTER))
	{
		return '\n';
	}
	if (key == KEY_SPACE)
	{
		return ' ';
	}
	if (key == KEY_DOT)
	{
		return '.';
	}
	if (key == KEY_FORWARD_SLASH)
	{
		return '/';
	}
	if (key == KEY_0)
	{
		return '0';
	}
	if (key >= KEY_1 && key <= KEY_9)
	{
		return _num[key - KEY_1];
	}
	if (key >= 0x10 && key <= 0x1C)
	{
		return _qwertzuiop[key - 0x10];
	}
	else if (key >= 0x1E && key <= 0x26)
	{
		return _asdfghjkl[key - 0x1E];
	}
	else if (key >= 0x2C && key <= 0x32)
	{
		return _yxcvbnm[key - 0x2C];
	}
	return 0;
}

void keyboard_init()
{
	keycache = (uint8_t *)malloc(PAGE_SIZE, keycachePage);
	memset(keycache, 0, PAGE_SIZE);

	i686_IRQ_RegisterHandler(1, keyboard_irq);

	ps2_init_keyboard();

	keyboard_layout_ptr = danish_keyboard_layout;
	keyboard_shift_layout_ptr = danish_shift_keyboard_layout;

	keyboard_leds.capslock = false;
	keyboard_leds.numlock = false;
	keyboard_leds.scrollock = false;
}
