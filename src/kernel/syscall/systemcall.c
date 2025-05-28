#include "systemcall.h"
#include "arch/i686/idt.h"
#include "arch/i686/isr.h"
#include "arch/i686/gdt.h"

void syscallHandler(Registers* regs)
{

}

void initSystemCall()
{
    i686_ISR_RegisterHandler(0x80, syscallHandler);
}