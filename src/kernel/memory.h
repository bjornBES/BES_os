#pragma once
#include "stdint.h"

#ifdef __i686__
#include "arch/i686/memory_i686.h"
#endif

void mmInit(uint32_t kernel_end);
void mmPrintStatus();

void* pmalloc(size_t size);
void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);
