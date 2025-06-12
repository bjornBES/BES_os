; void testcall(Registers* regs)
; global testcall
extern logf

section .text

; testcall:
;     push ebp
;     mov ebp, esp
; 
;     ; Get the pointer to Registers from stack (first argument)
;     mov eax, [ebp + 8]        ; eax = regs
; 
;     ; Push log arguments in reverse order (cdecl)
;     push dword [eax + 20]       ; ebx
;     push dword [eax + 32]       ; eax
;     push str_format             ; "Test syscall invoked with eax=%u, ebx=%u"
;     push 0                      ; LVL_DEBUG
;     push str_module             ; "Syscall"
;     call logf
;     add esp, 20               ; Clean up 5 arguments (5 x 4 bytes)
; 
;     pop ebp
;     ret
; 
; ; Static strings
; section .rodata
; str_module:  db "testcall", 0
; str_format:  db "[Syscall] Test syscall invoked with eax=%u, ebx=%u", 0
