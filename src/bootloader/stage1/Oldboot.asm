bits 16


%define ENDL 0x0D, 0x0A

%define fat12 1
%define fat16 2
%define fat32 3
%define ext2  4

;
; FAT12 header
; 
section .fsjump
    jmp short start
    nop

section .fsheaders

    %if (FILESYSTEM == fat12) || (FILESYSTEM == fat16) || (FILESYSTEM == fat32)

        bdb_oem:                    db "MSWIN4.1"           ; 8 bytes
        bdb_bytes_per_sector:       dw 512
        bdb_sectors_per_cluster:    db 1
        bdb_reserved_sectors:       dw 1
        bdb_fat_count:              db 2
        bdb_dir_entries_count:      dw 0E0h
        bdb_total_sectors:          dw 2880                 ; 2880 * 512 = 1.44MB
        bdb_media_descriptor_type:  db 0F0h                 ; F0 = 3.5" floppy disk
        bdb_sectors_per_fat:        dw 9                    ; 9 sectors/fat
        bdb_sectors_per_track:      dw 18
        bdb_heads:                  dw 2
        bdb_hidden_sectors:         dd 0
        bdb_large_sector_count:     dd 0
    
        %if (FILESYSTEM == fat32)
            fat32_sectors_per_fat:      dd 0
            fat32_flags:                dw 0
            fat32_fat_version_number:   dw 0
            fat32_rootdir_cluster:      dd 0
            fat32_fsinfo_sector:        dw 0
            fat32_backup_boot_sector:   dw 0
            fat32_reserved:             times 12 db 0
        %endif
    
        ; extended boot record
        ebr_drive_number:           db 0                    ; 0x00 floppy, 0x80 hdd, useless
                                    db 0                    ; reserved
        ebr_signature:              db 29h
        ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number, value doesn't matter
        ebr_volume_label:           db 'BJORN BES',ENDL,0        ; 11 bytes, padded with spaces
        ebr_system_id:              db 'FAT12   '           ; 8 bytes
    %endif
