#include "bios.h"

#include "memory.h"
#include "stdio.h"
#include "debug.h"

#include "arch/i686/gdt.h"
#include "arch/i686/idt.h"
#include "arch/i686/irq.h"
#include "arch/i686/io.h"
#include "arch/i686/i8259.h"

#define MODULE "BIOS"

void BIOS_Init()
{
    // Initialize the GDT
    i686_GDT_Initialize();
    i686_IDT_Initialize();

    // GDT_SetEntry(6, 0, 0xFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_16BIT | GDT_FLAG_GRANULARITY_1B)); // Kernel pmode 16 bit code segment
    // GDT_SetEntry(7, 0, 0xFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_16BIT | GDT_FLAG_GRANULARITY_1B)); // Kernel pmode 16 bit data segment

    // Load the GDT
    log_debug("GDT", "Loading GDT...");
    GDT_Load();
    log_debug("GDT", "GDT loaded.");
    // Load the TSS
    TSS_Load();
    // Load the IDT
    IDT_Load();
}

uint16_t ds()
{
    uint16_t seg;
    __asm__("movw %%ds,%0" : "=rm"(seg));
    return seg;
}

uint16_t fs()
{
    uint16_t seg;
    __asm__ volatile("movw %%fs,%0" : "=rm"(seg));
    return seg;
}

uint16_t gs()
{
    uint16_t seg;
    __asm__ volatile("movw %%gs,%0" : "=rm"(seg));
    return seg;
}
