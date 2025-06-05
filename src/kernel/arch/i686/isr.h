#pragma once
#include <stdint.h>

typedef struct 
{
    // in the reverse order they are pushed:
    uint32_t ds;                                            // data segment pushed by us
    union {
        uint32_t edi, esi, ebp, _esp, ebx, edx, ecx, eax;       // pusha
        struct
        {
            uint16_t di, hdi;
            uint16_t si, hsi;
            uint16_t bp, hbp;
            uint16_t _sp, _hsp;
            uint16_t bx, hbx;
            uint16_t dx, hdx;
            uint16_t cx, hcx;
            uint16_t ax, hax;                                  // pushad
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

    uint32_t interrupt, error;                              // we push interrupt, error is pushed automatically (or our dummy)
    uint32_t eip, cs, eflags, esp, ss;                      // pushed automatically by CPU
} __attribute__((packed)) Registers;

#define ISR_STACK_SIZE 16384  // 16 KB for ISR stack

struct tss_entry {
    uint32_t prev_tss;  // Previous TSS (not used)
    uint32_t esp0;      // Stack pointer for ring 0 (ISR stack)
    uint32_t ss0;       // Stack segment for ring 0
    uint32_t esp1;      // Stack pointer for ring 1 (not used)
    uint32_t ss1;       // Stack segment for ring 1 (not used)
    uint32_t esp2;      // Stack pointer for ring 2 (not used)
    uint32_t ss2;       // Stack segment for ring 2 (not used)
    uint32_t cr3;       // Page directory base (not used here)
    uint32_t eip;       // Instruction pointer (not used here)
    uint32_t eflags;    // Flags (not used here)
    uint32_t eax;       // General-purpose registers (not used here)
    uint32_t ecx;       // General-purpose registers (not used here)
    uint32_t edx;       // General-purpose registers (not used here)
    uint32_t ebx;       // General-purpose registers (not used here)
    uint32_t esp;       // Stack pointer (not used here)
    uint32_t ebp;       // Base pointer (not used here)
    uint32_t esi;       // Source index (not used here)
    uint32_t edi;       // Destination index (not used here)
    uint32_t es;        // Data segment (not used here)
    uint32_t cs;        // Code segment (not used here)
    uint32_t ss;        // Stack segment (not used here)
    uint32_t fs;        // Additional segment (not used here)
    uint32_t gs;        // Additional segment (not used here)
    uint32_t ldt;       // Local descriptor table (not used here)
    uint16_t trap;      // Trap flag (not used here)
    uint16_t iobase;    // I/O base (not used here)
} __attribute__((packed));

typedef void (*ISRHandler)(Registers* regs);

#define RETURNINTRUPT() asm volatile("iret")

void i686_ISR_Initialize();
void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler);
