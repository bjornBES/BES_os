#include "stdlib.h"
#include "SYScalls.h"

void *malloc(size_t size)
{
    return SYS_Map(size);
}

void free(void* p)
{
    SYS_UnMap(p);
}

void exit(int status)
{
    SYS_Exit(status);
}