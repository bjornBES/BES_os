#include "timer.h"
#include "pit.h"
#include "idt.h"
#include "isr.h"
#include "io.h"
#include "debug.h"
#include "i8259.h"

#define MILISECOND_PER_PIT_TICK 2
#define SECOUND_PER_PIT_TICK 1000 / MILISECOND_PER_PIT_TICK

void timer_phase(int hz)
{
    int divisor = 1193180 / hz;                                                                        /* Calculate our divisor */
    i686_outb(CommandRegister, SelectChannel0 | lobyteAndHibyte | squareWaveGenerator | Mode16BitBin); /* Set our command byte 0x36/0b00110110 */
    i686_outb(Channel0, divisor & 0xFF);                                                               /* Set low byte of divisor */
    i686_outb(Channel0, divisor >> 8);                                                                 /* Set high byte of divisor */
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
    }
    /*
     */
}
void timer_wait(int ticks)
{
    timer_ticks = 0;
    while (timer_ticks < ticks)
    {
        i686_HLT();
    }
}

void sleepMs(int ms)
{
    int ticks = ms / MILISECOND_PER_PIT_TICK; // Round up
    timer_wait(ticks);
}

void sleepSec(int sec)
{
    int ms = sec * 1000; // Fix integer division error
    sleepMs(ms);
}