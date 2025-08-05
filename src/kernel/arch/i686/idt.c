#include "idt.h"
#include "debug.h"
#include <stdint.h>
#include <util/binary.h>

void __attribute__((cdecl)) i686_IDT_Load(IDTDescriptor* idtDescriptor);

IDTEntry g_IDT[256];
IDTDescriptor g_IDTDescriptor = { sizeof(g_IDT) - 1, g_IDT };

void i686_IDT_SetGate(int interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags)
{
    g_IDT[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    g_IDT[interrupt].SegmentSelector = segmentDescriptor;
    g_IDT[interrupt].Reserved = 0;
    g_IDT[interrupt].Flags = flags;
    g_IDT[interrupt].BaseHigh = ((uint32_t)base >> 16) & 0xFFFF;
}

void i686_IDT_EnableGate(int interrupt)
{
    FLAG_SET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_DisableGate(int interrupt)
{
    FLAG_UNSET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void IDT_Load()
{
    i686_IDT_Load(&g_IDTDescriptor);
}

void i686_IDT_Initialize()
{
}

void IDT_Dump()
{
    for (int i = 0; i < 256; i++)
    {
        uint32_t base = ((uint32_t)g_IDT[i].BaseHigh << 16) | g_IDT[i].BaseLow;
        uint16_t sel  = g_IDT[i].SegmentSelector;
        uint8_t  flg  = g_IDT[i].Flags;

        // Only show entries that are present
        if (flg & 0x80)
        {
            log_info("IDT", "Vec %3d: base=0x%08X sel=0x%04X flags=0x%02X",
                     i, base, sel, flg);
        }
        else
        {
            log_info("IDT", "Vec %3d: NOT PRESENT", i);
        }
    }
}