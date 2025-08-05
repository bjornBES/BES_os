#pragma once

#include <defaultInclude.h>

enum RegisterType {
    REG_EAX,
    REG_EBX,
    REG_ECX,
    REG_EDX,
    REG_ESI,
    REG_EDI,
    REG_EBP,
    REG_ESP,
    REG_EIP,
    REG_EFLAGS
};

uint32_t GetRegister(enum RegisterType reg);
void SetRegister(enum RegisterType reg, uint32_t value);
void PrintRegisters();