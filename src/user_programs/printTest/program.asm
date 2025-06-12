[bits 32]

section .text

global start
start:
;    int     2
    call print_msg
    mov     eax,        2
    int     0x80

;----------------------------------------
; print_msg: prints msg to stdout (fd=1)
; Returns:
;   al = 0 on success, >=1 on error
;----------------------------------------
print_msg:
    mov     eax, 1          ; syscall number (sys_write)
    mov     ebx, 1          ; file descriptor (stdout)
    mov     esi, msg        ; pointer to string
    mov     ecx, msg_len    ; string length

    int     0x80           ; syscall

    cmp     eax, 0         ; check if error (eax < 0)
    jl      .error
    mov     al, 0          ; success
    ret
.error:
    mov     al, 1          ; error
    ret

section .rodata
msg: db "Hello world", 0xa
msg_len: equ $-msg
