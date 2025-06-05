
[bits 32]
section .text

;
; void CallInt(int interrupt_number, IntRegisters *regs)
; 
global CallInt
CallInt:
    push ebp              ; Standard function prologue
    mov ebp, esp          ; Set up stack frame
    ; Save registers
    pusha
   
    ; get arguments
    mov esi, [ebp + 12]  ; esi = IntRegisters pointer
    
    ; Load the input registers
    mov eax, [esi + 0]   ; eax = regs->eax
    mov ebx, [esi + 4]   ; ebx = regs->ebx
    mov ecx, [esi + 8]   ; ecx = regs->ecx
    mov edx, [esi + 12]  ; edx = regs->edx
    mov edi, [esi + 20]  ; edi = regs->edi
    mov esi, [esi + 16]  ; esi = regs->esi

    mov ax, word [ebp + 8]  ; Get the interrupt number (2 byte)

    int 0x80  ; Trigger the interrupt

    mov edi, [ebp + 12]
    mov [edi + 0], eax  ; Store the result in regs->eax
    mov [edi + 4], ebx  ; Store the result in regs->ebx
    mov [edi + 8], ecx  ; Store the result in regs->ecx
    mov [edi + 12], edx ; Store the result in regs->edx
    mov [edi + 16], esi ; Store the result in regs->esi
    mov [edi + 20], edi ; Store the result in regs->edi
    
    ; Restore registers
    popa
    pop ebp              ; Restore base pointer

    ret