;
; file memory.asm
; author: BjornBEs
; date: 2025-05-05
; description: Memory manipulation functions
;

[bits 32]

section .text

;
; memcpy(void *dest, const void *src, size_t n)
;
global memcpy
memcpy:
    push ebp
    mov ebp, esp
    cld
    push esi
    push edi

    mov edi, [ebp + 8]    ; dest
    mov esi, [ebp + 12]   ; src
    mov ecx, [ebp + 16]   ; n

    rep movsb
    mov eax, [ebp + 8]    ; return dest

    pop edi
    pop esi
    pop ebp
    ret

;
; memset(void *s, int c, size_t n)
;
global memset
memset:
    push ebp
    mov ebp, esp
    cld
    push edi

    mov edi, [ebp + 8]    ; s
    mov al, [ebp + 12]    ; c
    mov ecx, [ebp + 16]   ; n

    rep stosb

    mov eax, [ebp + 8]    ; return s
    pop edi
    pop ebp
    ret
    
;
; memset16(uint16 *s, uint16 c, uint16 n)
;
global memset16
memset16:
    push ebp
    mov ebp, esp
    cld
    push edi

    mov edi, [ebp + 8]    ; s
    mov al, [ebp + 12]    ; c
    mov ecx, [ebp + 16]   ; n
    
    
    rep stosw

    mov eax, [ebp + 8]    ; return s
    pop edi
    pop ebp
    ret
    
;
; memset32(uint32 *s, uint32 c, uint32 n)
;
global memset32
memset32:
    push ebp
    mov ebp, esp
    cld
    push edi

    mov edi, [ebp + 8]    ; s
    mov al, [ebp + 12]    ; c
    mov ecx, [ebp + 16]   ; n
    
    
    rep stosd

    mov eax, [ebp + 8]    ; return s
    pop edi
    pop ebp
    ret

;
; int memcmp(count char *cs, const char *ct)
;
; Compares the two strings pointed to by cs and ct.
global memcmp
memcmp:
    push ebp
    mov ebp, esp
    cld
    push esi
    push edi
    push ebx
    
    mov esi, [ebp + 8] ; cs
    mov edi, [ebp + 12] ; ct
    
.loop:
    lodsb ; load byte from cs
    mov bl, [edi]
    inc edi
    cmp al, bl ; compare with ct
    jne .diff ; if not equal, jump to end
    test al, al ; check for null terminator
    jnz .loop ; if null, jump to end
    xor eax, eax ; set result to zero if equal
    jmp .end ; continue comparing
.diff:
    sub eax, [edi] ; calculate difference
    jmp .end ; jump to end
.end:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret

;
; memmove(void *dest, const void *src, size_t n)
;
global memmove
memmove:
    push ebp
    mov ebp, esp
    cld
    push esi
    push edi

    mov edi, [ebp + 8]    ; dest
    mov esi, [ebp + 12]   ; src
    mov ecx, [ebp + 16]   ; n

    cmp edi, esi
    je .done
    jc .forward

    ; Backward copy for overlap
    lea esi, [esi + ecx - 1]
    lea edi, [edi + ecx - 1]
    std
    rep movsb
    cld
    jmp .exit

.forward:
    cld
    rep movsb

.exit:
    mov eax, [ebp + 8]    ; return dest

.done:
    pop edi
    pop esi
    pop ebp
    ret

;
; type memchr(const char *s, int c, size_t count)
;
; Finds the first occurrence of the character c in the first count bytes of the memory area pointed to by s.
global memchr
memchr:
    push ebp
    mov ebp, esp
    cld
    push edi
    push ecx
    
    mov edi, [ebp + 8] ; s
    mov eax, [ebp + 16] ; c
    mov ecx, [ebp + 12] ; count
    repne scasb ; scan for c
    jne .not_found ; if not found, jump to end
    lea eax, [edi - 1] ; return pointer to c
    jmp .done ; jump to end
.not_found:
    xor eax, eax ; return null if not found

.done:
    pop ecx
    pop edi
    pop ebp
    ret

;
; type memscan(void*s, int c, size_t size)
;
; Scans the memory area pointed to by s for the first occurrence of the character c, up to size bytes.
global memscan
memscan:
    push ebp
    mov ebp, esp
    cld
    push edi
    push ecx
    
    mov edi, [ebp + 8] ; s
    mov al, [ebp + 12] ; c
    mov ecx, [ebp + 16] ; size
    repne scasb ; scan for c
    mov eax, edi ; return pointer to c
    
    pop ecx
    pop edi
    pop ebp
    ret