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
	mov eax, (5 * 8) | 0 ; fifth 8-byte selector, symbolically OR-ed with 0 to set the RPL (requested privilege level).
	
    mov ax, (5 * 8) | 0 ; fifth 8-byte selector, symbolically OR-ed with 0 to set the RPL (requested privilege level).
    ltr ax
	
    ret

extern user_stack_top
%define USER_CS  (3 * 8) | 3
%define USER_DS  (4 * 8) | 3
%define USER_STACK_TOP user_stack_top  ; example top of user stack (adjust to your memory map)
extern usermodeFunc
;
; void Jump_usermode();
;
global Jump_usermode
Jump_usermode:
    mov ax, USER_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    
    mov eax, USER_STACK_TOP
    push USER_DS
    push eax
    pushf
    pop eax
    or eax, 0x200
    push eax
    push USER_CS
    push 0x10000
    
    iret
section .rodata
str_module:  db "GDT", 0
str_format:  db "usermodeFunc = 0x%p, USER_CS=0x%p", 0