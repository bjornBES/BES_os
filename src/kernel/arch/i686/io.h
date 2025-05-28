#pragma once
#include <stdint.h>

#define ASMCALL __attribute__((cdecl))

#define ASM_INT2() __asm__ __volatile__("int $0x02" : : : "memory")

void  ASMCALL i686_outb(uint16_t port, uint8_t value);
uint8_t ASMCALL i686_inb(uint16_t port);

void ASMCALL i686_outw(uint16_t port, uint16_t value);
uint16_t ASMCALL i686_inw(uint16_t port);

void ASMCALL i686_outd(uint16_t port, uint32_t value);
uint32_t ASMCALL i686_ind(uint16_t port);

#define OUTB(port, value) i686_outb(port, value)
#define INB(port) i686_inb(port)
#define OUTW(port, value) i686_outw(port, value)
#define INW(port) i686_inw(port)
#define OUTD(port, value) i686_outd(port, value)
#define IND(port) i686_ind(port)

void ASMCALL i686_SetStack(uint32_t address);

uint8_t ASMCALL i686_EnableInterrupts();
uint8_t ASMCALL i686_DisableInterrupts();

void ASMCALL i686_HLT();
void ASMCALL i686_int2();

void ASMCALL i686_EnableMCE();

void i686_iowait();
void ASMCALL i686_Panic();