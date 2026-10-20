; Included by RESIDENT.ASM in F0000: coherent timer prefix [0000,0382).
; Hardware timer, private interrupt stack, vertical-retrace calibration.
; Preserve original interrupt chaining and register/flag behavior.
ASSUME CS:F0000
tm_elapsed_low EQU 042H
tm_elapsed_high EQU 044H
tm_next_reload EQU 046H
tm_reload EQU 048H
tm_sync_count EQU 04AH
tm_divisor EQU 04CH
tm_fast_divisor EQU 04EH
tm_sample_low EQU 050H
tm_sample_high EQU 052H
tm_countdown EQU 054H
tm_phase EQU 056H
tm_vsync EQU 058H
tm_sync_delay EQU 059H
tm_installed EQU 05AH
tm_ticks EQU 05CH
tm_tick_phase EQU 05EH
PUBLIC _resident_init
_resident_init LABEL FAR
timer_install:
    push ds
    mov al,01H
    xchg DS:[tm_installed],al
    or al,al
    jnz SHORT tm_install_done
    mov WORD PTR DS:[tm_divisor],01H
    mov WORD PTR DS:[tm_countdown],01H
    mov WORD PTR DS:[tm_elapsed_low],00H
    mov WORD PTR DS:[tm_elapsed_high],00H
    call NEAR PTR timer_calibrate
    mov ah,035H
    mov al,08H
    int 021H
    mov WORD PTR CS:tm_old_vector,bx
    mov WORD PTR CS:tm_old_vector+2,es
    mov ah,025H
    mov al,08H
    lds dx,DWORD PTR CS:tm_handler_vector
    int 021H
tm_install_done:
    pop ds
    retf
PUBLIC _restore_timer
_restore_timer LABEL FAR
timer_restore:
    push ds
    mov al,00H
    xchg DS:[tm_installed],al
    or al,al
    jz SHORT tm_restore_done
    mov al,036H
    out 043H,al
    jmp SHORT tm_restore_low
tm_restore_low:
    xor al,al
    out 040H,al
    jmp SHORT tm_restore_high
tm_restore_high:
    out 040H,al
    mov ah,025H
    mov al,08H
    lds dx,DWORD PTR CS:tm_old_vector
    int 021H
tm_restore_done:
    pop ds
    retf
timer_fast:
    mov bx,DS:[tm_fast_divisor]
    mov DS:[tm_divisor],bx
    mov ax,DS:[tm_sample_low]
    xor dx,dx
    div bx
    mov DS:[tm_next_reload],ax
    mov BYTE PTR DS:[tm_sync_delay],01H
    retf
timer_slow:
    mov WORD PTR DS:[tm_divisor],01H
    mov ax,DS:[tm_sample_low]
    mov DS:[tm_next_reload],ax
    mov BYTE PTR DS:[tm_sync_delay],01H
    retf
; Private interrupt stack, followed by the installable handler pointer.
tm_stack DB 0100H DUP (0)
tm_stack_top DW 0
tm_handler_vector DW OFFSET timer_interrupt, SEG _F0000
timer_interrupt:
    push ax
    push bx
    push cx
    push dx
    push ds
    mov ax,sp
    mov bx,ss
    mov cx,SEG _F0000
    mov ss,cx
    mov sp,OFFSET tm_stack_top
    push bx
    push ax
    sti
    mov ax,SEG _F2B01
    mov ds,ax
    mov ax,DS:[tm_reload]
    add DS:[tm_elapsed_low],ax
    adc WORD PTR DS:[tm_elapsed_high],00H
    dec WORD PTR DS:[tm_countdown]
    jnz SHORT tm_poll_driver
    mov ax,DS:[tm_divisor]
    mov DS:[tm_countdown],ax
    call NEAR PTR timer_sync
    call FAR PTR timer_tick
tm_poll_driver:
    cmp WORD PTR DS:[tm_divisor],01H
    jz SHORT tm_check_chain
    call FAR PTR sound_fast_tick
tm_check_chain:
    cmp WORD PTR DS:[tm_elapsed_high],00H
    jnz SHORT tm_chain_old
    cli
    mov al,020H
    out 020H,al
    pop ax
    pop ss
    mov sp,ax
    pop ds
    pop dx
    pop cx
    pop bx
    pop ax
    iret
tm_chain_old:
    dec WORD PTR DS:[tm_elapsed_high]
    pop ax
    pop ss
    mov sp,ax
    pop ds
    pop dx
    pop cx
    pop bx
    pop ax
    cli
; Saved INT 08 vector patches the immediate operand of this chain template.
DB 0EAH ; far JMP opcode; the following vector is runtime-written data
tm_old_vector DD 0
timer_sync:
    dec BYTE PTR DS:[tm_sync_delay]
    jnz SHORT tm_sync_done
    mov al,014H
    mov DS:[tm_sync_delay],al
    xor bl,bl
    xor cx,cx
    mov dx,03DAH
    cmp BYTE PTR DS:[tm_vsync],00H
    jz SHORT tm_sync_reload
