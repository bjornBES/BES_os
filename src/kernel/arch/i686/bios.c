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

void (*rebased_bios32_helper)() = (void*)0x7C00;

void BIOS_Init()
{
    // Initialize the GDT
    i686_GDT_Initialize();
    i686_IDT_Initialize();

    // GDT_SetEntry(6, 0, 0xFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE), (GDT_FLAG_16BIT | GDT_FLAG_GRANULARITY_1B)); // Kernel pmode 16 bit code segment
    // GDT_SetEntry(7, 0, 0xFFFF, (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE), (GDT_FLAG_16BIT | GDT_FLAG_GRANULARITY_1B)); // Kernel pmode 16 bit data segment

    // Load the GDT
    GDT_Load();
    // Load the IDT
    IDT_Load();
}

uint16_t ds()
{
	uint16_t seg;
	__asm__("movw %%ds,%0" : "=rm" (seg));
	return seg;
}

uint16_t fs()
{
	uint16_t seg;
	__asm__ volatile("movw %%fs,%0" : "=rm" (seg));
	return seg;
}

uint16_t gs()
{
	uint16_t seg;
	__asm__ volatile("movw %%gs,%0" : "=rm" (seg));
	return seg;
}

void initregs(Registers16 *reg)
{
	memset(reg, 0, sizeof(*reg));
	reg->eflags |= 1;
	reg->ds = ds();
	reg->es = ds();
	reg->fs = fs();
	reg->gs = gs();
}

void print_reg(Registers * reg) {
    fprintf(3, "Registers dump:\n");
    fprintf(3, "eax 0x%x ebx 0x%x 0x%ecx 0x%x %edx 0x%x\n", reg->eax, reg->ebx, reg->ecx, reg->edx);
    fprintf(3, "edi 0x%x esi 0x%x %ebp 0x%x %esp 0x%x\n", reg->edi, reg->esi, reg->ebp, reg->esp);
    fprintf(3, "eip 0x%x cs 0x%x ss 0x%x eflags 0x%x\n", reg->eip, reg->ss, reg->eflags);
}
void print_reg16(Registers16 * reg) {
    fprintf(3, "Registers dump:\n");
    fprintf(3, "ax 0x%x bx 0x%x cx 0x%x dx 0x%x\n", reg->ax, reg->bx, reg->cx, reg->dx);
    fprintf(3, "di 0x%x si 0x%x bp 0x%x sp 0x%x\n", reg->di, reg->si, reg->bp, reg->sp);
    fprintf(3, "ds 0x%x es 0x%x fs 0x%x gs 0x%x eflags 0x%x\n", reg->ds, reg->es, reg->fs, reg->gs, reg->flags);
}