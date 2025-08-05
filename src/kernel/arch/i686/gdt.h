#pragma once

#include "defaultInclude.h"

#define i686_GDT_CODE_SEGMENT 0x08
#define i686_GDT_DATA_SEGMENT 0x10
#define i686_GDT_USER_CODE_SEGMENT 0x18
#define i686_GDT_USER_DATA_SEGMENT 0x20
#define i686_GDT_TSS_SEGMENT 0x28
#define i686_GDT_STACK_SEGMENT 0x30

typedef enum
{
    GDT_ACCESS_CODE_READABLE                = 0x02,
    GDT_ACCESS_DATA_WRITEABLE               = 0x02,

    GDT_ACCESS_CODE_CONFORMING              = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL        = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN          = 0x04,

    GDT_ACCESS_DATA_SEGMENT                 = 0x10,
    GDT_ACCESS_CODE_SEGMENT                 = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS               = 0x00,

    GDT_ACCESS_RING0                        = 0x00,
    GDT_ACCESS_RING1                        = 0x20,
    GDT_ACCESS_RING2                        = 0x40,
    GDT_ACCESS_RING3                        = 0x60,

    GDT_ACCESS_PRESENT                      = 0x80,

} GDT_ACCESS;

typedef enum 
{
    GDT_FLAG_64BIT                          = 0x20,
    GDT_FLAG_32BIT                          = 0x40,
    GDT_FLAG_16BIT                          = 0x00,

    GDT_FLAG_GRANULARITY_1B                 = 0x00,
    GDT_FLAG_GRANULARITY_4K                 = 0x80,
} GDT_FLAGS;

typedef struct
{
    uint16_t LimitLow;                  // limit (bits 0-15)
    uint16_t BaseLow;                   // base (bits 0-15)
    uint8_t BaseMiddle;                 // base (bits 16-23)
    uint8_t Access;                     // access
    uint8_t FlagsLimitHi;               // limit (bits 16-19) | flags
    uint8_t BaseHigh;                   // base (bits 24-31)
} __attribute__((packed)) GDTEntry;

typedef struct
{
    uint16_t Limit;                     // sizeof(gdt) - 1
    GDTEntry* Ptr;                      // address of GDT
} __attribute__((packed)) GDTDescriptor;

typedef struct tss_entry {
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
    uint32_t ds;        // Data segment (not used here)
    uint32_t fs;        // Additional segment (not used here)
    uint32_t gs;        // Additional segment (not used here)
    uint32_t ldt;       // Local descriptor table (not used here)
    uint16_t trap;      // Trap flag (not used here)
    uint16_t iobase;    // I/O base (not used here)
} __attribute__((packed)) tss_entry_t;

#define NUM_DESCRIPTORS 8

void flush_tss();
void GDT_Load();
void TSS_Load();
extern uint32_t usermodeFunc;
void Jump_usermode(uint32_t usermodeFunc, uint32_t userStack);
void retFromUser();
void GDT_SetEntry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void i686_GDT_Initialize();
void set_kernel_stack(uint32_t stack);