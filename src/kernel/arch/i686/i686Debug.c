#include "i686Debug.h"
#include <debug.h>

uint32_t GetRegister(enum RegisterType reg)
{
    uint32_t value = 0;
    switch (reg)
    {
        case REG_EAX: __asm__ __volatile__("mov %%eax, %0" : "=r"(value)); break;
        case REG_EBX: __asm__ __volatile__("mov %%ebx, %0" : "=r"(value)); break;
        case REG_ECX: __asm__ __volatile__("mov %%ecx, %0" : "=r"(value)); break;
        case REG_EDX: __asm__ __volatile__("mov %%edx, %0" : "=r"(value)); break;
        case REG_ESI: __asm__ __volatile__("mov %%esi, %0" : "=r"(value)); break;
        case REG_EDI: __asm__ __volatile__("mov %%edi, %0" : "=r"(value)); break;
        case REG_EBP: __asm__ __volatile__("mov %%ebp, %0" : "=r"(value)); break;
        case REG_ESP: __asm__ __volatile__("mov %%esp, %0" : "=r"(value)); break;
        case REG_EIP: __asm__ __volatile__("call 1f; 1: pop %0" : "=r"(value)); break;
        case REG_EFLAGS: __asm__ __volatile__("pushf; pop %0" : "=r"(value)); break;
    }
    return value;
}
void SetRegister(enum RegisterType reg, uint32_t value)
{
    switch (reg)
    {
        case REG_EAX: __asm__ __volatile__("mov %0, %%eax" : : "r"(value)); break;
        case REG_EBX: __asm__ __volatile__("mov %0, %%ebx" : : "r"(value)); break;
        case REG_ECX: __asm__ __volatile__("mov %0, %%ecx" : : "r"(value)); break;
        case REG_EDX: __asm__ __volatile__("mov %0, %%edx" : : "r"(value)); break;
        case REG_ESI: __asm__ __volatile__("mov %0, %%esi" : : "r"(value)); break;
        case REG_EDI: __asm__ __volatile__("mov %0, %%edi" : : "r"(value)); break;
        case REG_EBP: __asm__ __volatile__("mov %0, %%ebp" : : "r"(value)); break;
        case REG_ESP: __asm__ __volatile__("mov %0, %%esp" : : "r"(value)); break;
        case REG_EIP: __asm__ __volatile__("jmp *%0" : : "r"(value)); break;
        case REG_EFLAGS: __asm__ __volatile__("push %0; popf" : : "r"(value)); break;
    }
}
void PrintRegisters()
{
    log_debug("DEBUG", "Registers:");
    log_debug("DEBUG", "EAX: 0x%08x", GetRegister(REG_EAX));
    log_debug("DEBUG", "EBX: 0x%08x", GetRegister(REG_EBX));
    log_debug("DEBUG", "ECX: 0x%08x", GetRegister(REG_ECX));
    log_debug("DEBUG", "EDX: 0x%08x", GetRegister(REG_EDX));
    log_debug("DEBUG", "ESI: 0x%08x", GetRegister(REG_ESI));
    log_debug("DEBUG", "EDI: 0x%08x", GetRegister(REG_EDI));
    log_debug("DEBUG", "EBP: 0x%08x", GetRegister(REG_EBP));
    log_debug("DEBUG", "ESP: 0x%08x", GetRegister(REG_ESP));
    log_debug("DEBUG", "EIP: 0x%08x", GetRegister(REG_EIP));
    log_debug("DEBUG", "EFLAGS: 0x%08x", GetRegister(REG_EFLAGS));
}