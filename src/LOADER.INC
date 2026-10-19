; Included by RESIDENT.ASM in F0000: loader head [0A76,0B1E).
; F0000 loader head: preserve original RESIDENT/GRAPHICS contribution order.
; DOS overlay load, driver-vector installation and failure handling.
ASSUME CS:F0000
EXTRN driver_append:NEAR, driver_exec:NEAR, driver_fatal:NEAR
PUBLIC _load_program
_load_program:
    push bp
    mov bp,sp
    mov ah,048H
    mov bx,0FFFFH
    int 021H
    jc SHORT ld_alloc
    lea si,DS:[ld_alloc_unexpected]
    jmp NEAR PTR driver_fatal
ld_alloc:
    mov ah,048H
    sub bx,0100H
    mov DS:[ld_capacity],bx
    int 021H
    jnc SHORT ld_exec
    lea si,DS:[ld_alloc_failed]
    jmp NEAR PTR driver_fatal
ld_exec:
    push ds
    pop es
    lea bx,DS:[ld_exec_block]
    mov [bx],ax
    mov [bx+02H],ax
    mov ax,04B03H
    mov dx,[bp+06H]
    call NEAR PTR driver_exec
    jnc SHORT ld_loaded
    cmp ax,02H
    jnz SHORT ld_check_memory
    lea si,DS:[ld_file_error]
    jmp NEAR PTR driver_fatal
ld_check_memory:
    cmp ax,08H
    jnz SHORT ld_check_other
    lea si,DS:[ld_memory_error]
    jmp NEAR PTR driver_fatal
ld_check_other:
    jz SHORT ld_loaded
    lea si,DS:[ld_exec_error]
    jmp NEAR PTR driver_fatal
ld_loaded:
    mov es,DS:[ld_exec_block]
    mov bx,ES:[02AH]
    mov ax,ES:[02CH]
    add ax,0FH
    rcr ax,1
    shr ax,1
    shr ax,1
    shr ax,1
    add bx,ax
    mov dx,[bp+08H]
    or dx,dx
    jz SHORT ld_size_check
    call NEAR PTR driver_append
ld_size_check:
    mov ax,es
    sub bx,ax
    add bx,08H
    cmp bx,DS:[ld_capacity]
    jna SHORT ld_resize
    lea si,DS:[ld_capacity_error]
    jmp NEAR PTR driver_fatal
ld_resize:
    mov ah,04AH
    int 021H
    jnc SHORT ld_done
    lea si,DS:[ld_resize_error]
    jmp NEAR PTR driver_fatal
ld_done:
    mov ax,es
    pop bp
    retf
ASSUME CS:NOTHING
