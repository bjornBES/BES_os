#pragma once

#include <stdint.h>

#define Channel0            0x40
#define Channel1            0x41
#define Channel2            0x42
#define CommandRegister     0x43
#define ModeRegister        0x43

enum ChannelSelect
{
    SelectChannel0 = 0b00000000,
    SelectChannel2 = 0b10000000,
    SelectChannel1 = 0b01000000,
};

enum Accessmode
{
    LatchCountValueCommand = 0b00000000,
    lobyteOnly = 0b00010000,
    hibyteOnly = 0b00100000,
    lobyteAndHibyte = 0b00110000,
};

enum ModeSelect
{
    InterruptOnTerminalCount = 0b00000000,
    HardwareRetriggerable = 0b00000010,
    rateGenerator = 0b00000100,
    squareWaveGenerator = 0b00000110,
    softwareTriggeredStrobe = 0b00001000,
    hardwareTriggeredStrobe = 0b00001010,
};

#define Mode16BitBin 0
#define ModeBCD 1

extern int timer_ticks;

/*
Bits         Usage
6 and 7      Select channel :
                0 0 = Channel 0
                0 1 = Channel 1
                1 0 = Channel 2
                1 1 = Read-back command (8254 only)
4 and 5      Access mode :
                0 0 = Latch count value command
                0 1 = Access mode: lobyte only
                1 0 = Access mode: hibyte only
                1 1 = Access mode: lobyte/hibyte
1 to 3       Operating mode :
                0 0 0 = Mode 0 (interrupt on terminal count)
                0 0 1 = Mode 1 (hardware re-triggerable one-shot)
                0 1 0 = Mode 2 (rate generator)
                0 1 1 = Mode 3 (square wave generator)
                1 0 0 = Mode 4 (software triggered strobe)
                1 0 1 = Mode 5 (hardware triggered strobe)
                1 1 0 = Mode 2 (rate generator, same as 010b)
                1 1 1 = Mode 3 (square wave generator, same as 011b)
0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
*/

uint32_t read_pit_count(void);
void set_pit_count(uint32_t count);
void pit_init();
void sleep_ms(int ms);
void sleep_sec(int sec);

void setTick(uint32_t tick);
uint32_t getTick();