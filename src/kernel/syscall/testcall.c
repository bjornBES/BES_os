#include "testcall.h"

#include "debug.h"

void testcall(Registers* regs)
{
    log_debug("testcall", "Test syscall invoked with eax=%u, ebx=%u", regs->U32.eax, regs->U32.ebx);

    log_debug("testcall", "Going into a loop in the kernel");
    for (;;)
    {
        ;
    }
}