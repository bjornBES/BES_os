#pragma once
#include <stddef.h>  // For size_t

// Declare malloc, free, calloc, and realloc functions
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t size);
void printStatus();

// void free(void* mem);
// void* malloc(uint32_t size);