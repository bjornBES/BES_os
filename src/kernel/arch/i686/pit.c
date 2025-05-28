#include "pit.h"
#include "io.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "i8259.h"

int timer_ticks = 0;

#define MILISECOND_PER_PIT_TICK 2
#define SECOUND_PER_PIT_TICK 1000 / MILISECOND_PER_PIT_TICK

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

void timer_handler(Registers *r)
{
    /* Increment our 'tick count' */
    timer_ticks++;
    // log_debug("TIMER", "One tick has passed\n");

    /* Every 18 clocks (approximately 1 second), we will
     *  display a message on the screen */
    if (timer_ticks % SECOUND_PER_PIT_TICK == 0)
    {
        // log_debug("TIMER", "One second has passed\n");
        // printf("One second has passed\n");
    }
    i8259_SendEndOfInterrupt(0);
}

void timer_wait(int ticks)
{
    timer_ticks = 0;
    while (timer_ticks < ticks)
    {
        i686_HLT();
    }
}

void sleep_ms(int ms)
{
    int ticks = ms / MILISECOND_PER_PIT_TICK; // Round up
    timer_wait(ticks);
}

void sleep_sec(int sec)
{
    int ms = sec * 1000; // Fix integer division error
    sleep_ms(ms);
}

void pit_init()
{
    set_pit(500);
    timer_ticks = 0;
    i686_IRQ_RegisterHandler(0, timer_handler);
    // i686_ISR_RegisterHandler(32, timer_handler);
}