#include "gdt.h"
#include <stdint.h>
#include "memory.h"
#include "debug.h"

// Helper macros
#define GDT_LIMIT_LOW(limit) (limit & 0xFFFF)
#define GDT_BASE_LOW(base) (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base) ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags) (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base) ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MIDDLE(base),                      \
    access,                                     \
    GDT_FLAGS_LIMIT_HI(limit, flags),           \
    GDT_BASE_HIGH(base)}

void __attribute__((cdecl)) i686_GDT_Load(GDTDescriptor *descriptor, uint16_t codeSegment, uint16_t dataSegment);

uint32_t usermodeFunc = 0x00010000;
tss_entry_t tss_entry;
GDTEntry g_GDT[NUM_DESCRIPTORS];
GDTDescriptor g_GDTDescriptor = {sizeof(g_GDT) - 1, g_GDT};

void GDT_SetEntry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    log_debug("GDT", "Setting GDT entry %d: base=0x%08X, limit=0x%08X, access: 0x%02X, flags: %X", index, base, limit, access, flags);
    g_GDT[index] = (GDTEntry)GDT_ENTRY(base, limit, access, flags);
    log_debug("GDT", "Entry %d set.", index);
}

void GDT_Load()
{
    i686_GDT_Load(&g_GDTDescriptor, i686_GDT_CODE_SEGMENT, i686_GDT_DATA_SEGMENT);
}

void TSS_Load()
{
    log_debug("GDT", "Flushing TSS...");
    flush_tss();
    log_debug("GDT", "TSS flushed.");
}

void write_tss()
{
    // Ensure the TSS is initially zero'd.
    memset(&tss_entry, 0, sizeof(tss_entry_t));
    
    uint32_t stack_ptr = 0;
    __asm__("mov %%esp, %0" : "=r"(stack_ptr));
    tss_entry.ss0 = i686_GDT_DATA_SEGMENT; // Set the kernel stack segment.
    tss_entry.esp0 = stack_ptr;            // Set the kernel stack pointer.

    GDT_SetEntry(5, (uint32_t)&tss_entry, sizeof(tss_entry_t) - 1, 0x89, 0x00);

    log_debug("GDT", "Writing TSS: base=0x%08X, esp0=0x%08X, ss0=0x%04X", (uint32_t)&tss_entry, tss_entry.esp0, tss_entry.ss0);
}

void set_kernel_stack(uint32_t stack)
{ // Used when an interrupt occurs
    uint32_t stack_ptr = 0;
    __asm__("mov %%esp, %0" : "=r"(stack_ptr));
    tss_entry.esp0 = stack_ptr;
}

void i686_GDT_Initialize()
{
    GDT_SetEntry(0, 0, 0, 0, 0);                                                                                                                                            // NULL descriptor
    GDT_SetEntry(1, 0, 0xFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));  // Kernel pmode 32 bit code segment
    GDT_SetEntry(2, 0, 0xFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)); // Kernel pmode 32 bit data segment
    GDT_SetEntry(3, 0, 0xFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));
    GDT_SetEntry(4, 0, 0xFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K));

    write_tss();
}