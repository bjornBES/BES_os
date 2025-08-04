[bits 32]

section .text

global start
start:  
    mov     eax,        3               ; System call 3 (open)
    mov     esi,        path            ; Setting the path
    int     0x80                        ; syscall
    mov     [FileDis],  ebx
    
    mov     esi,        openErrorMsg    ; already? now? the error msg.
    mov     ecx,        openErrorMsgLen ; get the lenght of the string
    cmp     eax,        -1              ; if eax == -1
    je      .openerror                  ; yes

    xor     eax,        eax             ; System call 0 (read)
    mov     ebx,        [FileDis]       ; getting the fd
    mov     esi,        Buffer          ; Setting the Buffer
    mov     ecx,        512             ; Setting the Count to 1 sector
    int     0x80                        ; syscall

    mov     esi,        readErrorMsg    ; already? now? the error msg.
    mov     ecx,        readErrorMsgLen ; get the lenght of the string
    cmp     eax,        ecx             ; if eax == 512(ecx)
    je      .readError                  ; yes

    mov     ebx,        0               ; all good command
    mov     esi,        allgoodMsg      ; yes it's good my code is good
    mov     esi,        allgoodMsgLen   ; get the lenght of the you know the string
    jmp     .close                      ; now jump
.readError:
    mov     edx,        -1              ; error
.close:
    mov     eax,        4               ; System call 4 (close)
    push    ebx
    mov     ebx,        [FileDis]       ; getting the fd
    int     0x80                        ; syscall
    pop     ebx
    cmp     edx,        -1              ; edx == -1
    je      .openerror                  ; yes
    cmp     eax,        -1              ; eax == -1
    jne     .exitWithMsg                ; no? thats a first in this code
    mov     esi,        closeErrorMsg   ; already? now? the error msg.
    mov     ecx,        closeErrorMsgLen; get the len... you fucking already know it right?
.openerror:                             ; yes (at least to the eax not my wife)
    mov     ebx,        -1
    
.exitWithMsg:
    call    printMsg                    ; call to print a msg
exit:
    mov     eax,                    0x55AA ; syscall number for sys_exit
    mov     [gs:0x0100000],         eax
    mov     [gs:0], eax
    
    mov     eax,        2               ; System call 2 (exit)
    int     0x80                        ; syscall
    
printMsg:
    push    ebx
    mov     eax,        1               ; syscall number (sys_write)
    mov     ebx,        1               ; file descriptor (stdout)
    int     0x80                        ; syscall
    cmp     eax,        0               ; check if error (eax < 0)
    jl      .error                      ; yes
    pop     ebx
    ret
.error:
    pop     ebx
    mov     ebx,        -2              ; -2 for the thing we can print out
    jmp     exit                        ; exit the program


section .bss
FileDis:
    resb 4
global Buffer
Buffer:
    resb 512

section .rodata
allgoodMsg:
    db "all good returning 0", 0xa
allgoodMsgLen: equ $-allgoodMsg
readErrorMsg:
    db "Error: Read did not go good", 0xa
readErrorMsgLen: equ $-readErrorMsg
closeErrorMsg:
    db "Error: Close did not go good", 0xa
closeErrorMsgLen: equ $-closeErrorMsg
openErrorMsg:
    db "Error: Open did not go good", 0xa
openErrorMsgLen: equ $-openErrorMsg
path:
    db "/ata0/"
    db "bin/test.cod", 0