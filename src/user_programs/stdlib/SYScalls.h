#pragma once

void __attribute__((cdecl)) SystemCall(int interruptIndex, int ebx, int ecx, int edx, int esi, int edi);
void __attribute__((cdecl)) SYS_Exit(int status);
void __attribute__((cdecl)) SYS_Write(char* buffer, int count);