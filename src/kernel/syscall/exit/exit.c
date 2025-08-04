#include "exit.h"

#include "debug.h"
#include "task/process.h"
#include "arch/i686/io.h"
#include "arch/i686/gdt.h"
#include "arch/i686/idt.h"

extern uint32_t kernelStack;
extern uint32_t retAddress;

void retToKernel();

/*
// called from ISR
void retToKernel() {
    __asm__ volatile (
        "mov %[esp], %%esp\n\t"
        "popa\n\t"
        "jmp *%[ret]\n\t"
        : : [esp]"r"(kernelStack), [ret]"r"(retAddress));
    }
    */


void systemExit(Registers* regs)
{
    log_debug("exit system", "kernelStack = 0x%X", kernelStack);

    ASM_INT2();
    log_debug("exit syscall", "Systemcall: %d", regs->interrupt);
    log_debug("exit syscall", "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
    log_debug("exit syscall", "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
    log_debug("exit syscall", "  interrupt=%x  errorcode=%x", regs->interrupt, regs->error);

    killProcess(0);

    i686_ISR_Initialize();
    i686_IDT_EnableGate(0x80);
    IDT_Load();
    retToKernel();
}
