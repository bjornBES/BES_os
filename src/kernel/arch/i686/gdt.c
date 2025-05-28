#include "gdt.h"
#include <stdint.h>

// Helper macros
#define GDT_LIMIT_LOW(limit)                (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                  (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)               ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)    (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                 ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) {                     \
    GDT_LIMIT_LOW(limit),                                           \
    GDT_BASE_LOW(base),                                             \
    GDT_BASE_MIDDLE(base),                                          \
    access,                                                         \
    GDT_FLAGS_LIMIT_HI(limit, flags),                               \
    GDT_BASE_HIGH(base)                                             \
}

void __attribute__((cdecl)) i686_GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);

GDTEntry g_GDT[NUM_DESCRIPTORS];
GDTDescriptor g_GDTDescriptor = { sizeof(g_GDT) - 1, g_GDT};

void GDT_SetEntry(uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    g_GDT[index] = (GDTEntry)GDT_ENTRY(base, limit, access, flags);
}

void GDT_Load()
{
    i686_GDT_Load(&g_GDTDescriptor, i686_GDT_CODE_SEGMENT, i686_GDT_DATA_SEGMENT);
}

void i686_GDT_Initialize()
{
    GDT_SetEntry(0, 0, 0, 0, 0); // NULL descriptor
    GDT_SetEntry(1, 0, 0xFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)); // Kernel pmode 32 bit code segment
    GDT_SetEntry(2, 0, 0xFFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)); // Kernel pmode 32 bit data segment
    GDT_SetEntry(3, 0, 0,       (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_32BIT));
    GDT_SetEntry(4, 0, 0,       (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_32BIT));
}