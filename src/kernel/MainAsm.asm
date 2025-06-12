
[bits 32]

extern KernelStart
extern HAL_Initialize
extern stack_top

section .text

global start
start:
    
    mov ax, 0x10
    mov ss, ax
    
    mov eax, dword [esp + 4]
    mov esp, stack_top
    push eax
    
    call HAL_Initialize
    
    mov eax, cr4
    or eax, 0x40
    mov cr4, eax
    pop eax
    
    push eax
    call KernelStart

    hlt
    jmp $

times (0x1000 - ($ - start)) db 0