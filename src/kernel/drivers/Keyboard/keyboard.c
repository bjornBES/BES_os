
#include "keyboard.h"

#include "memory.h"

#include "stdio.h"

#include "string.h"

#include "debug.h"

#include "arch/i686/i8259.h"
#include "arch/i686/irq.h"
#include "arch/i686/io.h"

#include "drivers/PS2/8042_controller.h"
#include "drivers/PS2/PS2_keyboard.h"

#include <printfDriver/printf.h>

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

#define CACHE_SIZE 1024

uint16_t *keyboard_layout_ptr;
uint16_t *keyboard_shift_layout_ptr;

uint8_t lastkey = 0;
uint16_t key_loc = 0;

uint32_t *keycache = 0;
int16_t ReadPointer;
int16_t WritePointer;

keyboardLEDs keyboard_leds_state;
keyboardKeys keyboard_keys_state;

#define MODULE "KEYBOARD"

void PutBuffer(uint32_t data)
{
	keycache[WritePointer] = data;
	WritePointer++;
	if (WritePointer >= CACHE_SIZE)
	{
		WritePointer = 0;
	}
}
uint32_t TakeBuffer()
{

	if (ReadPointer == WritePointer)
	{
		return 0;
	}
	uint32_t data = keycache[ReadPointer];
	ReadPointer++;

	if (ReadPointer >= CACHE_SIZE)
	{
		ReadPointer = 0;
	}
	return data;
}

void updateLDEs()
{
	// ps2_setLEDs(keyboard_leds);
	return;
}

static uint16_t c = 0;
uint16_t KeyboardGetKey()
{
	uint32_t key = TakeBuffer();
	if (!keyboard_keys_state.shift && !keyboard_leds_state.capslock)
	{
		c = keyboard_layout_ptr[key];
	}
	else
	{
		c = keyboard_shift_layout_ptr[key];
	}

	if (c == 0)
	{
		return '\0';
	}


	return c;
}

void PressAnyKeyLoop()
{
	fprintf(VFS_FD_DEBUG, "Press any key...");
	while (true)
	{
		uint16_t key = KeyboardGetKey();
		if (key == '\0')
		{
			continue;
		}
		fprintf(VFS_FD_DEBUG, "Key %u", key);
		break;
	}
}

void keyboard_update_keys_state()
{
	keyboard_keys_state.shift = 0;
	keyboard_keys_state.ctrl = 0;
	keyboard_keys_state.alt = 0;
	
	keyboard_keys_state.shift |= keyboard_keys.shift;
	keyboard_keys_state.ctrl |= keyboard_keys.ctrl;
	keyboard_keys_state.alt |= keyboard_keys.alt;
}

void keyboard_process_code(uint32_t key)
{
	uint8_t byte1 = (key & 0x000000FF);
	// uint8_t byte2 = ((key & 0x0000FF00) >> 8);
	// uint8_t byte3 = ((key & 0x00FF0000) >> 16);
	// uint8_t byte4 = ((key & 0xFF000000) >> 24);

	// get unicode value
	if ((byte1 & 0x80) != 0x80)
	{
		PutBuffer(key);
	}
}

void keyboard_init()
{
	keycache = (uint32_t *)malloc(CACHE_SIZE);
	memset(keycache, 0, CACHE_SIZE);
	ReadPointer = 0;
	WritePointer = 0;

	keyboard_layout_ptr = danish_keyboard_layout;
	keyboard_shift_layout_ptr = danish_shift_keyboard_layout;

	memset(&keyboard_keys_state, 0, sizeof(keyboardKeys));
	memset(&keyboard_leds_state, 0, sizeof(keyboardLEDs));
	keyboard_leds_state.capslock = 0;
	keyboard_leds_state.numlock = 0;
	keyboard_leds_state.scrollock = 0;
	keyboard_keys_state.shift = 0;
	keyboard_keys_state.ctrl = 0;
	keyboard_keys_state.alt = 0;

	printf("Keyboard: %s\n", "before PS2_init_keyboard");
	ps2_init_keyboard();
	printf("Keyboard: %s\n", "after PS2_init_keyboard");
	mmPrintStatus();
}
