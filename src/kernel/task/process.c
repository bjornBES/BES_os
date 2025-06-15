#include "process.h"
#include "malloc.h"

process_t* Process;

void makeProcess(uint32_t address)
{
    if (Process == NULL)
    {
        Process = mallocToPage(sizeof(process_t), 0);
    }

    Process->processAddress = address;
}