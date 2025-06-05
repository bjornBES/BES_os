#pragma once

#include "arch/i686/isr.h"


typedef struct IntRegisters_t
{
    union
    {
        struct
        {
            uint32_t eax;
            uint32_t ebx;
            uint32_t ecx;
            uint32_t edx;
            uint32_t esi;
            uint32_t edi;
			uint32_t fsgs;
			uint32_t dses;
			uint32_t eflags;
        };
        struct
        {
            uint16_t ax, hax;
            uint16_t bx, hbx;
            uint16_t cx, hcx;
            uint16_t dx, hdx;
            uint16_t si, hsi;
            uint16_t di, hdi;
            uint16_t gs, fs;
			uint16_t es, ds;
        };
        struct
        {
            uint8_t al, ah, eax2, eax3;
            uint8_t bl, bh, ebx2, ebx3;
            uint8_t cl, ch, ecx2, ecx3;
            uint8_t dl, dh, edx2, edx3;
            uint8_t sil, sih, esi2, esi3;
            uint8_t dil, dih, edi2, edi3;
        };
    };
} __attribute__((packed)) IntRegisters;

#define ASMCALL __attribute__((cdecl))

ASMCALL void CallInt(int interrupt_number, IntRegisters *regs);