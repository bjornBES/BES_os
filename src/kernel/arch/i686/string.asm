;
; file string.asm
; author: BjornBEs
; date: 2025-05-05
; description: String manipulation functions
;

[bits 32]

section .text

;
; type strcpy(char *dest, const char *src)
;
; Copies the string pointed to by src (including the null terminator) to dest.
global strcpy
strcpy:
    push ebp
    mov ebp, esp
    cld
    push edi
    push esi
    
    mov edi, [ebp + 8] ; dest
    mov esi, [ebp + 12] ; src
    
    .L1:
    lodsb
    stosb
    test al, al
    jne .L1
    
    mov eax, edi ; return dest
    pop edi
    pop esi
    
    pop ebp
    ret

;
; type strncpy(char *dest, const char *src, size_t count)
;
; Copies up to count characters from the string pointed to by src to dest.
global strncpy
strncpy:
    push ebp
    mov ebp, esp
    cld
    push edi
    push esi
    push ecx
    
    mov edi, [ebp + 8] ; dest
    mov esi, [ebp + 12] ; src
    mov ecx, [ebp + 16] ; count
    xor eax, eax ; clear eax for null terminator

    .L1:
    test ecx, ecx
    je .L4 ; if count is zero, jump to end
    lodsb ; load byte from src
    stosb ; store byte to dest
    test al, al ; check for null terminator
    jnz .L3 ; if not null, continue copying
    .L2:
    
    dec ecx ; decrement count
    jz .L4 ; if count
    stosb
    jmp .L2 ; continue filling with nulls
    
.L3:
    dec ecx ; decrement count
    jmp .L1 ; continue copying
    
.L4:
    mov eax, edi ; return dest
    pop ecx
    pop esi
    pop edi
    pop ebp
    ret

;
; type strcat(char *dest, const char *src)
;
; Appends the string pointed to by src (including the null terminator) to dest.
global strcat
strcat:
    push ebp
    mov ebp, esp
    cld
    push edi
    push esi
    
    mov edi, [ebp + 8] ; dest
    mov esi, edi
    
    .find_end:
    lodsb ; load byte from dest
    test al, al ; check for null terminator
    jnz .find_end ; if null, jump to end
    dec esi ; move to next character in dest
    mov edi, esi ; update edi to point to end of dest
    mov esi, [ebp + 12] ; src
.copy_loop:
    lodsb ; load byte from src
    stosb ; store byte to dest
    test al, al ; check for null terminator
    jnz .copy_loop ; if not null, continue copying
    
    mov eax, [ebp + 8] ; return dest
    pop esi
    pop edi
    pop ebp
    ret

;
; type strncat(char *dest, const char *src, size_t count)
;
; Appends up to count characters from the string pointed to by src to dest.
global strncat
strncat:
    push ebp
    mov ebp, esp
    cld
    push edi
    push esi
    push ecx
    
    mov edi, [ebp + 8] ; dest
    mov esi, edi
    
    .find_end:
    lodsb ; load byte from dest
    test al, al ; check for null terminator
    jnz .find_end ; if null, jump to end
    dec esi ; move to next character in dest
    mov edi, esi ; update edi to point to end of dest
    mov esi, [ebp + 12] ; src
    mov ecx, [ebp + 16] ; count
.copy_loop:
    test ecx, ecx
    je .null_term
    lodsb ; load byte from src
    stosb ; store byte to dest
    test al, al ; check for null terminator
    je .null_term ; if null, jump to end
    dec ecx ; decrement count
    jmp .copy_loop ; continue copying
.null_term:
    mov byte [edi], 0 ; null terminate dest
    mov eax, [ebp + 8] ; return dest
    pop ecx
    pop esi
    pop edi
    pop ebp
    ret

;
; type strcmp(count char *cs, const char *ct)
;
; Compares the two strings pointed to by cs and ct.
;global strcoll
;global strcmp
;strcoll:
;strcmp:
;    push ebp
;    mov ebp, esp
;    cld
;    push esi
;    push edi
;    push ebx
;    
;    mov esi, [ebp + 8] ; cs
;    mov edi, [ebp + 12] ; ct
;    
;.loop:
;    mov al, [esi]
;    mov bl, [edi]
;    cmp al, bl ; compare with ct
;    jne .diff ; if not equal, jump to end
;    test al, al ; check for null terminator
;    je .done ; if null, jump to end
;    inc edi
;    inc esi
;    jmp .loop ; continue comparing
;.diff:
;    sub eax, ebx ; calculate difference
;    jmp .end ; jump to end
;.done:
;    xor eax, eax ; set result to zero if equal
;.end:
;    pop ebx
;    pop edi
;    pop esi
;    pop ebp
;    ret

;
; type strncmp(count char *cs, const char *ct, size_t count)
;
; Compares up to count characters from the two strings pointed to by cs and ct.
global strncmp
strncmp:
    push ebp
    mov ebp, esp
    cld
    push esi
    push edi
    push ecx
    
    mov esi, [ebp + 8] ; cs
    mov edi, [ebp + 12] ; ct
    mov ecx, [ebp + 16] ; count
    xor eax, eax ; result

.loop:
    test ecx, ecx
    je .done
    lodsb ; load byte from cs
    cmp al, [edi] ; compare with ct
    jne .diff ; if not equal, jump to end
    test al, al ; check for null terminator
    je .done ; if null, jump to end
    inc esi ; move to next character in cs
    inc edi ; move to next character in ct
    dec ecx ; decrement count
    jmp .loop ; continue comparing
.diff:
    sub eax, [edi] ; calculate difference
    jmp .end ; jump to end
.done:
    xor eax, eax ; set result to zero if equal
.end:
    pop ecx
    pop edi
    pop esi
    pop ebp
    ret

;
; type strchr(const char *s, char c)
;
; Finds the first occurrence of the character c in the string pointed to by s.
; global strchr
; strchr:
;     push ebp
;     mov ebp, esp
;     cld
;     push edi
;     push ecx
;     
;     mov edi, [ebp + 8] ; s
;     mov eax, [ebp + 12] ; c
;     mov ecx, -1 ; clear ecx for null terminator
;     int 0x2
;     clc
;     repne scasb ; scan for c
;     int 0x2
;     jne .notfound ; if not found, jump to end
;     lea eax, [edi] ; return pointer to c
;     jmp .end ; jump to end
; .notfound:
;     xor eax, eax ; return null if not found
; .end:
;     pop ecx
;     pop edi
;     pop ebp
;     ret

;
; type strlen(const char *s)
;
; Returns the length of the string pointed to by s, not including the null terminator.
global strlen
strlen:
    push ebp
    mov ebp, esp
    cld
    push edi
    push ecx
    
    mov edi, [ebp + 8] ; s
    mov ecx, -1 ; count
    xor eax, eax ; length
    repne scasb ; scan for null terminator

    not ecx ; negate count
    dec ecx ; adjust for null terminator
    mov eax, ecx ; return length

    pop ecx
    pop edi
    pop ebp
    ret
;
; type strnlen(const char *s, size_t count)
;
; Returns the length of the string pointed to by s, not including the null terminator, up to count bytes.
global strnlen
strnlen:
    push ebp
    mov ebp, esp
    cld
    push esi
    push ecx
    
    mov esi, [ebp + 8] ; s
    mov ecx, [ebp + 12] ; count
    xor eax, eax ; length

    repne scasb ; scan for null terminator
    mov eax, [ebp - 12]
    sub eax, ecx ; adjust count

    pop ecx
    pop esi
    pop ebp
    ret
