#pragma once
#include "defaultInclude.h"

#include "memory_allocator.h"

// Declare malloc, free, calloc, and realloc functions
void *malloc(size_t size, Page* page);
void free(void *ptr, Page* page);

void *calloc(size_t num, size_t size, Page* page);
void *realloc(void *ptr, size_t size, Page* page);

void *mallocToPage0(size_t size);

void printStatus();

// void free(void* mem);
// void* malloc(uint32_t size);