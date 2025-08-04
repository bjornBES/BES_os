#include "process.h"
#include "malloc.h"

process_t* Process[MAX_PROCESS];
int nextProcessIndex = 0;

void makeProcess(uint32_t address)
{
    if (Process[nextProcessIndex] == NULL)
    {
        Process[nextProcessIndex] = mallocToPage(sizeof(process_t), 0);
    }

    Process[nextProcessIndex]->processAddress = address;
    Process[nextProcessIndex]->code = (uint8_t*)address;

    Process[nextProcessIndex]->memoryPage = (Page*)allocate_page();
}

void killProcess(int id)
{
    process_t* currentProcess = Process[id];
    if (currentProcess == NULL)
    {

    }

    // free()

    freeToPage(currentProcess, 0);
    Process[nextProcessIndex] = NULL;
    nextProcessIndex--;
}