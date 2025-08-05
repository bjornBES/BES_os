#include "io.h"

#define UNUSED_PORT         0x80

void i686_iowait()
{
    i686_outb(UNUSED_PORT, 0);
}

void i686_outb(uint16_t port, uint8_t value)
{
    __asm__ ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t i686_inb(uint16_t port)
{
    uint8_t value;
    __asm__ ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void i686_outw(uint16_t port, uint16_t value)
{
    __asm__ ("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t i686_inw(uint16_t port)
{
    uint16_t value;
    __asm__ ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void i686_outd(uint16_t port, uint32_t value)
{
    i686_outw(port, value & 0xFFFF);
    i686_outw(port, (value >> 16) & 0xFFFF);
}

uint32_t i686_ind(uint16_t port)
{
    uint32_t value;
    value = i686_inw(port);
    value |= (i686_inw(port) << 16);
    return value;
}

