#include "systemcall.h"
#include "debug.h"
#include "arch/i686/io.h"
#include "arch/i686/idt.h"
#include "arch/i686/gdt.h"
#include "arch/i686/bios.h"
#include "memory.h"

SystemCall syscalls[MAX_SYSCALLS] = {0};
uint32_t syscallCnt = 0;

void syscallHandler(Registers* regs)
{
    uint32_t syscallId = regs->U16.ax; // Extract syscall ID from eax
    log_debug("Syscall", "Syscall ID: %u, EAX: 0x%08X", syscallId, regs->eax);
    if (syscalls[syscallId] == NULL)
    {
        log_err("Syscall", "Invalid syscall ID: %u", syscallId);
        i686_Panic();
        return; // Invalid syscall ID
    }

    log_info("Syscall", "Invoking syscall ID: %u", syscallId);
    syscalls[syscallId](regs);
    log_info("Syscall", "Syscall ID: %u completed", syscallId);
    return;
}

void registerSyscall(uint32_t syscallId, SystemCall syscallHandler)
{
    if (syscallId > MAX_SYSCALLS)
    {
        log_err("Syscall", "Invalid syscall ID: %u", syscallId);
        i686_Panic();
        return; // Invalid syscall ID
    }

    syscalls[syscallId] = syscallHandler;
    syscallCnt++;
}

void initregs(IntRegisters *reg)
{
	memset(reg, 0, sizeof(*reg));
	reg->eflags |= 1;
	reg->ds = ds();
	reg->es = ds();
	reg->fs = fs();
	reg->gs = gs();
}

void initSystemCall()
{
    i686_ISR_RegisterHandler(0x80, syscallHandler);

    registerSyscall(8, testcall); // Register test syscall
}