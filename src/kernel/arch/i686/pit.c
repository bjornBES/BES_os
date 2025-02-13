#include "pit.h"
#include "io.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "timer.h"

int timer_ticks = 0;

uint32_t read_pit_count(void)
{
    uint32_t count = 0;

    i686_DisableInterrupts();

    i686_outb(CommandRegister, SelectChannel0 | LatchCountValueCommand | InterruptOnTerminalCount | Mode16BitBin);

    count = i686_inb(Channel0);
    count |= i686_inb(Channel0) << 8;

    i686_EnableInterrupts();

    return count;
}

void set_pit(int hz)
{
    int divisor = 1193180 / hz;                                                                        /* Calculate our divisor */
    i686_outb(CommandRegister, SelectChannel0 | lobyteAndHibyte | squareWaveGenerator | Mode16BitBin); /* Set our command byte 0x36/0b00110110 */
    i686_outb(Channel0, divisor & 0xFF);                                                               /* Set low byte of divisor */
    i686_outb(Channel0, divisor >> 8);                                                                 /* Set high byte of divisor */
}

void set_pit_count(uint32_t count)
{
    i686_DisableInterrupts();
    i686_outb(Channel0, count & 0xFF);
    i686_outb(Channel0, (count & 0xFF00) >> 8);
    i686_EnableInterrupts();
}

void pit_init()
{
    set_pit(500);
    timer_ticks = 0;
    i686_IRQ_RegisterHandler(0, timer_handler);
    // i686_ISR_RegisterHandler(32, timer_handler);
}