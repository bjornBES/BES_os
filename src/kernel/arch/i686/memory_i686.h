#pragma once
#include "defaultInclude.h"

#define ASMCALL __attribute__((cdecl))

typedef struct {
	uint8_t status;
	uint32_t size;
} alloc_t;

ASMCALL void* memcpy(void* dst, const void* src, size_t num);
ASMCALL void* memset(void* ptr, int value, size_t num);
ASMCALL uint16_t* memset16(uint16_t* ptr, uint16_t value, size_t num);
ASMCALL uint32_t* memset32(uint32_t* ptr, uint32_t value, size_t num);
ASMCALL void* memmove(void* dst, const void* src, size_t num);
ASMCALL int memcmp(const void* ptr1, const void* ptr2, size_t num);
ASMCALL void* memchr(const void* ptr, int value, size_t num);

void pagingInit();
void pagingMapVirtToPhys(uint32_t virt, uint32_t phys);
void* getPhysAddress(void* virt);