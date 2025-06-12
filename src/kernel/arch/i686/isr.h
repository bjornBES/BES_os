#pragma once
#include <stdint.h>

typedef struct
{
    // in the reverse order they are pushed:
    uint32_t ds; // data segment pushed by us
    union
    {
        struct
        {
            uint32_t edi;
            uint32_t esi;
            uint32_t ebp;
            uint32_t _esp;
            uint32_t ebx;
            uint32_t edx;
            uint32_t ecx;
            uint32_t eax; // pusha
        } U32;
        struct
        {
            uint16_t di, hdi;
            uint16_t si, hsi;
            uint16_t bp, hbp;
            uint16_t _sp, _hsp;
            uint16_t bx, hbx;
            uint16_t dx, hdx;
            uint16_t cx, hcx;
            uint16_t ax, hax; // pushad
        } U16;
        struct
        {
            uint8_t dil, dih, edi2, edi3;
            uint8_t sil, sih, esi2, esi3;
            uint8_t bpl, bph, ebp2, ebp3;
            uint8_t _spl, _sph, _esp2, _esp3;
            uint8_t bl, bh, ebx2, ebx3;
            uint8_t dl, dh, edx2, edx3;
            uint8_t cl, ch, ecx2, ecx3;
            uint8_t al, ah, eax2, eax3;
        } U8;
    };

    uint32_t interrupt, error;         // we push interrupt, error is pushed automatically (or our dummy)
    uint32_t eip, cs, eflags, esp, ss; // pushed automatically by CPU
} __attribute__((packed)) Registers;

#define ISR_STACK_SIZE 16384 // 16 KB for ISR stack

typedef void (*ISRHandler)(Registers *regs);

#define RETURNINTRUPT() asm volatile("iret")

void i686_ISR_Initialize();
void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler);
