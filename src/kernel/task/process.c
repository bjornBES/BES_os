#include "process.h"
#include "memory.h"
#include "arch/i686/gdt.h"

process_t* Process[MAX_PROCESS];
int nextProcessIndex = 0;

process_t* makeProcess(uint32_t address, uint32_t userStack)
{
    if (Process[nextProcessIndex] == NULL)
    {
        Process[nextProcessIndex] = malloc(sizeof(process_t));
    }

    Process[nextProcessIndex]->processAddress = address;
    Process[nextProcessIndex]->code = (uint8_t*)address;
    Process[nextProcessIndex]->stack = userStack;
    return Process[nextProcessIndex];
}

void runProcess(process_t* process)
{
    Jump_usermode(process->processAddress, process->stack);
}

void killProcess(int id)
{
    process_t* currentProcess = Process[id];
    if (currentProcess == NULL)
    {

    }

    // free()

    free(currentProcess);
    Process[nextProcessIndex] = NULL;
    nextProcessIndex--;
}