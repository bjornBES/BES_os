
#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "stdio.h"

#define _4KB 1024 * 4
#define MaxAllocs 16

typedef struct
{
    uint8_t status;
    uint32_t size;
} alloc_t;

extern uint8_t *__end;

bool initMalloc;
uint32_t heap_begin;
uint32_t memory_used = 0;
uint32_t heap_end;
uint32_t last_alloc;

void mm_init()
{
    heap_begin = __end;
    last_alloc = heap_begin;
    heap_end = heap_begin + (MaxAllocs * _4KB);
    initMalloc = true;
}

void free(void* mem)
{
    alloc_t *alloc = (mem - sizeof(alloc_t));
    memory_used -= alloc->size + sizeof(alloc_t);
    alloc->status = 0;
}

void *malloc(uint32_t size)
{
    if (!size)
    {
        return 0;
    }

    uint8_t *mem = (uint8_t *)heap_begin;
    while ((uint32_t)mem < last_alloc)
    {
        alloc_t *a = (alloc_t *)mem;

        if (!a->size)
        {
            break;
        }

        if (a->status)
        {
            mem += a->size;
            mem += sizeof(alloc_t);
            mem += 4;
            continue;
        }

        if (a->size >= size)
        {
            a->status = 1;
            memset(mem + sizeof(alloc_t), 0, size);
            memory_used += size + sizeof(alloc_t);
            return (void *)(mem + sizeof(alloc_t));
        }

        mem += a->size;
        mem += sizeof(alloc_t);
        mem += 4;
    }

    if (last_alloc+size+sizeof(alloc_t) >= heap_end)
    {
        printf("Cannot allocate %d bytes! Out of memory.\r\n", size);
        panic();
    }

	alloc_t *alloc = (alloc_t *)last_alloc;
	alloc->status = 1;
	alloc->size = size;

	last_alloc += size;
	last_alloc += sizeof(alloc_t);
	last_alloc += 4;
	printf("Allocated %d bytes from 0x%x to 0x%x\n", size, (uint32_t)alloc + sizeof(alloc_t), last_alloc);
	memory_used += size + 4 + sizeof(alloc_t);
	memset((char *)((uint32_t)alloc + sizeof(alloc_t)), 0, size);
	return (char *)((uint32_t)alloc + sizeof(alloc_t));
}