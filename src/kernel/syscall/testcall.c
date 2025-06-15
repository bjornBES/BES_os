#include "testcall.h"

#include "debug.h"
#include "arch/i686/gdt.h"

void testcall(Registers* regs)
{
        log_crit("testcall", "Systemcall: %d", regs->interrupt);
        log_crit("testcall", "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x", regs->U32.eax, regs->U32.ebx, regs->U32.ecx, regs->U32.edx, regs->U32.esi, regs->U32.edi);
        log_crit("testcall", "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x", regs->esp, regs->U32.ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        log_crit("testcall", "  interrupt=%x  errorcode=%x", regs->interrupt, regs->error);

    log_debug("testcall", "Going into a loop in the kernel");
    __asm__ ("jmp [retFromUser]");
}