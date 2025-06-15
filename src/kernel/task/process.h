#pragma once

#include "defaultInclude.h"
#include "arch/i686/isr.h"

typedef struct process
{
    Registers *regs;
    uint32_t processAddress;
} process_t;

extern process_t* Process;

void makeProcess(uint32_t address);
