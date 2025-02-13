[bits 32]

;
; uint32 bus            [ebp + 8]
; uint16 selector       [ebp + 12]
; uint32 words          [ebp + 16]
; uint32 edi            [ebp + 20]
;
global ReceiveData
ReceiveData:
    push ebp                  ; Standard function prologue
    mov ebp, esp
    pusha
    push es                   ; Save ES register

    mov ax, [ebp + 12]        ; Load selector (first argument) into AX
    mov es, ax                ; Set ES to selector

    mov ecx, [ebp + 16]       ; Load words (second argument) into ECX
    mov edx, [ebp + 8]        ; Load bus (third argument) into EDX
    mov edi, [ebp + 20]       ; Load edi (fourth argument)

    rep insw                  ; Receive Data (from DX port to ES:EDI)

    pop es                    ; Restore ES
    popa
    pop ebp                   ; Standard function epilogue
    ret                       ; Return to caller

global SendPacket
SendPacket:
    push ebp                  ; Standard function prologue
    mov ebp, esp
    pusha

    mov ecx, 6                ; Fixed word count for ATAPI packet
    mov edx, [ebp + 8]        ; Load bus (first argument) into EDX
    mov esi, [ebp + 12]       ; Load pointer to atapi_packet (second argument)

    rep outsw                 ; Send Packet Data (from ESI to DX port)

    popa
    pop ebp                   ; Standard function epilogue
    ret


;
; uint32 bus            [ebp + 8]
; uint16 selector       [ebp + 12]
; uint32 words          [ebp + 16]
; uint32 edi            [ebp + 20]
;
global SendData
SendData:
    push ebp                  ; Standard function prologue
    mov ebp, esp
    pusha
    push ds                   ; Save DS register

    mov ax, [ebp + 12]        ; Load selector (second argument) into AX
    mov ds, ax                ; Set DS to selector

    mov ecx, [ebp + 16]       ; Load words (third argument) into ECX
    mov edx, [ebp + 8]        ; Load bus (first argument) into EDX
    mov esi, [ebp + 20]       ; Load edi (fourth argument) into ESI (source)

    rep outsw                 ; Send Data (from DS:ESI to DX port)

    pop ds                    ; Restore DS
    popa
    pop ebp                   ; Standard function epilogue
    ret

;
; uint32 port           [ebp + 8]
; uint32 addr           [ebp + 12]
; uint32 cnt            [ebp + 16]
;
global insl
insl:
    push ebp
    mov ebp, esp
    pusha

    mov ax, ds      ; Ensure ES is correctly set
    mov es, ax

    mov edx, [ebp + 8]   ; Load port
    mov edi, [ebp + 12]  ; Load destination address
    mov ecx, [ebp + 16]  ; Load count

    
    test ecx, ecx
    jz .done        ; Skip if count is zero
    
    cld             ; Ensure forward direction
    rep insd        ; Read from port to memory

.done:
    popa
    pop ebp
    ret