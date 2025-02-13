#pragma once
#include "stdint.h"

void* memcpy(void* dst, const void* src, uint16_t num);
void* memset(void* ptr, int value, uint32_t num);
int memcmp(const void* ptr1, const void* ptr2, uint16_t num);
void *memmove(void *destination, const void *source, uint32_t num);