tm_sync_wait:
    cli
    in al,dx
    test al,08H
    jnz SHORT tm_sync_reload
    sti
    and al,01H
    cmp al,bl
    jz SHORT tm_sync_wait
    xor bl,01H
    loop SHORT tm_sync_wait
    mov BYTE PTR DS:[tm_vsync],00H
    jmp SHORT tm_sync_done
    nop
tm_sync_reload:
    mov dx,DS:[tm_next_reload]
    cmp dx,DS:[tm_reload]
    jnz SHORT tm_set_reload
    jcxz SHORT tm_sync_done
tm_set_reload:
    mov DS:[tm_reload],dx
    mov al,036H
    out 043H,al
    jmp SHORT tm_reload_low
tm_reload_low:
    mov al,dl
    out 040H,al
    jmp SHORT tm_reload_high
tm_reload_high:
    mov al,dh
    out 040H,al
    inc WORD PTR DS:[tm_sync_count]
    neg cx
    mov DS:[tm_phase],cx
    jz SHORT tm_sync_done
tm_sync_done:
    sti
    ret
timer_calibrate:
    pushf
    cli
    mov BYTE PTR DS:[tm_sync_delay],01H
    xor ax,ax
    mov BYTE PTR DS:[tm_vsync],01H
    mov DS:[tm_sample_low],ax
    mov DS:[tm_sample_high],ax
    call NEAR PTR timer_sample
    mov bx,ax
    mov cx,010H
tm_cal_loop:
    push bx
    call NEAR PTR timer_sample
    pop bx
    sub bx,ax
    add DS:[tm_sample_low],bx
    adc WORD PTR DS:[tm_sample_high],00H
    mov bx,ax
    loop SHORT tm_cal_loop
    mov ax,DS:[tm_sample_low]
    mov dx,DS:[tm_sample_high]
    add DS:[tm_elapsed_low],ax
    adc DS:[tm_elapsed_high],dx
    mov cx,010H
    div cx
    shr ax,1
    mov DS:[tm_sample_low],ax
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    add ax,bx
    xor dx,dx
    mov bx,0F89H
    div bx
    cmp ax,04H
    jc SHORT tm_cal_default
    cmp ax,06H
    ja SHORT tm_cal_default
    jmp SHORT tm_cal_divisor
    nop
tm_cal_default:
    mov BYTE PTR DS:[tm_vsync],00H
    mov WORD PTR DS:[tm_sample_low],04DAEH
    mov ax,05H
tm_cal_divisor:
    mov DS:[tm_fast_divisor],ax
    cmp WORD PTR DS:[tm_divisor],01H
    jz SHORT tm_cal_reload
    mov DS:[tm_divisor],ax
tm_cal_reload:
    mov ax,DS:[tm_sample_low]
    xor dx,dx
    div WORD PTR DS:[tm_divisor]
    mov DS:[tm_reload],ax
    mov DS:[tm_next_reload],ax
    popf
    ret
timer_sample:
    pushf
    cli
    mov dx,03DAH
    xor bx,bx
tm_wait_display:
    dec bx
    jz SHORT tm_sample_done
    in al,dx
    test al,08H
    jnz SHORT tm_wait_display
    xor bx,bx
tm_wait_retrace:
    dec bx
    jz SHORT tm_sample_done
    in al,dx
    test al,08H
    jz SHORT tm_wait_retrace
    mov al,00H
    out 043H,al
    jmp SHORT tm_latch_low
tm_latch_low:
    in al,040H
    jmp SHORT tm_latch_high
tm_latch_high:
    mov bl,al
    in al,040H
    jmp SHORT tm_latch_done
tm_latch_done:
    mov bh,al
tm_sample_done:
    mov ax,bx
    popf
    ret
PUBLIC _timer_read
_timer_read LABEL FAR
timer_read:
    mov ax,DS:[tm_ticks]
    retf
PUBLIC _timer_reset
_timer_reset LABEL FAR
timer_reset:
    xor ax,ax
    xchg DS:[tm_ticks],ax
    retf
timer_tick:
    inc WORD PTR DS:[tm_ticks]
    call FAR PTR fade_tick
    call FAR PTR palette_tick
    call FAR PTR shake_step
    cmp WORD PTR DS:[tm_fast_divisor],04H
    jnz SHORT tm_tick_driver
    sub BYTE PTR DS:[tm_tick_phase],01H
    jg SHORT tm_tick_driver
    mov BYTE PTR DS:[tm_tick_phase],07H
    retf
tm_tick_driver:
    call FAR PTR sound_tick
    or ax,ax
    js SHORT tm_go_slow
    jnz SHORT tm_go_fast
    retf
tm_go_slow:
    jmp FAR PTR timer_slow
tm_go_fast:
    jmp FAR PTR timer_fast
ASSUME CS:NOTHING
