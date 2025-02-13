#pragma once
#include <stdint.h>

#define ASMCALL __attribute__((cdecl))

void  ASMCALL i686_outb(uint16_t port, uint8_t value);
uint8_t ASMCALL i686_inb(uint16_t port);

void ASMCALL i686_outw(uint16_t port, uint16_t value);
uint16_t ASMCALL i686_inw(uint16_t port);

void ASMCALL i686_outd(uint16_t port, uint32_t value);
uint32_t ASMCALL i686_ind(uint16_t port);

uint8_t ASMCALL i686_EnableInterrupts();
uint8_t ASMCALL i686_DisableInterrupts();

void ASMCALL i686_HLT();
void ASMCALL i686_int2();

void i686_iowait();
void ASMCALL i686_Panic();