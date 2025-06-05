#pragma once

#include "defaultInclude.h"
#include "arch/i686/isr.h"
#include "callInt.h"

#define MAX_SYSCALLS UINT16_MAX

__attribute__((cdecl)) void testcall(Registers* regs);

typedef void (*SystemCall)(Registers* regs);

#define SYSCALL_READ 1

void initregs(IntRegisters *reg);
void registerSyscall(uint32_t syscallId, SystemCall syscallHandler);
void initSystemCall();