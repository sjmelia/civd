; Included by RESIDENT.ASM in F0000: palette fading [04AA,0668).
; Capture, calibrate and interpolate VGA DAC entries during timer ticks.
; Low DS offsets and shared palette buffers name existing storage only.
ASSUME CS:F0000
.186
PUBLIC _install_palette, _set_palette
fd_fast_io EQU 060H
fd_start EQU 062H
fd_count EQU 064H
fd_rate EQU 066H
fd_pending EQU 068H
fd_fraction EQU 06AH
fd_fraction_step EQU 06CH
fd_target EQU 0BA06H
fd_initial EQU 0BD06H
fd_mixed EQU 0C006H
_install_palette:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    pop es
    mov cx,0300H
    mov si,[bp+08H]
    add si,06H
    lea di,DS:[fd_target]
    rep movsb
    call NEAR PTR fade_capture
    mov ax,[bp+06H]
    call NEAR PTR fade_prepare
fd_install_wait:
    cmp WORD PTR DS:[fd_pending],00H
    jnz SHORT fd_install_wait
    pop di
    pop si
    pop bp
    retf
_set_palette:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    pop es
    mov al,[bp+08H]
    mov ah,[bp+0AH]
    mov bl,[bp+0CH]
    lea di,DS:[fd_target]
    mov cx,0100H
fd_fill_loop:
    mov [di],al
    mov [di+01H],ah
    mov [di+02H],bl
    add di,03H
    loop SHORT fd_fill_loop
    call NEAR PTR fade_capture
    mov ax,[bp+06H]
    call NEAR PTR fade_prepare
fd_set_wait:
    cmp WORD PTR DS:[fd_pending],00H
    jnz SHORT fd_set_wait
    pop di
    pop si
    pop bp
    retf
fade_prepare:
    mov ah,06H
    mul ah
    mov bp,ax
    mov ax,DS:[fd_rate]
    or ax,ax
    jnz SHORT fd_rate_ok
    call NEAR PTR fade_calibrate
    mov DS:[fd_rate],ax
fd_rate_ok:
    xor dx,dx
    mov cx,0EH
    div cx
    or ah,ah
    jz SHORT fd_count_ok
    mov ax,0100H
fd_count_ok:
    mov DS:[fd_count],ax
    mov DS:[fd_start],ax
    neg WORD PTR DS:[fd_start]
    mul bp
    mov ch,dl
    mov cl,ah
    dec cx
    mov dx,01H
    mov ax,00H
    div cx
    mov DS:[fd_fraction_step],ax
    mov DS:[fd_fraction],ax
    call NEAR PTR fade_mix
    mov DS:[fd_pending],bp
    ret
fade_capture:
    cli
    mov dx,03DAH
fd_capture_retrace:
    in al,dx
    test al,08H
    jz SHORT fd_capture_retrace
    lea di,DS:[fd_initial]
    mov cx,0300H
    mov dl,0C7H
    mov al,00H
    out dx,al
    jmp SHORT fd_capture_read
fd_capture_read:
    mov dl,0C9H
fd_capture_loop:
    in al,dx
    stosb
    loop SHORT fd_capture_loop
    sti
    ret
fade_calibrate:
    mov ax,0FFFFH
    mov cl,020H
    shr ax,cl
    mov DS:[fd_fast_io],ax
    mov bx,00H
    cli
    mov dx,03DAH
fd_cal_retrace:
    in al,dx
    test al,08H
    jz SHORT fd_cal_retrace
fd_cal_display:
    in al,dx
    test al,08H
    jnz SHORT fd_cal_display
fd_cal_block:
    mov dl,0C8H
    mov al,00H
    out dx,al
    mov dl,0C9H
    lea si,DS:[fd_initial]
    mov cx,01EH
    or ah,ah
    jz SHORT fd_cal_slow
    rep outsb
    jmp SHORT fd_cal_next
    nop
fd_cal_slow:
    lodsb
    out dx,al
    loop SHORT fd_cal_slow
fd_cal_next:
    add bx,0AH
    mov dl,0DAH
    in al,dx
    test al,08H
    jz SHORT fd_cal_block
    sti
    mov ax,bx
    ret
fade_mix:
    push si
    push di
    push bp
    push es
    mov ax,DS:[fd_start]
    mov cx,DS:[fd_count]
    add ax,cx
    xor ah,ah
    mov DS:[fd_start],ax
    mov si,ax
    add si,ax
    add si,ax
    lea di,DS:[fd_mixed]
    push ds
    pop es
    mov ax,cx
    add cx,ax
    add cx,ax
fd_mix_loop:
    mov bx,00H
    mov bp,00H
    mov al,[si+fd_initial]
    xor ah,ah
    or ax,ax
    jz SHORT fd_mix_target
    mov dx,DS:[fd_fraction]
    not dx
    mul dx
    mov bp,ax
    mov bx,dx
fd_mix_target:
    mov al,[si+fd_target]
    xor ah,ah
    or ax,ax
    jz SHORT fd_mix_store
    mul WORD PTR DS:[fd_fraction]
    add bp,ax
    adc bx,dx
fd_mix_store:
    shl bp,1
    adc bx,00H
    mov al,bl
    stosb
    inc si
    cmp si,0300H
    jc SHORT fd_mix_next
    mov si,00H
    mov ax,DS:[fd_fraction_step]
    add DS:[fd_fraction],ax
    jnc SHORT fd_mix_next
    mov WORD PTR DS:[fd_fraction],0FFFFH
fd_mix_next:
    loop SHORT fd_mix_loop
    pop es
    pop bp
    pop di
    pop si
    ret
fade_tick:
    cmp WORD PTR DS:[fd_pending],00H
    jz SHORT fd_tick_done
    push si
    mov ax,DS:[fd_start]
    mov bx,DS:[fd_count]
    mov cx,bx
    add cx,bx
    add cx,bx
    lea si,DS:[fd_mixed]
    mov dx,03C8H
    out dx,al
    inc dx
    cmp WORD PTR DS:[fd_fast_io],00H
    jz SHORT fd_tick_slow
    rep outsb
    jmp SHORT fd_tick_mix
    nop
fd_tick_slow:
    lodsb
    out dx,al
    loop SHORT fd_tick_slow
fd_tick_mix:
    dec WORD PTR DS:[fd_pending]
    call NEAR PTR fade_mix
    pop si
fd_tick_done:
    retf
.8086
ASSUME CS:NOTHING
