
#include "keyboard.h"

#include "malloc.h"
#include "memory.h"

#include "stdio.h"

#include "arch/i686/i8259.h"
#include "arch/i686/irq.h"
#include "arch/i686/io.h"


#define KBD_SEND(byt) i686_outb(0x64, byt)
#define KBD_RECD() i686_inb(0x64)

uint8_t lastkey = 0;
uint8_t *keycache = 0;
uint16_t key_loc = 0;
uint8_t __kbd_enabled = 0;

Page* keycachePage;

void keyboard_irq(Registers *r)
{
	keycache[key_loc++] = KeyboardToAscii(i686_inb(0x60));
	i8259_SendEndOfInterrupt(1);
}

void keyboard_init()
{
    keycache = (uint8_t*)malloc(256, keycachePage);
    memset(keycache, 0, 256);

    i686_IRQ_RegisterHandler(1, keyboard_irq);

    __kbd_enabled = 1;
}
uint8_t KeyboardEnabled()
{
return __kbd_enabled;
}

void keyboard_read_key()
{
	lastkey = 0;
	if(i686_inb(0x64) & 1)
		lastkey = i686_inb(0x60);
}

static char c = 0;
char KeyboardGetKey()
{
	c = 0;
	if(key_loc == 0) goto out;
	c = *keycache;
	key_loc --;
	for(int i = 0; i < 256; i++)
	{
		keycache[i] = keycache[i+1];
	}
out:
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

static char* _qwertzuiop = "qwertzuiop"; // 0x10-0x1c
static char* _asdfghjkl = "asdfghjkl";
static char* _yxcvbnm = "yxcvbnm";
static char* _num = "123456789";
uint8_t KeyboardToAscii(uint8_t key)
{
	//kprintf("key=0x%x\n", key);
	if(key == 0x1C) return '\n';
	if(key == 0x39) return ' ';
	if(key == 0xE) return '\r';
	if(key == POINT_RELEASED) return '.';
	if(key == SLASH_RELEASED) return '/';
	if(key == ZERO_PRESSED) return '0';
	if(key >= ONE_PRESSED && key <= NINE_PRESSED)
		return _num[key - ONE_PRESSED];
	if(key >= 0x10 && key <= 0x1C)
	{
		return _qwertzuiop[key - 0x10];
	} else if(key >= 0x1E && key <= 0x26)
	{
		return _asdfghjkl[key - 0x1E];
	} else if(key >= 0x2C && key <= 0x32)
	{
		return _yxcvbnm[key - 0x2C];
	}
	return 0;
}