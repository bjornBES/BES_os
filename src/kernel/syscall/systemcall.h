#pragma once

#include "defaultInclude.h"
#include "arch/i686/isr.h"
#include "callInt.h"

#define MAX_SYSCALLS UINT16_MAX

typedef void (*SystemCall)(Registers* regs);

#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_EXIT 2

void initregs(IntRegisters *reg);
void registerSyscall(uint32_t syscallId, SystemCall syscallHandler);
void initSystemCall();