#pragma once

#include "defaultInclude.h"
#include "arch/i686/isr.h"
#include "allocator/memory_allocator.h"

#define MAX_PROCESS 1

typedef struct process
{
    uint32_t pid;
    Registers *regs;
    uint32_t processAddress;
    Page* memoryPage;
    uint8_t* code;
    uint8_t* data;
    uint8_t* roData;
} process_t;

extern process_t* Process[MAX_PROCESS];

void makeProcess(uint32_t address);
void killProcess(int id);
