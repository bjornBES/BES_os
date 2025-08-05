
;;
;; void  ASMCALL i686_outb(uint16_t port, uint8_t value);
;;
;global i686_outb
;i686_outb:
;    [bits 32]
;    push ebp
;    mov ebp, esp
;    push edx
;    push eax
;    xor edx, edx
;    mov dx, [ebp + 8]
;    xor eax, eax
;    mov al, [ebp + 12]
;    out dx, al
;    pop eax
;    pop edx
;    pop ebp
;    ret
;
;;
;; uint8_t ASMCALL i686_inb(uint16_t port);
;;
;global i686_inb
;i686_inb:
;    [bits 32]
;    push ebp
;    mov ebp, esp
;    push edx
;    xor edx, edx
;    mov dx, [ebp + 8]
;    xor eax, eax
;    in al, dx
;    pop edx
;    pop ebp
;    ret
;
;;
;; void ASMCALL i686_outw(uint16_t port, uint16_t value);
;;
;global i686_outw
;i686_outw:
;    [bits 32]
;    push ebp
;    mov ebp, esp
;    push edx
;    push eax
;    xor edx, edx
;    mov dx, [ebp + 8]
;    xor eax, eax
;    mov ax, [ebp + 12]
;    out dx, ax
;    pop eax
;    pop edx
;    pop ebp
;    ret
;
;;
;; uint16_t ASMCALL i686_inw(uint16_t port);
;;
;global i686_inw
;i686_inw:
;    [bits 32]
;    push ebp
;    mov ebp, esp
;    push edx
;    xor edx, edx
;    mov dx, [ebp + 8]
;    xor eax, eax
;    in ax, dx
;    pop edx
;    pop ebp
;    ret
;
;;
;; void ASMCALL i686_outd(uint16_t port, uint32_t value);
;;
;global i686_outd
;i686_outd:
;    [bits 32]
;    push ebp
;    mov ebp, esp
;    push edx
;    push eax
;    xor edx, edx
;    mov dx, [ebp + 8]
;    mov eax, [ebp + 12]
;    out dx, eax
;    pop eax
;    pop edx
;    pop ebp
;    ret
;
;;
;; uint32_t ASMCALL i686_ind(uint16_t port);
;;
;global i686_ind
;i686_ind:
;    [bits 32]
;    push ebp
;    mov ebp, esp
;    push edx
;    xor edx, edx
;    mov dx, [ebp + 8]
;    in eax, dx
;    pop edx
;    pop ebp
;    ret
;
;
; void ASMCALL i686_SetStack(uint32_t address);
;
global i686_SetStack
i686_SetStack:
    [bits 32]
    mov ebp, esp
    mov eax, [ebp + 8]
    mov esp, eax
    ret

global i686_Panic
i686_Panic:
    cli
    hlt

global i686_EnableInterrupts
i686_EnableInterrupts:
    sti
    ret

global i686_DisableInterrupts
i686_DisableInterrupts:
    cli
    ret
    
global i686_HLT
i686_HLT:
    hlt
    ret

global i686_int2
i686_int2:
    int 0x2
    ret

global i686_EnableMCE
i686_EnableMCE:
    push    eax
    mov     eax,    cr4    
    or      eax,    0x40
    mov     cr4,    eax
    pop     eax
    ret
    
global crash_me
crash_me:
    ; div by 0
    mov ecx, 0x1337
    mov eax, 0
    div eax
    int 0x80
    ret