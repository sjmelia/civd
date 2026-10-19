; Included by RESIDENT.ASM in F0000: file check and INT 24 [0668,0732).
; Preserve FCB search, drive-switching retries and critical-error behavior.
ASSUME CS:F0000
ck_old_segment EQU 06EH
ck_old_offset EQU 070H
ck_fcb EQU 073H
PUBLIC _file_exists
ck_error DW 0
_file_exists:
    push bp
    mov bp,sp
    push si
    push di
    push es
    mov ah,035H
    mov al,024H
    int 021H
    mov DS:[ck_old_segment],es
    mov DS:[ck_old_offset],bx
    mov ah,025H
    mov al,024H
    mov dx,OFFSET check_critical_handler
    push ds
    push cs
    pop ds
    int 021H
    pop ds
    mov ax,SEG _F2B01
    mov es,ax
    mov si,[bp+06H]
    mov di,ck_fcb
    mov ax,02900H
    int 021H
    mov WORD PTR CS:ck_error,00H
    mov ah,011H
    mov dx,ck_fcb
    int 021H
    or al,al
    jz SHORT ck_done
    mov ah,0EH
    mov dx,[bp+08H]
    int 021H
    mov WORD PTR CS:ck_error,00H
    mov ah,011H
    mov dx,ck_fcb
    int 021H
    or al,al
    jnz SHORT ck_count_drives
    cmp WORD PTR CS:ck_error,00H
    jz SHORT ck_done
ck_count_drives:
    int 011H
    rol al,1
    rol al,1
    and al,03H
    jnz SHORT ck_other_drive
ck_not_found:
    mov ax,0FFFFH
    jmp SHORT ck_done
    nop
ck_other_drive:
    mov WORD PTR CS:ck_error,00H
    mov ah,019H
    int 021H
    xor al,01H
    mov dl,al
    mov ah,0EH
    int 021H
    mov ah,011H
    mov dx,ck_fcb
    int 021H
    cmp WORD PTR CS:ck_error,00H
    jnz SHORT ck_not_found
ck_done:
    cbw
    push ax
    mov ah,025H
    mov al,024H
    push ds
    mov ds,DS:[ck_old_segment]
    mov dx,DS:[ck_old_offset]
    int 021H
    pop ds
    pop ax
    pop es
    pop di
    pop si
    pop bp
    retf
check_critical_handler:
    sti
    push si
    push di
    push es
    push ds
    push bx
    push cx
    push dx
    mov WORD PTR CS:ck_error,01H
    mov al,00H
    pop dx
    pop cx
    pop bx
    pop ds
    pop es
    pop di
    pop si
    iret
DB 0 ; original inter-function alignment byte
ASSUME CS:NOTHING
