; Included by GRAPHICS.ASM in F0000: loader tail [0B1E,0BFA).
; F0000 loader tail: preserve original RESIDENT/GRAPHICS contribution order.
; DOS overlay load, driver-vector installation and failure handling.
ASSUME CS:F0000
EXTRN driver_slots:BYTE
PUBLIC driver_append, driver_exec, driver_fatal, _run_program
ld_vector_segment EQU 028H
ld_first_slot EQU 02EH
ld_slot_count EQU 030H
driver_append:
    push ds
    mov ax,03D00H
    int 021H
    jnc SHORT ld_opened
    lea si,DS:[ld_file_error]
    jmp NEAR PTR driver_fatal
ld_opened:
    mov ds,bx
    mov bx,ax
ld_read_block:
    mov ah,03FH
    xor dx,dx
    mov cx,08000H
    int 021H
    jnc SHORT ld_read_ok
    lea si,DS:[ld_file_error]
    jmp NEAR PTR driver_fatal
ld_read_ok:
    cmp ax,cx
    jc SHORT ld_read_done
    mov ax,ds
    add ax,0800H
    mov ds,ax
    jmp SHORT ld_read_block
ld_read_done:
    add ax,0FH
    rcr ax,1
    shr ax,1
    shr ax,1
    shr ax,1
    mov dx,ds
    add dx,ax
    mov ah,03EH
    int 021H
    jnc SHORT ld_closed
    lea si,DS:[ld_file_error]
    jmp SHORT driver_fatal
    nop
ld_closed:
    mov bx,dx
    pop ds
    ret
_run_program:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov ds,[bp+06H]
    mov si,032H
    mov ax,DS:[ld_first_slot]
    mov dl,07H
    mul dl
    lea di,driver_slots
    add di,ax
    mov ax,SEG _F0000
    mov es,ax
    mov cx,DS:[ld_slot_count]
    mov ax,DS:[ld_vector_segment]
ld_install_slot:
    add di,03H
    movsw
    stosw
    loop SHORT ld_install_slot
    pop ds
    pop di
    pop si
    pop bp
    retf
driver_free:
    push bp
    mov bp,sp
    mov es,[bp+06H]
    mov ah,049H
    int 021H
    jnc SHORT ld_freed
    lea si,DS:[ld_free_error]
    jmp SHORT driver_fatal
    nop
ld_freed:
    pop bp
    retf
; DOS EXEC can alter the stack; restore the saved caller SS:SP.
ld_saved_sp DW 0
ld_saved_ss DW 0
driver_exec:
    push bp
    push si
    push di
    push ds
    push es
    mov CS:ld_saved_ss,ss
    mov CS:ld_saved_sp,sp
    int 021H
    mov ss,CS:ld_saved_ss
    mov sp,CS:ld_saved_sp
    pop es
    pop ds
    pop di
    pop si
    pop bp
    ret
driver_fatal:
    mov ah,02H
    cmp al,0AH
    sbb al,069H
    das
    mov dl,al
    int 021H
    lea dx,DS:[ld_error_prefix]
    mov ah,09H
    int 021H
    mov ah,09H
    mov dx,si
    int 021H
    mov ax,04C99H
    int 021H
DB 0 ; original alignment after the fatal exit path
ASSUME CS:NOTHING
