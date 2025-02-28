#include "kheap.h"
#include "malloc.h"
#include "memory.h"

/*
   malloc a s block and memset
   */
void *kcalloc(uint32_t num, uint32_t size)
{
    void *ptr = malloc(num * size);
    memset(ptr, 0, num * size);
    return ptr;
}