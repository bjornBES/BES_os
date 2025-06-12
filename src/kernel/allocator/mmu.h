#pragma once

#include "defaultInclude.h"

#define ASMCALL __attribute__((cdecl))

uint32_t ASMCALL CR3_Get();
void ASMCALL CR3_Set(uint32_t value);
void ASMCALL CR3_SetBit(uint32_t index);
void ASMCALL CR3_ClearBit(uint32_t index);
bool ASMCALL CR3_GetBit(uint32_t index);