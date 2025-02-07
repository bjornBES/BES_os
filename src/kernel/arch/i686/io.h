#pragma once
#include <stdint.h>

void __attribute__((cdecl)) i686_outb(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) i686_inb(uint16_t port);

void __attribute__((cdecl)) i686_outw(uint16_t port, uint16_t value);
uint16_t __attribute__((cdecl)) i686_inw(uint16_t port);

void __attribute__((cdecl)) i686_outd(uint16_t port, uint32_t value);
uint32_t __attribute__((cdecl)) i686_ind(uint32_t port);

uint8_t __attribute__((cdecl)) i686_EnableInterrupts();
uint8_t __attribute__((cdecl)) i686_DisableInterrupts();

void i686_iowait();
void __attribute__((cdecl)) i686_Panic();