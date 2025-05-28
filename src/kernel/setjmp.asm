
section .text


; 
; int setjmp(jmp_buf env)
;
setjmp:
    push    ebp
    mov     esp,        ebp

    xor     eax,        eax             ; eax = 0
    mov     [eax],      ebp
    mov     [eax + 4],  ebx
    mov     [eax + 8],  edi
    mov     [eax + 12], esi

    mov     ebx,        ebp
    add     ebx,        0x4
    mov     ecx,        [ebx]
    mov     [eax + 16], ecx

    pop     ebp
    ret