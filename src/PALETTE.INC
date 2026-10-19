; Included by RESIDENT.ASM in F0000: palette cycling [0382,04AA).
; Eight cycling slots in CYCLE_DATA; direct VGA DAC access.
CYCLE_DATA SEGMENT PARA PUBLIC 'FAR_DATA'
; Nonzero when REP OUTSB may be used (set by the shift-count probe).
pc_fast_io DW 0
; Eight slot pointers, then a -1 end marker for the tick scan.
pc_slots DW pc_slot_1, pc_slot_2, pc_slot_3, pc_slot_4
    DW pc_slot_5, pc_slot_6, pc_slot_7, pc_slot_8
    DW -1
pc_slot_1 DB 102 DUP (0)
pc_slot_2 DB 102 DUP (0)
pc_slot_3 DB 102 DUP (0)
pc_slot_4 DB 102 DUP (0)
pc_slot_5 DB 102 DUP (0)
pc_slot_6 DB 102 DUP (0)
pc_slot_7 DB 102 DUP (0)
pc_slot_8 DB 102 DUP (0)
CYCLE_DATA ENDS
ASSUME CS:F0000
.186
PUBLIC _set_cycle, _start_cycle, _stop_cycle
_set_cycle:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov ax,SEG pc_fast_io
    mov ds,ax
    mov ax,0FFFFH
    mov cl,020H
    shr ax,cl
    mov DS:[pc_fast_io],ax
    mov bx,[bp+06H]
    dec bx
    cmp bx,08H
    jnc SHORT pc_set_done
    shl bx,1
    mov bx,WORD PTR [bx+OFFSET pc_slots]
    mov ax,[bp+0AH]
    mov dx,[bp+0CH]
    sub dx,ax
    jna SHORT pc_set_done
    inc dx
    cmp dx,010H
    jna SHORT pc_count_ok
    mov dx,010H
pc_count_ok:
    mov cx,dx
    add cx,dx
    add cx,dx
    mov [bx+04H],cx
    mov [bx+06H],al
    push ds
    pop es
    lea di,[bx+09H]
    cli
    mov dx,03C7H
    mov al,[bx+06H]
    out dx,al
    jmp SHORT pc_read_dac
pc_read_dac:
    mov dl,0C9H
pc_read_loop:
    in al,dx
    stosb
    loop SHORT pc_read_loop
    sti
    mov cx,[bx+04H]
    sub cx,03H
    lea si,[bx+09H]
    rep movsb
    mov ax,[bp+08H]
    mov [bx+07H],al
    mov BYTE PTR [bx+08H],00H
    mov WORD PTR [bx+02H],00H
pc_set_done:
    pop ds
    pop di
    pop si
    pop bp
    retf
_start_cycle:
    push bp
    mov bp,sp
    push ds
    mov ax,SEG pc_fast_io
    mov ds,ax
    mov ax,[bp+06H]
    cwd
    xor ax,dx
    sub ax,dx
    dec ax
    cmp ax,08H
    jnc SHORT pc_start_done
    mov bx,ax
    shl bx,1
    mov bx,WORD PTR [bx+OFFSET pc_slots]
    mov ax,03H
    xor ax,dx
    sub ax,dx
    mov [bx],ax
    mov al,[bx+07H]
    mov [bx+08H],al
pc_start_done:
    pop ds
    pop bp
    retf
_stop_cycle:
    push bp
    mov bp,sp
    push ds
    mov ax,SEG pc_fast_io
    mov ds,ax
    mov bx,[bp+06H]
    dec bx
    cmp bx,08H
    jnc SHORT pc_stop_done
    shl bx,1
    mov bx,WORD PTR [bx+OFFSET pc_slots]
    mov BYTE PTR [bx+08H],00H
pc_stop_done:
    pop ds
    pop bp
    retf
palette_tick:
    push si
    push di
    push ds
    mov ax,SEG pc_fast_io
    mov ds,ax
    lea di,DS:[pc_slots]
pc_tick_next:
    mov bx,[di]
    add di,02H
    cmp bx,-01H
    jz SHORT pc_tick_done
    cmp BYTE PTR [bx+08H],00H
    jz SHORT pc_tick_next
    dec BYTE PTR [bx+08H]
    jnz SHORT pc_tick_next
    mov al,[bx+07H]
    mov [bx+08H],al
    mov cx,[bx+04H]
    mov si,[bx+02H]
    sub si,[bx]
    jnl SHORT pc_wrap_low
    add si,cx
pc_wrap_low:
    cmp si,cx
    jl SHORT pc_wrapped
    mov si,00H
pc_wrapped:
    mov [bx+02H],si
    lea ax,[bx+09H]
    add si,ax
    mov dx,03C8H
    mov al,[bx+06H]
    out dx,al
    inc dx
    cmp WORD PTR DS:[pc_fast_io],00H
    jz SHORT pc_slow_out
    rep outsb ; 186+ fast path, selected by the shift-count probe
    jmp SHORT pc_tick_next
pc_slow_out:
    lodsb
    out dx,al
    loop SHORT pc_slow_out
    jmp SHORT pc_tick_next
pc_tick_done:
    pop ds
    pop di
    pop si
    retf
DB 0 ; original inter-function alignment byte
.8086
ASSUME CS:NOTHING
