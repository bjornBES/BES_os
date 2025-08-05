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
    uint32_t stack;
    uint8_t* code;
    uint8_t* data;
    uint8_t* roData;
} process_t;

extern process_t* Process[MAX_PROCESS];

process_t* makeProcess(uint32_t address, uint32_t userStack);
void runProcess(process_t* process);
void killProcess(int id);
