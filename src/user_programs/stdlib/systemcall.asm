[bits 32]
section .text

;
; void SystemCall(int interruptIndex, int ebx, int ecx, int edx, int esi, int edi)
;
global SystemCall
SystemCall:
    push    ebp
    mov     ebp,            esp
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    
    mov     eax,            [ebp + 8]               ; interrupt number (eax)
    mov     ebx,            [ebp + 12]
    mov     ecx,            [ebp + 16]
    mov     edx,            [ebp + 20]
    mov     esi,            [ebp + 24]
    mov     edi,            [ebp + 28]
    int     0x80

    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     ebp
    ret
    
;
; void SYS_Exit(int status)
;
global SYS_Exit
SYS_Exit:
    push    ebp
    mov     ebp,            esp
    
    mov     ebx,            [ebp + 8]
    int     0x80

    pop     ebp
    ret

;
; void SYS_Write(char* buffer, int count)
;
global SYS_Write
SYS_Write:
    push    ebp
    mov     ebp,            esp

    push    esi
    push    ecx
    push    ebx

    mov     esi,            [ebp + 12]
    mov     ecx,            [ebp + 8]
    mov     ebx,            1
    mov     eax,            1
    int     0x80

    pop     ebx
    pop     ecx
    pop     esi

    pop     ebp
    ret