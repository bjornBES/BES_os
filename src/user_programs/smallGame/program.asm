extern cstart

section .text

global start
start:
    call    cstart
    mov     ebx,        eax
    mov     eax,        2
    int     0x80