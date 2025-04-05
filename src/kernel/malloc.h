#pragma once
#include <stddef.h>  // For size_t

#include "memory_allocator.h"

// Declare malloc, free, calloc, and realloc functions
void *malloc(size_t size, Page* page);
void free(void *ptr, Page* page);

void *calloc(size_t num, size_t size, Page* page);
void *realloc(void *ptr, size_t size, Page* page);

void printStatus();

// void free(void* mem);
// void* malloc(uint32_t size);