section .text

[bits 32]
;
; uint32_t ASMCALL CR3_Get()
;
global CR3_Get
CR3_Get:
    mov eax, cr3
    ret

;
; void ASMCALL CR3_Set(uint32_t value)
;
global CR3_Set
CR3_Set:
    push eax
    mov eax, [esp + 8]
    mov cr3, eax
    pop eax
    ret

;
; void ASMCALL CR3_SetBit(uint32_t index)
;
global CR3_SetBit
CR3_SetBit:
    push eax
    push ebx
    mov ebx, 1
    shl ebx, [esp + 12]
    mov eax, cr3
    or eax, ebx
    mov cr3, eax
    pop ebx
    pop eax
    ret

;
; void ASMCALL CR3_ClearBit(uint32_t index)
;
global CR3_ClearBit
CR3_ClearBit:
    push eax
    push ebx
    mov ebx, 1
    shl ebx, [esp + 12]
    not ebx
    mov eax, cr3
    and eax, ebx
    mov cr3, eax
    pop ebx
    pop eax
    ret

;
; bool ASMCALL CR3_GetBit(uint32_t index)
;
global CR3_GetBit
CR3_GetBit:
    push ebx
    mov ebx, 1
    shl ebx, [esp + 8]
    mov eax, cr3
    and eax, ebx
    shr eax, [esp + 8]
    pop ebx
    ret
