#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <drivers/VGA/vga.h>
#include <arch/i686/bios.h>

void HAL_Initialize()
{
    BIOS_Init();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
}
