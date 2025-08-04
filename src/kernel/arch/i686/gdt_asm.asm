[bits 32]

extern logf

; void __attribute__((cdecl)) i686_GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);
global i686_GDT_Load
i686_GDT_Load:
    
    ; make new call frame
    push ebp             ; save old call frame
    mov ebp, esp         ; initialize new call frame
    
    ; load gdt
    mov eax, [ebp + 8]
    lgdt [eax]

    ; reload code segment
    mov eax, [ebp + 12]
    push eax
    push .reload_cs
    retf

.reload_cs:

    ; reload data segments
    mov ax, [ebp + 16]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret

; C declaration: void flush_tss(void);
global flush_tss
flush_tss:
    mov ax, (5 * 8) | 0 ; fifth 8-byte selector, symbolically OR-ed with 0 to set the RPL (requested privilege level).
    ltr ax
	
    ret

extern user_stack_top
%define KERNEL_CS  (1 * 8)
%define KERNEL_DS  (2 * 8)
%define USER_CS  (3 * 8) | 3
%define USER_DS  (4 * 8) | 3
%define USER_STACK_TOP user_stack_top  ; example top of user stack (adjust to your memory map)
extern usermodeFunc
;
; void Jump_usermode(uint32_t usermodeFunc);
;
global Jump_usermode
Jump_usermode:
    mov [EBPTemp], ebp
    mov ebp, esp
    
    mov ax, USER_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, [esp]
    mov [retAddress], eax
    
    mov eax, USER_STACK_TOP
    push USER_DS
    push eax

    pushf
    pop eax
    or eax, 0x200
    push eax
    mov eax, [ebp + 8]  ; usermodeFunc

    push USER_CS
    push 0x10000
    
    iret
    
global retToKernel
retToKernel:
    mov     esp, [kernelStack]
    popa
    mov     esp, [kernelStack]
    mov     ebp, [EBPTemp]
    jmp     [retAddress]

global setSS
setSS:
    push ebp
    mov ebp, esp
    push eax
    mov eax, [ebp + 8]
    mov ss, eax
    pop eax
    pop ebp
    ret

section .data
EBPTemp:
    dd 0

global kernelStack
kernelStack:
    dd 0

global retAddress
retAddress:
    dd 0

tempEAX:
    dd 0