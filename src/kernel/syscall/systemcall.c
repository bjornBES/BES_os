#include "systemcall.h"
#include "debug.h"
#include "arch/i686/io.h"
#include "arch/i686/idt.h"
#include "arch/i686/gdt.h"
#include "arch/i686/bios.h"
#include "memory.h"

#include "testcall.h"

SystemCall syscalls[MAX_SYSCALLS] = {0};
uint32_t syscallCnt = 0;

void syscallHandler(Registers* regs)
{
    uint32_t syscallId = regs->U16.ax; // Extract syscall ID from eax
    log_debug("Syscall", "Syscall ID: %u, EAX: 0x%08X", syscallId, regs->U32.eax);
    printf("Debug: Syscall ID: %u, EAX: 0x%08X\n", syscallId, regs->U32.eax);
    if (syscalls[syscallId] == NULL)
    {
        log_err("Syscall", "syscallHandler: Invalid syscall ID: %u", syscallId);
        printf("ERROR: syscallHandler: Invalid syscall ID: %u\n", syscallId);
        i686_Panic();
        return; // Invalid syscall ID
    }

    log_info("Syscall", "Invoking syscall ID: %u", syscallId);
    printf("INFO: Invoking syscall ID: %u\n", syscallId);
    syscalls[syscallId](regs);
    log_info("Syscall", "Syscall ID: %u completed", syscallId);
    printf("INFO: Syscall ID: %u completed\n", syscallId);
    return;
}

void registerSyscall(uint32_t syscallId, SystemCall syscallFunc)
{
    if (syscallId > MAX_SYSCALLS)
    {
        log_err("Syscall", "registerSyscall: Invalid syscall ID: %u", syscallId);
        i686_Panic();
        return; // Invalid syscall ID
    }

    syscalls[syscallId] = syscallFunc;
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
    // i686_IDT_SetGate(0x80, syscallHandler, i686_GDT_CODE_SEGMENT, IDT_FLAG_RING3 | IDT_FLAG_GATE_32BIT_INT);
    IDT_Load();
    i686_ISR_RegisterHandler(0x80, syscallHandler);
    

    registerSyscall(SYSCALL_EXIT, testcall); // Register test syscall
}