;
; Code goes here
;
section .entry
    global start
    ;
    ; Code goes here
    ;

    start:
        ; setup data segments
        mov ax, 0           ; can't set ds/es directly
        mov ds, ax
        mov es, ax

        ; setup stack
        mov ss, ax
        mov sp, 0x7C00              ; stack grows downwards from where we are loaded in memory

        ; some BIOSes might start us at 07C0:0000 instead of 0000:7C00, make sure we are in the
        ; expected location
        push es
        push word .after
        retf

    .after:

        ; read something from floppy disk
        ; BIOS should set DL to drive number
        mov [ebr_drive_number], dl

        ; compute LBA of root directory = reserved + fats * sectors_per_fat
        ; note: this section can be hardcoded
        mov ax, 9
        mov bl, [bdb_fat_count]
        xor bh, bh
        mul bx                              ; ax = (fats * sectors_per_fat)
        add ax, [bdb_reserved_sectors]      ; ax = LBA of root directory
        push ax

        ; compute size of root directory = (32 * number_of_entries) / bytes_per_sector
        mov ax, [bdb_dir_entries_count]
        shl ax, 5                           ; ax *= 32
        xor dx, dx                          ; dx = 0
        div word [bdb_bytes_per_sector]     ; number of sectors we need to read

        test dx, dx                         ; if dx != 0, add 1
        jz .root_dir_after
        inc ax                              ; division remainder != 0, add 1
                                            ; this means we have a sector only partially filled with entries
    .root_dir_after:

        ; read root directory
        mov cl, al                          ; cl = number of sectors to read = size of root directory
        pop ax                              ; ax = LBA of root directory
        mov dl, [ebr_drive_number]          ; dl = drive number (we saved it previously)
        mov bx, buffer                      ; es:bx = buffer
        call disk_read

        ; search for kernel.bin
        xor bx, bx
        mov di, buffer

    .search_kernel:
        mov si, file_stage2_bin
        mov cx, 11                          ; compare up to 11 characters
        push di
        repe cmpsb
        pop di
        je .found_kernel

        add di, 32
        inc bx
        cmp bx, [bdb_dir_entries_count]
        jl .search_kernel

        ; kernel not found
        jmp kernel_not_found_error

    .found_kernel:

        ; di should have the address to the entry
        mov ax, [di + 26]                   ; first logical cluster field (offset 26)
        mov [stage2_cluster], ax

        ; load FAT from disk into memory
        mov ax, [bdb_reserved_sectors]
        mov bx, buffer
        mov cl, [bdb_sectors_per_fat]
        mov dl, [ebr_drive_number]
        call disk_read

        ; read kernel and process FAT chain
        mov bx, STAGE2_LOAD_SEGMENT
        mov es, bx
        mov bx, STAGE2_LOAD_OFFSET

    .load_kernel_loop:

        ; Read next cluster
        mov ax, [stage2_cluster]

        ; not nice :( hardcoded value
        add ax, 31                          ; first cluster = (stage2_cluster - 2) * sectors_per_cluster + start_sector
                                            ; start sector = reserved + fats + root directory size = 1 + 18 + 134 = 33
        mov cl, 1
        mov dl, [ebr_drive_number]
        call disk_read

        add bx, [bdb_bytes_per_sector]

        ; compute location of next cluster
        mov ax, [stage2_cluster]
        mov cx, 3
        mul cx
        mov cx, 2
        div cx                              ; ax = index of entry in FAT, dx = cluster mod 2

        mov si, buffer
        add si, ax
        mov ax, [ds:si]                     ; read entry from FAT table at index ax

        or dx, dx
        jz .even

    .odd:
        shr ax, 4
        jmp .next_cluster_after

    .even:
        and ax, 0x0FFF

    .next_cluster_after:
        cmp ax, 0x0FF8                      ; end of chain
        jae .read_finish

        mov [stage2_cluster], ax
        jmp .load_kernel_loop

    .read_finish:

        ; jump to our kernel
        mov dl, [ebr_drive_number]          ; boot device in dl

        mov ax, STAGE2_LOAD_SEGMENT         ; set segment registers
        mov ds, ax
        mov es, ax

        jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

        jmp wait_key_and_reboot             ; should never happen

        cli                                 ; disable interrupts, this way CPU can't get out of "halt" state
        hlt


section .text

    ;
    ; Error handlers
    ;

    floppy_error:
        mov si, msg_read_failed
        call puts
        jmp wait_key_and_reboot

    kernel_not_found_error:
        mov si, msg_stage2_not_found
        call puts
        jmp wait_key_and_reboot

    wait_key_and_reboot:
        mov ah, 0
        int 16h                     ; wait for keypress
        jmp 0FFFFh:0                ; jump to beginning of BIOS, should reboot

    .halt:
        cli                         ; disable interrupts, this way CPU can't get out of "halt" state
        hlt


    ;
    ; Prints a string to the screen
    ; Params:
    ;   - ds:si points to string
    ;
    puts:
        ; save registers we will modify
        push si
        push ax
        push bx

    .loop:
        lodsb               ; loads next character in al
        or al, al           ; verify if next character is null?
        jz .done

        mov ah, 0x0E        ; call bios interrupt
        mov bh, 0           ; set page number to 0
        int 0x10

        jmp .loop

    .done:
        pop bx
        pop ax
        pop si    
        ret

    ;
    ; Disk routines
    ;

    ;
    ; Converts an LBA address to a CHS address
    ; Parameters:
    ;   - ax: LBA address
    ; Returns:
    ;   - cx [bits 0-5]: sector number
    ;   - cx [bits 6-15]: cylinder
    ;   - dh: head
    ;

    lba_to_chs:

        push ax
        push dx

        xor dx, dx                          ; dx = 0
        div word [bdb_sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                            ; dx = LBA % SectorsPerTrack

        inc dx                              ; dx = (LBA % SectorsPerTrack + 1) = sector
        mov cx, dx                          ; cx = sector

        xor dx, dx                          ; dx = 0
        div word [bdb_heads]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                            ; dx = (LBA / SectorsPerTrack) % Heads = head
        mov dh, dl                          ; dh = head
        mov ch, al                          ; ch = cylinder (lower 8 bits)
        shl ah, 6
        or cl, ah                           ; put upper 2 bits of cylinder in CL

        pop ax
        mov dl, al                          ; restore DL
        pop ax
        ret


    ;
    ; Reads sectors from a disk
    ; Parameters:
    ;   - eax: LBA address
    ;   - cl: number of sectors to read (up to 128)
    ;   - dl: drive number
    ;   - es:bx: memory address where to store read data
    ;
    disk_read:

        push eax                            ; save registers we will modify
        push bx
        push cx
        push dx
        push si
        push di

        cmp byte [have_extensions], 1
        jne .no_disk_extensions

        ; with extensions
        mov [extensions_dap.lba], eax
        mov [extensions_dap.segment], es
        mov [extensions_dap.offset], bx
        mov [extensions_dap.count], cl

        mov ah, 0x42
        mov si, extensions_dap
        mov di, 3                           ; retry count
        jmp .retry

    .no_disk_extensions:
        push cx                             ; temporarily save CL (number of sectors to read)
        call lba_to_chs                     ; compute CHS
        pop ax                              ; AL = number of sectors to read
        
        mov ah, 02h
        mov di, 3                           ; retry count

    .retry:
        pusha                               ; save all registers, we don't know what bios modifies
        stc                                 ; set carry flag, some BIOS'es don't set it
        int 13h                             ; carry flag cleared = success
        jnc .done                           ; jump if carry not set

        ; read failed
        popa
        call disk_reset

        dec di
        test di, di
        jnz .retry

    .fail:
        ; all attempts are exhausted
        jmp floppy_error

    .done:
        popa

        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop eax                            ; restore registers modified
        ret


    ;
    ; Resets disk controller
    ; Parameters:
    ;   dl: drive number
    ;
    disk_reset:
        pusha
        mov ah, 0
        stc
        int 13h
        jc floppy_error
        popa
        ret

section .rodata

    msg_read_failed:        db 'Read failed!', 0
    msg_stage2_not_found:   db 'STAGE2.BIN not found!', ENDL, 0
    file_stage2_bin:        db 'STAGE2  BIN'

section .data

    have_extensions:        db 0
    extensions_dap:
        .size:              db 10h
                            db 0
        .count:             dw 0
        .offset:            dw 0
        .segment:           dw 0
        .lba:               dq 0

    STAGE2_LOAD_SEGMENT     equ 0x0
    STAGE2_LOAD_OFFSET      equ 0x500

    PARTITION_ENTRY_SEGMENT equ 0x2000
    PARTITION_ENTRY_OFFSET  equ 0x0

section .bss
    buffer:                 resb 512