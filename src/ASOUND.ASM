; ASOUND.CVL: CIV sound driver (AdLib, Sound Blaster Pro, Pro AudioSpectrum), recovered source.
; Built by MASM 5.10 and LINK 5.01 into the original byte for byte.
.8086
CODE SEGMENT BYTE PUBLIC 'CODE'
CODE ENDS
DATA SEGMENT PARA PUBLIC 'DATA'
DATA ENDS
BSS SEGMENT BYTE PUBLIC 'BSS'
BSS ENDS
DGROUP GROUP DATA, BSS

CODE SEGMENT
ASSUME CS:CODE, DS:NOTHING, ES:NOTHING
; A three-byte JMP to a nearby label (MASM 5.10 would make it short).
JMP_NEAR MACRO target
    DB 0E9H
    DW target - $ - 2
ENDM
header_pad DB 16 DUP (0)
start:                                  ; DOS entry (never run: loaded as an overlay)
driver_id DB 'RLND Cvlzatn12-03-91',0,0,0,0 ; Name and date.
DW SEG CODE                     ; 28H: segment of the slot routines
DW SEG DATA                     ; 2AH: data paragraph (CIV may append a file)
DW 076A0H ; bytes of data
DW 100                            ; 2EH: first slot
DW 11                           ; 30H: slot count
DW OFFSET sound_init
DW OFFSET sound_command
DW OFFSET sound_shutdown
DW OFFSET sound_tick
DW OFFSET sound_fast_tick
DW OFFSET sound_quiet_ticks
DW OFFSET sound_unused
DW OFFSET sound_unused
DW OFFSET sound_unused
DW OFFSET sound_unused
DW OFFSET sound_unused

; Set the playing voice's operator levels from its volume.
voice_level:
    push bp
    mov bp,sp
    DB 081H,0ECH,0CH,00H ; sub sp,0xc
    push di
    push si
ASSUME DS:DGROUP
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    mov bx,073C6H
    add bx,ax
    mov al,[bx+01H]
    cbw
    mov bx,ax
    mov al,operator_regs[bx]
    cbw
    add ax,040H
    mov si,ax
    mov bx,si
    mov al,opl_shadow[bx]
    and ax,0C0H
    mov di,ax
    mov bx,channel_now
    mov al,[bx+06H]
    and ax,0FFH
    mov bx,channel_now
    mov [bp-0CH],ax
    mov al,[bx+01DH]
    cbw
    mov cx,[bp-0CH]
    add cx,ax
    mov [bp-0AH],cx
    cmp WORD PTR card_found,01H
    jg SHORT voice_level_next
    jmp NEAR PTR voice_level_next6
voice_level_next:
    mov ax,[bp-0AH]
    mov bx,channel_now
    mov [bp-0CH],ax
    mov al,[bx+0DH]
    cbw
    mov bx,ax
    mov al,volume_levels[bx]
    and ax,0FFH
    mov cx,[bp-0CH]
    sub cx,ax
    mov [bp-06H],cx
    cmp WORD PTR [bp-06H],00H
    jnl SHORT voice_level_next2
    mov WORD PTR [bp-06H],00H
voice_level_next2:
    cmp WORD PTR [bp-06H],03FH
    jng SHORT voice_level_next3
    mov WORD PTR [bp-06H],03FH
voice_level_next3:
    mov ax,03FH
    sub ax,[bp-06H]
    or ax,di
    mov [bp-06H],ax
    mov ax,[bp-0AH]
    mov bx,channel_now
    mov [bp-0CH],ax
    mov al,[bx+0DH]
    cbw
    mov bx,07FH
    sub bx,ax
    mov al,volume_levels[bx]
    and ax,0FFH
    mov cx,[bp-0CH]
    sub cx,ax
    mov [bp-08H],cx
    cmp WORD PTR [bp-08H],00H
    jnl SHORT voice_level_next4
    mov WORD PTR [bp-08H],00H
voice_level_next4:
    cmp WORD PTR [bp-08H],03FH
    jng SHORT voice_level_next5
    mov WORD PTR [bp-08H],03FH
voice_level_next5:
    mov ax,03FH
    sub ax,[bp-08H]
    or ax,di
    mov [bp-08H],ax
    push WORD PTR [bp-06H]
    mov ax,si
    push ax
    mov ax,00H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    push WORD PTR [bp-08H]
    mov ax,si
    push ax
    mov ax,02H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    jmp SHORT voice_level_done
voice_level_next6:
    cmp WORD PTR [bp-0AH],00H
    jnl SHORT voice_level_next7
    mov WORD PTR [bp-0AH],00H
voice_level_next7:
    cmp WORD PTR [bp-0AH],03FH
    jng SHORT voice_level_next8
    mov WORD PTR [bp-0AH],03FH
voice_level_next8:
    mov ax,03FH
    sub ax,[bp-0AH]
    or ax,di
    mov [bp-06H],ax
    push ax
    mov ax,si
    push ax
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
voice_level_done:
    pop si
    pop di
    mov sp,bp
    pop bp
    ret
DB 090H

; Set the playing voice's frequency from its note.
voice_pitch:
    push bp
    mov bp,sp
    DB 081H,0ECH,0CH,00H ; sub sp,0xc
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    add ax,0A0H
    mov [bp-02H],ax
    mov bx,channel_now
    mov al,[bx+04H]
    and ax,0FFH
    mov cx,0CH
    sub dx,dx
    div cx
    mov [bp-08H],dl
    mov bx,channel_now
    mov al,[bx+04H]
    and ax,0FFH
    mov cx,0CH
    sub dx,dx
    div cx
    mov [bp-0AH],al
    mov bx,channel_now
    mov al,[bx+01CH]
    cbw
    mov [bp-0CH],ax
    mov al,[bp-08H]
    and ax,0FFH
    mov bx,ax
    shl bx,1
    mov ax,[bp-0CH]
    add ax,WORD PTR fnumbers[bx]
    mov [bp-04H],ax
    and ax,0FFH
    push ax
    mov ax,[bp-02H]
    push ax
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    add WORD PTR [bp-02H],010H
    mov bx,[bp-02H]
    mov al,opl_shadow[bx]
    and ax,020H
    mov [bp-06H],al
    mov cx,08H
    mov ax,[bp-04H]
    shr ax,cl
    mov [bp-0CH],ax
    mov al,[bp-0AH]
    and ax,0FFH
    shl ax,1
    shl ax,1
    mov cx,[bp-0CH]
    or cx,ax
    or [bp-06H],cl
    mov al,[bp-06H]
    and ax,0FFH
    push ax
    mov ax,[bp-02H]
    push ax
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    mov sp,bp
    pop bp
    ret

; Release the playing voice's note.
voice_key_off:
    push bp
    mov bp,sp
    DB 081H,0ECH,0AH,00H ; sub sp,0xa
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    add ax,0A0H
    mov [bp-02H],ax
    add ax,010H
    mov [bp-04H],ax
    mov bx,ax
    mov al,opl_shadow[bx]
    and ax,01FH
    mov cx,08H
    shl ax,cl
    mov [bp-06H],ax
    mov bx,[bp-02H]
    mov al,opl_shadow[bx]
    and ax,0FFH
    mov bx,channel_now
    mov [bp-0AH],ax
    mov al,[bx+01H]
    cbw
    mov cx,[bp-0AH]
    add cx,ax
    add [bp-06H],cx
    mov ax,[bp-06H]
    and ax,0FFH
    push ax
    mov ax,[bp-02H]
    push ax
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    mov bx,[bp-04H]
    mov al,opl_shadow[bx]
    and ax,020H
    mov [bp-08H],al
    mov cx,08H
    mov ax,[bp-06H]
    shr ax,cl
    or [bp-08H],al
    mov al,[bp-08H]
    and ax,0FFH
    push ax
    mov ax,[bp-04H]
    push ax
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    mov sp,bp
    pop bp
    ret

; Note timer expired: release the note.
voice_release:
    push bp
    mov bp,sp
    DB 081H,0ECH,02H,00H ; sub sp,0x2
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    add ax,0B0H
    mov [bp-02H],ax
    mov bx,ax
    mov al,opl_shadow[bx]
    and ax,0DFH
    push ax
    push bx
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    mov sp,bp
    pop bp
    ret
DB 090H

; Start the playing voice's next note.
voice_note:
    push bp
    mov bp,sp
    DB 081H,0ECH,04H,00H ; sub sp,0x4
    push di
    push si
    call NEAR PTR voice_level
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    mov bx,ax
    shl bx,1
    shl bx,1
    shl bx,1
    mov al,voice_instrument[bx]
    and ax,0FFH
    and ax,ax
    jnz SHORT voice_note_next
    jmp NEAR PTR voice_note_next9
voice_note_next:
    DB 08AH,06H,034H,075H ; mov al,[0x7534]
    cmp rhythm_mode,al
    jnz SHORT voice_note_next2
    mov BYTE PTR note_pending,00H
voice_note_next2:
    DB 08AH,06H,035H,075H ; mov al,[0x7535]
    cmp rhythm_mode,al
    jnz SHORT voice_note_next3
    mov BYTE PTR note_pending,01H
voice_note_next3:
    DB 08AH,06H,033H,075H ; mov al,[0x7533]
    and ax,0FFH
    and ax,ax
    jz SHORT voice_note_next4
    jmp NEAR PTR voice_note_next6
voice_note_next4:
    mov BYTE PTR note_pending,01H
    DB 08AH,06H,0EEH,074H ; mov al,[0x74ee]
    and ax,0FFH
    and ax,ax
    jz SHORT voice_note_next5
    DB 08AH,06H,034H,075H ; mov al,[0x7534]
    and ax,0FFH
    add ax,0B0H
    mov [bp-02H],ax
    mov bx,ax
    mov al,opl_shadow[bx]
    and ax,0DFH
    push ax
    push bx
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
voice_note_next5:
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    DB 088H,06H,034H,075H ; mov [0x7534],al
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    mov bx,ax
    shl bx,1
    shl bx,1
    shl bx,1
    mov al,voice_instrument[bx]
    DB 088H,06H,0EEH,074H ; mov [0x74ee],al
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bx+02H]
    DB 089H,06H,02EH,075H ; mov [0x752e],ax
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bx+04H]
    DB 089H,06H,012H,074H ; mov [0x7412],ax
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bx+06H]
    DB 089H,06H,016H,074H ; mov [0x7416],ax
    call NEAR PTR timer_fast
    jmp NEAR PTR voice_note_next8
DB 090H
voice_note_next6:
    mov BYTE PTR note_pending,00H
    DB 08AH,06H,0EEH,074H ; mov al,[0x74ee]
    and ax,0FFH
    and ax,ax
    jz SHORT voice_note_next7
    DB 08AH,06H,035H,075H ; mov al,[0x7535]
    and ax,0FFH
    add ax,0B0H
    mov [bp-02H],ax
    mov bx,ax
    mov al,opl_shadow[bx]
    and ax,0DFH
    push ax
    push bx
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
voice_note_next7:
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    DB 088H,06H,035H,075H ; mov [0x7535],al
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    mov bx,ax
    shl bx,1
    shl bx,1
    shl bx,1
    mov al,voice_instrument[bx]
    DB 088H,06H,0EH,075H ; mov [0x750e],al
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bx+02H]
    DB 089H,06H,030H,075H ; mov [0x7530],ax
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bx+04H]
    DB 089H,06H,014H,074H ; mov [0x7414],ax
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bx+06H]
    DB 089H,06H,018H,074H ; mov [0x7418],ax
    call NEAR PTR timer_fast
voice_note_next8:
    jmp voice_note_done
voice_note_next9:
    call NEAR PTR voice_pitch
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    add ax,0B0H
    mov [bp-02H],ax
    mov bx,ax
    mov al,opl_shadow[bx]
    and ax,0FFH
    or ax,020H
    push ax
    push bx
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
voice_note_done:
    pop si
    pop di
    mov sp,bp
    pop bp
    ret

; Write the current instrument's operator settings.
instrument_load:
    push bp
    mov bp,sp
    DB 081H,0ECH,04H,00H ; sub sp,0x4
    push di
    push si
    DB 08BH,06H,090H,076H ; mov ax,[0x7690]
    add ax,040H
    mov di,ax
    mov ax,03FH
    push ax
    push di
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08AH,06H,0F3H,075H ; mov al,[0x75f3]
    and ax,03FH
    mov si,ax
    DB 08AH,06H,0C0H,073H ; mov al,[0x73c0]
    and ax,0FFH
    and ax,ax
    jz SHORT instrument_load_next
    mov ax,080H
    jmp SHORT instrument_load_next2
instrument_load_next:
    mov ax,00H
instrument_load_next2:
    or si,ax
    DB 08AH,06H,0C2H,073H ; mov al,[0x73c2]
    and ax,0FFH
    and ax,ax
    jz SHORT instrument_load_next3
    mov ax,040H
    jmp instrument_load_next4
instrument_load_next3:
    mov ax,00H
instrument_load_next4:
    or si,ax
    mov ax,si
    push ax
    mov ax,0BDH
    push ax
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08AH,06H,0C4H,073H ; mov al,[0x73c4]
    and ax,0FFH
    and ax,ax
    jz SHORT instrument_load_next5
    mov ax,040H
    jmp SHORT instrument_load_next6
instrument_load_next5:
    mov ax,00H
instrument_load_next6:
    push ax
    mov ax,08H
    push ax
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08BH,06H,08EH,076H ; mov ax,[0x768e]
    add ax,0C0H
    mov di,ax
    mov bx,instrument_now
    mov al,[bx+0AH]
    and ax,0FFH
    shl ax,1
    mov si,ax
    mov bx,instrument_now
    mov al,[bx+0DH]
    and ax,0FFH
    and ax,ax
    jz SHORT instrument_load_next7
    mov ax,00H
    jmp SHORT instrument_load_next8
instrument_load_next7:
    mov ax,01H
instrument_load_next8:
    or si,ax
    mov ax,si
    push ax
    push di
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08BH,06H,090H,076H ; mov ax,[0x7690]
    add ax,060H
    mov di,ax
    mov bx,instrument_now
    mov al,[bx]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    shl ax,1
    mov si,ax
    mov bx,instrument_now
    mov al,[bx+01H]
    and ax,0FH
    or si,ax
    mov ax,si
    push ax
    push di
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08BH,06H,090H,076H ; mov ax,[0x7690]
    add ax,080H
    mov di,ax
    mov bx,instrument_now
    mov al,[bx+02H]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    shl ax,1
    mov si,ax
    mov bx,instrument_now
    mov al,[bx+03H]
    and ax,0FH
    or si,ax
    mov ax,si
    push ax
    push di
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08BH,06H,090H,076H ; mov ax,[0x7690]
    add ax,020H
    mov di,ax
    mov bx,instrument_now
    mov al,[bx+0BH]
    and ax,0FFH
    and ax,ax
    jz SHORT instrument_load_next9
    mov ax,080H
    jmp SHORT instrument_load_next10
instrument_load_next9:
    mov ax,00H
instrument_load_next10:
    mov si,ax
    mov bx,instrument_now
    mov al,[bx+0CH]
    and ax,0FFH
    and ax,ax
    jz SHORT instrument_load_next11
    mov ax,040H
    jmp SHORT instrument_load_next12
instrument_load_next11:
    mov ax,00H
instrument_load_next12:
    add si,ax
    mov bx,instrument_now
    mov al,[bx+04H]
    and ax,0FFH
    and ax,ax
    jz SHORT instrument_load_next13
    mov ax,020H
    jmp SHORT instrument_load_next14
instrument_load_next13:
    mov ax,00H
instrument_load_next14:
    add si,ax
    mov bx,instrument_now
    mov al,[bx+05H]
    and ax,0FFH
    and ax,ax
    jz SHORT instrument_load_next15
    mov ax,010H
    jmp SHORT instrument_load_next16
instrument_load_next15:
    mov ax,00H
instrument_load_next16:
    add si,ax
    mov bx,instrument_now
    mov al,[bx+09H]
    and ax,0FH
    add si,ax
    mov ax,si
    push ax
    push di
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08BH,06H,090H,076H ; mov ax,[0x7690]
    add ax,0E0H
    mov di,ax
    mov bx,instrument_now
    mov al,[bx+08H]
    and ax,03H
    push ax
    push di
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08BH,06H,090H,076H ; mov ax,[0x7690]
    add ax,040H
    mov di,ax
    mov bx,instrument_now
    mov al,[bx+06H]
    and ax,03FH
    mov cx,03FH
    sub cx,ax
    mov si,cx
    mov bx,instrument_now
    mov al,[bx+07H]
    and ax,0FFH
    mov cx,06H
    shl ax,cl
    or si,ax
    mov ax,si
    push ax
    push di
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    pop si
    pop di
    mov sp,bp
    pop bp
    ret
DB 090H

; Make CX the current instrument.
instrument_select:
    push bp
    mov bp,sp
    DB 081H,0ECH,02H,00H ; sub sp,0x2
    push di
    push si
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    add ax,0B0H
    DB 089H,06H,08EH,076H ; mov [0x768e],ax
    mov bx,ax
    mov al,opl_shadow[bx]
    and ax,0DFH
    push ax
    push bx
    mov ax,08H
    push ax
    call NEAR PTR opl_out
    add sp,06H
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    DB 089H,06H,08EH,076H ; mov [0x768e],ax
    mov al,[bp+04H]
    and ax,0FFH
    mov cx,02CH
    imul cx
    mov cx,0695AH
    add cx,ax
    mov instrument_now,cx
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    mov bx,ax
    shl bx,1
    mov al,voice_operators[bx]
    cbw
    mov bx,ax
    mov al,operator_regs[bx]
    cbw
    DB 089H,06H,090H,076H ; mov [0x7690],ax
    call NEAR PTR instrument_load
    mov bx,instrument_now
    mov al,[bx+0EH]
    mov [bp-02H],ax
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    mov bx,ax
    shl bx,1
    shl bx,1
    shl bx,1
    mov ax,[bp-02H]
    mov voice_instrument[bx],al
    mov bx,instrument_now
    mov ax,[bx+010H]
    mov [bp-02H],ax
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bp-02H]
    mov [bx+02H],ax
    mov bx,instrument_now
    mov ax,[bx+012H]
    mov [bp-02H],ax
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bp-02H]
    mov [bx+04H],ax
    mov bx,instrument_now
    mov ax,[bx+014H]
    mov [bp-02H],ax
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    shl ax,1
    shl ax,1
    mov bx,07636H
    add bx,ax
    mov ax,[bp-02H]
    mov [bx+06H],ax
    mov al,[bp+04H]
    and ax,0FFH
    mov cx,02CH
    imul cx
    mov cx,0695AH
    add cx,ax
    add cx,016H
    mov instrument_now,cx
    DB 08AH,06H,032H,075H ; mov al,[0x7532]
    and ax,0FFH
    shl ax,1
    mov bx,073C6H
    add bx,ax
    mov al,[bx+01H]
    cbw
    mov bx,ax
    mov al,operator_regs[bx]
    cbw
    DB 089H,06H,090H,076H ; mov [0x7690],ax
    call NEAR PTR instrument_load
    pop si
    pop di
    mov sp,bp
    pop bp
    ret

; Interpret one tick of the current channel's sequence.
channel_step:
    push bp
    mov bp,sp
    DB 081H,0ECH,0AH,00H ; sub sp,0xa
    push di
    push si
    mov bx,channel_now
    mov al,[bx]
    and ax,0FFH
    and ax,ax
    jnz SHORT channel_step_next
    jmp NEAR PTR channel_step_unused_next26
channel_step_next:
    mov bx,channel_now
    mov al,[bx+08H]
    and ax,0FFH
    and ax,ax
    jz SHORT channel_step_next2
    mov bx,channel_now
    dec BYTE PTR [bx+08H]
    mov al,[bx+08H]
    and ax,0FFH
    and ax,ax
    jnz SHORT channel_step_next2
    call NEAR PTR voice_release
channel_step_next2:
    mov bx,channel_now
    dec BYTE PTR [bx]
    mov al,[bx]
    and ax,0FFH
    and ax,ax
    jna SHORT channel_step_loop
    jmp NEAR PTR channel_step_unused_next18
channel_step_loop:
    mov bx,channel_now
    mov si,[bx+010H]
    mov bx,[bx+010H]
    mov al,[bx]
    cbw
    and ax,080H
    and ax,ax
    jnz SHORT channel_step_next3
    jmp NEAR PTR channel_step_unused_next14
channel_step_next3:
    cmp BYTE PTR [si],0F2H
    jg SHORT channel_step_next4
    jmp NEAR PTR channel_step_unused_next14
channel_step_next4:
    mov BYTE PTR [bp-04H],00H
    mov al,[si]
    cbw
    jmp NEAR PTR channel_step_unused_next
channel_step_loop2:
    mov bx,channel_now
    cmp WORD PTR [bx+016H],00H
    jz SHORT channel_step_next5
    JMP_NEAR channel_step_next8
channel_step_next5:
    inc si
    mov bx,si
    cmp BYTE PTR [bx],00H
    jnz SHORT channel_step_next6
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov bx,channel_now
    mov ax,[bx+010H]
    mov bx,channel_now
    mov [bx+012H],ax
    mov bx,channel_now
    mov WORD PTR [bx+016H],00H
    jmp SHORT channel_step_next7
channel_step_next6:
    mov al,[si]
    cbw
    mov bx,channel_now
    mov [bx+016H],ax
    mov bx,channel_now
    mov ax,[bx+012H]
    mov bx,channel_now
    mov [bx+010H],ax
channel_step_next7:
    jmp SHORT channel_step_next10
channel_step_next8:
    mov bx,channel_now
    dec WORD PTR [bx+016H]
    mov bx,channel_now
    cmp WORD PTR [bx+016H],00H
    jnz SHORT channel_step_next9
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov bx,channel_now
    mov ax,[bx+010H]
    mov bx,channel_now
    mov [bx+012H],ax
    jmp channel_step_next10
channel_step_next9:
    mov bx,channel_now
    mov ax,[bx+012H]
    mov bx,channel_now
    mov [bx+010H],ax
channel_step_next10:
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop3:
    mov bx,channel_now
    cmp WORD PTR [bx+018H],00H
    jz SHORT channel_step_next11
    JMP_NEAR channel_step_next14
channel_step_next11:
    inc si
    mov bx,si
    cmp BYTE PTR [bx],00H
    jnz SHORT channel_step_next12
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov bx,channel_now
    mov ax,[bx+010H]
    mov bx,channel_now
    mov [bx+014H],ax
    mov bx,channel_now
    mov ax,[bx+010H]
    mov bx,channel_now
    mov [bx+012H],ax
    mov bx,channel_now
    mov WORD PTR [bx+016H],00H
    mov bx,channel_now
    mov WORD PTR [bx+018H],00H
    jmp channel_step_next13
channel_step_next12:
    mov al,[si]
    cbw
    mov bx,channel_now
    mov [bx+018H],ax
    mov bx,channel_now
    mov ax,[bx+014H]
    mov bx,channel_now
    mov [bx+010H],ax
    mov bx,channel_now
    mov ax,[bx+014H]
    mov bx,channel_now
    mov [bx+012H],ax
channel_step_next13:
    JMP_NEAR channel_step_next17
DB 090H
channel_step_next14:
    mov bx,channel_now
    dec WORD PTR [bx+018H]
    mov bx,channel_now
    cmp WORD PTR [bx+018H],00H
    jnz SHORT channel_step_next15
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov bx,channel_now
    mov ax,[bx+010H]
    mov bx,channel_now
    mov [bx+014H],ax
    mov bx,channel_now
    mov ax,[bx+010H]
    mov bx,channel_now
    mov [bx+012H],ax
    jmp channel_step_next16
channel_step_next15:
    mov bx,channel_now
    mov ax,[bx+014H]
    mov bx,channel_now
    mov [bx+010H],ax
channel_step_next16:
    mov bx,channel_now
    mov ax,[bx+014H]
    mov bx,channel_now
    mov [bx+012H],ax
channel_step_next17:
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop4:
    mov bx,channel_now
    mov ax,[bx+01AH]
    mov bx,channel_now
    mov [bx+0EH],ax
    mov bx,channel_now
    mov ax,[bx+01AH]
    mov bx,channel_now
    mov [bx+010H],ax
    mov bx,channel_now
    mov ax,[bx+01AH]
    mov bx,channel_now
    mov [bx+012H],ax
    mov bx,channel_now
    mov ax,[bx+01AH]
    mov bx,channel_now
    mov [bx+014H],ax
    mov bx,channel_now
    mov BYTE PTR [bx+01H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+02H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+03H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+06H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+07H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+09H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+0BH],00H
    mov bx,channel_now
    mov WORD PTR [bx+016H],00H
    mov bx,channel_now
    mov WORD PTR [bx+018H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+01CH],00H
    mov bx,channel_now
    mov BYTE PTR [bx+01DH],00H
    mov bx,channel_now
    mov BYTE PTR [bx+0DH],040H
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop5:
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+05H],al
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov bx,channel_now
    mov al,[bx+05H]
    and ax,0FFH
    push ax
    call NEAR PTR instrument_select
    add sp,02H
    jmp NEAR PTR channel_step_unused_next13
channel_step_loop6:
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+07H],al
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop7:
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+01H],al
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop8:
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+01DH],al
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov BYTE PTR [bp-04H],01H
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop9:
    inc si
    mov bx,si
    mov al,[bx]
    sar al,1
    mov bx,channel_now
    mov [bx+06H],al
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov BYTE PTR [bp-04H],01H
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop10:
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+0AH],al
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+02H],al
    mov bx,channel_now
    mov BYTE PTR [bx+09H],01H
    mov bx,channel_now
    add WORD PTR [bx+010H],03H
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop11:
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+01CH],al
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop12:
    inc si
    mov bx,si
    mov al,[bx]
    cbw
    mov [bp-08H],ax
    inc si
    call FAR PTR sound_random
    mov ax,[bp-08H]
    dec ax
    and ax,random_state
    mov [bp-06H],ax
    mov bx,si
    add bx,ax
    mov al,[bx]
    mov bx,si
    add bx,[bp-08H]
    inc bx
    mov di,si
    add di,[bp-08H]
    mov [bp-0AH],ax
    mov al,[di]
    cbw
    add bx,ax
    mov ax,[bp-0AH]
    mov [bx],al
    mov ax,[bp-08H]
    add ax,03H
    mov bx,channel_now
    add [bx+010H],ax
    jmp NEAR PTR channel_step_unused_next13
channel_step_loop13:
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+0DH],al
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov BYTE PTR [bp-04H],01H
    jmp NEAR PTR channel_step_unused_next13
DB 090H
channel_step_loop14:
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+0CH],al
    inc si
    mov bx,si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+03H],al
    mov bx,channel_now
    mov BYTE PTR [bx+0BH],01H
    mov bx,channel_now
    add WORD PTR [bx+010H],03H
    JMP_NEAR channel_step_unused_next13
DB 090H

; Unreachable jump.
channel_step_unused:
    JMP_NEAR channel_step_unused_next13
DB 090H
channel_step_unused_next:
    cmp ax,0FFF3H
    jz SHORT channel_step_loop14
    cmp ax,0FFF4H
    jnz SHORT channel_step_unused_next2
    JMP_NEAR channel_step_loop13
channel_step_unused_next2:
    cmp ax,0FFF5H
    jnz SHORT channel_step_unused_next3
    jmp NEAR PTR channel_step_loop8
channel_step_unused_next3:
    cmp ax,0FFF6H
    jnz SHORT channel_step_unused_next4
    jmp NEAR PTR channel_step_loop12
channel_step_unused_next4:
    cmp ax,0FFF7H
    jnz SHORT channel_step_unused_next5
    jmp NEAR PTR channel_step_loop11
channel_step_unused_next5:
    cmp ax,0FFF8H
    jnz SHORT channel_step_unused_next6
    jmp NEAR PTR channel_step_loop10
channel_step_unused_next6:
    cmp ax,0FFF9H
    jnz SHORT channel_step_unused_next7
    jmp NEAR PTR channel_step_loop9
channel_step_unused_next7:
    cmp ax,0FFFAH
    jnz SHORT channel_step_unused_next8
    jmp NEAR PTR channel_step_loop7
channel_step_unused_next8:
    cmp ax,0FFFBH
    jnz SHORT channel_step_unused_next9
    jmp NEAR PTR channel_step_loop6
channel_step_unused_next9:
    cmp ax,0FFFCH
    jnz SHORT channel_step_unused_next10
    jmp NEAR PTR channel_step_loop5
channel_step_unused_next10:
    cmp ax,0FFFDH
    jnz SHORT channel_step_unused_next11
    jmp NEAR PTR channel_step_loop4
channel_step_unused_next11:
    cmp ax,0FFFEH
    jnz SHORT channel_step_unused_next12
    jmp NEAR PTR channel_step_loop3
channel_step_unused_next12:
    cmp ax,0FFFFH
    jnz SHORT channel_step_unused_next13
    jmp NEAR PTR channel_step_loop2
channel_step_unused_next13:
    jmp NEAR PTR channel_step_loop
channel_step_unused_next14:
    cmp BYTE PTR [bp-04H],00H
    jz SHORT channel_step_unused_next15
    call NEAR PTR voice_level
channel_step_unused_next15:
    mov bx,channel_now
    mov si,[bx+010H]
    mov bx,si
    inc si
    mov al,[bx]
    mov bx,channel_now
    mov [bx+04H],al
    mov bx,si
    inc si
    mov al,[bx]
    mov bx,channel_now
    mov [bx],al
    mov bx,channel_now
    add WORD PTR [bx+010H],02H
    mov bx,channel_now
    mov al,[bx+04H]
    and ax,0FFH
    and ax,ax
    jz SHORT channel_step_unused_next16
    mov bx,channel_now
    mov al,[bx]
    and ax,0FFH
    and ax,ax
    jnz SHORT channel_step_unused_next17
channel_step_unused_next16:
    call NEAR PTR voice_release
    jmp channel_step_unused_next18
channel_step_unused_next17:
    mov bx,channel_now
    mov al,[bx]
    and ax,0FFH
    mov bx,channel_now
    mov [bp-0AH],ax
    mov al,[bx+07H]
    cbw
    mov cx,[bp-0AH]
    sub cx,ax
    mov bx,channel_now
    mov [bx+08H],cl
    call NEAR PTR voice_note
channel_step_unused_next18:
    mov bx,channel_now
    cmp BYTE PTR [bx+01H],00H
    jz SHORT channel_step_unused_next19
    call NEAR PTR voice_key_off
channel_step_unused_next19:
    mov BYTE PTR [bp-04H],00H
    mov bx,channel_now
    mov al,[bx+09H]
    and ax,0FFH
    and ax,ax
    jnz SHORT channel_step_unused_next20
    mov bx,channel_now
    mov al,[bx+0BH]
    and ax,0FFH
    and ax,ax
    jnz SHORT channel_step_unused_next20
    jmp NEAR PTR channel_step_unused_next26
channel_step_unused_next20:
    mov bx,channel_now
    dec BYTE PTR [bx+09H]
    mov al,[bx+09H]
    and ax,0FFH
    and ax,ax
    jz SHORT channel_step_unused_next21
    JMP_NEAR channel_step_unused_next24
channel_step_unused_next21:
    mov bx,channel_now
    mov al,[bx+0AH]
    mov bx,channel_now
    mov [bx+09H],al
    mov bx,channel_now
    cmp BYTE PTR [bx+02H],00H
    jnz SHORT channel_step_unused_next22
    JMP_NEAR channel_step_unused_next24
channel_step_unused_next22:
    mov bx,channel_now
    mov al,[bx+02H]
    mov bx,channel_now
    add [bx+01DH],al
    mov BYTE PTR [bp-04H],01H
    mov bx,channel_now
    cmp BYTE PTR [bx+01DH],00H
    jg SHORT channel_step_unused_next23
    mov bx,channel_now
    mov BYTE PTR [bx+09H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+01DH],00H
    mov bx,channel_now
    mov BYTE PTR [bx+06H],00H
channel_step_unused_next23:
    mov bx,channel_now
    cmp BYTE PTR [bx+01DH],03FH
    jl SHORT channel_step_unused_next24
    mov bx,channel_now
    mov BYTE PTR [bx+09H],00H
    mov bx,channel_now
    mov BYTE PTR [bx+01DH],03FH
    mov bx,channel_now
    mov BYTE PTR [bx+06H],00H
channel_step_unused_next24:
    mov bx,channel_now
    dec BYTE PTR [bx+0BH]
    mov al,[bx+0BH]
    and ax,0FFH
    and ax,ax
    jnz SHORT channel_step_unused_next25
    mov bx,channel_now
    mov al,[bx+0CH]
    mov bx,channel_now
    mov [bx+0BH],al
    mov bx,channel_now
    cmp BYTE PTR [bx+03H],00H
    jz SHORT channel_step_unused_next25
    mov bx,channel_now
    mov al,[bx+03H]
    mov bx,channel_now
    add [bx+0DH],al
    mov BYTE PTR [bp-04H],01H
channel_step_unused_next25:
    cmp BYTE PTR [bp-04H],00H
    jz SHORT channel_step_unused_next26
    call NEAR PTR voice_level
channel_step_unused_next26:
    inc BYTE PTR rhythm_mode
    pop si
    pop di
    mov sp,bp
    pop bp
    ret

; Step all nine channels.
channels_step:
    mov BYTE PTR rhythm_mode,00H
    mov WORD PTR channel_now,0741CH
    call NEAR PTR channel_step
    mov WORD PTR channel_now,0743AH
    call NEAR PTR channel_step
    mov WORD PTR channel_now,07458H
    call NEAR PTR channel_step
    mov WORD PTR channel_now,07476H
    call NEAR PTR channel_step
    mov WORD PTR channel_now,07494H
    call NEAR PTR channel_step
    mov WORD PTR channel_now,074B2H
    call NEAR PTR channel_step
    mov WORD PTR channel_now,074D0H
    call NEAR PTR channel_step
    mov WORD PTR channel_now,074F0H
    call NEAR PTR channel_step
    mov WORD PTR channel_now,07510H
    call NEAR PTR channel_step
    ret
DB 00H,00H,00H,00H,00H,00H,00H,00H,00H
quiet_ticks DW 00H                       ; Ticks since the last sound command.
DB 00H,00H,00H,00H,00H,00H,00H,00H
card_port DW 0220H                       ; Sound Blaster Pro base port.
DB 07H
card_kind DW 00H                         ; 0 AdLib, 2 Sound Blaster Pro, 3 Pro AudioSpectrum.

; Find an AdLib, Sound Blaster Pro or Pro AudioSpectrum; AX 0 when found.
card_detect:
    mov WORD PTR CS:card_kind,00H
    mov WORD PTR card_found,00H
    mov dx,0B8AH
    mov al,00H
    out dx,al
    mov dx,0388H
    mov al,04H
    mov bl,060H
    call NEAR PTR mixer_write_dx
    mov dx,0388H
    mov bl,080H
    call NEAR PTR mixer_write_dx
    mov dx,0388H
    in al,dx
    push ax
    mov dx,0388H
    mov al,02H
    mov bl,0FFH
    call NEAR PTR mixer_write_dx
    mov dx,0388H
    mov al,04H
    mov bl,021H
    call NEAR PTR mixer_write_dx
    mov cx,0C8H
    mov dx,0388H
card_detect_loop:
    in al,dx
    loop SHORT card_detect_loop
    push ax
    mov dx,0388H
    mov al,04H
    mov bl,060H
    call NEAR PTR mixer_write_dx
    mov dx,0388H
    mov bl,080H
    call NEAR PTR mixer_write_dx
    pop ax
    pop bx
    mov dx,00H
    and al,0E0H
    cmp al,0C0H
    jnz SHORT card_detect_next
    and bl,0E0H
    cmp bl,00H
    jz SHORT card_detect_next2
card_detect_next:
    mov ax,041H
    jmp NEAR PTR card_detect_next7
card_detect_next2:
    mov ah,04H
    mov al,026H
    call NEAR PTR mixer_read
    push ax
    mov ah,04H
    mov al,026H
    mov bl,0AAH
    call NEAR PTR mixer_write
    call NEAR PTR mixer_read
    and al,0EEH
    cmp al,0AAH
    jnz SHORT card_detect_next3
    mov ah,04H
    mov al,026H
    mov bl,044H
    call NEAR PTR mixer_write
    call NEAR PTR mixer_read
    and al,0EEH
    cmp al,044H
    jnz SHORT card_detect_next3
    mov WORD PTR CS:card_kind,02H
    mov WORD PTR card_found,02H
card_detect_next3:
    mov ah,04H
    mov al,026H
    pop bx
    call NEAR PTR mixer_write
    cli
    mov dx,0B8BH
    mov al,0AH
    out dx,al
    jmp SHORT card_detect_next4
card_detect_next4:
    in al,dx
    and al,01FH
    cmp al,0AH
    jnz SHORT card_detect_next6
    mov al,015H
    out dx,al
    jmp SHORT card_detect_next5
card_detect_next5:
    in al,dx
    and al,01FH
    cmp al,015H
    jnz SHORT card_detect_next6
    mov WORD PTR CS:card_kind,03H
    mov WORD PTR card_found,03H
    mov WORD PTR CS:card_port,0388H
    mov dx,0B8AH
    mov al,039H
    out dx,al
    mov dx,0F8AH
    mov al,09H
    out dx,al
    mov dx,0B8BH
card_detect_next6:
    mov al,00H
    out dx,al
    sti
    mov ax,00H
card_detect_next7:
    push ax
    call NEAR PTR sound_reset
    pop ax
    ret

; Zero four bytes at BX.
clear_four:
    mov [bx],al
    mov [bx+01H],al
    mov [bx+02H],al
    mov [bx+03H],al
    ret

; 0: silence everything.
sound_reset:
    xor ax,ax
    push WORD PTR update_lock
    mov WORD PTR update_lock,0FFFFH
    mov BYTE PTR interlude_on,00H
    lea bx,DGROUP:channel1
    call NEAR PTR clear_four
    lea bx,DGROUP:channel2
    call NEAR PTR clear_four
    lea bx,DGROUP:channel3
    call NEAR PTR clear_four
    lea bx,DGROUP:channel4
    call NEAR PTR clear_four
    lea bx,DGROUP:channel5
    call NEAR PTR clear_four
    lea bx,DGROUP:channel6
    call NEAR PTR clear_four
    lea bx,DGROUP:channel7
    call NEAR PTR clear_four
    lea bx,DGROUP:channel8
    call NEAR PTR clear_four
    lea bx,DGROUP:channel9
    call NEAR PTR clear_four
    mov BYTE PTR sweep_a_count,al
    mov BYTE PTR sweep_b_count,al
    mov sweep_a_mask,ax
    mov sweep_b_mask,ax
    mov sweep_a_freq,ax
    mov sweep_b_freq,ax
    mov sweep_a_step,ax
    mov sweep_b_step,ax
    mov bx,03FH
    mov ax,04FH
sound_reset_loop:
    push ax
    mov ah,08H
    call NEAR PTR opl_write
    pop ax
    dec ax
    cmp ax,040H
    jnc SHORT sound_reset_loop
    mov bx,00H
    mov ax,0FFH
sound_reset_loop2:
    push ax
    mov ah,08H
    call NEAR PTR opl_write
    pop ax
    dec ax
    cmp ax,060H
    jnc SHORT sound_reset_loop2
    mov ax,03FH
sound_reset_loop3:
    push ax
    mov ah,08H
    call NEAR PTR opl_write
    pop ax
    dec ax
    jnz SHORT sound_reset_loop3
    mov ah,08H
    mov al,01H
    mov bl,020H
    call NEAR PTR opl_write
    pop WORD PTR update_lock
    ret

; Unreferenced variant of the reset.
sound_reset_unused:
    mov al,080H
    mov bl,0EH
sound_reset_unused_loop:
    push ax
    mov ah,08H
    call NEAR PTR opl_write
    pop ax
    inc ax
    cmp al,096H
    jnz SHORT sound_reset_unused_loop
    xor ax,ax
    push WORD PTR update_lock
    mov WORD PTR update_lock,0FFFFH
    lea bx,DGROUP:channel1
    mov BYTE PTR sweep_a_count,al
    mov BYTE PTR sweep_b_count,al
    mov sweep_a_mask,ax
    mov sweep_b_mask,ax
    mov sweep_a_freq,ax
    mov sweep_b_freq,ax
    mov sweep_a_step,ax
    mov sweep_b_step,ax
    lea cx,DGROUP:sound_reset_unused_part1
    lea bx,DGROUP:channel1
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    lea bx,DGROUP:channel2
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    lea bx,DGROUP:channel3
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    lea bx,DGROUP:channel4
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    lea bx,DGROUP:channel5
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    lea bx,DGROUP:channel6
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    lea bx,DGROUP:channel7
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    lea bx,DGROUP:channel8
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    lea bx,DGROUP:channel9
    mov BYTE PTR [bx],01H
    mov [bx+010H],cx
    pop WORD PTR update_lock
    ret

; Set operator AL's level to silent.
operator_mute:
    mov ah,00H
    mov di,ax
    mov bl,opl_shadow[di]
    or bl,03FH
    mov ah,08H
    jmp opl_write

; Unreferenced: set an operator's level bits.
operator_level_unused:
    xor bh,03FH
    mov ah,00H
    mov di,ax
    mov bl,opl_shadow[di]
    and bl,0C0H
    or bl,bh
    mov ah,08H
    jmp opl_write

; Read mixer register AL (port base+AH).
mixer_read:
    mov dl,ah
    mov dh,00H
    add dx,CS:card_port
    push ax
    mov cx,01EH
mixer_read_loop:
    in al,dx
    loop SHORT mixer_read_loop
    pop ax
    out dx,al
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    inc dx
    in al,dx
    ret

; Write BL to mixer register AL (port DX).
mixer_write_dx:
    push ax
    jmp mixer_write_next

; Write BL to mixer register AL (port base+AH).
mixer_write:
    push ax
    mov dl,ah
    mov dh,00H
    add dx,CS:card_port
mixer_write_next:
    push ax
    mov cx,01EH
mixer_write_loop:
    in al,dx
    loop SHORT mixer_write_loop
    pop ax
    out dx,al
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    mov al,bl
    inc dx
    out dx,al
    pop ax
    ret

; C entry: opl_write(bank, register, value).
opl_out:
    push bp
    mov bp,sp
    mov dx,[bp+04H]
    mov ax,[bp+06H]
    mov bl,[bp+08H]
    mov ah,dl
    pop bp

; Write BL to OPL register AL (bank AH: 8 both chips on a Sound Blaster Pro); keeps a copy in opl_shadow.
opl_write:
    mov cx,ax
    push di
    mov ah,00H
    mov di,ax
    mov opl_shadow[di],bl
    pop di
    mov dx,0388H
    cmp WORD PTR card_found,02H
    jc SHORT opl_write_next2
    cmp ch,08H
    jnz SHORT opl_write_next
    mov dx,CS:card_port
    push ax
    mov cx,01EH
opl_write_loop:
    in al,dx
    loop SHORT opl_write_loop
    pop ax
    out dx,al
    inc dx
    inc dx
    out dx,al
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    dec dx
    mov al,bl
    out dx,al
    inc dx
    inc dx
    out dx,al
    jmp opl_write_next3
opl_write_next:
    mov dl,ch
    mov dh,00H
    add dx,CS:card_port
opl_write_next2:
    push ax
    mov cx,01EH
opl_write_loop2:
    in al,dx
    loop SHORT opl_write_loop2
    pop ax
    out dx,al
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    inc dx
    mov al,bl
    out dx,al
opl_write_next3:
    mov ax,cx
    ret

; Set voice AL's frequency to BX and key it on.
voice_frequency:
    push ax
    add al,0A0H
    mov ah,08H
    call NEAR PTR opl_write
    pop ax
    add al,0B0H
    mov ah,08H
    mov bl,bh
    or bl,020H
    call NEAR PTR opl_write
    ret

; x = (x + 9248H) rotated right 3; FAR, also used by the commands.
sound_random:
    mov ax,09248H
    add ax,random_state
    ror ax,1
    ror ax,1
    ror ax,1
    mov random_state,ax
    retf

; The 60 Hz step: sequencer and pitch sweeps.
music_tick:
    call FAR PTR sound_random
    cmp WORD PTR update_lock,-01H
    jnz SHORT music_tick_next
    jmp NEAR PTR music_tick_done
music_tick_next:
    cmp BYTE PTR interlude_on,00H
    jz SHORT music_tick_next2
    cmp BYTE PTR channel9,00H
    jnz SHORT music_tick_next2
    call NEAR PTR interlude_end
music_tick_next2:
    push es
    push si
    push di
    push bp
    inc WORD PTR CS:quiet_ticks
    call NEAR PTR channels_step
    pop bp
    pop di
    pop si
    pop es
    mov al,BYTE PTR sweep_b_count
    or al,BYTE PTR sweep_a_count
    jz SHORT music_tick_next6
    cmp BYTE PTR sweep_a_count,00H
    jz SHORT music_tick_next4
    mov ax,sweep_a_freq
    add ax,sweep_a_step
    mov sweep_a_freq,ax
    dec BYTE PTR sweep_a_count
    jnz SHORT music_tick_next4
    mov al,sweep_a_voice
    cmp BYTE PTR sweep_b_count,00H
    jz SHORT music_tick_next3
    cmp al,sweep_b_voice
    jz SHORT music_tick_next4
music_tick_next3:
    push ax
    add al,0A0H
    mov ah,08H
    mov bl,00H
    call NEAR PTR opl_write
    pop ax
    add al,0B0H
    mov ah,08H
    mov bl,00H
    call NEAR PTR opl_write
music_tick_next4:
    cmp BYTE PTR sweep_b_count,00H
    jz SHORT music_tick_done
    mov ax,sweep_b_freq
    add ax,sweep_b_step
    mov sweep_b_freq,ax
    dec BYTE PTR sweep_b_count
    jnz SHORT music_tick_done
    mov al,sweep_b_voice
    cmp BYTE PTR sweep_a_count,00H
    jnz SHORT music_tick_next5
    cmp al,sweep_a_voice
    jz SHORT music_tick_done
music_tick_next5:
    push ax
    add al,0A0H
    mov ah,08H
    mov bl,00H
    call NEAR PTR opl_write
    pop ax
    add al,0B0H
    mov ah,08H
    mov bl,00H
    call NEAR PTR opl_write
music_tick_done:
    ret
music_tick_next6:
    cmp WORD PTR rate_now,-01H
    jz SHORT music_tick_done2
    mov WORD PTR rate_now,0FFFFH
    mov WORD PTR rate_request,0FFFFH
music_tick_done2:
    ret

; Advance the two pitch sweeps (fast timer).
sweep_tick:
    mov bx,09248H
    add bx,random_state
    ror bx,1
    ror bx,1
    ror bx,1
    mov random_state,bx
    cmp BYTE PTR sweep_a_count,00H
    jz SHORT sweep_tick_next
    xor bx,-01H
    and bx,sweep_a_mask
    add bx,sweep_a_freq
    mov al,sweep_a_voice
    call NEAR PTR voice_frequency
sweep_tick_next:
    cmp BYTE PTR sweep_b_count,00H
    jz SHORT sweep_tick_done
    mov bx,random_state
    and bx,sweep_b_mask
    add bx,sweep_b_freq
    mov al,sweep_b_voice
    call NEAR PTR voice_frequency
sweep_tick_done:
    ret

; Ask CIV for the fast timer (sweeps running).
timer_fast:
    cmp WORD PTR rate_now,01H
    jz SHORT timer_fast_done
    mov WORD PTR rate_now,01H
    mov WORD PTR rate_request,01H
timer_fast_done:
    ret

; Start sequence CX on channel 1 (similarly 2..9).
start_channel1:
    lea bx,DGROUP:channel1
    jmp channel_start
start_channel2:
    lea bx,DGROUP:channel2
    jmp channel_start
start_channel3:
    lea bx,DGROUP:channel3
    jmp channel_start
start_channel4:
    lea bx,DGROUP:channel4
    jmp channel_start
start_channel5:
    lea bx,DGROUP:channel5
    jmp channel_start
start_channel6:
    lea bx,DGROUP:channel6
    jmp channel_start
start_channel7:
    lea bx,DGROUP:channel7
    jmp channel_start
start_channel8:
    lea bx,DGROUP:channel8
    jmp channel_start
start_channel9:
    lea bx,DGROUP:channel9

; Start sequence CX on the channel record at BX.
channel_start:
    push WORD PTR update_lock
    mov WORD PTR update_lock,0FFFFH
    mov [bx+0EH],cx
    mov [bx+010H],cx
    mov [bx+012H],cx
    mov [bx+014H],cx
    mov [bx+01AH],cx
    mov BYTE PTR [bx+0AH],0FFH
    mov BYTE PTR [bx+01H],00H
    mov BYTE PTR [bx+02H],00H
    mov BYTE PTR [bx+03H],00H
    mov BYTE PTR [bx+01CH],00H
    mov BYTE PTR [bx+01DH],00H
    mov BYTE PTR [bx+06H],00H
    mov BYTE PTR [bx+09H],00H
    mov BYTE PTR [bx+0BH],00H
    mov WORD PTR [bx+016H],00H
    mov WORD PTR [bx+018H],00H
    mov BYTE PTR [bx+07H],00H
    mov BYTE PTR [bx+0DH],040H
    mov BYTE PTR [bx],01H
    pop WORD PTR update_lock
    ret

; 1: send every channel to the rest sequence.
music_end:
    lea cx,DGROUP:sound_reset_unused_part1
    lea bx,DGROUP:channel1
    call NEAR PTR channel_rest
    lea bx,DGROUP:channel2
    call NEAR PTR channel_rest
    lea bx,DGROUP:channel3
    call NEAR PTR channel_rest
    lea bx,DGROUP:channel4
    call NEAR PTR channel_rest
    lea bx,DGROUP:channel5
    call NEAR PTR channel_rest
    lea bx,DGROUP:channel6
    call NEAR PTR channel_rest
    lea bx,DGROUP:channel7
    call NEAR PTR channel_rest
    lea bx,DGROUP:channel8
    call NEAR PTR channel_rest
    lea bx,DGROUP:channel9

; Point a channel at the rest sequence.
channel_rest:
    mov [bx+010H],cx
    mov BYTE PTR [bx+02H],0FFH
    mov BYTE PTR [bx+0AH],01H
    mov BYTE PTR [bx+09H],01H
    ret

; Unreferenced.
sweeps_unused:
    mov ax,00H
    mov bl,BYTE PTR sweep_a_count
    mov BYTE PTR spare_word2,bl
    mov BYTE PTR sweep_a_count,al
    mov bl,BYTE PTR sweep_b_count
    mov BYTE PTR spare_word2+1,bl
    mov BYTE PTR sweep_b_count,al
    mov WORD PTR update_lock,0FFFFH

; Silence every carrier operator.
voices_mute:
    push di
    mov al,043H
    call NEAR PTR operator_mute
    mov al,044H
    call NEAR PTR operator_mute
    mov al,045H
    call NEAR PTR operator_mute
    mov al,04BH
    call NEAR PTR operator_mute
    mov al,04CH
    call NEAR PTR operator_mute
    mov al,04DH
    call NEAR PTR operator_mute
    mov al,053H
    call NEAR PTR operator_mute
    mov al,054H
    call NEAR PTR operator_mute
    mov al,055H
    call NEAR PTR operator_mute
    pop di
    ret

; Unreferenced.
voices_unused:
    push di
    mov al,043H
    lea di,DGROUP:channel1
    mov bh,[di+06H]
    call NEAR PTR operator_level_unused
    mov al,044H
    lea di,DGROUP:channel2
    mov bh,[di+06H]
    call NEAR PTR operator_level_unused
    mov al,045H
    lea di,DGROUP:channel3
    mov bh,[di+06H]
    call NEAR PTR operator_level_unused
    mov al,04BH
    lea di,DGROUP:channel4
    mov bh,[di+06H]
    call NEAR PTR operator_level_unused
    mov al,04CH
    lea di,DGROUP:channel5
    mov bh,[di+06H]
    call NEAR PTR operator_level_unused
    mov al,04DH
    lea di,DGROUP:channel6
    mov bh,[di+06H]
    call NEAR PTR operator_level_unused
    mov al,053H
    lea di,DGROUP:channel7
    mov bh,[di+06H]
    call NEAR PTR operator_level_unused
    mov al,054H
    lea di,DGROUP:channel8
    mov bh,[di+06H]
    mov al,054H
    lea di,DGROUP:channel9
    mov bh,[di+06H]
    call NEAR PTR operator_level_unused
    mov ah,BYTE PTR spare_word2
    mov BYTE PTR sweep_a_count,ah
    mov al,BYTE PTR spare_word2+1
    mov BYTE PTR sweep_b_count,al
    or al,ah
    jz SHORT voices_unused_next
    call NEAR PTR timer_fast
voices_unused_next:
    mov WORD PTR update_lock,00H
    pop di
    ret
DB 032H,0E4H,0A0H,01CH,074H,0AH,06H,03AH,074H,0AH,06H,058H,074H,0AH,06H,076H
DB 074H,0AH,06H,094H,074H,0AH,06H,0B2H,074H,0AH,06H,0D0H,074H,0AH,06H,0F0H
DB 074H,0AH,06H,010H,075H,0C3H

; 2: nonzero while any channel plays.
music_playing:
    xor ax,ax
    mov al,channel1
    or al,channel2
    or al,channel3
    or al,channel4
    or al,channel5
    or al,channel6
    or al,channel7
    or al,channel8
    or al,channel9
    ret

; 44: random street sounds (city view).
city_ambience:
    call FAR PTR sound_random
    and ax,07H
    add ax,0EH
    lea bx,DGROUP:city_ambience_part1
    mov [bx+01H],al
    mov cx,bx
    call NEAR PTR start_channel1
    call FAR PTR sound_random
    and ax,07H
    add ax,01CH
    lea bx,DGROUP:city_ambience_part2
    mov [bx+01H],al
    mov cx,bx
    call NEAR PTR start_channel2
    call FAR PTR sound_random
    and ax,07H
    add ax,02AH
    lea bx,DGROUP:city_ambience_part3
    mov [bx+01H],al
    mov cx,bx
    call NEAR PTR start_channel3
    lea cx,DGROUP:city_ambience_part4
    mov WORD PTR channel1+18,cx
    mov WORD PTR channel2+18,cx
    mov WORD PTR channel3+18,cx
    jmp NEAR PTR start_channel4

; 37: a land unit moves.
unit_move:
    lea cx,DGROUP:unit_move_part1
    jmp NEAR PTR start_channel1

; 38: the player's side wins (light units).
battle_won:
    lea cx,DGROUP:battle_won_part1
battle_won_loop:
    call FAR PTR sound_random
    and ax,0707H
    add ax,0140CH
    mov bx,cx
    add bx,05H
    mov [bx],al
    mov [bx+02H],ah
    add al,ah
    mov BYTE PTR battle_lost_part1+1,al
    jmp NEAR PTR start_channel2

; 39: the player's side loses (light units).
battle_lost:
    lea cx,DGROUP:battle_lost_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:battle_lost_part2
    jmp SHORT battle_won_loop

; 40
battle_won_heavy:
    lea cx,DGROUP:battle_won_heavy_part1
    jmp NEAR PTR start_channel7

; 41
battle_lost_heavy:
    lea cx,DGROUP:battle_lost_heavy_part1
    call NEAR PTR start_channel3
    lea cx,DGROUP:battle_lost_heavy_part2
    jmp NEAR PTR start_channel4

; 42
nuclear_blast:
    lea cx,DGROUP:nuclear_blast_part1
    call NEAR PTR start_channel5
    lea cx,DGROUP:nuclear_blast_part2
    jmp NEAR PTR start_channel6

; 43: a bomber wins its attack.
bomber_hit:
    lea cx,DGROUP:bomber_hit_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:bomber_hit_part2
    jmp NEAR PTR start_channel2
interlude_tunes DW 0208H,027EH,050H,098H,0F2H,0138H,01A4H ; Phrases audience_interlude plays in turn.

; 33: pause the music for a short phrase (audience screen).
audience_interlude:
    push WORD PTR update_lock
    mov WORD PTR update_lock,0FFFFH
    xor ax,ax
    mov al,channel1
    mov interlude_saved,al
    mov channel1,ah
    mov al,channel2
    mov interlude_saved+1,al
    mov channel2,ah
    mov al,channel3
    mov interlude_saved+2,al
    mov channel3,ah
    mov al,channel4
    mov interlude_saved+3,al
    mov channel4,ah
    mov al,channel5
    mov interlude_saved+4,al
    mov channel5,ah
    mov al,channel6
    mov interlude_saved+5,al
    mov channel6,ah
    mov al,channel7
    mov interlude_saved+6,al
    mov channel7,ah
    mov al,channel8
    mov interlude_saved+7,al
    mov channel8,ah
    call NEAR PTR voices_mute
    mov BYTE PTR interlude_on,0FFH
    pop WORD PTR update_lock
    inc WORD PTR interlude_next
    mov bx,interlude_next
    cmp bx,06H
    jna SHORT audience_interlude_next
    xor bx,bx
    mov interlude_next,bx
audience_interlude_next:
    shl bx,1
    mov cx,WORD PTR CS:interlude_tunes[bx]
    jmp NEAR PTR start_channel9

; Give the channels back after an audience interlude.
interlude_end:
    mov al,interlude_saved
    mov channel1,al
    mov al,interlude_saved+1
    mov channel2,al
    mov al,interlude_saved+2
    mov channel3,al
    mov al,interlude_saved+3
    mov channel4,al
    mov al,interlude_saved+4
    mov channel5,al
    mov al,interlude_saved+5
    mov channel6,al
    mov al,interlude_saved+6
    mov channel7,al
    mov al,interlude_saved+7
    mov channel8,al
    lea cx,DGROUP:sound_reset_unused_part1
    mov WORD PTR channel9+16,cx
    mov BYTE PTR interlude_on,00H
    ret

; 3: title and intro music.
title_theme:
    lea cx,DGROUP:title_theme_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:title_theme_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:title_theme_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:title_theme_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:title_theme_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:title_theme_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:title_theme_part7
    call NEAR PTR start_channel7
    lea cx,DGROUP:title_theme_part8
    jmp NEAR PTR start_channel8

; 36: famine, civil disorder, nuclear incident.
bad_news:
    lea cx,DGROUP:bad_news_part1
    call NEAR PTR start_channel2
    lea cx,DGROUP:bad_news_part2
    call NEAR PTR start_channel3
    lea cx,DGROUP:bad_news_part3
    call NEAR PTR start_channel4
    lea cx,DGROUP:bad_news_part4
    call NEAR PTR start_channel5
    lea cx,DGROUP:bad_news_part5
    call NEAR PTR start_channel6
    lea cx,DGROUP:bad_news_part6
    jmp NEAR PTR start_channel7

; 4: world creation.
creation_theme:
    lea cx,DGROUP:creation_theme_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:creation_theme_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:creation_theme_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:creation_theme_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:creation_theme_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:creation_theme_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:creation_theme_part7
    call NEAR PTR start_channel7
    lea cx,DGROUP:creation_theme_part8
    jmp NEAR PTR start_channel8

; 34: celebration (We Love the King Day, Hall of Fame).
celebration:
    lea cx,DGROUP:celebration_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:celebration_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:celebration_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:celebration_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:celebration_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:celebration_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:celebration_part7
    call NEAR PTR start_channel7
    lea cx,DGROUP:celebration_part8
    jmp NEAR PTR start_channel8
growth_tune DB 00H                       ; Which rapture_growth tune is next.

; 35: city grows in rapture (two tunes, alternately).
rapture_growth:
    xor BYTE PTR CS:growth_tune,0FFH
    jz SHORT rapture_growth_next
    lea cx,DGROUP:rapture_growth_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:rapture_growth_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:rapture_growth_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:rapture_growth_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:rapture_growth_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:rapture_growth_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:rapture_growth_part7
    jmp NEAR PTR start_channel7
rapture_growth_next:
    lea cx,DGROUP:rapture_growth_part8
    call NEAR PTR start_channel1
    lea cx,DGROUP:rapture_growth_part9
    call NEAR PTR start_channel2
    lea cx,DGROUP:rapture_growth_part10
    call NEAR PTR start_channel3
    lea cx,DGROUP:rapture_growth_part11
    call NEAR PTR start_channel4
    lea cx,DGROUP:rapture_growth_part12
    jmp NEAR PTR start_channel5

american_parts DW OFFSET american_parts_0
DW OFFSET american_parts_1
DW OFFSET american_parts_2
DW OFFSET american_parts_3

; 5-18: civilization anthems (civt .f4), argument 2 picks the arrangement.
american_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:american_parts[bx]
american_parts_3:
    lea cx,DGROUP:american_anthem_part1
    call NEAR PTR start_channel1
american_parts_2:
    lea cx,DGROUP:american_anthem_part2
    call NEAR PTR start_channel2
american_parts_1:
    lea cx,DGROUP:american_anthem_part3
    call NEAR PTR start_channel3
american_parts_0:
    lea cx,DGROUP:american_anthem_part4
    jmp NEAR PTR start_channel4

aztec_parts DW OFFSET aztec_parts_0
DW OFFSET aztec_parts_1
DW OFFSET aztec_parts_2
DW OFFSET aztec_parts_3
aztec_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:aztec_parts[bx]
aztec_parts_3:
    lea cx,DGROUP:aztec_anthem_part1
    call NEAR PTR start_channel1
aztec_parts_2:
    lea cx,DGROUP:aztec_anthem_part2
    call NEAR PTR start_channel2
aztec_parts_1:
    lea cx,DGROUP:aztec_anthem_part3
    call NEAR PTR start_channel3
aztec_parts_0:
    lea cx,DGROUP:aztec_anthem_part4
    jmp NEAR PTR start_channel4

egyptian_parts DW OFFSET egyptian_parts_0
DW OFFSET egyptian_parts_1
DW OFFSET egyptian_parts_2
DW OFFSET egyptian_parts_2
egyptian_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:egyptian_parts[bx]
egyptian_parts_2:
    lea cx,DGROUP:egyptian_anthem_part1
    call NEAR PTR start_channel5
    lea cx,DGROUP:egyptian_anthem_part2
    call NEAR PTR start_channel1
egyptian_parts_1:
    lea cx,DGROUP:egyptian_anthem_part3
    call NEAR PTR start_channel2
    lea cx,DGROUP:egyptian_anthem_part4
    call NEAR PTR start_channel3
    lea cx,DGROUP:egyptian_anthem_part5
    call NEAR PTR start_channel4
egyptian_parts_0:
    lea cx,DGROUP:egyptian_anthem_part6
    jmp NEAR PTR start_channel7

zulu_parts DW OFFSET zulu_parts_0
DW OFFSET zulu_parts_1
DW OFFSET zulu_parts_2
DW OFFSET zulu_parts_3
zulu_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:zulu_parts[bx]
zulu_parts_3:
    lea cx,DGROUP:zulu_anthem_part1
    call NEAR PTR start_channel1
zulu_parts_2:
    lea cx,DGROUP:zulu_anthem_part2
    call NEAR PTR start_channel2
zulu_parts_1:
    lea cx,DGROUP:zulu_anthem_part3
    call NEAR PTR start_channel3
zulu_parts_0:
    lea cx,DGROUP:zulu_anthem_part4
    jmp NEAR PTR start_channel7

french_parts DW OFFSET french_parts_0
DW OFFSET french_parts_1
DW OFFSET french_parts_2
DW OFFSET french_parts_3
french_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:french_parts[bx]
french_parts_3:
    lea cx,DGROUP:french_anthem_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:french_anthem_part2
    call NEAR PTR start_channel2
french_parts_2:
    lea cx,DGROUP:french_anthem_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:french_anthem_part4
    call NEAR PTR start_channel4
french_parts_1:
    lea cx,DGROUP:french_anthem_part5
    call NEAR PTR start_channel7
    lea cx,DGROUP:french_anthem_part6
    call NEAR PTR start_channel6
french_parts_0:
    lea cx,DGROUP:french_anthem_part7
    call NEAR PTR start_channel8
    lea cx,DGROUP:french_anthem_part8
    jmp NEAR PTR start_channel5

roman_parts DW OFFSET roman_parts_0
DW OFFSET roman_parts_1
DW OFFSET roman_parts_2
DW OFFSET roman_parts_3
roman_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:roman_parts[bx]
roman_parts_3:
    lea cx,DGROUP:roman_anthem_part1
    call NEAR PTR start_channel1
roman_parts_2:
    lea cx,DGROUP:roman_anthem_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:roman_anthem_part3
    call NEAR PTR start_channel6
roman_parts_1:
    lea cx,DGROUP:roman_anthem_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:roman_anthem_part5
    call NEAR PTR start_channel8
    lea cx,DGROUP:roman_anthem_part6
    call NEAR PTR start_channel5
roman_parts_0:
    lea cx,DGROUP:roman_anthem_part7
    call NEAR PTR start_channel7
    lea cx,DGROUP:roman_anthem_part8
    jmp NEAR PTR start_channel3

russian_parts DW OFFSET russian_parts_0
DW OFFSET russian_parts_1
DW OFFSET russian_parts_2
DW OFFSET russian_parts_2
russian_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:russian_parts[bx]
russian_parts_2:
    lea cx,DGROUP:russian_anthem_part1
    call NEAR PTR start_channel3
    lea cx,DGROUP:russian_anthem_part2
    call NEAR PTR start_channel4
    lea cx,DGROUP:russian_anthem_part3
    call NEAR PTR start_channel5
    lea cx,DGROUP:russian_anthem_part4
    call NEAR PTR start_channel6
russian_parts_1:
    lea cx,DGROUP:russian_anthem_part5
    call NEAR PTR start_channel1
    lea cx,DGROUP:russian_anthem_part6
    call NEAR PTR start_channel2
russian_parts_0:
    lea cx,DGROUP:russian_anthem_part7
    call NEAR PTR start_channel7
    lea cx,DGROUP:russian_anthem_part8
    jmp NEAR PTR start_channel8

greek_parts DW OFFSET greek_parts_0
DW OFFSET greek_parts_1
DW OFFSET greek_parts_2
DW OFFSET greek_parts_2
greek_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:greek_parts[bx]
greek_parts_2:
    lea cx,DGROUP:greek_anthem_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:greek_anthem_part2
    call NEAR PTR start_channel2
greek_parts_1:
    lea cx,DGROUP:greek_anthem_part3
    call NEAR PTR start_channel8
    lea cx,DGROUP:greek_anthem_part4
    call NEAR PTR start_channel3
greek_parts_0:
    lea cx,DGROUP:greek_anthem_part5
    call NEAR PTR start_channel4
    lea cx,DGROUP:greek_anthem_part6
    call NEAR PTR start_channel5
    lea cx,DGROUP:greek_anthem_part7
    call NEAR PTR start_channel6
    lea cx,DGROUP:greek_anthem_part8
    jmp NEAR PTR start_channel7

english_parts DW OFFSET english_parts_0
DW OFFSET english_parts_1
DW OFFSET english_parts_2
DW OFFSET english_parts_2
english_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:english_parts[bx]
english_parts_2:
    lea cx,DGROUP:english_anthem_part1
    call NEAR PTR start_channel1
english_parts_1:
    lea cx,DGROUP:english_anthem_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:english_anthem_part3
    call NEAR PTR start_channel5
english_parts_0:
    lea cx,DGROUP:english_anthem_part4
    call NEAR PTR start_channel3
    lea cx,DGROUP:english_anthem_part5
    jmp NEAR PTR start_channel4

babylonian_parts DW OFFSET babylonian_parts_0
DW OFFSET babylonian_parts_1
DW OFFSET babylonian_parts_2
DW OFFSET babylonian_parts_2
babylonian_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:babylonian_parts[bx]
babylonian_parts_2:
    lea cx,DGROUP:babylonian_anthem_part1
    call NEAR PTR start_channel1
babylonian_parts_1:
    lea cx,DGROUP:babylonian_anthem_part2
    call NEAR PTR start_channel2
babylonian_parts_0:
    lea cx,DGROUP:babylonian_anthem_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:babylonian_anthem_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:babylonian_anthem_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:babylonian_anthem_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:babylonian_anthem_part7
    jmp NEAR PTR start_channel7

chinese_parts DW OFFSET chinese_parts_0
DW OFFSET chinese_parts_1
DW OFFSET chinese_parts_2
DW OFFSET chinese_parts_3
chinese_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:chinese_parts[bx]
chinese_parts_3:
    lea cx,DGROUP:chinese_anthem_part1
    call NEAR PTR start_channel1
chinese_parts_2:
    lea cx,DGROUP:chinese_anthem_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:chinese_anthem_part3
    call NEAR PTR start_channel7
chinese_parts_1:
    lea cx,DGROUP:chinese_anthem_part4
    call NEAR PTR start_channel3
chinese_parts_0:
    lea cx,DGROUP:chinese_anthem_part5
    call NEAR PTR start_channel4
    lea cx,DGROUP:chinese_anthem_part6
    call NEAR PTR start_channel5
    lea cx,DGROUP:chinese_anthem_part7
    jmp NEAR PTR start_channel6

mongol_parts DW OFFSET mongol_parts_0
DW OFFSET mongol_parts_1
DW OFFSET mongol_parts_2
DW OFFSET mongol_parts_2
mongol_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:mongol_parts[bx]
mongol_parts_2:
    lea cx,DGROUP:mongol_anthem_part1
    call NEAR PTR start_channel5
    lea cx,DGROUP:mongol_anthem_part2
    call NEAR PTR start_channel7
mongol_parts_1:
    lea cx,DGROUP:mongol_anthem_part3
    call NEAR PTR start_channel3
mongol_parts_0:
    lea cx,DGROUP:mongol_anthem_part4
    call NEAR PTR start_channel1
    lea cx,DGROUP:mongol_anthem_part5
    call NEAR PTR start_channel6
    lea cx,DGROUP:mongol_anthem_part6
    call NEAR PTR start_channel4
    lea cx,DGROUP:mongol_anthem_part7
    call NEAR PTR start_channel8
    lea cx,DGROUP:mongol_anthem_part8
    jmp NEAR PTR start_channel2

indian_parts DW OFFSET indian_parts_0
DW OFFSET indian_parts_1
DW OFFSET indian_parts_2
DW OFFSET indian_parts_3
indian_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:indian_parts[bx]
indian_parts_3:
    lea cx,DGROUP:indian_anthem_part1
    call NEAR PTR start_channel1
indian_parts_2:
    lea cx,DGROUP:indian_anthem_part2
    call NEAR PTR start_channel2
indian_parts_1:
    lea cx,DGROUP:indian_anthem_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:indian_anthem_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:indian_anthem_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:indian_anthem_part6
    call NEAR PTR start_channel6
indian_parts_0:
    lea cx,DGROUP:indian_anthem_part7
    call NEAR PTR start_channel7
    lea cx,DGROUP:indian_anthem_part8
    jmp NEAR PTR start_channel8

german_parts DW OFFSET german_parts_0
DW OFFSET german_parts_1
DW OFFSET german_parts_2
DW OFFSET german_parts_3
german_anthem:
    mov bx,[bp+08H]
    shl bx,1
    and bx,06H
    jmp WORD PTR CS:german_parts[bx]
german_parts_3:
    lea cx,DGROUP:german_anthem_part1
    call NEAR PTR start_channel6
german_parts_2:
    lea cx,DGROUP:german_anthem_part2
    call NEAR PTR start_channel1
german_parts_1:
    lea cx,DGROUP:german_anthem_part3
    call NEAR PTR start_channel2
    lea cx,DGROUP:german_anthem_part4
    call NEAR PTR start_channel5
german_parts_0:
    lea cx,DGROUP:german_anthem_part5
    call NEAR PTR start_channel3
    lea cx,DGROUP:german_anthem_part6
    jmp NEAR PTR start_channel4

; 19-32: civilization motifs (civt .f3).
american_motif:
    lea cx,DGROUP:american_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:american_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:american_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:american_motif_part4
    jmp NEAR PTR start_channel4
aztec_motif:
    lea cx,DGROUP:aztec_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:aztec_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:aztec_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:aztec_motif_part4
    jmp NEAR PTR start_channel4
egyptian_motif:
    lea cx,DGROUP:egyptian_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:egyptian_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:egyptian_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:egyptian_motif_part4
    jmp NEAR PTR start_channel7
zulu_motif:
    lea cx,DGROUP:zulu_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:zulu_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:zulu_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:zulu_motif_part4
    jmp NEAR PTR start_channel7
french_motif:
    lea cx,DGROUP:french_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:french_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:french_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:french_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:french_motif_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:french_motif_part6
    jmp NEAR PTR start_channel6
roman_motif:
    lea cx,DGROUP:roman_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:roman_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:roman_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:roman_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:roman_motif_part5
    jmp NEAR PTR start_channel5
russian_motif:
    lea cx,DGROUP:russian_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:russian_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:russian_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:russian_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:russian_motif_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:russian_motif_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:russian_motif_part7
    call NEAR PTR start_channel7
    lea cx,DGROUP:russian_motif_part8
    jmp NEAR PTR start_channel8
greek_motif:
    lea cx,DGROUP:greek_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:greek_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:greek_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:greek_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:greek_motif_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:greek_motif_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:greek_motif_part7
    jmp NEAR PTR start_channel7
english_motif:
    lea cx,DGROUP:english_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:english_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:english_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:english_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:english_motif_part5
    jmp NEAR PTR start_channel5
babylonian_motif:
    lea cx,DGROUP:babylonian_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:babylonian_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:babylonian_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:babylonian_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:babylonian_motif_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:babylonian_motif_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:babylonian_motif_part7
    jmp NEAR PTR start_channel7
chinese_motif:
    lea cx,DGROUP:chinese_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:chinese_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:chinese_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:chinese_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:chinese_motif_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:chinese_motif_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:chinese_motif_part7
    jmp NEAR PTR start_channel7
mongol_motif:
    lea cx,DGROUP:mongol_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:mongol_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:mongol_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:mongol_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:mongol_motif_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:mongol_motif_part6
    call NEAR PTR start_channel6
    lea cx,DGROUP:mongol_motif_part7
    call NEAR PTR start_channel7
    lea cx,DGROUP:mongol_motif_part8
    jmp NEAR PTR start_channel8
indian_motif:
    lea cx,DGROUP:indian_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:indian_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:indian_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:indian_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:indian_motif_part5
    call NEAR PTR start_channel5
    lea cx,DGROUP:indian_motif_part6
    jmp NEAR PTR start_channel6
german_motif:
    lea cx,DGROUP:german_motif_part1
    call NEAR PTR start_channel1
    lea cx,DGROUP:german_motif_part2
    call NEAR PTR start_channel2
    lea cx,DGROUP:german_motif_part3
    call NEAR PTR start_channel3
    lea cx,DGROUP:german_motif_part4
    call NEAR PTR start_channel4
    lea cx,DGROUP:german_motif_part5
    jmp NEAR PTR start_channel5

; Sound commands 0..44 (slot 101).
commands DW OFFSET sound_reset
DW OFFSET music_end
DW OFFSET music_playing
DW OFFSET title_theme
DW OFFSET creation_theme
DW OFFSET american_anthem
DW OFFSET aztec_anthem
DW OFFSET egyptian_anthem
DW OFFSET zulu_anthem
DW OFFSET french_anthem
DW OFFSET roman_anthem
DW OFFSET russian_anthem
DW OFFSET greek_anthem
DW OFFSET english_anthem
DW OFFSET babylonian_anthem
DW OFFSET chinese_anthem
DW OFFSET mongol_anthem
DW OFFSET indian_anthem
DW OFFSET german_anthem
DW OFFSET american_motif
DW OFFSET aztec_motif
DW OFFSET egyptian_motif
DW OFFSET zulu_motif
DW OFFSET french_motif
DW OFFSET roman_motif
DW OFFSET russian_motif
DW OFFSET greek_motif
DW OFFSET english_motif
DW OFFSET babylonian_motif
DW OFFSET chinese_motif
DW OFFSET mongol_motif
DW OFFSET indian_motif
DW OFFSET german_motif
DW OFFSET audience_interlude
DW OFFSET celebration
DW OFFSET rapture_growth
DW OFFSET bad_news
DW OFFSET unit_move
DW OFFSET battle_won
DW OFFSET battle_lost
DW OFFSET battle_won_heavy
DW OFFSET battle_lost_heavy
DW OFFSET nuclear_blast
DW OFFSET bomber_hit
DW OFFSET city_ambience

; Slot 100: detect the card and silence it.
sound_init:
    push bp
    mov bp,sp
ASSUME DS:NOTHING
    mov bx,[bp+06H]
    push si
    push di
    push ds
    mov ax,SEG DATA
    mov ds,ax
ASSUME DS:DGROUP
    mov init_argument,bx
    call NEAR PTR card_detect
    pop ds
    pop di
    pop si
    pop bp
    retf

; Slot 101: start sound command n (0..44); a second argument picks a theme's arrangement.
sound_command:
    push bp
    mov bp,sp
ASSUME DS:NOTHING
    mov bx,[bp+06H]
    cmp bx,02CH
    jg SHORT sound_command_next
    push ds
    mov ax,SEG DATA
    mov ds,ax
    shl bx,1
    xor ax,ax
    call WORD PTR CS:commands[bx]
    pop ds
sound_command_next:
    pop bp
    mov WORD PTR CS:quiet_ticks,00H

; Slots 106-110: nothing.
sound_unused:
    retf
bcd_to_binary:
    push ax
    shr al,1
    shr al,1
    shr al,1
    shr al,1
    mov bl,0AH
    mul bl
    pop bx
    and bl,0FH
    add al,bl
    ret

; Set the DOS time from the real-time clock (the fast timer made it drift).
dos_time_restore:
    mov ah,02H
    int 01AH
    mov al,ch
    call NEAR PTR bcd_to_binary
    mov ch,al
    mov al,cl
    call NEAR PTR bcd_to_binary
    mov cl,al
    mov al,dh
    call NEAR PTR bcd_to_binary
    mov dh,al
    mov dl,00H
    mov ah,02DH
    int 021H
    ret

; Slot 102: silence the card and set the DOS clock from the real-time clock.
sound_shutdown:
    push di
    push bp
    push ds
    mov ax,SEG DATA
    mov ds,ax
    call NEAR PTR sound_reset
    call NEAR PTR dos_time_restore
ASSUME DS:DGROUP
    mov ax,rate_request
    mov WORD PTR rate_request,00H
    pop ds
    pop bp
    pop di
    retf

; Slot 103: one music step (timer, about 60 Hz); returns a timer rate request (1 faster, -1 slower, 0 none).
sound_tick:
    push ds
    mov ax,SEG DATA
    mov ds,ax
    call NEAR PTR music_tick
    mov ax,rate_request
    mov WORD PTR rate_request,00H
    pop ds
    retf

; Slot 104: pitch sweeps, at the fast timer rate.
sound_fast_tick:
    push ds
    mov ax,SEG DATA
    mov ds,ax
    call NEAR PTR sweep_tick
    pop ds
    xor ax,ax
    retf

; Slot 105: ticks since the last sound command (CIV uses it as a random seed).
sound_quiet_ticks:
    mov ax,CS:quiet_ticks
    retf
CODE ENDS

DATA SEGMENT
ASSUME DS:DGROUP
reserved DB 16 DUP (0)
copyright DB 'Copyright (C) 1991 by MicroProse Software, All Rights Reserved.',0
DB 0FCH,037H,0F8H,07H,0FFH,0FBH,03H,0FFH,00H,0F4H,07FH,0F3H,0AH,0FBH,0F5H,051H
DB 0EH,0AH,0F5H,03DH,07H,0AH,0EH,0AH,07H,0AH,0EH,0AH,0EH,0AH,07H,0AH
DB 0EH,0AH,07H,0AH,0EH,0AH,0F4H,00H,0F3H,0AH,05H,0F5H,051H,0EH,0AH,0F5H
DB 03BH,07H,0AH,0EH,0AH,07H,0AH,0EH,0AH,012H,0AH,07H,0AH,0EH,0AH,07H
DB 0AH,0EH,0AH,00H,028H,00H,00H,00H,0FCH,037H,0F9H,00H,0F5H,039H,012H,0AH
DB 0F8H,07H,0FFH,0F5H,051H,0F4H,020H,013H,05H,0F4H,040H,018H,05H,0F4H,060H,01AH
DB 0AH,0F5H,03BH,0F4H,07FH,0F3H,0AH,0FBH,07H,0AH,0EH,0AH,012H,0AH,07H,0AH
DB 0EH,0AH,07H,0AH,0EH,0AH,0F3H,00H,00H,012H,0AH,0F5H,051H,0F4H,060H,07H
DB 05H,0F4H,040H,0CH,05H,0F4H,020H,0EH,0AH,0F5H,03DH,0F4H,00H,0F3H,0AH,05H
DB 07H,0AH,0EH,0AH,012H,0AH,07H,0AH,0EH,0AH,07H,0AH,0EH,0AH,00H,028H
DB 00H,00H,0FCH,037H,0F9H,00H,0F5H,03BH,012H,0AH,0F5H,04FH,0F4H,020H,013H,05H
DB 0F4H,040H,018H,05H,0F4H,060H,01AH,0AH,0F5H,03BH,0F4H,07FH,0F8H,0AH,0FFH,0F3H
DB 0AH,0F9H,07H,0AH,0EH,0AH,012H,0AH,07H,0AH,0EH,0AH,07H,0AH,0EH,0AH
DB 012H,0AH,07H,0AH,0EH,0AH,07H,0AH,0EH,0AH,0EH,0AH,07H,0AH,0EH,0AH
DB 07H,0AH,0EH,0AH,00H,028H,00H,00H,0FCH,037H,0F8H,07H,0FFH,0F9H,00H,012H
DB 0AH,0F5H,051H,0F4H,060H,07H,05H,0F4H,040H,0CH,05H,0F4H,020H,0EH,0AH,0F5H
DB 039H,0F4H,00H,07H,0AH,0EH,0AH,012H,0AH,0F5H,051H,0F4H,020H,013H,05H,0F4H
DB 040H,018H,05H,0F4H,060H,01AH,0AH,0F5H,03BH,0F4H,07FH,07H,0AH,0EH,0AH,0F5H
DB 04FH,0F4H,060H,07H,05H,0F4H,040H,0CH,05H,0F4H,020H,0EH,0AH,0F5H,039H,0F4H
DB 00H,07H,0AH,0EH,0AH,012H,0AH,0F5H,051H,0F4H,020H,013H,05H,0F4H,040H,018H
DB 05H,0F4H,060H,01AH,0AH,0F5H,03FH,0F4H,07FH,07H,0AH,0CH,0AH,0EH,0AH,00H
DB 028H,00H,00H,00H,0FCH,037H,0F8H,07H,0FFH,0F9H,00H,0F4H,020H,0EH,0AH,0F5H
DB 03BH,0F4H,00H,07H,0AH,0EH,0AH,07H,0AH,0EH,0AH,012H,0AH,0F5H,04FH,0F4H
DB 020H,018H,05H,0F4H,040H,018H,05H,0F4H,060H,01AH,0AH,0F5H,039H,0F4H,07FH,07H
DB 0AH,0EH,0AH,012H,0AH,0F5H,04FH,0F4H,060H,07H,05H,0F4H,040H,0CH,05H,0F4H
DB 020H,0EH,0AH,0F5H,037H,0F4H,00H,07H,0AH,0EH,0AH,0F5H,04FH,0F4H,020H,013H
DB 05H,0F4H,040H,018H,05H,0F4H,060H,01AH,0AH,0F5H,037H,0F4H,07FH,07H,0AH,0CH
DB 0AH,0EH,0AH,00H,028H,00H,00H,00H,0FCH,037H,0F9H,00H,0F8H,07H,0FFH,0F5H
DB 04FH,0F4H,060H,07H,05H,0F4H,040H,0CH,05H,0F4H,020H,0EH,0AH,0F5H,037H,0F4H
DB 00H,07H,0AH,0EH,0AH,0F5H,04FH,0F4H,020H,0EH,05H,0F4H,040H,013H,05H,0F4H
DB 060H,015H,0AH,0F4H,07FH,07H,05H,0F4H,040H,0CH,05H,0F4H,020H,0EH,0AH,0F5H
DB 037H,0F4H,00H,07H,0AH,0EH,0AH,0F8H,00H,00H,0F5H,04FH,0F4H,020H,013H,05H
DB 0F4H,040H,018H,05H,0F4H,060H,01AH,0AH,0F5H,037H,0F4H,07FH,07H,0AH,0EH,0AH
DB 012H,0AH,012H,0AH,012H,0AH,012H,0AH,0F8H,03H,0FFH,0F5H,04FH,0F4H,060H,07H
DB 05H,0F4H,040H,0CH,05H,0F4H,020H,0EH,0AH,00H,028H,00H,00H,00H,0FCH,037H
DB 0F9H,00H,0F8H,07H,0FFH,0F5H,04FH,0F4H,020H,013H,05H,0F4H,040H,018H,05H,0F4H
DB 060H,01AH,0AH,0F5H,037H,0F4H,07FH,07H,0AH,0EH,0AH,012H,0AH,012H,0AH,012H
DB 0AH,012H,0AH,0F8H,03H,0FFH,0F5H,04FH,0F4H,060H,013H,05H,0F4H,040H,013H,05H
DB 0F4H,020H,015H,0AH,0F3H,0AH,07H,0F4H,00H,0FFH,00H,0EH,0AH,0EH,05H,0EH
DB 0AH,0EH,0AH,0EH,05H,0EH,0AH,0FFH,02H,0EH,0AH,00H,028H,00H,00H,00H
chinese_anthem_part1 LABEL BYTE
DB 0FCH,030H,0F9H,054H,0F5H,08H,0FBH,01H,03CH,0EH,048H,0EH,03EH,0EH,04AH,0EH
DB 040H,0EH,04CH,0EH,043H,0EH,04FH,0EH,040H,0EH,04CH,0EH,045H,0EH,051H,0EH
DB 043H,0EH,04FH,0EH,040H,0EH,04CH,0CH,00H,02H,03CH,0EH,048H,0EH,03EH,0EH
DB 04AH,0EH,03EH,0EH,04AH,0EH,03EH,0EH,04AH,0EH,040H,0EH,04CH,0EH,03EH,0EH
DB 04AH,0EH,03EH,0EH,04AH,0EH,03EH,0EH,04AH,0CH,00H,02H,03CH,0EH,048H,0EH
DB 03EH,0EH,04AH,0EH,040H,0EH,04CH,0EH,043H,0EH,04FH,0EH,040H,0EH,04CH,0EH
DB 045H,0EH,051H,0EH,043H,0EH,04FH,0EH,040H,0EH,04CH,0CH,00H,02H,03EH,0EH
DB 04AH,0EH,03CH,0EH,048H,0EH,039H,0EH,045H,0EH,037H,0EH,043H,0EH,03CH,0EH
DB 048H,0EH,03EH,0EH,04AH,0EH,039H,0EH,040H,0EH,03CH,0EH,043H,0CH,00H,02H
DB 0FDH,00H
chinese_anthem_part2 LABEL BYTE
DB 0F4H,022H,0F5H,08H,0FCH,02FH,0F9H,05AH,02BH,01CH,02DH,01CH,030H,01CH,032H,01CH
DB 02DH,01CH,034H,01CH,032H,01CH,030H,01CH,02BH,01CH,02DH,01CH,02BH,01CH,032H,01CH
DB 02DH,01CH,02DH,01CH,032H,01CH,032H,01CH,02BH,01CH,02DH,01CH,030H,01CH,032H,01CH
DB 02DH,01CH,034H,01CH,032H,01CH,030H,01CH,02DH,01CH,02BH,01CH,028H,01CH,026H,01CH
DB 028H,01CH,02BH,01CH,02DH,01CH,00H,01CH,0FDH,00H
chinese_anthem_part3 LABEL BYTE
DB 0F4H,05EH,0F5H,08H,0FCH,02FH,0F9H,054H,030H,01CH,032H,01CH,034H,01CH,037H,01CH
DB 034H,01CH,039H,01CH,037H,01CH,034H,01CH,030H,01CH,032H,01CH,030H,01CH,039H,01CH
DB 034H,01CH,032H,01CH,037H,01CH,037H,01CH,030H,01CH,032H,01CH,034H,01CH,037H,01CH
DB 034H,01CH,039H,01CH,037H,01CH,034H,01CH,032H,01CH,030H,01CH,02DH,01CH,02BH,01CH
DB 030H,01CH,032H,01CH,039H,0EH,034H,0EH,030H,0EH,037H,0EH,0FDH,00H
chinese_anthem_part4 LABEL BYTE
DB 0F4H,060H,0F5H,08H,0FCH,030H,0F9H,065H,024H,038H,028H,01CH,026H,01CH,028H,01CH
DB 02DH,01CH,02BH,035H,00H,03H,024H,038H,026H,01CH,024H,01CH,028H,01CH,026H,01CH
DB 021H,035H,00H,03H,024H,038H,028H,01CH,026H,01CH,028H,01CH,02DH,01CH,02BH,035H
DB 00H,03H,026H,01CH,024H,01CH,021H,01CH,01FH,019H,024H,01CH,01FH,01CH,021H,038H
DB 00H,03H,0FDH,00H
chinese_anthem_part5 LABEL BYTE
DB 0F4H,07FH,0FCH,02FH,0F5H,08H,0F9H,058H,024H,038H,028H,038H,028H,038H,02BH,038H
DB 024H,038H,026H,038H,028H,038H,02BH,038H,024H,038H,028H,038H,028H,038H,02BH,038H
DB 026H,038H,021H,038H,024H,038H,021H,038H,0FDH,00H
chinese_anthem_part6 LABEL BYTE
DB 0FCH,031H,0F9H,068H,0F5H,08H,013H,038H,018H,038H,01AH,038H,015H,038H,015H,038H
DB 018H,038H,015H,038H,0EH,038H,013H,038H,018H,038H,01AH,038H,015H,038H,0EH,038H
DB 0CH,038H,0CH,038H,09H,038H,0FDH,00H
chinese_anthem_part7 LABEL BYTE
DB 0F4H,020H,0FCH,031H,0F5H,08H,0F9H,066H,01FH,038H,024H,038H,026H,038H,021H,038H
DB 021H,038H,024H,038H,021H,038H,01AH,038H,01FH,038H,024H,038H,026H,038H,021H,038H
DB 01AH,038H,018H,038H,018H,038H,015H,038H,0FDH,00H
chinese_motif_part1 LABEL BYTE
DB 0FCH,030H,0F5H,08H,0F9H,054H,0FBH,01H,034H,0EH,040H,0EH,036H,0EH,042H,0EH
DB 038H,0EH,044H,0EH,03BH,0EH,047H,0EH,03DH,0EH,049H,0EH,00H,00H
chinese_motif_part2 LABEL BYTE
DB 0F4H,022H,0FCH,02FH,0F5H,08H,0F9H,05EH,023H,01CH,025H,01CH,028H,01CH,02AH,01CH
DB 02CH,01CH,00H,00H
chinese_motif_part3 LABEL BYTE
DB 0F4H,060H,0FCH,030H,0F5H,08H,0F9H,06FH,01CH,038H,020H,01CH,01EH,01CH,020H,01CH
DB 00H,00H
chinese_motif_part4 LABEL BYTE
DB 0F4H,07FH,0FCH,02FH,0F5H,08H,0F9H,05CH,01CH,038H,020H,038H,023H,038H,00H,00H
chinese_motif_part5 LABEL BYTE
DB 0FCH,031H,0F9H,068H,0F5H,08H,017H,038H,010H,038H,012H,038H,00H,00H
chinese_motif_part6 LABEL BYTE
DB 0F4H,020H,0FCH,031H,0F5H,08H,0F9H,066H,017H,038H,01CH,038H,01EH,038H,00H,00H
chinese_motif_part7 LABEL BYTE
DB 0F4H,05EH,0FCH,02FH,0F5H,08H,0F9H,05EH,028H,01CH,02AH,01CH,02CH,01CH,02FH,01CH
DB 031H,01CH,00H,00H
aztec_anthem_part1 LABEL BYTE
DB 0FCH,01CH,0F5H,0CH,0FBH,05H,0F4H,04BH,0FFH,00H,00H,078H,0F9H,06AH,035H,0AH
DB 0F9H,061H,035H,0AH,0F9H,066H,038H,014H,0F9H,062H,038H,0AH,031H,0AH,035H,0AH
DB 0F9H,066H,033H,014H,035H,014H,0F9H,062H,035H,0AH,0F9H,066H,031H,014H,0F9H,062H
DB 02EH,014H,031H,0AH,0F9H,066H,033H,0AH,00H,03CH,0F9H,06AH,035H,0AH,0F9H,062H
DB 035H,0AH,0F9H,066H,038H,014H,0F9H,062H,038H,0AH,031H,0AH,035H,0AH,0F9H,066H
DB 033H,014H,035H,014H,0F9H,062H,035H,0AH,0F9H,066H,031H,014H,02EH,014H,0F9H,062H
DB 02CH,0AH,0F9H,066H,02EH,0AH,00H,03CH,0F5H,00H,0F4H,034H,0FFH,01H,0FDH,00H
aztec_anthem_part2 LABEL BYTE
DB 0FCH,029H,0F9H,06AH,0F4H,065H,0FFH,00H,0F5H,05H,0F9H,06AH,031H,0AH,0F9H,056H
DB 031H,014H,0F9H,06AH,031H,0AH,0F9H,056H,031H,0AH,031H,0AH,0F9H,06AH,031H,014H
DB 0F9H,056H,031H,014H,031H,0AH,031H,014H,0F9H,06AH,031H,014H,0F9H,056H,031H,0AH
DB 031H,014H,0F9H,06AH,031H,014H,0F9H,056H,031H,014H,031H,0AH,031H,014H,0F9H,06AH
DB 031H,014H,0F9H,056H,031H,0AH,031H,014H,0F9H,06AH,031H,0AH,0F9H,056H,031H,014H
DB 0F9H,06AH,031H,0AH,0F9H,056H,031H,0AH,031H,0AH,0F9H,06AH,031H,014H,0F9H,056H
DB 031H,014H,031H,0AH,031H,014H,0F9H,06AH,031H,014H,0F9H,056H,031H,0AH,031H,014H
DB 0F9H,06AH,031H,014H,0F9H,056H,031H,014H,031H,0AH,031H,014H,0F9H,06AH,031H,014H
DB 0F9H,056H,031H,0AH,031H,014H,0F9H,06AH,031H,014H,0F9H,056H,031H,014H,031H,0AH
DB 031H,014H,0F9H,06AH,031H,014H,0F5H,00H,0F9H,056H,031H,0AH,031H,014H,0F9H,06AH
DB 031H,014H,0F9H,056H,031H,0AH,031H,0AH,031H,0AH,031H,014H,0F9H,06AH,031H,0AH
DB 031H,0AH,0F9H,056H,031H,0AH,031H,014H,0FFH,01H,0FDH,00H
aztec_anthem_part3 LABEL BYTE
DB 0FCH,02AH,0F9H,06AH,0F4H,040H,0F5H,05H,0FEH,00H,00H,014H,0F9H,05BH,020H,0AH
DB 0F9H,065H,024H,014H,0F9H,05BH,020H,0AH,0F5H,0FEH,0FFH,04H,0F5H,07H,0FFH,00H
DB 00H,014H,0F9H,05BH,020H,05H,0F9H,05DH,020H,05H,0F9H,05FH,024H,014H,0F9H,062H
DB 020H,0AH,0F5H,01H,0FFH,08H,0FDH,00H
aztec_anthem_part4 LABEL BYTE
DB 0FCH,02AH,0FBH,05H,0F5H,07H,0F4H,01BH,0FFH,00H,0F9H,06AH,018H,0AH,0F9H,05BH
DB 018H,014H,01BH,014H,014H,0AH,0F5H,03H,0FFH,01H,0F5H,0FFH,0F9H,06AH,018H,0AH
DB 0F9H,05BH,018H,014H,01BH,014H,014H,0AH,0F5H,08H,0FFH,00H,0F9H,06AH,018H,0AH
DB 0F9H,05BH,018H,014H,01BH,014H,014H,0AH,0F5H,04H,0FFH,04H,0F5H,00H,0FEH,01H
DB 0FDH,00H
aztec_motif_part1 LABEL BYTE
DB 0FCH,01CH,0F9H,065H,0F5H,0CH,0FBH,05H,035H,0AH,0F9H,06AH,035H,0AH,0F9H,060H
DB 0F8H,0AH,03H,038H,064H,0F8H,00H,00H,00H,00H
aztec_motif_part2 LABEL BYTE
DB 0FCH,029H,0F9H,067H,031H,0AH,031H,014H,031H,0AH,031H,0AH,031H,0AH,031H,014H
DB 031H,014H,031H,0AH,00H,00H
aztec_motif_part3 LABEL BYTE
DB 0FCH,02AH,0F9H,067H,024H,014H,020H,0AH,024H,014H,020H,0AH,020H,0AH,020H,05H
DB 020H,05H,020H,0AH,024H,014H,020H,0AH,00H,00H
aztec_motif_part4 LABEL BYTE
DB 0FCH,02AH,0F9H,06AH,0FFH,00H,018H,0AH,018H,014H,01BH,014H,014H,0AH,0FFH,01H
DB 00H,00H
egyptian_anthem_part2 LABEL BYTE
DB 0FCH,01CH,0F9H,064H,0FBH,01H,0F5H,0CH,032H,014H,02EH,014H,032H,014H,034H,014H
DB 037H,028H,032H,014H,035H,02CH,0FBH,0FFH,032H,04H,02BH,04H,032H,04H,0FBH,00H
DB 035H,04H,0FBH,04H,037H,064H,0FBH,01H,03BH,0AH,03AH,0AH,037H,01EH,034H,0AH
DB 0FBH,0FFH,035H,04H,032H,04H,02EH,04H,032H,04H,0FBH,00H,034H,04H,0FBH,02H
DB 032H,028H,032H,014H,02EH,014H,032H,014H,034H,014H,037H,028H,032H,014H,0FBH,0FFH
DB 037H,04H,034H,04H,032H,04H,030H,04H,0FBH,00H,02BH,04H,0FBH,04H,0F8H,06H
DB 0FFH,02EH,078H,0F8H,00H,00H,0F5H,0AH,0FBH,01H,032H,014H,02EH,014H,032H,014H
DB 034H,014H,037H,028H,032H,014H,035H,02CH,0FBH,0FFH,032H,04H,02BH,04H,032H,04H
DB 0FBH,00H,037H,04H,0FBH,03H,03AH,050H,0FBH,01H,03AH,014H,03BH,0AH,03AH,0AH
DB 037H,01EH,039H,0AH,0FBH,0FFH,03CH,04H,039H,04H,035H,04H,039H,04H,0FBH,00H
DB 03AH,04H,0FBH,03H,03EH,028H,0FBH,01H,03AH,014H,037H,014H,035H,014H,034H,014H
DB 02EH,028H,035H,014H,0FBH,0FFH,037H,04H,034H,04H,032H,04H,02BH,04H,0FBH,00H
DB 02EH,04H,0FBH,02H,0F8H,06H,0FFH,032H,078H,0FDH
egyptian_anthem_part3 LABEL BYTE
DB 0FCH,0FH,0F9H,06EH,0F4H,00H,024H,050H,0F4H,07FH,022H,03CH,0F4H,00H,018H,03CH
DB 01CH,028H,0F4H,07FH,0F3H,05H,0F6H,0F9H,070H,0F8H,0AH,03H,01FH,0AH,022H,0AH
DB 026H,0AH,028H,0AH,0F3H,00H,00H,0F8H,00H,00H,0F5H,00H,0F9H,06EH,024H,050H
DB 0F4H,07FH,022H,03CH,0F4H,00H,024H,050H,0F4H,040H,022H,03CH,0F4H,07FH,0F3H,05H
DB 0F6H,0F9H,06EH,0F8H,0AH,03H,013H,0AH,01AH,0AH,01CH,0AH,01FH,0AH,022H,0AH
DB 0F9H,064H,0F3H,00H,00H,013H,05AH,0FDH
egyptian_anthem_part4 LABEL BYTE
DB 0FCH,0FH,0F9H,06CH,0F4H,00H,018H,050H,0F4H,07FH,016H,03CH,0F4H,00H,01FH,03CH
DB 028H,028H,0F4H,07FH,0F3H,05H,0F6H,0F9H,06EH,0F8H,0AH,03H,02BH,0AH,02EH,0AH
DB 032H,0AH,034H,0AH,0F8H,00H,00H,0F3H,00H,00H,0F5H,00H,0F4H,00H,0F9H,06CH
DB 030H,050H,0F4H,07FH,016H,03CH,0F4H,00H,018H,050H,0F4H,040H,016H,03CH,0F4H,07FH
DB 0F3H,05H,0F6H,0F9H,06EH,0F8H,0AH,03H,024H,0AH,02BH,0AH,02DH,0AH,030H,0AH
DB 033H,0AH,0F9H,064H,0F3H,00H,00H,013H,05AH,0FDH
egyptian_anthem_part5 LABEL BYTE
DB 0FCH,01BH,0F9H,064H,0F4H,00H,018H,050H,0F4H,07FH,016H,03CH,0F4H,00H,018H,03CH
DB 010H,028H,0F4H,07FH,0F3H,05H,0F6H,0F8H,0AH,03H,013H,0AH,016H,0AH,0EH,0AH
DB 010H,0AH,0F8H,00H,00H,0F3H,00H,00H,0F5H,00H,0F4H,00H,0F9H,064H,010H,050H
DB 0F4H,07FH,016H,03CH,0F4H,00H,018H,050H,0F4H,040H,016H,03CH,0F4H,07FH,0F3H,05H
DB 0F6H,0F8H,0AH,03H,02BH,0AH,032H,0AH,034H,0AH,037H,0AH,03AH,0AH,0F5H,00H
DB 0F8H,07H,0FFH,0F3H,00H,00H,013H,05AH,0FDH,00H
egyptian_anthem_part1 LABEL BYTE
DB 0FCH,023H,0F9H,05BH,044H,08CH,00H,08CH,00H,08CH,044H,08CH,00H,08CH,0FDH,00H
egyptian_anthem_part6 LABEL BYTE
DB 0F4H,050H,0FCH,02BH,00H,014H,0F8H,0AH,0FFH,0FFH,00H,0F9H,05DH,02BH,02H,0F9H
DB 055H,02BH,02H,00H,010H,0FFH,05H,0F5H,00H,0F8H,0AH,02H,0F4H,028H,0FCH,02CH
DB 0F9H,046H,024H,0AH,024H,0AH,024H,0AH,024H,0AH,00H,0F0H,0F5H,00H,00H,014H
DB 0F4H,050H,0FCH,02BH,0F8H,0AH,0FFH,0FFH,00H,0F9H,060H,02BH,02H,0F9H,058H,02BH
DB 02H,00H,010H,0FFH,05H,0F5H,00H,00H,03CH,0F4H,028H,0FCH,02CH,0F9H,046H,0F8H
DB 0AH,02H,024H,0AH,024H,0AH,024H,0AH,024H,0AH,00H,028H,0F5H,00H,0FDH,00H
egyptian_motif_part1 LABEL BYTE
DB 0FCH,0FH,0F5H,0CH,0F4H,07FH,0F3H,05H,0F6H,0F9H,06EH,0F8H,0AH,03H,013H,0AH
DB 01AH,0AH,01CH,0AH,01FH,0AH,022H,0AH,0F9H,064H,0F3H,00H,00H,013H,05AH,00H
DB 00H,00H
egyptian_motif_part2 LABEL BYTE
DB 0FCH,0FH,0F5H,0CH,0F4H,07FH,0F3H,05H,0F6H,0F9H,06EH,0F8H,0AH,03H,024H,0AH
DB 02BH,0AH,02DH,0AH,030H,0AH,033H,0AH,0F9H,064H,0F3H,00H,00H,013H,05AH,00H
DB 00H,00H
egyptian_motif_part3 LABEL BYTE
DB 0FCH,01BH,0F5H,0CH,0F4H,07FH,0F3H,05H,0F6H,0F9H,064H,0F8H,0AH,03H,02BH,0AH
DB 032H,0AH,034H,0AH,037H,0AH,03AH,0AH,0F5H,00H,0F8H,04H,0FFH,0F3H,05H,0FFH
DB 013H,05AH,00H,00H
egyptian_motif_part4 LABEL BYTE
DB 0FCH,02CH,0F9H,05AH,0F8H,0AH,0FEH,0FFH,00H,024H,0AH,0FFH,07H,0F8H,00H,00H
DB 00H,00H
zulu_anthem_part1 LABEL BYTE
DB 0F4H,04AH,0FCH,02CH,0FFH,00H,0F9H,064H,0FBH,08H,025H,010H,0F9H,050H,025H,010H
DB 0FBH,04H,025H,08H,0FBH,08H,0F9H,064H,025H,010H,0F9H,050H,025H,010H,025H,010H
DB 0FBH,04H,025H,08H,0FBH,08H,0F9H,064H,025H,010H,0F9H,050H,025H,010H,0FBH,04H
DB 025H,08H,0FBH,08H,0F9H,063H,025H,010H,0F9H,050H,025H,010H,025H,010H,0FBH,04H
DB 025H,08H,0FBH,08H,0F9H,065H,025H,010H,0F9H,04FH,025H,010H,0FBH,04H,025H,08H
DB 0FBH,08H,0F9H,062H,0FBH,02H,025H,04H,0F9H,050H,025H,04H,0FBH,04H,025H,08H
DB 0FBH,08H,025H,010H,025H,010H,0FBH,04H,025H,08H,0FBH,08H,0F9H,063H,025H,010H
DB 0F9H,050H,025H,010H,0F9H,061H,0FBH,04H,025H,08H,0F9H,050H,025H,08H,0FBH,02H
DB 025H,04H,025H,04H,0FBH,08H,025H,010H,025H,010H,0FBH,02H,025H,04H,025H,04H
DB 0FFH,01H,0FDH,00H
zulu_anthem_part2 LABEL BYTE
DB 0F4H,05EH,0FCH,02DH,0FEH,00H,0F9H,06EH,02CH,010H,0F9H,080H,02DH,08H,0F9H,06EH
DB 02CH,08H,0F9H,081H,02DH,08H,0F9H,06EH,02DH,010H,0F9H,080H,02DH,08H,0F9H,06EH
DB 02CH,08H,0F9H,07FH,02DH,08H,0F9H,06EH,02CH,08H,0F9H,081H,02DH,08H,0F9H,06EH
DB 0FFH,01H,02CH,010H,0F9H,082H,02DH,08H,0F9H,06EH,02CH,08H,02DH,04H,02DH,04H
DB 0F9H,080H,02DH,010H,02DH,08H,0F9H,06EH,02CH,08H,0F9H,081H,02DH,08H,0F9H,06EH
DB 02CH,08H,0F9H,07FH,02DH,08H,0F9H,06EH,02CH,010H,02CH,04H,02CH,04H,0F9H,081H
DB 02DH,08H,0F9H,06EH,02DH,08H,0F9H,080H,02DH,04H,0F9H,06EH,02DH,0CH,02DH,08H
DB 02CH,08H,02DH,08H,02CH,04H,02CH,04H,0F9H,082H,02DH,08H,0F9H,06BH,0FFH,01H
DB 0F5H,0F6H,0FEH,01H,0FDH,00H
zulu_anthem_part3 LABEL BYTE
DB 0F4H,036H,0FCH,02EH,0FFH,00H,0FBH,04H,0F9H,05FH,01CH,010H,0F9H,064H,01EH,08H
DB 0F9H,06EH,01CH,010H,01EH,08H,01CH,010H,0F9H,05FH,01CH,08H,0F9H,064H,01EH,010H
DB 0F9H,06EH,01EH,08H,0FFH,01H,0F9H,064H,01CH,010H,0F9H,05FH,01EH,08H,0F9H,06EH
DB 01CH,010H,0F9H,064H,01EH,08H,0F9H,05FH,0FBH,02H,01CH,04H,01CH,04H,0FBH,04H
DB 01CH,08H,0F9H,077H,01CH,08H,0F9H,05FH,01EH,010H,01EH,08H,0F9H,062H,01CH,010H
DB 0F9H,05FH,01EH,08H,0F9H,06EH,01CH,08H,0F9H,064H,0FBH,02H,01EH,04H,0F9H,066H
DB 01EH,04H,0F9H,068H,0FBH,04H,01CH,08H,0F9H,06AH,01CH,010H,0F9H,06CH,01CH,08H
DB 0F9H,078H,0FBH,02H,01EH,04H,0F9H,069H,01EH,04H,0F9H,064H,0FBH,04H,01CH,08H
DB 01EH,08H,0FFH,02H,0F5H,0F8H,0FEH,01H,0FDH,00H
zulu_anthem_part4 LABEL BYTE
DB 0F4H,022H,0FCH,01EH,0FFH,00H,0F9H,081H,0EH,04H,00H,04H,0F9H,075H,0CH,04H
DB 00H,04H,0EH,04H,00H,04H,0CH,04H,00H,04H,0F9H,083H,0CH,04H,00H,0CH
DB 0F9H,077H,0EH,04H,00H,04H,0F9H,07FH,0CH,04H,00H,0CH,0F9H,077H,0EH,04H
DB 00H,04H,0F9H,07DH,0CH,04H,00H,0CH,0F9H,077H,0EH,02H,00H,06H,0EH,02H
DB 00H,06H,0EH,02H,0F9H,083H,00H,06H,0F9H,075H,0CH,02H,00H,06H,0F9H,083H
DB 0CH,02H,00H,0EH,0F9H,073H,0EH,02H,00H,06H,0CH,02H,00H,0EH,0EH,02H
DB 00H,06H,0F9H,083H,0CH,02H,00H,0EH,0F9H,077H,0EH,02H,00H,06H,0EH,02H
DB 00H,06H,0EH,02H,00H,06H,0F9H,075H,0CH,02H,00H,02H,0F9H,07DH,0CH,02H
DB 00H,02H,0F9H,083H,0EH,02H,00H,0EH,0F9H,075H,0EH,02H,00H,06H,0CH,02H
DB 00H,0EH,0EH,02H,00H,06H,0CH,02H,00H,0EH,0F9H,079H,0EH,02H,00H,02H
DB 0F9H,07DH,0EH,02H,00H,02H,0F9H,083H,0EH,02H,00H,06H,0F9H,075H,0EH,02H
DB 00H,06H,0F9H,079H,0CH,02H,00H,02H,0F9H,07FH,0CH,02H,00H,02H,0F9H,083H
DB 0CH,02H,00H,0EH,0F9H,077H,0EH,02H,00H,06H,0CH,02H,00H,0EH,0EH,02H
DB 00H,06H,0CH,02H,00H,0EH,0FFH,02H,0F5H,0F6H,0FEH,01H,0FDH,00H
zulu_motif_part1 LABEL BYTE
DB 0F4H,04AH,0FCH,02CH,0FFH,00H,0F9H,065H,025H,010H,0F9H,04FH,025H,010H,0FBH,04H
DB 025H,08H,0F9H,062H,0FBH,02H,025H,04H,0F9H,050H,025H,04H,0FFH,01H,0F8H,04H
DB 01H,0FFH,00H,025H,04H,0FFH,0DH,0F8H,00H,00H,00H,00H
zulu_motif_part2 LABEL BYTE
DB 0F4H,05EH,0FCH,02DH,0FFH,00H,02CH,010H,0F9H,082H,02DH,08H,0F9H,06EH,02CH,08H
DB 02DH,04H,02DH,04H,0F9H,080H,02DH,08H,0FFH,01H,0F9H,06EH,0F8H,04H,01H,0FFH
DB 00H,02DH,04H,0FFH,0DH,00H,00H,00H
zulu_motif_part3 LABEL BYTE
DB 0F4H,036H,0FCH,02EH,0FFH,00H,0F9H,064H,01CH,010H,0F9H,05FH,01EH,08H,0F9H,06EH
DB 01CH,010H,0F9H,064H,01EH,08H,0FFH,01H,0F9H,05FH,0FBH,02H,0F8H,04H,01H,0FFH
DB 00H,01EH,04H,0FFH,0DH,00H,00H,00H
zulu_motif_part4 LABEL BYTE
DB 0F4H,022H,0FCH,01EH,0FFH,00H,0F9H,075H,0EH,02H,00H,06H,0EH,02H,00H,06H
DB 0EH,02H,00H,06H,0F9H,076H,0CH,02H,00H,02H,0F9H,07BH,0CH,02H,00H,02H
DB 0EH,02H,00H,0EH,0FFH,01H,0F9H,06DH,0F8H,08H,01H,0FFH,00H,0EH,02H,00H
DB 02H,0FFH,0DH,00H,00H,00H
roman_anthem_part1 LABEL BYTE
DB 00H,050H,00H,050H,0FBH,01H,00H,050H,0F4H,018H,0FCH,036H,0F9H,05FH,0F8H,01EH
DB 01H,035H,050H,035H,050H,0F8H,00H,00H,0FBH,03H,037H,050H,0FBH,00H,0FCH,05H
DB 0FEH,00H,0F5H,00H,0F9H,05FH,0F8H,078H,01H,045H,0AH,00H,0AH,048H,028H,043H
DB 0AH,00H,0AH,045H,028H,04AH,0AH,00H,0AH,040H,014H,045H,0AH,00H,0AH,04AH
DB 014H,048H,0AH,00H,0AH,043H,014H,045H,0AH,00H,0AH,04CH,028H,03EH,0AH,00H
DB 0AH,046H,028H,03AH,014H,041H,0AH,00H,0AH,048H,028H,04CH,014H,045H,0AH,00H
DB 0AH,04AH,028H,03EH,014H,045H,0AH,00H,0AH,04CH,028H,04FH,014H,048H,0AH,00H
DB 0AH,04DH,028H,041H,014H,048H,0AH,00H,0AH,04FH,028H,051H,014H,04AH,0AH,00H
DB 0AH,0F9H,060H,0F8H,00H,00H,0FBH,03H,051H,050H,051H,050H,04FH,050H,04FH,050H
DB 043H,050H,0FEH,05H,0FDH,00H
roman_anthem_part2 LABEL BYTE
DB 0FCH,036H,0F9H,06DH,0FBH,02H,0F4H,051H,0F5H,04H,02EH,05H,00H,05H,0F5H,00H
DB 02EH,01EH,0F4H,02FH,0F5H,04H,030H,05H,00H,05H,0F5H,00H,030H,01EH,0F4H,051H
DB 0F5H,04H,034H,05H,00H,05H,0F5H,00H,034H,01EH,0F4H,02FH,0F5H,04H,032H,05H
DB 00H,05H,0F5H,00H,032H,01EH,0F4H,018H,0F9H,063H,0F8H,01EH,01H,0FBH,02H,037H
DB 050H,03AH,050H,030H,050H,0F8H,00H,00H,0FBH,03H,032H,050H,0FBH,00H,0FEH,00H
DB 0F5H,00H,0F9H,05DH,0F8H,050H,01H,039H,0AH,00H,0AH,03CH,028H,037H,0AH,00H
DB 0AH,039H,028H,03EH,0AH,00H,0AH,034H,014H,039H,0AH,00H,0AH,03EH,014H,03CH
DB 0AH,00H,0AH,037H,014H,039H,0AH,00H,0AH,040H,028H,032H,0AH,00H,0AH,02EH
DB 028H,029H,014H,030H,0AH,00H,0AH,030H,028H,039H,014H,032H,0AH,00H,0AH,032H
DB 028H,02DH,014H,034H,0AH,00H,0AH,034H,028H,03CH,014H,035H,0AH,00H,0AH,035H
DB 028H,030H,014H,037H,0AH,00H,0AH,037H,028H,039H,014H,037H,0AH,00H,0AH,0FBH
DB 03H,0F8H,00H,00H,039H,050H,039H,050H,037H,050H,02BH,050H,02BH,050H,0FEH,05H
DB 0FDH,00H
roman_anthem_part8 LABEL BYTE
DB 0FCH,036H,0F9H,071H,0FBH,02H,0F4H,02FH,0F5H,04H,029H,014H,0F5H,00H,029H,05H
DB 00H,05H,0F4H,051H,0F5H,04H,029H,01EH,0F5H,00H,029H,05H,00H,05H,0F4H,02FH
DB 0F5H,04H,029H,01EH,0F5H,0FEH,02DH,05H,00H,05H,0F4H,051H,02DH,01EH,028H,05H
DB 00H,05H,02DH,05H,00H,05H,0F4H,054H,030H,078H,0F5H,00H,032H,0AH,032H,01EH
DB 0F8H,014H,01H,037H,05H,00H,05H,037H,032H,03EH,05H,00H,05H,041H,05H,00H
DB 05H,0FBH,03H,0F8H,00H,00H,03AH,050H,0F5H,00H,0FCH,05H,0FEH,00H,0F5H,00H
DB 0F9H,051H,0F8H,050H,01H,0FBH,05H,0FFH,00H,030H,014H,037H,0AH,032H,0AH,02DH
DB 014H,030H,014H,037H,0AH,032H,0AH,02DH,014H,030H,014H,037H,0AH,032H,0AH,0FFH
DB 04H,0F8H,00H,00H,030H,014H,037H,0AH,032H,0AH,02DH,014H,030H,014H,030H,014H
DB 037H,0AH,032H,0AH,02DH,014H,030H,014H,037H,0AH,032H,0AH,02DH,014H,030H,014H
DB 037H,0AH,032H,0AH,037H,0AH,032H,0AH,02DH,014H,030H,014H,037H,0AH,032H,0AH
DB 037H,0AH,032H,0AH,02DH,014H,030H,014H,037H,0AH,032H,0AH,0FEH,05H,0FDH,00H
roman_anthem_part7 LABEL BYTE
DB 0FFH,00H,00H,050H,0FFH,05H,0F4H,054H,0FCH,05H,0FEH,00H,0F5H,00H,0F9H,051H
DB 0F8H,050H,01H,0FBH,05H,0FFH,00H,037H,014H,03EH,0AH,039H,0AH,028H,014H,037H
DB 014H,03EH,0AH,039H,0AH,034H,014H,037H,014H,03EH,0AH,039H,0AH,0FFH,04H,0F8H
DB 00H,00H,037H,014H,03EH,0AH,039H,0AH,034H,014H,037H,014H,037H,014H,03EH,0AH
DB 039H,0AH,034H,014H,037H,014H,03EH,0AH,039H,0AH,034H,014H,037H,014H,03EH,0AH
DB 039H,0AH,03EH,0AH,039H,0AH,034H,014H,037H,014H,03EH,0AH,039H,0AH,032H,0AH
DB 039H,0AH,034H,014H,037H,014H,03EH,0AH,039H,0AH,0FEH,05H,0FDH,00H
roman_anthem_part4 LABEL BYTE
DB 0FCH,05H,0F9H,073H,0FBH,05H,022H,014H,0F5H,04H,022H,0AH,0F5H,00H,022H,01EH
DB 0F5H,04H,024H,0AH,0F5H,00H,024H,01EH,0F5H,04H,028H,0AH,0F5H,00H,028H,01EH
DB 026H,0AH,026H,0AH,0F4H,02CH,0F5H,02H,02BH,08CH,02BH,0AH,02BH,01EH,0F8H,014H
DB 01H,032H,0AH,035H,0AH,03AH,0AH,03AH,0AH,03AH,0AH,03AH,0AH,0FBH,03H,0F8H
DB 00H,00H,030H,050H,0FBH,05H,0FCH,05H,0FEH,00H,0F5H,00H,0F9H,059H,0F8H,040H
DB 01H,0FFH,00H,029H,0AH,032H,0AH,02DH,014H,037H,0AH,032H,0AH,035H,0AH,039H
DB 0AH,035H,0AH,032H,0AH,039H,014H,035H,0AH,032H,0AH,02DH,014H,0FFH,04H,0F8H
DB 00H,00H,029H,0AH,032H,0AH,02DH,014H,037H,0AH,032H,0AH,035H,0AH,039H,0AH
DB 029H,0AH,032H,0AH,02DH,014H,037H,0AH,032H,0AH,035H,0AH,039H,0AH,035H,0AH
DB 032H,0AH,039H,014H,035H,0AH,032H,0AH,02DH,014H,030H,0AH,032H,0AH,039H,014H
DB 035H,0AH,032H,0AH,02DH,014H,03CH,0AH,032H,0AH,039H,014H,035H,0AH,032H,0AH
DB 02DH,014H,0FEH,05H,0FDH,00H
roman_anthem_part5 LABEL BYTE
DB 0F4H,02CH,0FFH,00H,00H,050H,0FFH,05H,0FCH,05H,0FBH,05H,0FEH,00H,0F5H,00H
DB 0F9H,059H,0F8H,040H,01H,0FFH,00H,030H,0AH,039H,0AH,034H,014H,03EH,0AH,039H
DB 0AH,03CH,0AH,040H,0AH,03CH,0AH,039H,0AH,040H,014H,03CH,0AH,039H,0AH,034H
DB 014H,0FFH,04H,0F8H,00H,00H,030H,0AH,039H,0AH,034H,014H,03EH,0AH,039H,0AH
DB 03CH,0AH,040H,0AH,030H,0AH,039H,0AH,034H,014H,03EH,0AH,039H,0AH,03CH,0AH
DB 040H,0AH,03CH,0AH,039H,0AH,040H,014H,03CH,0AH,039H,0AH,034H,014H,029H,0AH
DB 039H,0AH,040H,014H,03CH,0AH,039H,0AH,034H,014H,029H,0AH,039H,0AH,040H,014H
DB 03CH,0AH,039H,0AH,034H,014H,0FEH,05H,0FDH,00H
roman_anthem_part6 LABEL BYTE
DB 0FCH,03H,0F9H,071H,0FBH,05H,0F4H,00H,011H,050H,0F4H,07FH,015H,050H,0F4H,054H
DB 0F8H,0AH,02H,018H,0AH,018H,0AH,018H,0AH,018H,0AH,018H,0AH,018H,0AH,018H
DB 0AH,018H,0AH,0F9H,069H,0F8H,00H,00H,0F5H,00H,01FH,078H,011H,028H,0F9H,065H
DB 0F8H,0AH,02H,010H,0AH,01CH,0AH,01CH,0AH,01CH,0AH,01CH,0AH,01CH,0AH,01CH
DB 0AH,01CH,0AH,0FCH,06H,0FEH,00H,0F5H,00H,0F9H,069H,0F8H,078H,01H,0FBH,0AH
DB 01FH,03CH,01DH,014H,01FH,03CH,01DH,014H,01FH,03CH,01DH,014H,01FH,03CH,01DH,014H
DB 01FH,050H,021H,050H,022H,050H,024H,050H,026H,050H,028H,050H,029H,050H,029H,050H
DB 02BH,050H,01FH,050H,01FH,050H,0FEH,05H,0F8H,00H,00H,0FDH
roman_anthem_part3 LABEL BYTE
DB 0FCH,036H,0F9H,067H,0F4H,051H,0F5H,04H,03AH,05H,00H,05H,0F5H,00H,03AH,01EH
DB 0F4H,02FH,0F5H,04H,03CH,05H,00H,05H,0F5H,00H,03CH,01EH,0F4H,051H,0F5H,04H
DB 040H,05H,00H,05H,0F5H,00H,040H,01EH,0F4H,02FH,0F5H,04H,03EH,05H,00H,05H
DB 0F5H,00H,03EH,01EH,0F4H,018H,0F9H,05BH,0F8H,01EH,01H,043H,050H,030H,050H,02EH
DB 050H,0F8H,00H,00H,02DH,050H,0FEH,00H,0F5H,00H,0F9H,051H,0F8H,050H,01H,039H
DB 0AH,00H,0AH,03CH,028H,037H,0AH,00H,0AH,039H,028H,03EH,0AH,00H,0AH,034H
DB 014H,039H,0AH,00H,0AH,03EH,014H,03CH,0AH,00H,0AH,037H,014H,039H,0AH,00H
DB 0AH,040H,028H,032H,0AH,00H,0AH,02EH,028H,029H,014H,030H,0AH,00H,0AH,030H
DB 028H,039H,014H,032H,0AH,00H,0AH,032H,028H,02DH,014H,034H,0AH,00H,0AH,034H
DB 028H,03CH,014H,035H,0AH,00H,0AH,035H,028H,030H,014H,037H,0AH,00H,0AH,037H
DB 028H,039H,014H,037H,0AH,00H,0AH,0FBH,00H,0F8H,00H,00H,03EH,050H,03EH,050H
DB 043H,050H,043H,050H,037H,050H,0FEH,05H,0FDH,00H
roman_motif_part1 LABEL BYTE
DB 0FCH,036H,0F9H,05BH,0F4H,07FH,0F5H,04H,03BH,05H,00H,05H,0F5H,00H,03BH,01EH
DB 0F4H,00H,0F5H,04H,03DH,05H,00H,05H,0F5H,00H,03DH,01EH,0F4H,07FH,0F5H,04H
DB 041H,05H,00H,05H,0F5H,00H,041H,01EH,0F4H,00H,0F5H,04H,03FH,05H,00H,05H
DB 0F5H,00H,03FH,01EH,0F4H,07FH,0F9H,047H,0F8H,01EH,01H,044H,050H,0F8H,00H,00H
DB 00H,00H
roman_motif_part2 LABEL BYTE
DB 0FCH,036H,0F9H,06DH,0FBH,02H,0F4H,07FH,0F5H,04H,02FH,05H,00H,05H,0F5H,00H
DB 02FH,01EH,0F4H,00H,0F5H,04H,031H,05H,00H,05H,0F5H,00H,031H,01EH,0F4H,07FH
DB 0F5H,04H,036H,05H,00H,05H,0F5H,00H,035H,01EH,0F4H,00H,0F5H,04H,033H,05H
DB 00H,05H,0F5H,00H,033H,01EH,0F4H,07FH,0F9H,05BH,0F8H,01EH,01H,0FBH,02H,038H
DB 050H,0F8H,00H,00H,00H,00H
roman_motif_part3 LABEL BYTE
DB 0FCH,036H,0F9H,067H,0FBH,02H,0F4H,00H,0F5H,04H,02AH,014H,0F5H,00H,02AH,05H
DB 00H,05H,0F4H,07FH,0F5H,04H,02AH,01EH,0F5H,00H,02AH,05H,00H,05H,0F4H,00H
DB 0F5H,04H,02AH,01EH,0F5H,0FEH,02EH,05H,00H,05H,0F4H,07FH,02EH,01EH,029H,05H
DB 00H,05H,02EH,05H,00H,05H,0F4H,00H,031H,050H,00H,00H
roman_motif_part4 LABEL BYTE
DB 0FCH,05H,0F9H,06BH,0FBH,05H,023H,014H,0F5H,04H,023H,0AH,0F5H,00H,023H,01EH
DB 0F5H,04H,025H,0AH,0F5H,00H,025H,01EH,0F5H,04H,029H,0AH,0F5H,00H,029H,01EH
DB 027H,0AH,027H,0AH,0F5H,02H,02CH,050H,00H,00H
roman_motif_part5 LABEL BYTE
DB 0FCH,03H,0F9H,067H,0FBH,05H,0F4H,00H,012H,050H,0F4H,07FH,016H,050H,0F4H,068H
DB 0F8H,0AH,02H,019H,0AH,019H,0AH,019H,0AH,019H,0AH,019H,0AH,019H,0AH,019H
DB 0AH,019H,0AH,0F8H,00H,00H,019H,0FH,00H,00H
babylonian_anthem_part1 LABEL BYTE
DB 0F4H,036H,0FCH,030H,0F9H,074H,0FBH,02H,0F8H,018H,01H,03CH,018H,03DH,0CH,03CH
DB 06H,00H,06H,040H,012H,00H,06H,043H,012H,00H,03H,0FBH,0FFH,048H,03H,0FBH
DB 02H,045H,018H,043H,018H,042H,018H,046H,012H,00H,06H,0F8H,018H,0FFH,048H,015H
DB 0FBH,0FFH,04CH,03H,0FBH,02H,049H,0CH,048H,06H,00H,06H,040H,012H,00H,06H
DB 043H,012H,00H,03H,0FBH,0FFH,046H,03H,0FBH,02H,045H,012H,00H,06H,043H,015H
DB 0FBH,0FFH,045H,03H,0FBH,02H,042H,030H,0F5H,00H,0F8H,018H,01H,03CH,012H,00H
DB 03H,0FBH,0FFH,040H,03H,0FBH,02H,03DH,0CH,03CH,09H,0FBH,0FFH,043H,03H,0FBH
DB 02H,040H,012H,00H,06H,043H,012H,00H,03H,0FBH,0FFH,046H,03H,0FBH,02H,045H
DB 0CH,048H,09H,0FBH,0FFH,046H,03H,0FBH,02H,043H,0CH,049H,0CH,048H,012H,00H
DB 06H,040H,012H,00H,06H,0F8H,018H,0FFH,042H,012H,00H,03H,0FBH,0FFH,040H,03H
DB 0FBH,02H,03DH,0CH,03CH,0CH,03AH,012H,00H,06H,037H,012H,00H,03H,0FBH,0FFH
DB 03CH,03H,0FBH,02H,039H,012H,00H,03H,0FBH,0FFH,040H,03H,0FBH,02H,03DH,012H
DB 00H,03H,0FBH,0FFH,03FH,03H,0FBH,02H,03CH,078H,0F8H,00H,00H,0F5H,04H,03DH
DB 012H,00H,03H,0F5H,00H,0FBH,0FFH,03FH,03H,0FBH,02H,03CH,075H,00H,03H,0F5H
DB 04H,0F8H,06H,01H,043H,06H,045H,06H,046H,06H,045H,06H,0F8H,06H,0FFH,043H
DB 06H,042H,06H,040H,06H,03FH,06H,03DH,06H,03CH,0A2H,0F8H,00H,00H,00H,00H
babylonian_anthem_part2 LABEL BYTE
DB 0F4H,030H,0FCH,014H,0F9H,062H,0F8H,018H,01H,030H,018H,031H,0CH,030H,06H,00H
DB 06H,034H,012H,00H,06H,037H,012H,00H,03H,0FBH,0FFH,03CH,03H,0FBH,02H,039H
DB 018H,037H,018H,036H,018H,03AH,012H,00H,06H,0F8H,018H,0FFH,03CH,015H,0FBH,0FFH
DB 040H,03H,0FBH,02H,03DH,0CH,03CH,06H,00H,06H,034H,012H,00H,06H,037H,012H
DB 00H,03H,0FBH,0FFH,03AH,03H,0FBH,02H,039H,012H,00H,06H,037H,015H,0FBH,0FFH
DB 039H,03H,0FBH,02H,036H,030H,0F5H,00H,0F8H,018H,01H,030H,012H,00H,03H,0FBH
DB 0FFH,034H,03H,0FBH,02H,031H,0CH,030H,09H,0FBH,0FFH,037H,03H,0FBH,02H,034H
DB 012H,00H,06H,037H,012H,00H,03H,0FBH,0FFH,03AH,03H,0FBH,02H,039H,0CH,03CH
DB 09H,0FBH,0FFH,03AH,03H,0FBH,02H,037H,0CH,03DH,0CH,03CH,012H,00H,06H,034H
DB 012H,00H,06H,0F8H,018H,0FFH,036H,012H,00H,03H,0FBH,0FFH,034H,03H,0FBH,02H
DB 031H,0CH,030H,0CH,02EH,012H,00H,06H,02BH,012H,00H,03H,030H,03H,02DH,012H
DB 00H,03H,0FBH,0FFH,034H,03H,0FBH,02H,031H,012H,00H,03H,0FBH,0FFH,033H,03H
DB 0FBH,02H,030H,078H,0F8H,00H,00H,0F5H,08H,031H,012H,00H,03H,0F5H,00H,0FBH
DB 0FFH,033H,03H,0FBH,02H,030H,075H,00H,03H,0F5H,04H,0F8H,06H,01H,037H,06H
DB 039H,06H,03AH,06H,039H,06H,0F8H,06H,0FFH,037H,06H,036H,06H,034H,06H,033H
DB 06H,031H,06H,030H,0A2H,0F8H,00H,00H,00H,00H
babylonian_anthem_part3 LABEL BYTE
DB 0F4H,028H,0FCH,013H,0F9H,06EH,0F8H,02H,0FFH,0FEH,00H,00H,012H,0F5H,0AH,02AH
DB 06H,02EH,06H,02BH,042H,0FFH,00H,00H,012H,0F5H,0AH,01EH,06H,022H,06H,01FH
DB 042H,0FFH,01H,00H,060H,0FEH,01H,0F8H,07H,0FFH,00H,042H,0F5H,0AH,01EH,06H
DB 022H,06H,01FH,0D2H,00H,030H,0F5H,0AH,0F8H,0AH,0FFH,00H,012H,02AH,06H,02EH
DB 06H,02BH,072H,0FFH,00H,00H,012H,01EH,06H,022H,06H,01FH,072H,0FFH,01H,00H
DB 00H,00H
babylonian_anthem_part4 LABEL BYTE
DB 0F4H,07FH,0FBH,06H,0FCH,00H,0F9H,050H,0FEH,00H,0F5H,08H,027H,048H,0F5H,02H
DB 027H,018H,0FFH,02H,0F5H,0AH,027H,048H,0F5H,00H,024H,018H,0FEH,01H,027H,048H
DB 024H,018H,0FFH,01H,027H,018H,025H,018H,027H,018H,024H,018H,0FFH,02H,0F8H,0CH
DB 0FFH,027H,018H,024H,018H,027H,018H,025H,018H,027H,018H,024H,018H,027H,018H,024H
DB 018H,027H,018H,025H,018H,027H,018H,024H,018H,0F8H,00H,00H,00H,00H
babylonian_anthem_part5 LABEL BYTE
DB 0F4H,0AH,0FBH,06H,0FCH,00H,0F9H,050H,0FEH,00H,0F5H,08H,024H,048H,00H,018H
DB 0FFH,02H,0F5H,0AH,024H,048H,00H,018H,0FEH,01H,024H,048H,00H,018H,0FFH,01H
DB 024H,018H,00H,018H,024H,018H,00H,018H,0FFH,02H,0F8H,0CH,0FFH,024H,018H,00H
DB 018H,024H,018H,00H,018H,024H,018H,00H,018H,024H,018H,00H,018H,024H,018H,00H
DB 018H,024H,018H,00H,018H,0F8H,00H,00H,00H,00H
babylonian_anthem_part6 LABEL BYTE
DB 0F4H,014H,0FBH,06H,0FCH,00H,0F9H,050H,0FEH,00H,0F5H,08H,01CH,048H,0F5H,0CH
DB 01BH,018H,0FFH,02H,0F5H,0AH,01CH,048H,016H,018H,0FEH,01H,01CH,048H,016H,018H
DB 0FFH,01H,01BH,018H,01BH,018H,01CH,018H,016H,018H,0FFH,02H,0F8H,0CH,0FFH,01CH
DB 018H,016H,018H,01CH,018H,01BH,018H,01CH,018H,016H,018H,01CH,018H,016H,018H,01CH
DB 018H,01BH,018H,01CH,018H,016H,018H,0F8H,00H,00H,00H,00H
babylonian_anthem_part7 LABEL BYTE
DB 0FFH,00H,0F4H,00H,0FCH,02DH,0F9H,06CH,02DH,06H,00H,012H,0F4H,07FH,0FCH,02AH
DB 0F9H,066H,022H,0CH,022H,0CH,0F4H,040H,0F9H,06CH,01DH,0CH,00H,0CH,0F9H,066H
DB 01DH,0CH,00H,0CH,0FFH,05H,0F4H,00H,0FCH,02DH,0F9H,06CH,02DH,06H,00H,012H
DB 0F4H,07FH,0FCH,02AH,0F9H,068H,022H,06H,00H,06H,0F4H,040H,0F9H,06CH,01FH,06H
DB 00H,06H,01CH,06H,00H,012H,0F4H,00H,0F9H,06AH,02DH,06H,00H,012H,0F4H,040H
DB 0FCH,02DH,0F9H,06CH,037H,06H,00H,012H,037H,06H,00H,06H,0F4H,020H,0FCH,02AH
DB 0F9H,068H,016H,06H,00H,01EH,0F4H,040H,0FCH,02DH,0F9H,06CH,037H,06H,00H,06H
DB 0F4H,020H,0FCH,02AH,0F9H,06AH,016H,06H,00H,06H,0F4H,07FH,01FH,06H,00H,012H
DB 0F4H,040H,0FCH,02DH,0F9H,06CH,037H,06H,00H,06H,0F4H,020H,0FCH,02AH,0F9H,06AH
DB 016H,06H,00H,06H,0F4H,040H,0FCH,02DH,0F9H,06CH,037H,06H,00H,012H,0F4H,07FH
DB 0FCH,02AH,0F9H,068H,01FH,06H,00H,06H,01FH,06H,00H,01EH,01FH,06H,00H,06H
DB 01FH,06H,00H,06H,0F4H,040H,0FCH,02DH,0F9H,06CH,037H,06H,00H,012H,0F4H,07FH
DB 0FCH,02AH,0F9H,06AH,01FH,06H,00H,06H,01FH,06H,00H,06H,0F4H,040H,0FCH,02DH
DB 0F9H,06CH,037H,06H,00H,012H,037H,06H,00H,06H,0F4H,020H,0FCH,02AH,0F9H,068H
DB 016H,06H,00H,01EH,0F4H,040H,0FCH,02DH,0F9H,06CH,037H,06H,00H,06H,0F4H,020H
DB 0FCH,02AH,0F9H,06AH,016H,06H,00H,06H,0F4H,07FH,01FH,06H,00H,012H,0F4H,040H
DB 0FCH,02DH,0F9H,06CH,037H,06H,00H,06H,0F4H,020H,0FCH,02AH,0F9H,068H,016H,06H
DB 00H,06H,0F5H,0CH,0F8H,018H,0FFH,0FFH,00H,0F4H,040H,0FCH,02DH,0F9H,06CH,02BH
DB 06H,00H,012H,0F4H,07FH,0FCH,02AH,0F9H,06AH,01FH,06H,00H,06H,01FH,06H,00H
DB 01EH,01FH,06H,00H,06H,01FH,06H,00H,06H,0F4H,040H,0FCH,02DH,0F9H,06CH,037H
DB 06H,00H,012H,0F4H,07FH,0FCH,02AH,0F9H,06AH,01FH,06H,00H,06H,01FH,06H,00H
DB 06H,0FFH,02H,0F8H,00H,00H,00H,00H
babylonian_motif_part1 LABEL BYTE
DB 0F4H,036H,0FCH,01CH,0F9H,074H,0F8H,0CH,0FFH,03CH,0FCH,0F8H,00H,00H,00H,00H
babylonian_motif_part2 LABEL BYTE
DB 0F4H,030H,0FCH,014H,0F9H,06AH,0F8H,0CH,0FFH,030H,0FCH,0F8H,00H,00H,00H,00H
babylonian_motif_part3 LABEL BYTE
DB 0F4H,028H,0FCH,01CH,0F9H,06EH,00H,012H,01EH,06H,022H,06H,01FH,042H,00H,012H
DB 0F9H,06AH,02AH,06H,02EH,06H,02BH,042H,00H,012H,036H,06H,03AH,06H,037H,042H
DB 00H,00H
babylonian_motif_part4 LABEL BYTE
DB 0F4H,00H,0FBH,06H,0FCH,00H,0F9H,05AH,0F8H,018H,0FFH,0FFH,00H,027H,048H,025H
DB 018H,0FFH,02H,027H,048H,00H,00H,00H
babylonian_motif_part5 LABEL BYTE
DB 0F4H,0AH,0FBH,06H,0FCH,00H,0F9H,05AH,0F8H,018H,0FFH,0FFH,00H,024H,048H,00H
DB 018H,0FFH,02H,024H,048H,00H,00H,00H
babylonian_motif_part6 LABEL BYTE
DB 0F4H,014H,0FBH,06H,0FCH,00H,0F9H,05AH,0F8H,018H,0FFH,0FFH,00H,01CH,048H,01BH
DB 018H,0FFH,02H,01CH,048H,00H,00H,00H
babylonian_motif_part7 LABEL BYTE
DB 0F8H,018H,0FFH,0FFH,00H,0F4H,00H,0FCH,02DH,0F9H,062H,02DH,06H,00H,012H,0F4H
DB 07FH,0FCH,02AH,0F9H,05CH,022H,0CH,022H,0CH,0F4H,040H,0F9H,062H,01DH,0CH,00H
DB 0CH,0F9H,05CH,01DH,0CH,00H,0CH,0FFH,02H,0F8H,00H,00H,02DH,06H,00H,00H
mongol_anthem_part4 LABEL BYTE
DB 0FCH,01DH,0F4H,060H,0F9H,07EH,01FH,040H,0F4H,020H,01FH,030H,0F4H,060H,01FH,020H
DB 0F4H,020H,0F9H,068H,01FH,040H,01FH,08H,01FH,010H,0F9H,07EH,01FH,020H,0F4H,060H
DB 01FH,020H,0F9H,072H,01FH,020H,01FH,020H,01FH,028H,0F9H,05CH,016H,020H,00H,020H
DB 016H,020H,00H,020H,00H,020H,016H,020H,00H,020H,016H,020H,016H,020H,00H,010H
DB 016H,020H,00H,020H,016H,020H,00H,010H,0FDH,00H
mongol_anthem_part7 LABEL BYTE
DB 0F7H,0AH
mongol_anthem_part8 LABEL BYTE
DB 0FCH,033H,0F5H,08H,00H,08H,0FBH,018H,0F9H,07CH,0F4H,020H,013H,040H,0F4H,060H
DB 013H,028H,0F4H,020H,013H,020H,0F9H,072H,013H,040H,013H,010H,013H,018H,0F4H,060H
DB 0F9H,078H,013H,020H,0FBH,08H,013H,018H,0F9H,07EH,013H,010H,0F4H,020H,013H,010H
DB 013H,030H,013H,010H,0FBH,010H,0F9H,062H,00H,020H,013H,020H,00H,020H,013H,020H
DB 013H,020H,00H,020H,013H,020H,013H,020H,00H,020H,0FBH,08H,013H,010H,0FBH,010H
DB 00H,020H,013H,020H,00H,020H,0FBH,08H,013H,010H,0FDH,00H
mongol_anthem_part3 LABEL BYTE
DB 0FCH,020H,00H,010H,0F4H,06FH,0F9H,070H,01FH,040H,0F4H,011H,01FH,020H,01FH,018H
DB 0F4H,06FH,0F9H,076H,01FH,040H,01FH,018H,01FH,020H,0F9H,070H,01FH,020H,01FH,038H
DB 0F9H,078H,01FH,08H,01FH,020H,01FH,010H,0FCH,02AH,0F9H,05CH,013H,020H,013H,020H
DB 013H,020H,013H,020H,013H,020H,013H,020H,013H,020H,013H,020H,013H,020H,013H,010H
DB 013H,020H,013H,020H,013H,020H,013H,010H,0FDH,00H
mongol_anthem_part5 LABEL BYTE
DB 0F7H,0AH
mongol_anthem_part6 LABEL BYTE
DB 0FCH,022H,0F9H,072H,0F5H,08H,00H,018H,0F4H,011H,02BH,040H,0F4H,06FH,02BH,018H
DB 0F4H,011H,02BH,010H,02BH,040H,02BH,020H,02BH,020H,02BH,08H,02BH,020H,02BH,018H
DB 02BH,08H,02BH,018H,02BH,018H,02BH,018H,00H,0E0H,00H,0C0H,0FDH,00H
mongol_anthem_part1 LABEL BYTE
DB 0F7H,0AH
mongol_anthem_part2 LABEL BYTE
DB 0FCH,01FH,0F9H,078H,0F5H,08H,00H,018H,01FH,040H,01FH,028H,0F9H,07EH,01FH,040H
DB 01FH,048H,0F9H,076H,01FH,028H,01FH,018H,0F9H,07CH,01FH,020H,01FH,018H,01FH,010H
DB 0FCH,02DH,0F9H,064H,032H,010H,03AH,08H,03BH,08H,032H,010H,03AH,010H,03BH,08H
DB 032H,08H,03AH,010H,03BH,08H,032H,08H,03AH,08H,03BH,08H,0F9H,060H,032H,010H
DB 03AH,08H,03BH,08H,032H,010H,03AH,010H,03BH,08H,032H,08H,03AH,010H,032H,010H
DB 03AH,08H,03BH,08H,032H,010H,03AH,010H,03BH,08H,032H,08H,0F9H,05CH,032H,010H
DB 03AH,08H,03BH,08H,032H,010H,03AH,010H,03BH,08H,032H,08H,03AH,010H,03BH,08H
DB 032H,08H,0FDH,00H
mongol_motif_part1 LABEL BYTE
DB 0FCH,01DH,0F9H,078H,0F5H,08H,01FH,040H,01FH,018H,01FH,030H,00H,00H
mongol_motif_part5 LABEL BYTE
DB 0F7H,0AH
mongol_motif_part2 LABEL BYTE
DB 0FCH,01FH,0F9H,072H,0F5H,08H,00H,08H,013H,040H,013H,018H,013H,030H,00H,00H
mongol_motif_part3 LABEL BYTE
DB 0FCH,021H,0F9H,06AH,0F5H,08H,00H,010H,01FH,040H,01FH,018H,01FH,020H,00H,00H
mongol_motif_part6 LABEL BYTE
DB 0F7H,0AH
mongol_motif_part4 LABEL BYTE
DB 0FCH,022H,0F9H,06CH,0F5H,08H,00H,018H,02BH,040H,02BH,018H,02BH,018H,00H,00H
mongol_motif_part7 LABEL BYTE
DB 0F7H,0AH
mongol_motif_part8 LABEL BYTE
DB 0FCH,01FH,0F9H,072H,0F5H,08H,00H,018H,01FH,040H,01FH,018H,01FH,028H,00H,00H
indian_anthem_part1 LABEL BYTE
DB 0F4H,054H,0FCH,01CH,0F9H,079H,00H,0C0H,00H,040H,0FBH,02H,035H,040H,037H,020H
DB 0FBH,0FFH,039H,040H,0FAH,0FDH,039H,010H,0FAH,03H,038H,010H,0FAH,00H,039H,01EH
DB 0FBH,02H,00H,02H,03CH,020H,0FBH,0FFH,039H,010H,0FAH,0FDH,039H,010H,0FAH,03H
DB 038H,010H,0FAH,00H,039H,02CH,00H,04H,035H,040H,0FAH,0FEH,035H,020H,0FAH,02H
DB 033H,020H,0FAH,00H,035H,05EH,0FBH,02H,00H,02H,035H,040H,037H,020H,0FBH,0FFH
DB 039H,040H,0FAH,0FDH,039H,010H,0FAH,03H,038H,010H,0FAH,00H,039H,01EH,00H,02H
DB 0FBH,0FFH,03CH,010H,0FAH,0FDH,03CH,0FH,0FBH,02H,0FAH,00H,00H,01H,039H,060H
DB 0FBH,0FFH,035H,034H,0FAH,0FEH,035H,025H,0FAH,02H,033H,025H,0FAH,00H,035H,0A0H
DB 00H,02H,0FBH,0FFH,041H,016H,0FAH,0FEH,041H,013H,0FAH,02H,040H,013H,0FAH,00H
DB 00H,04H,03EH,01CH,0FAH,0FEH,03EH,010H,0FAH,02H,03DH,010H,0FAH,00H,00H,04H
DB 03CH,020H,0FAH,0FEH,03CH,0EH,0FAH,02H,03BH,0EH,0FAH,00H,00H,04H,039H,042H
DB 0FAH,0FDH,039H,01FH,0FAH,03H,037H,01FH,0FAH,00H,039H,03EH,0FBH,04H,00H,02H
DB 035H,040H,037H,020H,0FBH,0FFH,039H,040H,0FAH,0FDH,039H,010H,0FAH,03H,038H,010H
DB 0FAH,00H,039H,01EH,00H,02H,03CH,010H,0FAH,0FDH,03CH,0FH,0FAH,00H,00H,01H
DB 039H,05EH,00H,02H,035H,036H,0FAH,0FEH,035H,025H,0FAH,02H,033H,025H,0FAH,00H
DB 035H,03EH,00H,02H,041H,018H,0FAH,0FEH,041H,013H,0FAH,02H,040H,013H,0FAH,00H
DB 00H,02H,03EH,01EH,0FAH,0FEH,03EH,010H,0FAH,02H,03DH,010H,0FAH,00H,00H,02H
DB 03CH,022H,0FAH,0FDH,03CH,0EH,0FAH,03H,03BH,0EH,0FAH,00H,00H,02H,039H,01EH
DB 0FAH,0FDH,039H,010H,0FAH,03H,038H,010H,0FAH,00H,00H,02H,037H,014H,0FAH,0FEH
DB 037H,015H,0FAH,02H,036H,015H,0FAH,00H,00H,02H,035H,018H,0FAH,0FEH,035H,013H
DB 0FAH,02H,034H,013H,0FAH,00H,00H,02H,032H,020H,0FAH,0FEH,032H,010H,0FAH,02H
DB 031H,010H,0FAH,00H,032H,010H,0FAH,0FEH,032H,010H,0FAH,02H,031H,010H,0FAH,00H
DB 032H,02EH,00H,02H,035H,034H,0FAH,0FEH,035H,025H,0FAH,02H,033H,025H,0FAH,00H
DB 035H,060H,0FBH,02H,00H,02H,037H,020H,0FBH,0FFH,039H,040H,0FAH,0FDH,039H,010H
DB 0FAH,03H,038H,010H,0FAH,00H,039H,01EH,00H,02H,03CH,010H,0FAH,0FDH,03CH,0FH
DB 0FBH,00H,0FAH,00H,00H,01H,039H,060H,0FBH,0FFH,035H,034H,0FAH,0FEH,035H,025H
DB 0FAH,02H,033H,025H,0FAH,00H,035H,040H,0FBH,02H,00H,02H,0FDH,00H
indian_anthem_part2 LABEL BYTE
DB 0F4H,02CH,0FCH,011H,0F9H,07AH,00H,0C0H,00H,040H,0FBH,04H,01DH,040H,01FH,020H
DB 0FBH,0FFH,021H,040H,0FAH,0FDH,021H,010H,0FAH,03H,020H,010H,0FAH,00H,021H,01EH
DB 0FBH,03H,00H,02H,024H,020H,0FBH,0FFH,021H,010H,0FAH,0FDH,021H,010H,0FAH,03H
DB 020H,010H,0FAH,00H,021H,02CH,00H,04H,01DH,034H,0FAH,0FEH,01DH,025H,0FAH,02H
DB 01BH,025H,0FAH,00H,01DH,060H,0FBH,03H,00H,02H,01DH,040H,01FH,020H,0FBH,0FFH
DB 021H,040H,0FAH,0FDH,021H,010H,0FAH,03H,020H,010H,0FAH,00H,021H,01EH,00H,02H
DB 024H,010H,0FAH,0FDH,024H,0FH,0FBH,02H,0FAH,00H,00H,01H,021H,060H,0FBH,0FFH
DB 01DH,038H,0FAH,0FEH,01DH,023H,0FAH,02H,01CH,023H,0FAH,00H,01DH,0A0H,00H,02H
DB 029H,020H,0FAH,0FEH,029H,013H,0FAH,02H,028H,0BH,0FAH,00H,00H,02H,026H,020H
DB 0FAH,0FEH,026H,010H,0FAH,02H,025H,0EH,0FAH,00H,00H,02H,024H,020H,0FAH,0FEH
DB 024H,010H,0FAH,02H,023H,0EH,0FAH,00H,00H,02H,021H,040H,0FAH,0FDH,021H,020H
DB 0FAH,03H,020H,020H,0FAH,00H,021H,03EH,0FBH,03H,00H,02H,01DH,040H,01FH,020H
DB 0FBH,0FFH,021H,040H,0FAH,0FDH,021H,010H,0FAH,03H,020H,010H,0FAH,00H,021H,01EH
DB 00H,02H,024H,010H,0FAH,0FDH,024H,0FH,0FBH,02H,0FAH,00H,00H,01H,021H,060H
DB 0FBH,0FFH,01DH,034H,0FAH,0FEH,01DH,025H,0FAH,02H,01BH,025H,0FAH,00H,01DH,040H
DB 00H,02H,029H,018H,0FAH,0FEH,029H,013H,0FAH,02H,028H,013H,0FAH,00H,00H,02H
DB 026H,020H,0FAH,0FEH,026H,010H,0FAH,02H,025H,0EH,0FAH,00H,00H,02H,024H,020H
DB 0FAH,0FEH,024H,0EH,0FAH,02H,023H,0EH,0FAH,00H,00H,04H,021H,01EH,0FAH,0FDH
DB 021H,010H,0FAH,03H,020H,010H,0FAH,00H,00H,02H,01FH,014H,0FAH,0FEH,01FH,015H
DB 0FAH,02H,01EH,015H,0FAH,00H,00H,02H,01DH,018H,0FAH,0FEH,01DH,013H,0FAH,02H
DB 01CH,013H,0FAH,00H,00H,02H,01AH,020H,0FAH,0FEH,01AH,010H,0FAH,02H,019H,010H
DB 0FAH,00H,01AH,010H,0FAH,0FEH,01AH,010H,0FAH,02H,019H,010H,0FAH,00H,01AH,02EH
DB 00H,02H,01DH,034H,0FAH,0FEH,01DH,025H,0FAH,02H,01BH,025H,0FAH,00H,01DH,060H
DB 0FBH,03H,00H,02H,01FH,020H,0FBH,0FFH,021H,040H,0FAH,0FDH,021H,010H,0FAH,03H
DB 020H,010H,0FAH,00H,021H,01EH,00H,02H,024H,010H,0FAH,0FDH,024H,0FH,0FBH,02H
DB 0FAH,00H,00H,01H,021H,060H,0FBH,0FFH,01DH,034H,0FAH,0FEH,01DH,025H,0FAH,02H
DB 01BH,025H,0FAH,00H,01DH,040H,0FBH,02H,00H,02H,0FDH,00H
indian_anthem_part3 LABEL BYTE
DB 0F4H,04AH,0FCH,09H,0F9H,05CH,0FBH,0FFH,0FFH,00H,0F8H,08H,01H,01DH,060H,0F8H
DB 08H,0FFH,01DH,05DH,00H,03H,0F8H,00H,00H,0F5H,00H,0FFH,012H,0FDH
indian_anthem_part4 LABEL BYTE
DB 0FCH,09H,0F9H,05BH,0FFH,00H,0FBH,0FFH,0F8H,08H,01H,01CH,060H,0F8H,08H,0FFH
DB 01CH,05EH,00H,02H,0F8H,00H,00H,0F5H,00H,0FFH,012H,0FDH
indian_anthem_part5 LABEL BYTE
DB 0FCH,09H,0F9H,05BH,0FFH,00H,0FBH,0FFH,0F8H,08H,01H,011H,060H,0F8H,08H,0FFH
DB 011H,05EH,00H,02H,0F8H,00H,00H,0F5H,00H,0FFH,012H,0FDH
indian_anthem_part6 LABEL BYTE
DB 0F4H,036H,0FCH,032H,0F9H,076H,0FFH,00H,0FBH,04H,01CH,020H,01DH,020H,0FAH,0FFH
DB 01DH,020H,0FAH,00H,011H,060H,0FFH,012H,0FDH,00H
indian_anthem_part7 LABEL BYTE
DB 0F4H,072H,0FCH,01EH,0F9H,071H,018H,010H,00H,08H,0F4H,019H,0FCH,02EH,0F9H,06FH
DB 030H,08H,00H,028H,029H,08H,0F4H,072H,0FCH,01EH,0F9H,06CH,021H,010H,00H,010H
DB 0F9H,071H,018H,08H,0F4H,019H,0FCH,02EH,0F9H,06AH,030H,08H,00H,020H,0F4H,072H
DB 0FCH,01EH,0F9H,06CH,01DH,08H,01DH,08H,0F4H,019H,0FCH,02EH,0F9H,064H,0F8H,08H
DB 02H,030H,08H,028H,08H,030H,08H,028H,08H,030H,08H,028H,08H,0F8H,00H,00H
DB 0F5H,00H,0FDH,00H
indian_anthem_part8 LABEL BYTE
DB 0FCH,02CH,0F9H,03AH,0FBH,08H,00H,010H,0F8H,08H,02H,024H,010H,024H,010H,024H
DB 010H,024H,010H,0F8H,00H,00H,00H,010H,0F8H,08H,0FEH,024H,010H,024H,010H,024H
DB 010H,024H,010H,024H,010H,00H,010H,0F8H,00H,00H,0F5H,00H,00H,020H,0FDH,00H
indian_motif_part1 LABEL BYTE
DB 0FCH,09H,0F9H,05DH,0FBH,0FFH,0F8H,08H,01H,01DH,060H,0F8H,08H,0FFH,01DH,05DH
DB 00H,00H
indian_motif_part2 LABEL BYTE
DB 0FCH,09H,0F9H,05CH,0FBH,0FFH,0F8H,08H,01H,01CH,060H,0F8H,08H,0FFH,01CH,05EH
DB 00H,00H
indian_motif_part3 LABEL BYTE
DB 0FCH,09H,0F9H,05BH,0FBH,0FFH,0F8H,08H,01H,011H,060H,0F8H,08H,0FFH,011H,05EH
DB 00H,00H
indian_motif_part4 LABEL BYTE
DB 0FCH,032H,0F9H,075H,0F5H,03H,0FBH,04H,01CH,020H,01DH,020H,0FAH,0FFH,01DH,020H
DB 0FAH,00H,011H,060H,00H,00H
indian_motif_part5 LABEL BYTE
DB 0F5H,04H,0F4H,06EH,0FCH,01EH,0F9H,071H,018H,010H,00H,08H,0F4H,019H,0FCH,02EH
DB 0F9H,06FH,030H,08H,00H,020H,029H,08H,0F4H,06EH,0FCH,01EH,0F9H,06CH,021H,010H
DB 00H,010H,0F9H,071H,018H,08H,0F4H,019H,0FCH,02EH,0F9H,06AH,030H,08H,00H,028H
DB 0F4H,06EH,0FCH,01EH,0F9H,06CH,01DH,08H,01DH,08H,0F4H,019H,00H,00H
indian_motif_part6 LABEL BYTE
DB 0FCH,02CH,0F9H,03AH,0FBH,08H,00H,010H,0F8H,08H,02H,024H,010H,024H,010H,024H
DB 010H,024H,010H,0F8H,00H,00H,00H,010H,0F8H,08H,0FEH,024H,010H,024H,010H,024H
DB 010H,024H,010H,024H,010H,00H,010H,00H,00H,00H
greek_anthem_part1 LABEL BYTE
DB 0F4H,064H,0FCH,01CH,0F9H,07EH,00H,0EH,0FBH,07H,03EH,0EH,03EH,0EH,03EH,0EH
DB 0FBH,02H,03EH,0EH,040H,0EH,041H,01CH,040H,0EH,041H,0EH,040H,0EH,03EH,0EH
DB 03CH,0EH,03EH,038H,00H,07EH,0F9H,074H,0F8H,01CH,01H,0FBH,07H,03AH,0EH,03AH
DB 0EH,03AH,0EH,0FBH,02H,03AH,0EH,03CH,0EH,03EH,02AH,03CH,0EH,03AH,0EH,03CH
DB 07H,0F8H,00H,00H,00H,07H,0F9H,078H,039H,0EH,0F9H,074H,037H,02AH,0F5H,00H
DB 0F8H,01CH,01H,00H,0EH,0FBH,07H,03AH,0EH,03AH,0EH,03AH,0EH,0FBH,02H,03AH
DB 0EH,03CH,0EH,03EH,0EH,03CH,0EH,03EH,07H,00H,07H,03CH,0EH,03AH,0EH,03CH
DB 07H,0F8H,00H,00H,00H,07H,0F9H,078H,039H,0EH,0F9H,074H,037H,02AH,00H,0EH
DB 0F9H,078H,0F8H,01CH,01H,0F5H,00H,0FBH,07H,03AH,0EH,03AH,0EH,03AH,0EH,0FBH
DB 02H,03AH,0EH,03CH,0EH,03EH,07H,00H,07H,0F9H,07CH,039H,0EH,0F9H,078H,0F5H
DB 00H,037H,0EH,0FBH,07H,03AH,0EH,03AH,0EH,03AH,0EH,0FBH,02H,0F5H,00H,0F9H
DB 07CH,039H,0EH,0F9H,078H,037H,0EH,03AH,07H,00H,07H,03AH,07H,00H,07H,0F8H
DB 00H,00H,0F5H,0EH,03CH,0EH,039H,01CH,0F5H,08H,037H,01CH,035H,01CH,037H,01CH
DB 0F5H,0EH,039H,01CH,0F5H,08H,037H,01CH,035H,01CH,037H,01CH,0F5H,0EH,039H,02AH
DB 0FEH,00H,0F5H,00H,0F9H,086H,0FBH,07H,03EH,0EH,03EH,0EH,03EH,0EH,03EH,0EH
DB 03EH,0EH,040H,0EH,0FBH,01H,041H,01CH,040H,0EH,041H,0EH,040H,0EH,03EH,0EH
DB 03CH,0EH,03EH,02AH,00H,07EH,0F9H,078H,0F8H,01CH,01H,0FBH,07H,035H,0EH,035H
DB 0EH,035H,0EH,0FBH,02H,035H,0EH,037H,0EH,039H,02AH,037H,0EH,035H,0EH,037H
DB 07H,00H,07H,034H,0EH,032H,02AH,00H,0EH,0F5H,0AH,0FBH,07H,035H,0EH,035H
DB 0EH,035H,0EH,0FBH,02H,035H,0EH,037H,0EH,039H,0EH,037H,0EH,039H,07H,00H
DB 07H,037H,0EH,035H,0EH,037H,07H,00H,07H,034H,0EH,032H,02AH,00H,0EH,0F5H
DB 0CH,0FBH,07H,035H,0EH,035H,0EH,035H,0EH,0FBH,02H,035H,0EH,037H,0EH,039H
DB 07H,00H,07H,034H,0EH,032H,0EH,0F5H,0EH,0FBH,07H,035H,0EH,035H,0EH,035H
DB 0EH,0FBH,02H,034H,0EH,032H,0EH,035H,07H,00H,07H,035H,07H,00H,07H,0F8H
DB 00H,00H,0F5H,0EH,037H,0EH,034H,01CH,032H,01CH,030H,01CH,032H,01CH,034H,01CH
DB 032H,01CH,030H,01CH,032H,01CH,034H,01CH,037H,07H,00H,04DH,0F5H,014H,0FFH,00H
DB 0FBH,07H,03AH,0EH,03AH,0EH,03AH,0EH,0FBH,02H,03EH,0EH,03CH,07H,00H,07H
DB 03AH,0EH,03CH,0EH,00H,0EH,0FFH,01H,0F5H,016H,0FBH,07H,03AH,0EH,03AH,0EH
DB 03AH,0EH,037H,0EH,03AH,0EH,03AH,0EH,03AH,0EH,0F5H,0EH,0FBH,02H,03CH,0EH
DB 039H,01CH,037H,01CH,039H,01CH,037H,01CH,039H,02AH,0FEH,02H,0FDH,00H
greek_anthem_part2 LABEL BYTE
DB 0F4H,05AH,0FCH,013H,0F9H,078H,00H,0EH,0FBH,07H,039H,0EH,039H,0EH,039H,0EH
DB 0FBH,02H,039H,0EH,03AH,0EH,03CH,01CH,03AH,0EH,03CH,0EH,03AH,0EH,039H,0EH
DB 037H,0EH,039H,038H,00H,07EH,0F9H,068H,0F8H,01CH,01H,0FBH,07H,037H,0EH,037H
DB 0EH,037H,0EH,0FBH,02H,037H,0EH,039H,0EH,03AH,02AH,039H,0EH,037H,0EH,039H
DB 07H,0F8H,00H,00H,00H,07H,0F9H,070H,035H,0EH,0F9H,068H,034H,02AH,00H,0EH
DB 0F8H,01CH,01H,0F5H,0AH,0FBH,07H,037H,0EH,037H,0EH,037H,0EH,0FBH,02H,037H
DB 0EH,039H,0EH,03AH,0EH,039H,0EH,03AH,07H,00H,07H,039H,0EH,037H,0EH,039H
DB 07H,0F8H,00H,00H,00H,07H,0F9H,070H,035H,0EH,0F9H,068H,034H,02AH,00H,0EH
DB 0F9H,06CH,0F8H,01CH,01H,0F5H,0AH,0FBH,07H,037H,0EH,037H,0EH,037H,0EH,0FBH
DB 02H,037H,0EH,039H,0EH,03AH,07H,0F8H,00H,00H,00H,07H,0F9H,074H,035H,0EH
DB 0F9H,06CH,0F8H,01CH,01H,0F5H,0AH,034H,0EH,0FBH,07H,037H,0EH,037H,0EH,037H
DB 0EH,0FBH,02H,0F5H,00H,0F9H,074H,035H,0EH,0F9H,06CH,034H,0EH,037H,07H,00H
DB 07H,037H,07H,00H,07H,0F8H,00H,00H,0F5H,0FH,039H,0EH,035H,01CH,0F5H,0AH
DB 034H,01CH,032H,01CH,034H,01CH,0F5H,0EH,035H,01CH,0F5H,08H,034H,01CH,032H,01CH
DB 034H,01CH,0F5H,0EH,035H,02AH,0FEH,00H,0F5H,00H,0F9H,072H,0FBH,07H,039H,0EH
DB 039H,0EH,039H,0EH,039H,0EH,039H,0EH,03AH,0EH,0FBH,01H,03CH,01CH,03AH,0EH
DB 03CH,0EH,03AH,0EH,039H,0EH,037H,0EH,039H,02AH,00H,0EH,00H,070H,0F9H,06CH
DB 0F8H,01CH,01H,0FBH,07H,032H,0EH,032H,0EH,032H,0EH,0FBH,02H,032H,0EH,034H
DB 0EH,035H,02AH,034H,0EH,032H,0EH,034H,07H,00H,07H,030H,0EH,02EH,02AH,00H
DB 0EH,0F5H,0AH,0FBH,07H,032H,0EH,032H,0EH,032H,0EH,0FBH,02H,032H,0EH,034H
DB 0EH,035H,0EH,034H,0EH,035H,07H,00H,07H,034H,0EH,032H,0EH,034H,07H,00H
DB 07H,030H,0EH,02EH,02AH,00H,0EH,0F5H,010H,0FBH,07H,032H,0EH,032H,0EH,032H
DB 0EH,032H,0EH,034H,0EH,035H,07H,00H,07H,030H,0EH,02EH,0EH,0F5H,012H,0FBH
DB 07H,032H,0EH,032H,0EH,032H,0EH,0FBH,02H,030H,0EH,02EH,0EH,032H,07H,00H
DB 07H,032H,07H,00H,07H,0F8H,00H,00H,0F5H,0EH,034H,0EH,030H,01CH,02EH,01CH
DB 02DH,01CH,02EH,01CH,030H,01CH,02EH,01CH,02DH,01CH,02EH,01CH,030H,01CH,034H,07H
DB 00H,04DH,0F5H,014H,0FFH,00H,0FBH,07H,037H,0EH,037H,0EH,037H,0EH,0FBH,02H
DB 03AH,0EH,039H,07H,00H,07H,037H,0EH,039H,0EH,00H,0EH,0FFH,01H,0FBH,07H
DB 037H,0EH,037H,0EH,037H,0EH,034H,0EH,037H,0EH,037H,0EH,037H,0EH,0F5H,0CH
DB 0FBH,02H,039H,0EH,035H,01CH,034H,01CH,035H,01CH,034H,01CH,035H,02AH,0FEH,02H
DB 0FDH,00H
greek_anthem_part4 LABEL BYTE
DB 0F4H,02CH,0FCH,034H,0F9H,072H,00H,0EH,0FBH,07H,032H,0EH,032H,0EH,032H,0EH
DB 0FBH,02H,032H,0EH,034H,0EH,035H,01CH,034H,0EH,035H,0EH,034H,0EH,032H,0EH
DB 030H,0EH,032H,038H,00H,07EH,0F9H,042H,0F8H,01CH,01H,0FBH,07H,030H,0EH,030H
DB 0EH,030H,0EH,0FBH,02H,030H,0EH,032H,0EH,037H,02AH,032H,0EH,030H,0EH,032H
DB 07H,00H,07H,02EH,0EH,02DH,02AH,00H,0EH,0F5H,00H,0FBH,07H,030H,0EH,030H
DB 0EH,030H,0EH,0FBH,02H,030H,0EH,032H,0EH,034H,0EH,032H,0EH,034H,07H,00H
DB 07H,032H,0EH,030H,0EH,032H,07H,00H,07H,02EH,0EH,02DH,02AH,00H,0EH,0F5H
DB 00H,0FBH,07H,030H,0EH,030H,0EH,030H,0EH,0FBH,02H,030H,0EH,032H,0EH,034H
DB 07H,00H,07H,02EH,0EH,0F5H,00H,02DH,0EH,0FBH,07H,030H,0EH,030H,0EH,030H
DB 0EH,0FBH,02H,0F5H,00H,02EH,0EH,02DH,0EH,030H,07H,00H,07H,030H,07H,00H
DB 07H,0F8H,00H,00H,0F5H,0FCH,032H,0EH,02EH,01CH,02DH,01CH,02BH,01CH,030H,01CH
DB 02EH,01CH,02DH,01CH,02BH,01CH,030H,01CH,02EH,02AH,0FEH,00H,0F5H,00H,0F9H,062H
DB 0FBH,07H,032H,0EH,032H,0EH,032H,0EH,032H,0EH,032H,0EH,034H,0EH,0FBH,01H
DB 035H,01CH,034H,0EH,035H,0EH,034H,0EH,032H,0EH,030H,0EH,032H,02AH,00H,070H
DB 0F9H,044H,0F8H,01CH,01H,00H,0EH,0FBH,07H,02BH,0EH,02BH,0EH,02BH,0EH,0FBH
DB 02H,02BH,0EH,02DH,0EH,032H,02AH,02DH,0EH,02BH,0EH,02DH,07H,00H,07H,029H
DB 0EH,028H,02AH,00H,0EH,0F5H,00H,0FBH,07H,02BH,0EH,02BH,0EH,02BH,0EH,0FBH
DB 02H,02BH,0EH,02DH,0EH,02EH,0EH,02DH,0EH,02EH,07H,00H,07H,02DH,0EH,02BH
DB 0EH,02DH,07H,00H,07H,029H,0EH,028H,02AH,00H,0EH,0F5H,02H,0FBH,07H,02BH
DB 0EH,02BH,0EH,02BH,0EH,0FBH,02H,02BH,0EH,02DH,0EH,02EH,07H,00H,07H,029H
DB 0EH,028H,0EH,0F5H,02H,0FBH,07H,02BH,0EH,02BH,0EH,02BH,0EH,0FBH,02H,029H
DB 0EH,028H,0EH,02BH,07H,00H,07H,02BH,07H,00H,07H,0F8H,00H,00H,0F5H,0FEH
DB 02DH,0EH,029H,01CH,028H,01CH,026H,01CH,02BH,01CH,029H,01CH,028H,01CH,026H,01CH
DB 02BH,01CH,029H,01CH,029H,07H,00H,04DH,0F5H,08H,0FFH,00H,0FBH,07H,030H,0EH
DB 030H,0EH,030H,0EH,0FBH,02H,034H,0EH,032H,07H,00H,07H,030H,0EH,032H,0EH
DB 00H,0EH,0FFH,01H,0FBH,07H,030H,0EH,030H,0EH,030H,0EH,02DH,0EH,030H,0EH
DB 030H,0EH,030H,0EH,0F5H,02H,0FBH,02H,032H,0EH,02EH,01CH,030H,01CH,02EH,01CH
DB 030H,01CH,02EH,02AH,0FEH,02H,0FDH,00H
greek_anthem_part5 LABEL BYTE
DB 0F4H,02CH,0FCH,035H,0F9H,070H,00H,0EH,0FBH,07H,026H,0EH,026H,0EH,026H,0EH
DB 0FBH,02H,026H,0EH,028H,0EH,029H,01CH,028H,0EH,029H,0EH,028H,0EH,026H,0EH
DB 024H,0EH,026H,038H,0F9H,044H,0FFH,00H,024H,01CH,02BH,0EH,02BH,0EH,0FFH,012H
DB 0FEH,00H,0F9H,062H,0FBH,07H,01AH,0EH,01AH,0EH,01AH,0EH,01AH,0EH,01AH,0EH
DB 01CH,0EH,0FBH,02H,01DH,01CH,01CH,0EH,01DH,0EH,01CH,0EH,01AH,0EH,018H,0EH
DB 01AH,02AH,0F9H,046H,0FFH,00H,01FH,01CH,026H,0EH,022H,07H,00H,07H,01FH,0EH
DB 029H,07H,00H,07H,026H,01CH,0FFH,08H,01FH,01CH,026H,0EH,022H,07H,00H,07H
DB 0FFH,00H,024H,01CH,02BH,01CH,024H,01CH,01FH,01CH,0FFH,04H,0FEH,02H,0FDH,00H
greek_anthem_part6 LABEL BYTE
DB 0F4H,054H,0FCH,035H,0F9H,06EH,00H,0EH,0FBH,07H,01AH,0EH,01AH,0EH,01AH,0EH
DB 0FBH,02H,01AH,0EH,01CH,0EH,01DH,01CH,01CH,0EH,01DH,0EH,01CH,0EH,01AH,0EH
DB 018H,0EH,01AH,038H,0F9H,04CH,0FFH,00H,018H,01CH,01FH,0EH,01FH,0EH,0FFH,012H
DB 0FEH,00H,0F9H,05EH,0FBH,07H,015H,0EH,015H,0EH,015H,0EH,015H,0EH,015H,0EH
DB 016H,0EH,0FBH,02H,018H,01CH,016H,0EH,018H,0EH,016H,0EH,015H,0EH,013H,0EH
DB 015H,02AH,0F9H,04EH,0FFH,00H,013H,01CH,01AH,0EH,016H,07H,00H,07H,013H,0EH
DB 01DH,07H,00H,07H,01AH,01CH,0FFH,08H,013H,01CH,01AH,0EH,016H,07H,00H,07H
DB 0FFH,00H,018H,01CH,01FH,01CH,018H,01CH,013H,01CH,0FFH,04H,0FEH,02H,0FDH,00H
greek_anthem_part7 LABEL BYTE
DB 0F4H,022H,0FCH,015H,0F9H,07AH,00H,0EH,0FBH,07H,01AH,0EH,01AH,0EH,01AH,0EH
DB 0FBH,02H,01AH,0EH,01CH,0EH,01DH,01CH,01CH,0EH,01DH,0EH,01CH,0EH,01AH,0EH
DB 018H,0EH,01AH,038H,0F4H,040H,0FCH,0AH,0F9H,05EH,0FFH,00H,022H,0EH,01FH,01CH
DB 018H,01CH,01FH,01CH,018H,0EH,0FFH,08H,022H,0EH,01FH,01CH,018H,0EH,0FEH,00H
DB 0F4H,02CH,0FCH,015H,0F9H,05EH,0FBH,07H,01AH,0EH,01AH,0EH,01AH,0EH,01AH,0EH
DB 0FBH,02H,01AH,0EH,01CH,0EH,01DH,01CH,01CH,0EH,01DH,0EH,01CH,0EH,01AH,0EH
DB 018H,0EH,01AH,02AH,0F4H,040H,0FCH,0AH,0F9H,05EH,0FFH,00H,01DH,07H,00H,07H
DB 01AH,01CH,013H,01CH,0EH,01CH,013H,0EH,0FFH,08H,01DH,07H,00H,07H,01AH,01CH
DB 013H,0EH,0FFH,00H,022H,07H,00H,07H,01FH,01CH,018H,0EH,016H,07H,00H,07H
DB 013H,01CH,018H,0EH,0FFH,03H,022H,07H,00H,07H,01FH,01CH,018H,0EH,016H,07H
DB 00H,07H,013H,01CH,0CH,0EH,0FEH,02H,0FDH,00H
greek_anthem_part8 LABEL BYTE
DB 0F4H,05EH,0FCH,015H,0F9H,07AH,00H,0EH,0FBH,07H,021H,0EH,021H,0EH,021H,0EH
DB 0FBH,01H,021H,0EH,022H,0EH,024H,01CH,022H,0EH,024H,0EH,022H,0EH,021H,0EH
DB 01FH,0EH,021H,038H,0FFH,00H,00H,070H,0FFH,08H,00H,038H,0FEH,00H,0FBH,07H
DB 021H,0EH,021H,0EH,021H,0EH,021H,0EH,0FBH,01H,021H,0EH,022H,0EH,024H,01CH
DB 022H,0EH,024H,0EH,022H,0EH,021H,0EH,01FH,0EH,021H,02AH,0FFH,00H,00H,070H
DB 0FFH,0DH,00H,038H,0FEH,02H,0FDH,00H
greek_anthem_part3 LABEL BYTE
DB 0F4H,022H,0FCH,034H,0F9H,068H,00H,070H,00H,07EH,00H,07EH,0F9H,044H,0F8H,01CH
DB 01H,0FBH,07H,034H,0EH,034H,0EH,034H,0EH,0FBH,02H,034H,0EH,035H,0EH,034H
DB 02AH,0F8H,00H,00H,035H,0EH,034H,0EH,035H,07H,00H,07H,032H,0EH,030H,02AH
DB 00H,0EH,0F5H,00H,0F8H,01CH,01H,0FBH,07H,034H,0EH,034H,0EH,034H,0EH,0FBH
DB 02H,034H,0EH,035H,0EH,037H,0EH,035H,0EH,0F8H,00H,00H,037H,07H,00H,07H
DB 035H,0EH,034H,0EH,035H,07H,00H,07H,032H,0EH,030H,02AH,00H,0EH,0F5H,00H
DB 0F8H,01CH,01H,0FBH,07H,034H,0EH,034H,0EH,034H,0EH,0FBH,02H,034H,0EH,035H
DB 0EH,037H,07H,00H,07H,032H,0EH,0F5H,04H,030H,0EH,0FBH,07H,034H,0EH,034H
DB 0EH,034H,0EH,0FBH,02H,0F5H,04H,032H,0EH,030H,0EH,034H,07H,00H,07H,034H
DB 07H,00H,07H,0F8H,00H,00H,0F5H,0FEH,035H,0EH,032H,01CH,030H,01CH,02EH,01CH
DB 02DH,01CH,032H,01CH,030H,01CH,02EH,01CH,02DH,01CH,032H,02AH,0FEH,00H,00H,070H
DB 00H,070H,00H,070H,0F9H,044H,0F8H,01CH,01H,00H,0EH,0FBH,07H,02EH,0EH,02EH
DB 0EH,02EH,0EH,0FBH,02H,02EH,0EH,030H,0EH,02EH,02AH,030H,0EH,02EH,0EH,030H
DB 07H,00H,07H,02DH,0EH,02BH,02AH,00H,0EH,0F5H,00H,0FBH,07H,02EH,0EH,02EH
DB 0EH,02EH,0EH,0FBH,00H,02EH,0EH,030H,0EH,032H,0EH,030H,0EH,032H,07H,00H
DB 07H,030H,0EH,02EH,0EH,030H,07H,00H,07H,02DH,0EH,02BH,02AH,00H,0EH,0F5H
DB 02H,0FBH,07H,02EH,0EH,02EH,0EH,02EH,0EH,0FBH,02H,02EH,0EH,030H,0EH,032H
DB 07H,00H,07H,02DH,0EH,02BH,0EH,0F5H,02H,0FBH,07H,02EH,0EH,02EH,0EH,02EH
DB 0EH,0FBH,02H,02DH,0EH,02BH,0EH,02EH,07H,00H,07H,02EH,07H,00H,07H,0F8H
DB 00H,00H,0F5H,00H,030H,0EH,02DH,01CH,02BH,01CH,029H,01CH,028H,01CH,02DH,01CH
DB 02BH,01CH,029H,01CH,028H,01CH,02DH,01CH,02DH,07H,00H,04DH,0F5H,06H,0FFH,00H
DB 0FBH,07H,034H,0EH,034H,0EH,034H,0EH,0FBH,02H,037H,0EH,035H,07H,00H,07H
DB 034H,0EH,035H,0EH,00H,0EH,0FFH,01H,0FBH,07H,034H,0EH,034H,0EH,034H,0EH
DB 024H,0EH,034H,0EH,034H,0EH,034H,0EH,0F5H,02H,0FBH,02H,035H,0EH,032H,01CH
DB 02DH,01CH,032H,01CH,02DH,01CH,032H,02AH,0FEH,02H,0FDH,00H
greek_motif_part1 LABEL BYTE
DB 0F4H,072H,0FCH,01CH,0F9H,07EH,00H,0EH,0FBH,07H,040H,0EH,040H,0EH,040H,0EH
DB 0FBH,02H,040H,0EH,042H,0EH,043H,01CH,042H,0EH,043H,0EH,042H,0EH,040H,0EH
DB 03EH,0EH,040H,038H,00H,00H
greek_motif_part2 LABEL BYTE
DB 0F4H,05EH,0FCH,013H,0F9H,068H,00H,0EH,0FBH,07H,03BH,0EH,03BH,0EH,03BH,0EH
DB 0FBH,02H,03BH,0EH,03CH,0EH,03EH,01CH,03CH,0EH,03EH,0EH,03CH,0EH,03BH,0EH
DB 039H,0EH,03BH,038H,00H,00H
greek_motif_part3 LABEL BYTE
DB 0F4H,022H,0FCH,034H,0F9H,062H,00H,0EH,0FBH,07H,034H,0EH,034H,0EH,034H,0EH
DB 0FBH,02H,034H,0EH,036H,0EH,037H,01CH,036H,0EH,037H,0EH,036H,0EH,034H,0EH
DB 032H,0EH,034H,038H,00H,00H
greek_motif_part4 LABEL BYTE
DB 0F4H,036H,0FCH,035H,0F9H,062H,00H,0EH,0FBH,07H,028H,0EH,028H,0EH,028H,0EH
DB 0FBH,02H,028H,0EH,02AH,0EH,02BH,01CH,02AH,0EH,02BH,0EH,02AH,0EH,028H,0EH
DB 026H,0EH,028H,038H,00H,00H
greek_motif_part5 LABEL BYTE
DB 0F4H,04AH,0FCH,035H,0F9H,05EH,00H,0EH,0FBH,07H,01CH,0EH,01CH,0EH,01CH,0EH
DB 0FBH,02H,01CH,0EH,01EH,0EH,01FH,01CH,01EH,0EH,01FH,0EH,01EH,0EH,01CH,0EH
DB 01AH,0EH,01CH,038H,00H,00H
greek_motif_part6 LABEL BYTE
DB 0F4H,02CH,0FCH,015H,0F9H,06AH,00H,0EH,0FBH,07H,01CH,0EH,01CH,0EH,01CH,0EH
DB 0FBH,02H,01CH,0EH,01EH,0EH,01FH,01CH,01EH,0EH,01FH,0EH,01EH,0EH,01CH,0EH
DB 01AH,0EH,01CH,038H,00H,00H
greek_motif_part7 LABEL BYTE
DB 0F4H,054H,0FCH,015H,0F9H,06AH,00H,0EH,0FBH,07H,023H,0EH,023H,0EH,023H,0EH
DB 0FBH,01H,023H,0EH,024H,0EH,026H,01CH,024H,0EH,026H,0EH,024H,0EH,023H,0EH
DB 021H,0EH,023H,038H,00H,00H
american_anthem_part1 LABEL BYTE
DB 0FCH,05H,0F4H,07CH,0F9H,050H,0F5H,0FH,0FBH,01H,00H,0E0H,0F8H,016H,01H,030H
DB 07H,030H,015H,030H,07H,030H,015H,02EH,07H,02CH,015H,030H,07H,035H,015H,037H
DB 07H,0F8H,016H,0FFH,038H,015H,038H,07H,038H,015H,037H,07H,035H,01CH,035H,015H
DB 033H,07H,0F8H,016H,01H,031H,015H,030H,07H,031H,015H,033H,07H,035H,015H,033H
DB 07H,035H,015H,031H,07H,0F8H,016H,0FFH,030H,015H,031H,07H,030H,015H,02CH,07H
DB 030H,01CH,030H,015H,030H,07H,0F8H,016H,01H,030H,015H,030H,07H,030H,015H,02EH
DB 07H,02CH,015H,030H,07H,035H,015H,037H,07H,038H,015H,038H,07H,038H,015H,037H
DB 07H,035H,01CH,035H,01CH,037H,01CH,037H,01CH,035H,01CH,034H,01CH,0F8H,00H,00H
DB 035H,070H,0F5H,0FH,0F8H,0BH,01H,030H,031H,02EH,07H,02CH,015H,030H,07H,035H
DB 015H,037H,07H,0F8H,0BH,0FFH,038H,038H,035H,038H,0F8H,0BH,01H,031H,031H,033H
DB 07H,035H,015H,034H,07H,035H,015H,031H,07H,0F8H,0BH,0FFH,030H,038H,02CH,01CH
DB 030H,015H,030H,07H,0F8H,016H,01H,030H,015H,030H,07H,030H,015H,02EH,07H,02CH
DB 015H,030H,07H,035H,015H,037H,07H,038H,015H,038H,07H,038H,015H,037H,07H,035H
DB 01CH,035H,01CH,0F8H,0BH,01H,037H,01CH,037H,01CH,035H,01CH,034H,01CH,0F8H,0BH
DB 0FFH,035H,069H,0FDH
american_anthem_part2 LABEL BYTE
DB 0FCH,027H,0F4H,02CH,0F9H,03CH,0F5H,023H,00H,0E7H,00H,0E0H,02EH,038H,031H,015H
DB 030H,07H,031H,015H,02EH,07H,02CH,015H,02EH,07H,0F8H,05H,0FFH,02CH,015H,029H
DB 07H,02CH,01CH,02BH,01CH,0F8H,0BH,01H,029H,054H,027H,01CH,0F8H,0BH,0FFH,025H
DB 038H,029H,01CH,02CH,01CH,0F8H,0BH,01H,02EH,01CH,031H,01CH,030H,01CH,02EH,01CH
DB 0F8H,00H,00H,02CH,070H,0F8H,0BH,01H,02CH,01CH,02CH,015H,02BH,07H,029H,015H
DB 02CH,07H,031H,015H,033H,07H,0F8H,0BH,0FFH,035H,01CH,033H,01CH,031H,01CH,030H
DB 01CH,0F8H,0BH,01H,02EH,031H,030H,07H,031H,015H,030H,07H,031H,015H,02EH,07H
DB 0F8H,0BH,0FFH,02CH,01CH,02BH,01CH,029H,01CH,028H,01CH,0F8H,0BH,01H,029H,054H
DB 027H,01CH,0F8H,0BH,0FFH,025H,038H,029H,01CH,02CH,01CH,0F8H,0BH,01H,02EH,01CH
DB 031H,01CH,030H,01CH,02EH,01CH,0F8H,0BH,0FFH,02CH,069H,0FDH
american_anthem_part3 LABEL BYTE
DB 0FCH,027H,0F4H,054H,0F9H,050H,0F5H,0FH,0FBH,03H,0FFH,00H,02CH,07H,02CH,07H
DB 00H,0EH,030H,07H,030H,07H,00H,0EH,02CH,07H,02CH,07H,00H,0EH,030H,07H
DB 030H,07H,00H,0EH,0FFH,03H,02DH,07H,029H,07H,00H,0EH,02CH,07H,02EH,07H
DB 00H,0EH,02CH,07H,02EH,07H,00H,0EH,02CH,07H,02EH,07H,00H,0EH,029H,07H
DB 030H,07H,00H,0EH,02BH,07H,02CH,07H,00H,0EH,029H,07H,030H,07H,00H,0EH
DB 02EH,07H,02EH,07H,00H,0EH,02EH,07H,02CH,07H,00H,0EH,02BH,07H,02CH,07H
DB 00H,0EH,029H,07H,030H,07H,00H,0EH,02CH,07H,02CH,07H,00H,0EH,02BH,07H
DB 029H,07H,00H,0EH,02BH,07H,02CH,07H,00H,0EH,027H,07H,025H,07H,00H,0EH
DB 02BH,07H,02CH,07H,00H,0EH,027H,07H,0F5H,0FH,029H,07H,00H,0EH,02CH,07H
DB 02EH,07H,00H,0EH,02CH,07H,02BH,07H,00H,0EH,030H,07H,02EH,07H,00H,0EH
DB 02EH,07H,02CH,07H,00H,0EH,02CH,07H,029H,07H,00H,0EH,02EH,07H,02CH,07H
DB 00H,0EH,02CH,07H,030H,07H,00H,0EH,030H,07H,02CH,07H,00H,0EH,029H,07H
DB 02CH,07H,00H,0EH,027H,07H,02BH,07H,00H,0EH,027H,07H,02BH,07H,00H,0EH
DB 02BH,07H,029H,07H,00H,0EH,025H,07H,029H,07H,00H,0EH,024H,07H,027H,07H
DB 00H,0EH,024H,07H,027H,07H,00H,0EH,027H,07H,025H,07H,00H,0EH,022H,07H
DB 025H,07H,00H,0EH,022H,07H,027H,07H,00H,0EH,024H,07H,028H,07H,00H,0EH
DB 02BH,07H,029H,07H,00H,0EH,025H,07H,029H,07H,00H,0EH,028H,07H,02BH,07H
DB 00H,0EH,028H,07H,02BH,07H,00H,0EH,02EH,07H,029H,07H,00H,0EH,029H,07H
DB 029H,07H,00H,0EH,025H,07H,024H,07H,00H,0EH,025H,07H,025H,07H,00H,0EH
DB 025H,07H,025H,07H,00H,0EH,027H,07H,025H,07H,00H,0EH,020H,07H,022H,07H
DB 00H,0EH,027H,07H,025H,07H,00H,0EH,0F8H,0EH,01H,025H,07H,025H,07H,00H
DB 0EH,029H,07H,02BH,07H,00H,0EH,029H,07H,024H,07H,00H,0EH,02CH,07H,02BH
DB 07H,00H,0EH,02EH,07H,02CH,07H,00H,0EH,030H,07H,031H,07H,00H,0EH,034H
DB 07H,035H,07H,00H,0EH,030H,07H,030H,07H,00H,0EH,0FDH
american_anthem_part4 LABEL BYTE
DB 0F5H,0FH,0FCH,01BH,0F4H,0EH,00H,07H,0F9H,064H,01DH,07H,00H,015H,018H,07H
DB 00H,015H,0F9H,05AH,01DH,07H,00H,015H,018H,07H,00H,015H,0F9H,050H,0FFH,00H
DB 01DH,07H,00H,015H,018H,07H,00H,015H,0F9H,046H,0FFH,04H,01DH,01CH,021H,01CH
DB 022H,01CH,01DH,01CH,022H,01CH,016H,01CH,01DH,01CH,018H,01CH,01DH,01CH,018H,01CH
DB 01DH,01CH,018H,01CH,01DH,01CH,01BH,01CH,019H,01CH,014H,01CH,019H,01CH,014H,01CH
DB 016H,01CH,022H,01CH,018H,01CH,024H,01CH,01DH,01CH,024H,01CH,01DH,07H,00H,031H
DB 029H,01CH,01DH,01CH,029H,01CH,027H,01CH,025H,01CH,019H,01CH,025H,01CH,024H,01CH
DB 022H,01CH,016H,01CH,022H,01CH,024H,01CH,029H,01CH,01DH,01CH,024H,01CH,018H,01CH
DB 0FBH,015H,01DH,01CH,018H,01CH,01DH,01CH,01BH,01CH,0F9H,05AH,019H,01CH,014H,01CH
DB 019H,01CH,014H,01CH,016H,01CH,022H,01CH,018H,01CH,024H,01CH,01DH,07H,00H,062H
DB 0FDH,00H
american_motif_part1 LABEL BYTE
DB 0FCH,05H,0F4H,07CH,0F9H,050H,0F5H,0FH,0F8H,03H,01H,0FBH,04H,02BH,07H,02BH
DB 015H,02BH,07H,02BH,015H,029H,07H,027H,015H,02BH,07H,030H,015H,032H,07H,0F8H
DB 00H,00H,033H,054H,00H,00H
american_motif_part2 LABEL BYTE
DB 0FCH,027H,0F9H,03CH,0F4H,02CH,0F5H,01EH,0FBH,0EH,00H,07H,029H,01CH,02CH,01CH
DB 02BH,01CH,029H,01CH,027H,054H,00H,00H
american_motif_part3 LABEL BYTE
DB 0FCH,01AH,0F4H,054H,0F9H,050H,0F5H,0FH,00H,07H,020H,07H,00H,0EH,024H,07H
DB 026H,07H,00H,0EH,024H,07H,01FH,07H,00H,0EH,01BH,07H,01AH,07H,00H,0EH
DB 01DH,07H,01BH,07H,00H,0EH,02BH,07H,02CH,07H,00H,0EH,02FH,07H,030H,07H
DB 00H,00H
american_motif_part4 LABEL BYTE
DB 0FCH,01BH,0F4H,0EH,0F9H,050H,0F5H,014H,00H,07H,0FBH,015H,011H,01CH,01DH,01CH
DB 013H,01CH,01FH,01CH,018H,01CH,013H,01CH,0CH,01CH,00H,00H
creation_theme_part1 LABEL BYTE
DB 0FCH,0FH,0F5H,023H,0F9H,014H,0F4H,07FH,0FBH,06H,02FH,0CH,034H,0CH,03BH,0CH
DB 032H,0CH,037H,0CH,03EH,0CH,034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH
DB 045H,0CH,00H,090H,0F5H,023H,0FCH,015H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH
DB 02BH,0CH,032H,0CH,028H,0CH,02DH,0CH,034H,0CH,02BH,0CH,032H,0CH,039H,0CH
DB 00H,060H,0FCH,0FH,0FBH,03H,02FH,06H,02AH,06H,02FH,06H,034H,06H,036H,06H
DB 03BH,06H,032H,06H,037H,06H,03EH,06H,037H,06H,039H,06H,03EH,06H,040H,06H
DB 039H,06H,034H,06H,039H,06H,03BH,06H,040H,06H,043H,06H,03EH,06H,039H,06H
DB 03EH,06H,045H,06H,040H,06H,03BH,06H,036H,06H,03BH,06H,040H,06H,042H,06H
DB 047H,06H,04AH,06H,04FH,06H,047H,06H,042H,06H,03BH,06H,040H,06H,042H,06H
DB 03BH,06H,03EH,06H,037H,06H,032H,06H,037H,06H,039H,06H,03EH,06H,040H,06H
DB 039H,06H,034H,06H,039H,06H,03BH,06H,034H,06H,037H,06H,032H,06H,039H,06H
DB 03EH,06H,039H,06H,034H,06H,02FH,06H,02AH,06H,02FH,06H,034H,06H,036H,06H
DB 03BH,06H,032H,06H,02BH,06H,0FFH,00H,00H,0C0H,0FFH,03H,0F5H,025H,0F8H,0CH
DB 01H,0FBH,00H,0FCH,01AH,034H,0C0H,0F8H,024H,0FFH,037H,090H,0F8H,030H,01H,0FFH
DB 00H,0FBH,06H,0FCH,012H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH
DB 0CH,034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,041H,0CH,039H
DB 0CH,040H,0CH,047H,0CH,00H,030H,0FCH,015H,0F8H,00H,00H,0FFH,01H,00H,0C0H
DB 00H,0C0H,00H,03CH,0FCH,010H,0F5H,02DH,0FBH,06H,039H,0CH,03BH,0CH,036H,0CH
DB 037H,0CH,042H,0CH,044H,0CH,03FH,0CH,044H,0CH,03BH,0CH,042H,0CH,03DH,0CH
DB 03DH,0CH,042H,0CH,03BH,0CH,044H,0CH,03FH,0CH,045H,0CH,043H,0CH,037H,0CH
DB 035H,0CH,03BH,0CH,039H,0CH,00H,03CH,0F5H,029H,0FCH,012H,0FFH,00H,00H,0C0H
DB 0FFH,01H,00H,0C0H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH
DB 034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,041H,0CH,039H,0CH
DB 040H,0CH,047H,0CH,0FFH,01H,0FCH,01AH,0F5H,027H,0F8H,012H,01H,0FBH,0FFH,02AH
DB 018H,02BH,024H,02DH,024H,032H,018H,0F8H,014H,0FFH,034H,0FCH,00H,0CH,0F8H,0EH
DB 01H,036H,018H,037H,024H,039H,024H,03CH,018H,0F8H,014H,0FFH,03BH,0FCH,00H,0CH
DB 0F5H,029H,0F8H,0AH,01H,02FH,018H,030H,024H,032H,018H,035H,0CH,037H,018H,03CH
DB 018H,0F8H,010H,0FFH,03EH,0E4H,00H,0CH,0F5H,02CH,0F8H,0DH,01H,040H,018H,03DH
DB 024H,039H,018H,03BH,0CH,037H,018H,036H,018H,0F8H,019H,0FFH,032H,0E4H,0F8H,00H
DB 00H,00H,0CH,00H,0C0H,00H,0C0H,00H,0C0H,00H,0C0H,00H,018H,0F8H,015H,01H
DB 0F5H,02BH,031H,024H,034H,024H,02FH,030H,02AH,024H,034H,0CH,031H,024H,032H,024H
DB 034H,024H,039H,030H,0F8H,015H,0FFH,03BH,0B4H,036H,0A2H,00H,04EH,00H,0C0H,0FCH
DB 012H,0F5H,035H,0FBH,06H,0FFH,00H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H
DB 0CH,03EH,0CH,034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,041H
DB 0CH,039H,0CH,040H,0CH,047H,0CH,00H,0C0H,0F5H,02FH,0FFH,01H,0F5H,032H,0F8H
DB 011H,01H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH,034H,0CH
DB 039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,039H,0CH,040H,0CH,047H,0CH
DB 03BH,0CH,042H,0CH,049H,0CH,03EH,0CH,045H,0CH,04AH,0CH,040H,0CH,045H,0CH
DB 04CH,0CH,043H,0CH,04AH,0CH,051H,0CH,045H,0CH,04CH,0CH,053H,06H,00H,00H
creation_theme_part2 LABEL BYTE
DB 0FCH,0FH,0F5H,023H,0F9H,014H,0F4H,07FH,0FBH,06H,02AH,0CH,02FH,0CH,034H,0CH
DB 02BH,0CH,032H,0CH,037H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH
DB 040H,0CH,00H,090H,0F5H,023H,0FCH,015H,01EH,0CH,023H,0CH,028H,0CH,01FH,0CH
DB 026H,0CH,02BH,0CH,021H,0CH,028H,0CH,02FH,0CH,026H,0CH,02DH,0CH,034H,0CH
DB 0FFH,00H,00H,0C0H,0FFH,0CH,00H,0CH,0FCH,010H,0F5H,02DH,0FBH,06H,039H,0CH
DB 03BH,0CH,036H,0CH,037H,0CH,042H,0CH,03DH,0CH,038H,0CH,03DH,0CH,034H,0CH
DB 03BH,0CH,036H,0CH,036H,0CH,03BH,0CH,034H,0CH,03DH,0CH,038H,0CH,03EH,0CH
DB 043H,0CH,037H,0CH,035H,0CH,03BH,0CH,039H,0CH,00H,03CH,0F5H,033H,0FCH,012H
DB 0FFH,00H,00H,0C0H,0FFH,05H,0FCH,01AH,0F5H,027H,0F8H,08H,01H,0FBH,0FFH,023H
DB 018H,026H,024H,028H,024H,02BH,018H,0F8H,014H,0FFH,02DH,0FCH,00H,0CH,0F8H,0EH
DB 01H,02FH,018H,032H,024H,034H,024H,037H,018H,0F8H,014H,0FFH,036H,0FCH,00H,0CH
DB 0F5H,029H,0F8H,0AH,01H,02AH,018H,02BH,024H,02DH,018H,030H,0CH,032H,018H,035H
DB 018H,0F8H,010H,0FFH,039H,0E4H,00H,0CH,0F5H,02BH,0F8H,0DH,01H,03BH,018H,038H
DB 024H,034H,018H,036H,0CH,032H,018H,02FH,018H,0F8H,019H,0FFH,02DH,0E4H,00H,0CH
DB 00H,0C0H,00H,0C0H,00H,0C0H,00H,0C0H,00H,018H,0F8H,015H,01H,0F5H,02BH,02AH
DB 024H,02DH,024H,02AH,030H,025H,024H,02FH,0CH,02AH,024H,02BH,024H,02DH,024H,032H
DB 030H,0F8H,015H,0FFH,034H,0B4H,02FH,0A2H,00H,04EH,0FCH,012H,00H,0C0H,00H,0C0H
DB 00H,0C0H,00H,0C0H,00H,0C0H,0F7H,06H,0F5H,032H,0F8H,011H,01H,02FH,0CH,034H
DB 0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH,034H,0CH,039H,0CH,040H,0CH,037H
DB 0CH,03EH,0CH,045H,0CH,039H,0CH,040H,0CH,047H,0CH,03BH,0CH,042H,0CH,049H
DB 0CH,03EH,0CH,045H,0CH,04AH,0CH,040H,0CH,045H,0CH,04CH,0CH,043H,0CH,04AH
DB 0CH,051H,0CH,045H,0CH,04CH,0CH,053H,06H,00H,00H,00H
creation_theme_part3 LABEL BYTE
DB 0FCH,01AH,0F5H,028H,0F9H,014H,0F4H,040H,02AH,024H,02BH,024H,02DH,024H,032H,018H
DB 040H,0CH,00H,090H,02AH,024H,02BH,024H,02DH,024H,032H,018H,040H,0CH,00H,060H
DB 02FH,018H,032H,024H,034H,024H,037H,018H,040H,0CH,00H,0FCH,0FCH,010H,0F8H,09H
DB 0FFH,0FBH,06H,02BH,0CH,028H,0CH,02BH,0CH,028H,0CH,02BH,0CH,028H,0CH,02BH
DB 0CH,028H,0CH,00H,060H,0F8H,00H,00H,0F5H,02EH,0FFH,00H,02AH,0CH,02FH,0CH
DB 036H,0CH,02DH,0CH,032H,0CH,039H,0CH,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH
DB 039H,0CH,040H,0CH,035H,0CH,034H,0CH,03BH,0CH,042H,0CH,00H,060H,0FFH,01H
DB 00H,0C0H,0FBH,00H,0FCH,01AH,0F5H,03FH,0F8H,0AH,0FFH,030H,090H,0FBH,06H,0F5H
DB 02EH,0F8H,00H,00H,0FFH,00H,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H,0CH
DB 037H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,035H,0CH
DB 034H,0CH,03BH,0CH,042H,0CH,00H,030H,0FFH,01H,0FBH,00H,0FCH,01AH,0F8H,07H
DB 01H,0F5H,022H,02FH,018H,032H,024H,034H,024H,037H,018H,0F8H,017H,0FFH,0FBH,0FFH
DB 039H,048H,039H,0C0H,039H,0C0H,0FBH,00H,0F8H,00H,00H,0FFH,00H,00H,0C0H,0FFH
DB 02H,00H,0C0H,0FBH,06H,0F5H,026H,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H
DB 0CH,037H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,035H
DB 0CH,034H,0CH,03BH,0CH,042H,0CH,0FFH,01H,0F5H,02BH,0FCH,013H,0FFH,00H,00H
DB 0C0H,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H,0CH,037H,0CH,02DH,0CH,034H
DB 0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,035H,0CH,034H,0CH,03BH,0CH,042H
DB 0CH,0FCH,015H,0FFH,01H,0FCH,01AH,0FFH,00H,00H,0C0H,02AH,0CH,02FH,0CH,036H
DB 0CH,02BH,0CH,032H,0CH,037H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H
DB 0CH,040H,0CH,041H,0CH,039H,0CH,040H,0CH,047H,0CH,0FCH,015H,0FFH,01H,0F8H
DB 00H,00H
bad_news_part1 LABEL BYTE
DB 0F5H,030H,0F9H,014H,0F4H,07FH,00H,078H,0FBH,09H,0FCH,015H,02FH,0CH,02AH,0CH
DB 025H,0CH,026H,0CH,02DH,0CH,02FH,0CH,00H,0D8H,0FCH,015H,031H,06H,00H,01EH
DB 034H,06H,00H,01EH,02FH,06H,00H,012H,02FH,0CH,02AH,0CH,02AH,0CH,026H,0CH
DB 02DH,0CH,02FH,0CH,031H,06H,00H,01EH,032H,06H,00H,01EH,034H,06H,00H,01EH
DB 039H,06H,00H,02AH,03BH,06H,00H,036H,0FCH,013H,03DH,06H,00H,01EH,040H,06H
DB 00H,01EH,03BH,06H,00H,012H,0FCH,015H,03BH,0CH,036H,0CH,036H,0CH,032H,0CH
DB 039H,0CH,040H,0CH,03DH,06H,00H,01EH,03EH,06H,00H,01EH,040H,06H,00H,01EH
DB 045H,06H,00H,02AH,047H,06H,00H,036H,0FCH,07H,03DH,06H,00H,01EH,040H,06H
DB 00H,01EH,03BH,06H,00H,012H,047H,0CH,042H,0CH,03DH,0CH,03EH,0CH,045H,0CH
DB 047H,0CH,03DH,06H,00H,01EH,03EH,06H,00H,01EH,040H,06H,00H,01EH,045H,06H
DB 00H,02AH,047H,06H,00H,01EH,00H,078H,038H,0CH,031H,0CH,02CH,0CH,02DH,0CH
DB 034H,0CH,036H,0CH,0FCH,015H,0FFH,00H,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH
DB 032H,0CH,037H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH
DB 035H,0CH,034H,0CH,03BH,0CH,042H,0CH,00H,078H,0FCH,013H,036H,0CH,031H,0CH
DB 02CH,0CH,02DH,0CH,034H,0CH,036H,0CH,0FCH,015H,0FFH,01H,0FCH,07H,0F8H,0EH
DB 01H,0F5H,02EH,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H,0CH,037H,0CH,02DH
DB 0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,034H,0CH,03BH,0CH,042H
DB 0CH,036H,0CH,03DH,0CH,044H,0CH,037H,0CH,03EH,0CH,045H,0CH,039H,0CH,040H
DB 0CH,047H,0CH,03EH,0CH,045H,0CH,04CH,0CH,040H,0CH,047H,0CH,04EH,06H,00H
DB 00H,00H
creation_theme_part4 LABEL BYTE
DB 0FCH,01AH,0F5H,028H,0F9H,014H,0F4H,040H,00H,0C0H,00H,0C0H,00H,090H,02AH,018H
DB 02BH,024H,02DH,024H,032H,018H,039H,0CH,00H,0FCH,0FCH,010H,0F5H,032H,0F8H,09H
DB 0FFH,0FBH,06H,028H,0CH,024H,0CH,028H,0CH,024H,0CH,028H,0CH,024H,0CH,028H
DB 0CH,024H,0CH,00H,060H,0F8H,00H,00H,0F5H,02EH,0FFH,00H,00H,0C0H,0FFH,06H
DB 00H,030H,0FBH,00H,0FCH,01AH,0F8H,07H,01H,0F5H,022H,02AH,018H,02BH,024H,02DH
DB 024H,032H,018H,0F8H,00H,00H,00H,048H,0FFH,00H,00H,0C0H,0FFH,08H,0F5H,028H
DB 0F8H,012H,01H,01EH,018H,01FH,024H,021H,024H,026H,018H,0F8H,00H,00H,0FBH,0FFH
DB 00H,048H,0FBH,06H,0F5H,02BH,0FCH,013H,0FFH,00H,02FH,0CH,034H,0CH,03BH,0CH
DB 032H,0CH,037H,0CH,03EH,0CH,034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH
DB 045H,0CH,041H,0CH,039H,0CH,040H,0CH,047H,0CH,00H,0C0H,0FCH,015H,0FFH,01H
DB 0FCH,01AH,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH,034H,0CH
DB 039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,041H,0CH,039H,0CH,040H,0CH
DB 047H,0CH,00H,0C0H,0FCH,015H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH
DB 03EH,0CH,034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,041H,0CH
DB 039H,0CH,040H,0CH,047H,0CH
bad_news_part2 LABEL BYTE
DB 0FCH,015H,0FBH,09H,0F4H,060H,0F5H,030H,0F9H,014H,0F7H,06H,00H,078H,02FH,0CH
DB 02AH,0CH,025H,0CH,026H,0CH,02DH,0CH,02FH,0CH,00H,0D8H,0FCH,015H,02AH,06H
DB 00H,01EH,02DH,06H,00H,01EH,02AH,06H,00H,012H,02FH,0CH,02AH,0CH,025H,0CH
DB 026H,0CH,02DH,0CH,034H,0CH,025H,06H,00H,01EH,02BH,06H,00H,01EH,02DH,06H
DB 00H,01EH,032H,06H,00H,02AH,034H,06H,00H,036H,0FCH,013H,036H,06H,00H,01EH
DB 039H,06H,00H,01EH,036H,06H,00H,012H,0FCH,015H,02FH,0CH,02AH,0CH,025H,0CH
DB 026H,0CH,02DH,0CH,02FH,0CH,036H,06H,00H,01EH,037H,06H,00H,01EH,039H,06H
DB 00H,01EH,03EH,06H,00H,02AH,040H,06H,00H,036H,0FCH,07H,036H,06H,00H,01EH
DB 039H,06H,00H,01EH,036H,06H,00H,012H,03BH,0CH,036H,0CH,031H,0CH,032H,0CH
DB 039H,0CH,040H,0CH,036H,06H,00H,01EH,037H,06H,00H,01EH,039H,06H,00H,01EH
DB 03EH,06H,00H,02AH,040H,06H,00H,01EH,00H,078H,02FH,0CH,02AH,0CH,025H,0CH
DB 026H,0CH,02DH,0CH,02FH,0CH,0FCH,015H,0FFH,00H,02AH,0CH,02FH,0CH,036H,0CH
DB 02BH,0CH,032H,0CH,037H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH
DB 040H,0CH,035H,0CH,034H,0CH,03BH,0CH,042H,0CH,00H,078H,0FCH,013H,031H,0CH
DB 02AH,0CH,025H,0CH,026H,0CH,02DH,0CH,02FH,0CH,0FCH,015H,0FFH,01H,0FCH,07H
DB 0F8H,0EH,01H,0F5H,02EH,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H,0CH,037H
DB 0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,034H,0CH,03BH
DB 0CH,042H,0CH,036H,0CH,03DH,0CH,044H,0CH,037H,0CH,03EH,0CH,045H,0CH,039H
DB 0CH,040H,0CH,047H,0CH,03EH,0CH,045H,0CH,04CH,0CH,040H,0CH,047H,0CH,04EH
DB 06H,00H,00H,00H
creation_theme_part5 LABEL BYTE
DB 0FCH,010H,0F5H,02BH,0F9H,014H,0F4H,00H,00H,0C0H,0FBH,06H,0F8H,04H,0FFH,02BH
DB 0CH,028H,0CH,02BH,0CH,028H,0CH,0F8H,00H,00H,00H,060H,00H,0C0H,00H,0C0H
DB 0FCH,013H,0F5H,032H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH
DB 034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,00H,030H,0FCH,010H
DB 0F8H,09H,0FFH,037H,0CH,034H,0CH,037H,0CH,034H,0CH,037H,0CH,034H,0CH,037H
DB 0CH,034H,0CH,00H,060H,0F8H,00H,00H,0FCH,07H,0FFH,00H,023H,0CH,028H,0CH
DB 02FH,0CH,026H,0CH,02BH,0CH,032H,0CH,028H,0CH,02DH,0CH,034H,0CH,02BH,0CH
DB 032H,0CH,039H,0CH,035H,0CH,02DH,0CH,034H,0CH,03BH,0CH,00H,060H,0FFH,01H
DB 00H,0C0H,0F5H,035H,0F8H,09H,0FFH,0FCH,010H,02BH,0CH,028H,0CH,02BH,0CH,028H
DB 0CH,02BH,0CH,028H,0CH,02BH,0CH,028H,0CH,0F8H,00H,00H,00H,030H,0F5H,035H
DB 0FFH,00H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH,02BH,0CH,032H,0CH,028H,0CH
DB 02DH,0CH,034H,0CH,02BH,0CH,032H,0CH,039H,0CH,029H,0CH,02DH,0CH,034H,0CH
DB 03BH,0CH,00H,030H,0FFH,01H,00H,0C0H,00H,0C0H,031H,0CH,02AH,0CH,031H,0CH
DB 026H,0CH,02DH,0CH,032H,0CH,034H,0CH,02FH,0CH,030H,0CH,03BH,0CH,036H,0CH
DB 031H,0CH,036H,0CH,02DH,0CH,034H,0CH,03DH,0CH,03BH,0CH,034H,0CH,02DH,0CH
DB 036H,0CH,031H,0CH,036H,0CH,03BH,0CH,030H,0CH,02FH,0CH,034H,0CH,032H,0CH
DB 02DH,0CH,026H,0CH,031H,0CH,02AH,0CH,031H,0CH,0F5H,030H,00H,0C0H,00H,0C0H
DB 0FCH,013H,0F4H,040H,0FFH,00H,00H,078H,03BH,0CH,036H,0CH,031H,0CH,032H,0CH
DB 039H,0CH,03BH,0CH,0FCH,07H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH,02BH,0CH
DB 032H,0CH,028H,0CH,02DH,0CH,034H,0CH,02BH,0CH,032H,0CH,039H,0CH,029H,0CH
DB 02DH,0CH,034H,0CH,03BH,0CH,0FCH,015H,0FFH,01H,01EH,0CH,023H,0CH,02AH,0CH
DB 01FH,0CH,026H,0CH,02DH,0CH,021H,0CH,028H,0CH,02AH,0CH,028H,0CH,0FCH,013H
DB 03BH,0CH,036H,0CH,031H,0CH,032H,0CH,039H,0CH,03BH,0CH,0FCH,07H,023H,0CH
DB 028H,0CH,02FH,0CH,026H,0CH,02BH,0CH,032H,0CH,028H,0CH,02DH,0CH,034H,0CH
DB 02BH,0CH,032H,0CH,039H,0CH,029H,0CH,02DH,0CH,034H,0CH,03BH,0CH,0FCH,015H
DB 0FFH,01H,0FCH,015H,01EH,0CH,023H,0CH,02AH,0CH,01FH,0CH,026H,0CH,02DH,0CH
DB 021H,0CH,028H,0CH,02AH,0CH,028H,0CH,03BH,0CH,036H,0CH,031H,0CH,032H,0CH
DB 039H,0CH,03BH,0CH,023H,0CH,028H,0CH,02FH,0CH,026H,0CH,02BH,0CH,032H,0CH
DB 028H,0CH,02DH,0CH,034H,0CH,02BH,0CH,032H,0CH,034H,0CH,035H,0CH,02DH,0CH
DB 034H,0CH,03BH,0CH,00H,0C0H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH,02BH,0CH
DB 032H,0CH,028H,0CH,02DH,0CH,034H,0CH,02BH,0CH,032H,0CH,034H,0CH,035H,0CH
DB 02DH,0CH,034H,0CH,03BH,0CH
bad_news_part3 LABEL BYTE
DB 0FCH,038H,0F5H,030H,0F9H,014H,0F4H,040H,00H,018H,0FBH,00H,06H,024H,09H,024H
DB 06H,030H,01H,024H,0BH,0CH,0FFH,00H,04H,024H,0DH,030H,06H,024H,0BH,0CH
DB 09H,0CH,07H,0CH,06H,03CH,06H,024H,09H,024H,06H,030H,01H,024H,0BH,0CH
DB 0FFH,03H,0F5H,02EH,0FCH,07H,0FBH,09H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH
DB 02BH,0CH,032H,0CH,028H,0CH,02DH,0CH,034H,0CH,02BH,0CH,032H,0CH,039H,0CH
DB 029H,0CH,02DH,0CH,034H,0CH,03BH,0CH,0FBH,00H,00H,018H,0FCH,038H,06H,024H
DB 09H,024H,06H,030H,01H,024H,0BH,0CH,0FFH,01H,0F8H,015H,01H,0F5H,029H,0FBH
DB 09H,0FCH,07H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH,02BH,0CH,032H,0CH,028H
DB 0CH,02DH,0CH,034H,0CH,02BH,0CH,032H,0CH,039H,0CH,02DH,0CH,034H,0CH,03BH
DB 0CH,02FH,0CH,036H,0CH,03DH,0CH,032H,0CH,039H,0CH,03EH,0CH,034H,0CH,039H
DB 0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,039H,0CH,040H,0CH,047H,06H,00H
DB 00H,00H
creation_theme_part6 LABEL BYTE
DB 0FCH,010H,0F5H,02BH,0F9H,014H,0F4H,00H,00H,0C0H,0FBH,06H,0F8H,04H,0FFH,028H
DB 0CH,024H,0CH,028H,0CH,024H,0CH,0F8H,00H,00H,00H,060H,00H,0C0H,00H,0C0H
DB 0F5H,032H,0FCH,013H,02AH,0CH,02FH,0CH,034H,0CH,02BH,0CH,032H,0CH,037H,0CH
DB 02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,00H,030H,0FCH,010H
DB 0F8H,09H,0FFH,034H,0CH,030H,0CH,034H,0CH,030H,0CH,034H,0CH,030H,0CH,034H
DB 0CH,030H,0CH,00H,060H,0F8H,00H,00H,00H,0C0H,00H,0C0H,00H,0C0H,00H,0C0H
DB 0F5H,035H,0F8H,09H,0FFH,034H,0CH,030H,0CH,034H,0CH,030H,0CH,034H,0CH,030H
DB 0CH,034H,0CH,030H,0CH,0F8H,00H,00H,00H,090H,0FFH,00H,00H,0C0H,0FFH,0DH
DB 0FCH,01AH,0FBH,0FFH,0F5H,029H,0F8H,0EH,01H,02AH,018H,02BH,024H,02DH,024H,032H
DB 018H,036H,0FCH,00H,0CH,023H,018H,026H,024H,028H,018H,02BH,0CH,02DH,018H,02FH
DB 018H,0F8H,010H,0FFH,034H,0E4H,00H,0CH,0F5H,02BH,0F8H,0DH,01H,034H,018H,033H
DB 024H,02FH,018H,031H,0CH,02DH,018H,02AH,018H,0F8H,019H,0FFH,028H,0E4H,00H,0CH
bad_news_part4 LABEL BYTE
DB 0FCH,038H,0F5H,030H,0F8H,00H,00H,0F4H,020H,0F9H,014H,0F7H,06H,00H,018H,0FBH
DB 00H,06H,024H,09H,024H,06H,030H,01H,024H,0BH,0CH,0FFH,00H,04H,024H,0DH
DB 030H,06H,024H,0BH,0CH,09H,0CH,07H,0CH,01H,03CH,06H,024H,09H,024H,06H
DB 030H,01H,024H,0BH,0CH,0FFH,03H,0F5H,02EH,0FCH,07H,0FBH,09H,023H,0CH,028H
DB 0CH,02FH,0CH,026H,0CH,02BH,0CH,032H,0CH,028H,0CH,02DH,0CH,034H,0CH,02BH
DB 0CH,032H,0CH,039H,0CH,029H,0CH,02DH,0CH,034H,0CH,03BH,0CH,00H,018H,0FCH
DB 038H,06H,024H,09H,024H,06H,030H,01H,024H,0BH,0CH,0FFH,01H,0F8H,015H,01H
DB 0F5H,029H,0FBH,09H,0FCH,07H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH,02BH,0CH
DB 032H,0CH,028H,0CH,02DH,0CH,034H,0CH,02BH,0CH,032H,0CH,039H,0CH,02DH,0CH
DB 034H,0CH,03BH,0CH,02FH,0CH,036H,0CH,03DH,0CH,032H,0CH,039H,0CH,03EH,0CH
DB 034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,039H,0CH,040H,0CH
DB 047H,06H,00H,00H
creation_theme_part7 LABEL BYTE
DB 0F4H,07FH,0FCH,09H,0F5H,023H,0F9H,014H,0FBH,0FFH,01DH,0C0H,01DH,0C0H,01DH,0F0H
DB 0FBH,00H,021H,0C0H,023H,060H,0F8H,09H,0FFH,024H,0C0H,0F8H,00H,00H,0F5H,02AH
DB 0FCH,010H,0FBH,06H,0FFH,00H,012H,0CH,017H,0CH,01EH,0CH,015H,0CH,01AH,0CH
DB 021H,0CH,017H,0CH,01CH,0CH,023H,0CH,01AH,0CH,021H,0CH,028H,0CH,024H,0CH
DB 01CH,0CH,023H,0CH,02AH,0CH,00H,060H,0FFH,01H,0F8H,09H,01H,0F5H,02CH,01EH
DB 0CH,023H,0CH,01EH,0CH,01FH,0CH,026H,0CH,01FH,0CH,021H,0CH,028H,0CH,029H
DB 0CH,028H,0CH,02FH,0CH,02AH,0CH,02FH,0CH,026H,0CH,02DH,0CH,028H,0CH,0F8H
DB 07H,0FFH,013H,0CH,028H,0CH,024H,0CH,037H,0CH,02BH,0CH,034H,0CH,02BH,0CH
DB 028H,0CH,0FFH,00H,0F8H,00H,00H,00H,030H,012H,0CH,017H,0CH,01EH,0CH,013H
DB 0CH,01AH,0CH,01FH,0CH,015H,0CH,01CH,0CH,023H,0CH,01AH,0CH,021H,0CH,028H
DB 0CH,01DH,0CH,01CH,0CH,023H,0CH,02AH,0CH,0FFH,01H,01AH,0CH,013H,0CH,015H
DB 0CH,01CH,0CH,0F8H,013H,01H,0F5H,02AH,012H,0CH,017H,0CH,012H,0CH,013H,0CH
DB 01AH,0CH,013H,0CH,015H,0CH,01CH,0CH,01DH,0CH,01CH,0CH,023H,0CH,01EH,0CH
DB 023H,0CH,01AH,0CH,021H,0CH,01CH,0CH,01EH,0CH,023H,0CH,02AH,0CH,01FH,0CH
DB 026H,0CH,01FH,0CH,021H,0CH,028H,0CH,029H,0CH,028H,0CH,02FH,0CH,02AH,0CH
DB 02FH,0CH,026H,0CH,02DH,0CH,028H,0CH,02AH,0CH,023H,0CH,02AH,0CH,01FH,0CH
DB 026H,0CH,02BH,0CH,02DH,0CH,028H,0CH,029H,0CH,034H,0CH,02FH,0CH,02AH,0CH
DB 02FH,0CH,026H,0CH,02DH,0CH,028H,0CH,0F8H,013H,0FFH,028H,0CH,02DH,0CH,026H
DB 0CH,02FH,0CH,02AH,0CH,02FH,0CH,034H,0CH,029H,0CH,028H,0CH,02DH,0CH,02BH
DB 0CH,026H,0CH,01FH,0CH,02AH,0CH,023H,0CH,02AH,0CH,028H,0CH,02DH,0CH,026H
DB 0CH,02FH,0CH,02AH,0CH,02FH,0CH,028H,0CH,029H,0CH,028H,0CH,021H,0CH,01FH
DB 0CH,026H,0CH,01FH,0CH,02AH,0CH,023H,0CH,01EH,0CH,00H,0CH,021H,0CH,01AH
DB 0CH,023H,0CH,01EH,0CH,023H,0CH,01CH,0CH,01DH,0CH,01CH,0CH,015H,0CH,013H
DB 0CH,01AH,0CH,013H,0CH,012H,0CH,017H,0CH,012H,0CH,0F8H,0DH,01H,0FFH,00H
DB 0F5H,027H,01EH,0CH,023H,0CH,02AH,0CH,01FH,0CH,026H,0CH,02DH,0CH,021H,0CH
DB 028H,0CH,02AH,0CH,028H,0CH,02FH,0CH,02AH,0CH,025H,0CH,026H,0CH,02DH,0CH
DB 02FH,0CH,0F5H,02BH,0FCH,07H,01EH,0CH,023H,0CH,02AH,0CH,01FH,0CH,026H,0CH
DB 02BH,0CH,021H,0CH,028H,0CH,02FH,0CH,026H,0CH,02DH,0CH,034H,0CH,029H,0CH
DB 028H,0CH,02FH,0CH,036H,0CH,0F5H,022H,0FCH,010H,0FFH,05H
bad_news_part5 LABEL BYTE
DB 0F5H,030H,0FCH,010H,0F9H,01EH,0F4H,00H,0FBH,09H,01EH,0CH,023H,0CH,02AH,0CH
DB 01FH,0CH,026H,0CH,02DH,0CH,021H,0CH,028H,0CH,02AH,0CH,028H,0CH,02FH,0CH
DB 02AH,0CH,025H,0CH,026H,0CH,02DH,0CH,02FH,0CH,0F8H,09H,0FFH,01EH,0CH,023H
DB 0CH,02AH,0CH,01FH,0CH,026H,0CH,02BH,0CH,021H,0CH,028H,0CH,02FH,0CH,026H
DB 0CH,02DH,0CH,034H,0CH,029H,0CH,028H,0CH,02FH,0CH,036H,0CH,0FFH,04H,0F8H
DB 030H,01H,0F5H,033H,01EH,0CH,023H,0CH,02AH,0CH,01FH,0CH,026H,0CH,02BH,0CH
DB 021H,0CH,028H,0CH,02FH,0CH,026H,0CH,02DH,0CH,034H,0CH,029H,0CH,028H,0CH
DB 02FH,0CH,036H,0CH,0FCH,07H,01EH,0CH,023H,0CH,02AH,0CH,01FH,0CH,026H,0CH
DB 02BH,0CH,021H,0CH,028H,0CH,02FH,0CH,026H,0CH,02DH,0CH,034H,0CH,029H,0CH
DB 028H,0CH,02FH,0CH,036H,0CH,0FCH,010H,01EH,0CH,023H,0CH,02AH,0CH,01FH,0CH
DB 026H,0CH,02BH,0CH,021H,0CH,028H,0CH,02FH,0CH,026H,0CH,02DH,0CH,034H,0CH
DB 029H,0CH,028H,0CH,02FH,0CH,036H,0CH,0FCH,07H,01EH,0CH,023H,0CH,02AH,0CH
DB 01FH,0CH,026H,0CH,02BH,0CH,021H,0CH,028H,0CH,02FH,0CH,026H,0CH,02DH,0CH
DB 034H,0CH,028H,0CH,02FH,0CH,036H,0CH,02AH,0CH,031H,0CH,038H,0CH,02BH,0CH
DB 032H,0CH,039H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH
DB 034H,0CH,03BH,0CH,042H,06H,00H,00H
creation_theme_part8 LABEL BYTE
DB 0F4H,00H,0FCH,09H,0F5H,02CH,0F9H,014H,0FBH,0FFH,011H,0C0H,011H,0C0H,011H,0F0H
DB 0FBH,00H,015H,0C0H,017H,060H,0F8H,09H,0FFH,018H,0C0H,0F8H,00H,00H,0F5H,029H
DB 017H,024H,01AH,024H,01CH,024H,01FH,018H,028H,0CH,00H,030H,0F5H,02FH,0FCH,038H
DB 0EH,0CH,08H,0CH,0EH,0CH,08H,018H,08H,018H,0BH,0CH,0F5H,029H,0FCH,02H
DB 017H,024H,01AH,024H,01CH,024H,01FH,018H,028H,0CH,00H,030H,0F5H,02FH,0FCH,038H
DB 0EH,0CH,08H,0CH,0EH,0CH,08H,018H,08H,0CH,06H,0CH,0BH,0CH,0F5H,029H
DB 0FBH,06H,0FCH,02H,06H,0CH,0BH,0CH,012H,0CH,07H,0CH,0EH,0CH,013H,0CH
DB 09H,0CH,010H,0CH,012H,0CH,010H,0CH,017H,0CH,01EH,0CH,017H,0CH,0EH,0CH
DB 015H,0CH,01CH,0CH,0F5H,02FH,00H,048H,0FCH,038H,0EH,0CH,08H,018H,08H,018H
DB 0BH,0CH,0F5H,029H,0FBH,00H,0FCH,02H,023H,018H,026H,024H,028H,024H,02BH,018H
DB 02DH,0CH,00H,0CH,0F5H,02FH,0FCH,038H,0EH,0CH,08H,0CH,0EH,0CH,08H,018H
DB 08H,018H,0BH,0CH,0F5H,029H,0FCH,02H,023H,018H,026H,024H,028H,024H,02BH,018H
DB 02DH,0CH,02FH,0CH,0F5H,02FH,0FCH,038H,0EH,0CH,08H,0CH,0EH,0CH,08H,018H
DB 08H,018H,0BH,0CH,0FCH,02H,0F8H,018H,01H,0F5H,029H,0FBH,06H,06H,0CH,0BH
DB 0CH,012H,0CH,07H,0CH,0EH,0CH,013H,0CH,09H,0CH,010H,0CH,011H,0CH,010H
DB 0CH,017H,0CH,01EH,0CH,017H,0CH,0EH,0CH,015H,0CH,01CH,0CH,012H,0CH,017H
DB 0CH,01EH,0CH,013H,0CH,0EH,0CH,01FH,0CH,015H,0CH,01CH,0CH,01DH,0CH,01CH
DB 0CH,023H,0CH,02AH,0CH,023H,0CH,01AH,0CH,021H,0CH,028H,0CH,01EH,0CH,017H
DB 0CH,01EH,0CH,013H,0CH,01AH,0CH,01FH,0CH,021H,0CH,01CH,0CH,01DH,0CH,01CH
DB 0CH,023H,0CH,02AH,0CH,023H,0CH,01AH,0CH,021H,0CH,028H,0CH,0F8H,017H,0FFH
DB 02FH,0CH,021H,0CH,01AH,0CH,023H,0CH,02AH,0CH,023H,0CH,01CH,0CH,01DH,0CH
DB 01CH,0CH,021H,0CH,01FH,0CH,01AH,0CH,013H,0CH,01EH,0CH,017H,0CH,01EH,0CH
DB 028H,0CH,0F5H,037H,021H,0CH,01AH,0CH,023H,0CH,02AH,0CH,023H,0CH,01CH,0CH
DB 01DH,0CH,01CH,0CH,015H,0CH,01FH,0CH,0EH,0CH,013H,0CH,01EH,0CH,017H,0CH
DB 012H,0CH,00H,0CH,015H,0CH,0EH,0CH,017H,0CH,01EH,0CH,017H,0CH,010H,0CH
DB 011H,0CH,010H,0CH,09H,0CH,013H,0CH,0EH,0CH,07H,0CH,012H,0CH,0BH,0CH
DB 06H,0CH,0F8H,013H,01H,0FFH,00H,0F5H,028H,012H,0CH,017H,0CH,01EH,0CH,013H
DB 0CH,01AH,0CH,021H,0CH,015H,0CH,01CH,0CH,01EH,0CH,01CH,0CH,023H,0CH,01EH
DB 0CH,019H,0CH,01AH,0CH,021H,0CH,023H,0CH,028H,0CH,021H,0CH,01AH,0CH,019H
DB 0CH,01EH,0CH,023H,0CH,01CH,0CH,01EH,0CH,01CH,0CH,015H,0CH,021H,0CH,01AH
DB 0CH,013H,0CH,01EH,0CH,017H,0CH,012H,0CH,0FFH,05H,0F8H,00H,00H
bad_news_part6 LABEL BYTE
DB 0FCH,07H,0F8H,0CH,01H,0F5H,030H,0F9H,0AH,0F4H,07FH,0FBH,09H,012H,0CH,017H
DB 0CH,01EH,0CH,013H,0CH,01AH,0CH,021H,0CH,015H,0CH,01CH,0CH,01EH,0CH,01CH
DB 0CH,023H,0CH,01EH,0CH,019H,0CH,01AH,0CH,021H,0CH,023H,0CH,0F8H,09H,0FFH
DB 01CH,0CH,021H,0CH,01AH,0CH,019H,0CH,01EH,0CH,023H,0CH,01CH,0CH,01EH,0CH
DB 01CH,0CH,015H,0CH,015H,0CH,01AH,0CH,011H,0CH,01EH,0CH,017H,0CH,012H,0CH
DB 0FFH,02H,0F5H,030H,0F8H,013H,01H,01CH,0CH,021H,0CH,01AH,0CH,019H,0CH,01EH
DB 0CH,023H,0CH,01CH,0CH,01EH,0CH,01CH,0CH,015H,0CH,015H,0CH,01AH,0CH,013H
DB 0CH,01EH,0CH,017H,0CH,012H,0CH,01CH,0CH,021H,0CH,01AH,0CH,019H,0CH,01EH
DB 0CH,023H,0CH,01CH,0CH,01EH,0CH,01CH,0CH,015H,0CH,015H,0CH,01AH,0CH,013H
DB 0CH,01EH,0CH,017H,0CH,012H,0CH,0FFH,02H,0F8H,00H,00H,01CH,0CH,021H,0CH
DB 01AH,0CH,019H,0CH,01EH,0CH,023H,0CH,01CH,0CH,01EH,0CH,01CH,0CH,015H,0CH
DB 015H,0CH,01AH,0CH,013H,0CH,01EH,0CH,017H,0CH,012H,0CH,0F5H,02FH,0FCH,03H
DB 028H,0CH,021H,0CH,01AH,0CH,019H,0CH,01EH,0CH,023H,0CH,01CH,0CH,01EH,0CH
DB 01CH,0CH,015H,0CH,021H,0CH,01AH,0CH,013H,0CH,01EH,0CH,017H,0CH,012H,0CH
DB 0F8H,0DH,01H,028H,0CH,021H,0CH,01AH,0CH,019H,0CH,01EH,0CH,023H,0CH,01CH
DB 0CH,01EH,0CH,01CH,0CH,015H,0CH,021H,0CH,01AH,0CH,012H,0CH,017H,06H,00H
DB 00H,00H
russian_anthem_part5 LABEL BYTE
DB 0FCH,0FH,0F4H,07FH,0FFH,00H,00H,0A0H,0FFH,05H,0F5H,019H,0F9H,028H,0F8H,028H
DB 01H,02BH,028H,02CH,028H,02CH,014H,02CH,014H,02EH,028H,030H,028H,032H,028H,035H
DB 014H,035H,014H,037H,028H,037H,028H,033H,028H,032H,028H,030H,028H,033H,028H,035H
DB 028H,032H,014H,032H,014H,033H,028H,0FCH,09H,0F5H,0FH,0F8H,00H,00H,03AH,03CH
DB 03AH,014H,03AH,014H,03AH,014H,03CH,014H,03EH,014H,03FH,0A0H,041H,03CH,041H,014H
DB 041H,014H,041H,014H,043H,014H,044H,014H,043H,0A0H,048H,050H,046H,050H,048H,050H
DB 043H,050H,048H,028H,04BH,028H,04AH,028H,048H,028H,046H,014H,043H,0AH,00H,0AH
DB 048H,028H,0F8H,015H,0FFH,0FBH,0FFH,04FH,0A0H,04FH,0A0H,00H,050H,0FDH
russian_anthem_part6 LABEL BYTE
DB 0FCH,0FH,0F4H,00H,0FFH,00H,00H,0A0H,0FFH,05H,0F5H,019H,0F9H,028H,0F8H,028H
DB 01H,024H,028H,027H,028H,029H,014H,029H,014H,02BH,028H,02BH,028H,02BH,028H,029H
DB 014H,027H,014H,024H,028H,026H,028H,027H,028H,029H,028H,02BH,028H,02CH,028H,029H
DB 028H,02BH,014H,027H,014H,024H,028H,0FCH,08H,0F5H,019H,0F8H,00H,00H,03AH,03CH
DB 03AH,014H,03AH,014H,038H,014H,037H,014H,035H,014H,033H,014H,00H,014H,0F8H,00H
DB 00H,03AH,028H,037H,050H,03AH,032H,00H,0AH,03AH,0AH,00H,0AH,03AH,014H,038H
DB 014H,037H,014H,035H,014H,033H,014H,00H,014H,03AH,028H,037H,050H,03CH,028H,03CH
DB 014H,03CH,014H,037H,028H,037H,028H,03FH,028H,03EH,014H,03CH,014H,03AH,01EH,00H
DB 0AH,037H,028H,03CH,01EH,00H,0AH,03FH,028H,03EH,028H,03CH,01EH,00H,0AH,03AH
DB 014H,037H,0AH,00H,0AH,03CH,028H,0F8H,015H,0FFH,0FBH,0FFH,043H,0A0H,043H,0A0H
DB 00H,050H,0FDH,00H
russian_anthem_part1 LABEL BYTE
DB 0FCH,09H,0F4H,040H,0FBH,04H,00H,0A0H,0FFH,01H,0F5H,019H,0F9H,032H,0F8H,0B4H
DB 01H,01BH,014H,018H,0AH,00H,0AH,01DH,014H,00H,014H,018H,03CH,01AH,014H,01BH
DB 014H,018H,0AH,00H,0AH,01DH,014H,00H,014H,018H,03CH,01AH,014H,01BH,028H,020H
DB 028H,01FH,028H,01DH,028H,01BH,014H,018H,0AH,00H,0AH,01DH,014H,00H,014H,018H
DB 046H,00H,0AH,01BH,014H,018H,0AH,00H,0AH,01DH,014H,00H,014H,018H,03CH,01AH
DB 014H,01BH,014H,018H,0AH,00H,0AH,01DH,014H,00H,014H,018H,03CH,01AH,014H,01BH
DB 028H,020H,028H,01FH,028H,01DH,028H,01BH,014H,018H,0AH,00H,0AH,01DH,014H,00H
DB 014H,018H,046H,00H,0AH,0FCH,08H,0F5H,023H,0F8H,00H,00H,02EH,03CH,02EH,014H
DB 02EH,014H,02CH,014H,02BH,014H,029H,014H,027H,028H,02EH,028H,02BH,050H,02EH,03CH
DB 02EH,014H,02EH,014H,02CH,014H,02BH,014H,029H,014H,027H,028H,02EH,028H,02BH,050H
DB 030H,028H,030H,014H,030H,014H,02BH,028H,02BH,028H,033H,028H,032H,014H,030H,014H
DB 02EH,028H,02BH,028H,030H,028H,033H,028H,032H,028H,030H,01EH,00H,0AH,02EH,014H
DB 02BH,014H,030H,028H,0F8H,015H,0FFH,0FBH,0FFH,02BH,0A0H,02BH,0A0H,00H,050H,0FDH
russian_anthem_part2 LABEL BYTE
DB 0FCH,018H,0F4H,07FH,0F5H,023H,0F9H,01EH,0F8H,060H,0FFH,0FFH,00H,01FH,028H,01BH
DB 028H,01AH,028H,018H,028H,0FFH,02H,0F8H,00H,00H,0FFH,00H,00H,0A0H,0FFH,06H
DB 0FCH,09H,00H,028H,0F5H,023H,03AH,028H,03AH,028H,038H,028H,037H,050H,033H,014H
DB 030H,014H,032H,014H,033H,014H,035H,050H,035H,028H,035H,028H,033H,050H,032H,014H
DB 030H,014H,033H,014H,032H,014H,030H,050H,037H,050H,03CH,050H,037H,050H,03CH,050H
DB 033H,050H,0F8H,010H,01H,037H,028H,038H,014H,03AH,014H,03CH,014H,03EH,014H,03FH
DB 014H,041H,014H,043H,028H,0F8H,08H,0FFH,03FH,028H,03EH,028H,03CH,028H,037H,028H
DB 033H,028H,032H,028H,030H,01EH,00H,0AH,0FDH,00H
russian_anthem_part3 LABEL BYTE
DB 0F4H,00H,0FFH,00H,00H,0A0H,0FFH,09H,0F5H,019H,0F9H,03CH,0FCH,09H,00H,028H
DB 035H,028H,035H,028H,035H,028H,033H,050H,033H,014H,030H,014H,032H,014H,033H,014H
DB 02EH,050H,030H,028H,032H,028H,033H,050H,032H,014H,030H,014H,033H,014H,032H,014H
DB 02BH,050H,032H,050H,037H,050H,032H,050H,037H,050H,02BH,050H,0F8H,010H,01H,02BH
DB 028H,02CH,014H,02EH,014H,030H,014H,032H,014H,033H,014H,035H,014H,037H,028H,0F8H
DB 08H,0FFH,033H,028H,032H,028H,030H,028H,02BH,028H,027H,028H,026H,028H,024H,01EH
DB 00H,0AH,0FDH,00H
russian_anthem_part4 LABEL BYTE
DB 0F4H,07FH,0FFH,00H,00H,0A0H,0FFH,09H,0F5H,019H,0F9H,03CH,0FCH,09H,00H,028H
DB 022H,028H,024H,028H,026H,028H,01FH,050H,01BH,014H,018H,014H,01AH,014H,018H,014H
DB 022H,050H,024H,028H,026H,028H,02BH,050H,026H,014H,024H,014H,027H,014H,026H,014H
DB 01FH,050H,026H,050H,02BH,050H,026H,050H,02BH,050H,01BH,050H,0F8H,010H,01H,01FH
DB 028H,020H,014H,022H,014H,024H,014H,026H,014H,027H,014H,029H,014H,02BH,028H,0F8H
DB 08H,0FFH,027H,028H,026H,028H,024H,028H,01FH,028H,01BH,028H,01AH,028H,018H,01EH
DB 00H,0AH,0FDH,00H
russian_anthem_part7 LABEL BYTE
DB 0FCH,01BH,0F4H,07FH,0FFH,00H,00H,0A0H,0FFH,09H,0F5H,019H,0F9H,03CH,00H,028H
DB 022H,028H,024H,028H,026H,028H,027H,050H,027H,014H,029H,014H,02BH,014H,02CH,014H
DB 02EH,028H,022H,028H,024H,028H,026H,028H,027H,050H,029H,014H,02BH,014H,02CH,014H
DB 02EH,014H,030H,028H,024H,050H,01FH,028H,024H,014H,026H,014H,027H,014H,029H,014H
DB 02BH,028H,01FH,028H,024H,028H,026H,028H,027H,028H,029H,028H,0F8H,010H,01H,01FH
DB 028H,020H,014H,022H,014H,024H,014H,026H,014H,027H,014H,029H,014H,02BH,028H,0F8H
DB 08H,0FFH,027H,028H,026H,014H,026H,014H,024H,028H,01FH,028H,01BH,028H,01AH,014H
DB 01AH,014H,018H,01EH,00H,0AH,0FDH,00H
russian_anthem_part8 LABEL BYTE
DB 0FCH,0FH,0F4H,00H,0F5H,0FH,0F9H,046H,0FBH,04H,0FFH,00H,01FH,028H,01BH,028H
DB 01AH,014H,01AH,014H,018H,028H,0FFH,02H,01FH,028H,01DH,028H,01BH,014H,01BH,014H
DB 01AH,028H,01FH,028H,024H,028H,022H,014H,022H,014H,020H,028H,0FFH,00H,01FH,028H
DB 01DH,028H,01BH,014H,01BH,014H,01AH,028H,0FFH,02H,01FH,028H,024H,028H,022H,014H
DB 022H,014H,020H,028H,01FH,028H,01BH,028H,01AH,014H,01AH,014H,018H,028H,0F5H,019H
DB 0F9H,03CH,0FCH,01BH,00H,028H,016H,028H,018H,028H,01AH,028H,01BH,050H,01BH,014H
DB 01DH,014H,01FH,014H,020H,014H,022H,028H,016H,028H,018H,028H,01AH,028H,01BH,050H
DB 01DH,014H,01FH,014H,020H,014H,022H,014H,024H,028H,018H,050H,013H,028H,018H,014H
DB 01AH,014H,01BH,014H,01DH,014H,01FH,028H,013H,028H,018H,028H,01AH,028H,01BH,028H
DB 01DH,028H,0F8H,010H,01H,013H,028H,014H,014H,016H,014H,018H,014H,01AH,014H,01BH
DB 014H,01DH,014H,01FH,028H,0F8H,08H,0FFH,01BH,028H,01AH,014H,01AH,014H,018H,028H
DB 013H,028H,0FH,028H,0EH,014H,0EH,014H,0CH,01EH,00H,0AH,0FDH,00H
russian_motif_part1 LABEL BYTE
DB 0FCH,07H,0F5H,019H,0F9H,028H,0F4H,07FH,038H,028H,035H,028H,03AH,050H,035H,050H
DB 00H,00H
russian_motif_part2 LABEL BYTE
DB 0FCH,07H,0F5H,023H,0F9H,028H,0F4H,00H,02CH,028H,029H,028H,02EH,050H,029H,050H
DB 00H,00H
russian_motif_part3 LABEL BYTE
DB 0FCH,07H,0F5H,02DH,0F9H,032H,0F4H,040H,020H,028H,01DH,028H,022H,050H,01DH,050H
DB 00H,00H
russian_motif_part4 LABEL BYTE
DB 0FCH,09H,0F5H,019H,0F9H,01EH,0F4H,07FH,035H,028H,036H,014H,038H,014H,03AH,014H
DB 03CH,014H,03DH,014H,03FH,014H,041H,050H,00H,00H
russian_motif_part5 LABEL BYTE
DB 0FCH,09H,0F5H,019H,0F9H,03CH,0F4H,07FH,029H,028H,02AH,014H,02CH,014H,02EH,014H
DB 030H,014H,031H,014H,033H,014H,035H,050H,00H,00H
russian_motif_part6 LABEL BYTE
DB 0FCH,09H,0F5H,023H,0F9H,014H,0F4H,07FH,01DH,028H,01EH,014H,020H,014H,022H,014H
DB 024H,014H,025H,014H,027H,014H,029H,050H,00H,00H
russian_motif_part7 LABEL BYTE
DB 0FCH,01BH,0F5H,019H,0F9H,03CH,0F4H,07FH,011H,028H,01EH,014H,020H,014H,022H,014H
DB 024H,014H,025H,014H,027H,014H,029H,050H,00H,00H
russian_motif_part8 LABEL BYTE
DB 0FCH,01BH,0F5H,019H,0F9H,03CH,0F4H,00H,0F8H,05H,01H,011H,028H,012H,014H,014H
DB 014H,016H,014H,018H,014H,019H,014H,01BH,014H,01DH,014H,00H,00H,00H
german_anthem_part2 LABEL BYTE
DB 0F5H,0FH,0FCH,01AH,0FFH,00H,0FBH,0FFH,00H,014H,0F4H,040H,0F9H,05DH,034H,014H
DB 030H,0FH,00H,05H,037H,014H,0F9H,060H,032H,0FH,00H,05H,037H,0FH,00H,05H
DB 03CH,050H,0F9H,050H,03BH,0AH,00H,0AAH,0F4H,00H,0FCH,014H,0F9H,05EH,032H,014H
DB 035H,01EH,0F9H,050H,032H,0AH,034H,0AH,00H,032H,0F4H,040H,0FCH,01AH,0F9H,058H
DB 039H,014H,03CH,0FH,00H,05H,036H,0FH,00H,05H,03EH,028H,037H,0FH,00H,05H
DB 03CH,03CH,0FCH,014H,0F4H,00H,03BH,0FH,00H,05H,039H,028H,037H,0FH,00H,05H
DB 035H,028H,034H,028H,032H,0FH,00H,05H,037H,014H,036H,028H,0F9H,046H,037H,0AH
DB 00H,01EH,0F5H,05H,0FFH,01H,0FCH,017H,00H,014H,0F4H,040H,0F9H,05DH,03BH,014H
DB 037H,0FH,00H,05H,03CH,0FH,00H,05H,0F9H,060H,040H,03CH,03EH,0AH,03CH,0AH
DB 03BH,0AH,039H,0AH,038H,0AH,039H,0AH,03BH,03CH,0F4H,00H,039H,014H,03CH,0FH
DB 00H,05H,036H,0FH,00H,05H,038H,014H,039H,028H,0F9H,05EH,038H,0AH,036H,0AH
DB 034H,0AH,032H,0AH,0F9H,050H,030H,0AH,02FH,0AH,02DH,014H,00H,028H,0F4H,040H
DB 0FCH,01AH,0F9H,058H,035H,014H,039H,0FH,00H,05H,032H,0FH,00H,05H,034H,014H
DB 037H,0FH,00H,05H,030H,0FH,00H,05H,0F4H,00H,035H,03CH,037H,014H,035H,014H
DB 034H,0AH,035H,0AH,037H,032H,035H,0AH,037H,0AH,035H,0AH,034H,0AH,032H,0AH
DB 037H,0FH,00H,05H,030H,014H,032H,028H,0F9H,046H,030H,0AH,00H,01EH,0FDH,00H
german_anthem_part3 LABEL BYTE
DB 0F5H,0FH,0FCH,01AH,0FBH,0FFH,0FFH,00H,00H,050H,0F4H,020H,0F9H,05EH,02BH,014H
DB 02FH,0FH,00H,05H,028H,014H,030H,0FH,00H,05H,035H,014H,034H,0AH,0F9H,050H
DB 030H,0AH,032H,014H,00H,064H,0FCH,014H,0F9H,05EH,029H,014H,02DH,0FH,00H,05H
DB 026H,0FH,00H,05H,02BH,03CH,0F9H,046H,030H,014H,00H,0A0H,0F4H,07FH,0FCH,01AH
DB 0F9H,05EH,036H,014H,039H,0FH,00H,05H,0F4H,020H,0FCH,014H,032H,028H,030H,028H
DB 0F4H,07FH,0FCH,01AH,02FH,014H,032H,0FH,00H,05H,02BH,014H,02DH,0FH,00H,05H
DB 030H,0FH,00H,05H,02AH,014H,032H,0FH,00H,05H,0F4H,020H,0FCH,014H,030H,01EH
DB 0F9H,046H,02DH,0AH,02FH,014H,00H,014H,0F5H,05H,0FFH,01H,00H,050H,0F4H,020H
DB 0F5H,0FH,0F9H,05EH,0FCH,017H,037H,014H,034H,0FH,00H,05H,030H,014H,032H,0FH
DB 00H,05H,035H,014H,02FH,028H,00H,028H,0F9H,050H,0FCH,014H,034H,014H,030H,0FH
DB 00H,05H,0F9H,05EH,02DH,0FH,00H,05H,02FH,014H,030H,0FH,00H,05H,02FH,03CH
DB 0F9H,046H,02DH,028H,00H,028H,0F4H,07FH,032H,014H,035H,0FH,00H,05H,02FH,0FH
DB 00H,05H,030H,014H,034H,0FH,00H,05H,0FCH,01AH,0F9H,05EH,02DH,0FH,00H,05H
DB 032H,03CH,0F4H,020H,02FH,014H,032H,028H,0F4H,07FH,030H,014H,034H,0FH,00H,05H
DB 02DH,03CH,0F9H,046H,026H,014H,02BH,014H,029H,028H,0F4H,020H,028H,014H,00H,014H
DB 0FDH,00H
german_anthem_part5 LABEL BYTE
DB 0FCH,01AH,0F5H,0AH,0FBH,0FFH,0FFH,00H,00H,08CH,0F4H,060H,0F9H,062H,028H,014H
DB 024H,0FH,00H,05H,02BH,028H,00H,028H,0FCH,014H,0F4H,07FH,024H,014H,028H,0FH
DB 00H,05H,021H,014H,026H,0FH,00H,05H,024H,028H,023H,0AH,021H,0AH,023H,014H
DB 024H,014H,0F9H,046H,01FH,0AH,00H,06EH,0F4H,00H,0FCH,01AH,0F9H,062H,02BH,014H
DB 02FH,0FH,00H,05H,0F4H,07FH,0FCH,014H,028H,0FH,00H,05H,02DH,028H,00H,014H
DB 0F4H,00H,0FCH,01AH,02AH,014H,02DH,0FH,00H,05H,0F4H,07FH,0FCH,014H,026H,050H
DB 0FCH,01AH,0F4H,00H,024H,014H,028H,0FH,00H,05H,021H,014H,023H,0FH,00H,05H
DB 0FCH,014H,021H,014H,0F4H,07FH,01FH,028H,00H,014H,0F5H,05H,0FFH,01H,00H,050H
DB 0F4H,060H,0FCH,017H,0F9H,062H,02BH,014H,030H,0FH,00H,05H,02DH,0FH,00H,05H
DB 029H,03CH,0F4H,07FH,028H,0AH,026H,0AH,024H,0AH,023H,0AH,024H,0AH,026H,0AH
DB 028H,032H,00H,0AH,026H,014H,028H,0FH,00H,05H,029H,0FH,00H,05H,028H,03CH
DB 024H,014H,0F9H,046H,028H,0FH,00H,05H,0F4H,00H,0FCH,01AH,0F9H,062H,021H,014H
DB 026H,0AH,024H,0AH,023H,0AH,024H,0AH,0F4H,07FH,02BH,03CH,029H,014H,023H,014H
DB 026H,0FH,00H,05H,0F4H,00H,01FH,064H,0F4H,07FH,024H,028H,0F4H,00H,026H,014H
DB 029H,0FH,00H,05H,023H,014H,024H,0FH,00H,05H,026H,014H,0F4H,07FH,024H,0AH
DB 026H,0AH,028H,014H,00H,014H,0FEH,01H,0FDH,00H
german_anthem_part6 LABEL BYTE
DB 0FCH,01AH,0F5H,05H,0FFH,00H,0F4H,020H,0F9H,065H,0FBH,0AH,024H,03CH,023H,03CH
DB 021H,03CH,00H,014H,0FBH,0FFH,01FH,014H,0FBH,0AH,023H,014H,01CH,03CH,01DH,03CH
DB 01FH,03CH,00H,014H,0FBH,0FFH,01CH,014H,0FBH,0AH,01FH,014H,018H,014H,024H,03CH
DB 023H,03CH,0FBH,0FFH,021H,014H,0FBH,0AH,024H,014H,01EH,014H,0FBH,0FFH,026H,014H
DB 0FBH,0AH,02AH,014H,023H,03CH,024H,014H,018H,03CH,0FBH,0FFH,017H,014H,0FBH,0AH
DB 01AH,014H,013H,01EH,0F5H,00H,0F9H,050H,01FH,0AH,0F9H,055H,021H,0AH,0F9H,05AH
DB 023H,0AH,0FFH,01H,0FCH,017H,0F9H,065H,0FBH,0FFH,02BH,028H,029H,0FH,00H,05H
DB 028H,028H,024H,0FH,00H,05H,029H,0FH,00H,05H,023H,014H,026H,014H,028H,0FH
DB 00H,05H,020H,014H,01CH,0FH,00H,05H,021H,014H,024H,0FH,00H,05H,021H,014H
DB 026H,0FH,00H,05H,023H,014H,021H,0FH,00H,05H,026H,014H,028H,0FH,00H,05H
DB 01CH,014H,021H,028H,01FH,014H,01DH,050H,01CH,03CH,0FCH,01BH,01AH,014H,01DH,0FH
DB 00H,05H,017H,014H,013H,0FH,00H,05H,017H,014H,018H,0FH,00H,05H,01CH,014H
DB 018H,0FH,00H,05H,01DH,050H,01CH,014H,01FH,0FH,00H,05H,018H,01EH,0F5H,0EH
DB 0F9H,050H,01FH,0AH,0F9H,04BH,021H,0AH,0F9H,046H,023H,0AH,0FDH,00H,00H,00H
german_anthem_part4 LABEL BYTE
DB 0FCH,014H,0F5H,04H,0FFH,00H,0F4H,060H,0F9H,065H,0FBH,0AH,018H,03CH,017H,03CH
DB 015H,03CH,00H,014H,0FBH,0FFH,013H,014H,0FBH,0AH,017H,014H,010H,03CH,011H,03CH
DB 013H,03CH,00H,014H,0FBH,0FFH,010H,014H,0FBH,0AH,013H,014H,0CH,014H,018H,03CH
DB 017H,03CH,015H,014H,0FBH,0AH,018H,014H,012H,014H,01AH,014H,01EH,014H,017H,03CH
DB 018H,014H,0CH,03CH,0FBH,0FFH,0BH,014H,0FBH,0AH,0EH,014H,07H,01EH,0F5H,00H
DB 0F9H,050H,013H,0AH,0F9H,055H,015H,0AH,0F9H,05AH,017H,0AH,0FFH,01H,0FCH,017H
DB 0F9H,065H,0FBH,0FFH,01FH,028H,01DH,0FH,00H,05H,01CH,028H,018H,0FH,00H,05H
DB 01DH,0FH,00H,05H,017H,014H,01AH,014H,01CH,0FH,00H,05H,014H,014H,010H,0FH
DB 00H,05H,015H,014H,018H,0FH,00H,05H,015H,014H,01AH,0FH,00H,05H,017H,014H
DB 015H,0FH,00H,05H,01AH,014H,01CH,0FH,00H,05H,010H,014H,015H,028H,013H,014H
DB 011H,050H,010H,03CH,0FCH,01BH,0EH,014H,011H,0FH,00H,05H,0BH,014H,07H,0FH
DB 00H,05H,0BH,014H,0CH,0FH,00H,05H,010H,014H,0CH,0FH,00H,05H,011H,050H
DB 010H,014H,013H,0FH,00H,05H,0CH,01EH,0F5H,0EH,0F9H,050H,07H,0AH,0F9H,04BH
DB 09H,0AH,0F9H,046H,0BH,0AH,0FDH,00H
german_anthem_part1 LABEL BYTE
DB 0FCH,012H,0FFH,00H,00H,078H,0F4H,040H,0FFH,06H,00H,064H,0FFH,00H,0F5H,014H
DB 0F8H,08H,01H,0F9H,053H,037H,05H,00H,05H,037H,05H,00H,05H,0F8H,09H,0FFH
DB 037H,078H,00H,064H,0FFH,03H,0F9H,05DH,0F4H,07FH,0F5H,0FH,0F8H,08H,01H,043H
DB 05H,00H,05H,043H,05H,00H,05H,0F8H,09H,0FFH,043H,078H,00H,064H,0F4H,00H
DB 0F5H,0FH,0F8H,08H,01H,040H,05H,00H,05H,040H,05H,00H,05H,0F8H,09H,0FFH
DB 040H,078H,00H,064H,0F4H,07FH,0F5H,0FH,0F8H,08H,01H,045H,05H,00H,05H,045H
DB 05H,00H,05H,0F8H,09H,0FFH,045H,078H,00H,064H,0F4H,00H,0F5H,0FH,0F8H,08H
DB 01H,048H,05H,00H,05H,048H,05H,00H,05H,0F8H,09H,0FFH,0FBH,0FFH,048H,0A0H
DB 047H,028H,048H,028H,0FDH,00H
german_motif_part1 LABEL BYTE
DB 0F5H,0FH,0FCH,01AH,0F4H,040H,0F9H,05DH,0FBH,05H,028H,0AH,028H,0AH,028H,028H
DB 026H,0AH,026H,0AH,026H,028H,024H,0BH,024H,0BH,024H,060H,00H,00H
german_motif_part2 LABEL BYTE
DB 0FCH,04H,0F5H,0FH,0F4H,060H,0F9H,067H,0FBH,05H,01FH,0AH,01FH,0AH,02BH,028H
DB 02BH,0AH,02BH,0AH,02BH,028H,037H,0BH,037H,0BH,037H,060H,00H,00H
german_motif_part3 LABEL BYTE
DB 0FCH,04H,0F5H,0FH,0F4H,060H,0F9H,067H,0FBH,05H,037H,0AH,037H,0AH,037H,028H
DB 037H,0AH,037H,0AH,037H,028H,043H,0BH,043H,0BH,043H,060H,00H,00H
german_motif_part4 LABEL BYTE
DB 0FCH,01BH,0F5H,0FAH,0F4H,060H,0F9H,065H,024H,050H,01FH,03EH,0CH,060H,00H,00H
german_motif_part5 LABEL BYTE
DB 0FCH,01BH,0F5H,0FAH,0F4H,07FH,0F9H,065H,018H,050H,013H,03EH,0CH,060H,00H,00H
title_theme_part1 LABEL BYTE
DB 0FCH,0CH,0F4H,00H,00H,060H,0F5H,010H,0F9H,014H,0F8H,054H,01H,0FBH,0FFH,043H
DB 060H,043H,0F0H,043H,0A8H,045H,0D8H,045H,060H,048H,090H,04AH,0C0H,04AH,084H,04AH
DB 0FCH,0F8H,012H,0FFH,0F5H,02BH,04CH,0C0H,04CH,0C0H,00H,0C0H,0FCH,015H,0F8H,0BH
DB 01H,0F5H,01AH,0FBH,06H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH
DB 0CH,034H,0CH,039H,0CH,0FBH,0FFH,0F8H,01CH,0FFH,040H,060H,040H,0C0H,00H,0C0H
DB 0FBH,06H,0F5H,027H,0F8H,0BH,01H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H
DB 0CH,03EH,0CH,034H,0CH,039H,0CH,0FBH,0FFH,0F8H,010H,0FFH,040H,060H,040H,0C0H
DB 0FBH,06H,0F5H,029H,0F8H,011H,01H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H
DB 0CH,03EH,0CH,034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,0FCH
DB 03H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH,034H,0CH,039H
DB 0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,039H,0CH,040H,0CH,047H,0CH,0F8H
DB 00H,00H,00H,0C0H,00H,0C0H,00H,0C0H,00H,078H,0F5H,029H,0F8H,025H,0FFH,02FH
DB 0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH,034H,0CH,039H,0CH,040H
DB 0CH,037H,0CH,03EH,0CH,045H,0CH,00H,078H,0FCH,01H,0F5H,01AH,02AH,0CH,02FH
DB 0CH,036H,0CH,02BH,0CH,032H,0CH,039H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H
DB 0CH,039H,0CH,040H,0CH,00H,060H,0FCH,015H,0F5H,024H,02AH,0CH,02FH,0CH,036H
DB 0CH,02BH,0CH,032H,0CH,039H,0CH,02DH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H
DB 0CH,040H,0CH,00H,060H,0FCH,013H,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H
DB 0CH,039H,0CH,02DH,0CH,034H,0CH,00H,060H,0FCH,0FH,0F5H,01FH,0F8H,00H,00H
DB 0FFH,00H,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H,0CH,039H,0CH,02DH,0CH
DB 034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,00H,090H,0FFH,0FFH,00H,00H
title_theme_part2 LABEL BYTE
DB 0FCH,02H,0F5H,029H,0F9H,014H,0F4H,07FH,02BH,018H,030H,018H,0F8H,013H,0FFH,037H
DB 0B0H,0F5H,029H,035H,020H,02EH,020H,029H,090H,0F5H,029H,02BH,018H,030H,018H,037H
DB 060H,0F5H,029H,030H,0CH,037H,0CH,039H,018H,0F5H,029H,034H,020H,02FH,0D0H,0F5H
DB 02EH,030H,018H,035H,018H,03CH,090H,0F5H,02EH,03EH,018H,03AH,018H,033H,018H,0F5H
DB 02EH,02EH,078H,0F5H,033H,030H,018H,035H,018H,03CH,048H,0F5H,033H,041H,0CH,043H
DB 0CH,03EH,018H,039H,018H,0F5H,033H,034H,060H,0FBH,0FFH,02DH,060H,0F8H,021H,0FFH
DB 0F5H,021H,02DH,0C0H,00H,0C0H,00H,0C0H,0F8H,0BH,01H,0F5H,01AH,0FBH,06H,02AH
DB 0CH,02FH,0CH,036H,0CH,02BH,0CH,032H,0CH,037H,0CH,02DH,0CH,034H,0CH,0FBH
DB 0FFH,0F8H,01CH,0FFH,039H,060H,039H,0C0H,039H,0C0H,0FBH,06H,0F5H,025H,0F8H,0BH
DB 01H,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H,0CH,037H,0CH,02DH,0CH,034H
DB 0CH,0FBH,0FFH,0F8H,010H,0FFH,039H,060H,039H,0C0H,0FBH,06H,0F5H,01FH,0FCH,015H
DB 0F8H,013H,01H,02AH,0CH,02FH,0CH,036H,0CH,02DH,0CH,032H,0CH,039H,0CH,02FH
DB 0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,02AH,0CH,02FH,0CH,036H
DB 0CH,0FCH,03H,02DH,0CH,032H,0CH,039H,0CH,02FH,0CH,034H,0CH,03BH,0CH,032H
DB 0CH,039H,0CH,040H,0CH,034H,0CH,03BH,0CH,042H,0CH,0F5H,036H,0F8H,00H,00H
DB 0FBH,00H,030H,0CH,035H,0CH,03CH,090H,00H,018H,03EH,018H,03AH,018H,033H,018H
DB 02EH,078H,037H,0CH,03CH,0CH,043H,090H,00H,018H,041H,018H,043H,018H,03EH,030H
DB 039H,060H,0F8H,019H,0FFH,034H,060H,032H,060H,0FBH,0FFH,0FFH,00H,034H,0C0H,0FFH
DB 05H,00H,00H,00H
title_theme_part3 LABEL BYTE
DB 0FCH,0CH,0F5H,01FH,0F9H,014H,0F4H,00H,0FFH,00H,00H,060H,0FFH,0DH,00H,024H
DB 043H,0FCH,0FCH,09H,0F5H,029H,015H,0C0H,015H,060H,0FFH,00H,0F8H,013H,0FFH,013H
DB 0C0H,0F8H,013H,01H,015H,0C0H,0FFH,02H,0F8H,013H,0FFH,013H,060H,0FCH,015H,0FBH
DB 06H,0F5H,01FH,0F8H,01CH,01H,02AH,0CH,02FH,0CH,036H,0CH,02DH,0CH,032H,0CH
DB 039H,0CH,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,02AH,0CH
DB 02FH,0CH,036H,0CH,02DH,0CH,032H,0CH,039H,0CH,0FCH,07H,02FH,0CH,034H,0CH
DB 03BH,0CH,032H,0CH,039H,0CH,040H,0CH,034H,0CH,03BH,0CH,042H,0CH,0F5H,036H
DB 0F8H,00H,00H,0FBH,00H,030H,0CH,035H,0CH,03CH,090H,00H,018H,03EH,018H,03AH
DB 018H,033H,018H,02EH,078H,030H,0CH,035H,0CH,03CH,090H,00H,018H,03CH,018H,03EH
DB 018H,039H,030H,034H,060H,0FBH,0FFH,0F8H,019H,0FFH,0FFH,00H,02DH,0C0H,0FFH,06H
DB 00H,00H
title_theme_part4 LABEL BYTE
DB 0FCH,02H,0F4H,00H,0F9H,014H,0FFH,00H,00H,060H,0FFH,08H,0F5H,01FH,0F8H,013H
DB 0FFH,02BH,018H,030H,018H,037H,090H,0F5H,01FH,039H,018H,035H,018H,02EH,018H,0F5H
DB 01FH,029H,078H,0F5H,024H,02BH,018H,030H,018H,037H,048H,0F5H,024H,03CH,0CH,03EH
DB 0CH,039H,018H,034H,018H,02FH,060H,026H,060H,0F4H,07FH,0FCH,09H,0F5H,01FH,0F8H
DB 00H,00H,0FBH,0FFH,02AH,0C0H,02AH,060H,0FFH,00H,0F8H,013H,0FFH,02BH,0C0H,0F8H
DB 013H,01H,02AH,0C0H,0FFH,02H,0F8H,013H,0FFH,02BH,0C0H,0F8H,013H,01H,02AH,030H
DB 0FCH,015H,0FBH,06H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH,02BH,0CH,032H,0CH
DB 028H,0CH,02DH,0CH,034H,0CH,0FCH,0EH,02BH,0CH,032H,0CH,039H,0CH,02DH,0CH
DB 034H,0CH,03BH,0CH,0F8H,00H,00H,0F5H,036H,029H,0CH,030H,0CH,030H,090H,00H
DB 018H,032H,018H,02EH,018H,027H,018H,029H,078H,029H,0CH,030H,0CH,030H,090H,00H
DB 018H,035H,018H,037H,018H,032H,030H,02DH,060H,0F8H,019H,0FFH,02FH,060H,02BH,060H
DB 0FBH,0FFH,0FFH,00H,024H,0C0H,0FFH,05H,00H,00H
title_theme_part5 LABEL BYTE
DB 0FCH,02H,0F4H,00H,0F5H,01FH,0F9H,01EH,0FFH,00H,00H,060H,0FFH,08H,0F8H,013H
DB 0FFH,02BH,018H,030H,018H,037H,090H,0F5H,01FH,039H,018H,035H,018H,02EH,018H,0F5H
DB 01FH,029H,078H,0F5H,01FH,02BH,018H,030H,018H,037H,048H,0F5H,01FH,03CH,0CH,03EH
DB 0CH,039H,018H,034H,018H,02FH,060H,026H,060H,0FCH,09H,0F5H,029H,021H,0C0H,021H
DB 05EH,00H,02H,0FFH,00H,0F8H,013H,0FFH,021H,0C0H,0F8H,013H,01H,021H,0C0H,0FFH
DB 02H,0F8H,013H,0FFH,021H,0C0H,0F8H,013H,01H,021H,030H,0F5H,01FH,0FCH,015H,0FBH
DB 06H,023H,0CH,028H,0CH,02FH,0CH,026H,0CH,02BH,0CH,032H,0CH,028H,0CH,02DH
DB 0CH,034H,0CH,02BH,0CH,032H,0CH,039H,0CH,0FCH,0EH,02DH,0CH,034H,0CH,03BH
DB 0CH,0F8H,00H,00H,0F5H,036H,018H,0CH,01DH,0CH,02BH,090H,00H,018H,02DH,018H
DB 029H,018H,022H,018H,016H,078H,018H,0CH,01DH,0CH,02BH,090H,00H,018H,024H,018H
DB 026H,018H,021H,030H,01CH,060H,0F8H,019H,0FFH,01CH,060H,01AH,060H,0FBH,0FFH,0FFH
DB 00H,013H,0C0H,0FFH,05H,00H,00H,00H
title_theme_part6 LABEL BYTE
DB 0FCH,02H,0F5H,01AH,0F9H,01EH,0F4H,07FH,0FFH,00H,00H,0C0H,0FFH,06H,00H,048H
DB 0F5H,024H,041H,0CH,043H,0CH,03EH,018H,039H,018H,02FH,060H,032H,030H,0FCH,07H
DB 0F5H,024H,023H,0C0H,023H,060H,0FFH,00H,0F8H,013H,0FFH,024H,0C0H,0F8H,013H,01H
DB 023H,0C0H,0FFH,02H,0F8H,013H,0FFH,024H,0C0H,0F8H,0AH,01H,023H,030H,0FCH,015H
DB 0F5H,01FH,0FBH,06H,012H,0CH,017H,0CH,01EH,0CH,015H,0CH,01AH,0CH,021H,0CH
DB 017H,0CH,01CH,0CH,023H,0CH,01AH,0CH,021H,0CH,028H,0CH,01CH,0CH,023H,0CH
DB 02AH,0CH,0FCH,09H,0F8H,00H,00H,0F5H,026H,0F9H,014H,024H,090H,02BH,030H,026H
DB 090H,027H,060H,020H,090H,02BH,090H,028H,030H,021H,090H,01FH,030H,0FBH,0FFH,01DH
DB 0C0H,0FFH,00H,01DH,0C0H,0FFH,0FFH,00H,00H,00H
title_theme_part7 LABEL BYTE
DB 0FCH,02H,0F9H,014H,0F4H,00H,0FFH,00H,00H,0C0H,0FFH,06H,00H,048H,0F5H,024H
DB 03CH,0CH,03EH,0CH,039H,018H,034H,018H,034H,060H,02DH,030H,0FCH,07H,0FBH,0FFH
DB 01CH,0C0H,01CH,060H,0FFH,00H,0F8H,0CH,0FFH,01CH,0C0H,0F8H,0CH,01H,01CH,0C0H
DB 0FFH,02H,0F8H,08H,0FFH,01CH,0C0H,0F8H,0AH,01H,01CH,030H,0FCH,09H,0F5H,01FH
DB 0FBH,06H,012H,0CH,017H,0CH,01EH,0CH,015H,0CH,01AH,0CH,021H,0CH,017H,0CH
DB 01CH,0CH,023H,0CH,01AH,0CH,021H,0CH,028H,0CH,01CH,0CH,023H,0CH,02AH,0CH
DB 0F8H,00H,00H,0F5H,02CH,0F9H,014H,018H,090H,01FH,030H,01AH,090H,01BH,060H,014H
DB 090H,01FH,090H,01CH,030H,015H,090H,013H,030H,0FBH,0FFH,011H,0C0H,0FFH,00H,011H
DB 0C0H,0FFH,0FFH,00H,00H,00H
title_theme_part8 LABEL BYTE
DB 0FCH,038H,0F5H,02EH,0F9H,01EH,0FBH,06H,0F4H,00H,0FFH,00H,00H,0C0H,0FFH,07H
DB 00H,048H,0FFH,00H,07H,0CH,0FFH,01H,09H,030H,0FFH,06H,09H,018H,07H,0CH
DB 07H,0CH,0FFH,00H,09H,030H,0FFH,03H,00H,060H,0FFH,01H,09H,030H,0FFH,02H
DB 09H,018H,07H,0CH,07H,0CH,0FFH,00H,09H,030H,0FFH,03H,00H,060H,0FFH,01H
DB 09H,030H,0FFH,06H,07H,024H,09H,024H,07H,024H,09H,024H,07H,024H,0F5H,040H
DB 00H,030H,0CH,0CH,0CH,0CH,0CH,0CH,0CH,0CH,0CH,018H,0CH,018H,07H,030H
DB 0EH,030H,00H,060H,0FH,030H,00H,030H,08H,0CH,08H,0CH,08H,0CH,08H,0CH
DB 08H,018H,08H,018H,08H,030H,0F4H,07FH,07H,078H,0FCH,015H,0F5H,029H,0F8H,025H
DB 0FFH,02AH,0CH,02FH,0CH,036H,0CH,02BH,0CH,032H,0CH,039H,0CH,02DH,0CH,034H
DB 0CH,03BH,0CH,032H,0CH,039H,0CH,040H,0CH,00H,048H,0FCH,038H,04H,030H,0FCH
DB 01H,0F5H,01AH,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH,034H
DB 0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,00H,060H,0FCH,015H,0F5H
DB 024H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH,034H,0CH,039H
DB 0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH,00H,060H,0FCH,013H,02FH,0CH,034H
DB 0CH,03BH,0CH,032H,0CH,037H,0CH,03EH,0CH,034H,0CH,039H,0CH,00H,060H,0FCH
DB 0FH,0F8H,00H,00H,0F5H,01FH,0FFH,00H,02FH,0CH,034H,0CH,03BH,0CH,032H,0CH
DB 037H,0CH,03EH,0CH,034H,0CH,039H,0CH,040H,0CH,037H,0CH,03EH,0CH,045H,0CH
DB 00H,090H,0FFH,0FFH,00H,00H
celebration_part1 LABEL BYTE
DB 0FCH,012H,0F4H,07FH,0F5H,028H,0F9H,02DH,03BH,01EH,03BH,01EH,03CH,01EH,03EH,01EH
DB 03EH,01EH,03CH,01EH,03BH,01EH,039H,01EH,037H,01EH,037H,01EH,039H,01EH,03BH,01EH
DB 03BH,032H,039H,0AH,039H,028H,00H,014H,03BH,01EH,03BH,01EH,03CH,01EH,03EH,01EH
DB 03EH,01EH,03CH,01EH,03BH,01EH,039H,01EH,037H,01EH,037H,01EH,039H,01EH,03BH,01EH
DB 039H,032H,037H,0AH,0FEH,00H,037H,028H,00H,014H,039H,01EH,039H,01EH,03BH,01EH
DB 037H,01EH,039H,01EH,03BH,014H,03CH,0AH,03BH,01EH,037H,01EH,039H,01EH,03BH,014H
DB 03CH,0AH,03BH,01EH,039H,01EH,037H,01EH,039H,01EH,032H,01EH,03BH,03CH,03BH,01EH
DB 03CH,01EH,03EH,01EH,03EH,01EH,03CH,01EH,03BH,01EH,03CH,014H,039H,0AH,037H,01EH
DB 037H,01EH,039H,01EH,03BH,01EH,039H,032H,037H,0AH,0FEH,01H,037H,014H,00H,00H
celebration_part2 LABEL BYTE
DB 0FCH,012H,0F4H,07FH,0F5H,028H,0F9H,02AH,037H,01EH,037H,01EH,039H,01EH,03BH,01EH
DB 03BH,01EH,039H,01EH,032H,01EH,030H,01EH,02FH,01EH,02FH,01EH,032H,01EH,037H,01EH
DB 037H,032H,036H,0AH,036H,028H,00H,014H,037H,01EH,037H,01EH,037H,01EH,037H,01EH
DB 037H,01EH,037H,01EH,037H,01EH,037H,01EH,02FH,01EH,02FH,01EH,036H,01EH,037H,01EH
DB 036H,032H,032H,0AH,0FEH,00H,032H,028H,00H,014H,036H,01EH,036H,01EH,037H,01EH
DB 032H,01EH,036H,01EH,037H,014H,039H,0AH,037H,01EH,032H,01EH,036H,01EH,037H,014H
DB 039H,0AH,036H,01EH,036H,01EH,034H,01EH,031H,01EH,032H,01EH,037H,03CH,037H,01EH
DB 037H,01EH,037H,01EH,037H,01EH,037H,01EH,037H,01EH,030H,014H,030H,0AH,02FH,01EH
DB 02FH,01EH,032H,01EH,037H,01EH,036H,032H,02FH,0AH,0FEH,01H,02FH,014H,00H,00H
celebration_part3 LABEL BYTE
DB 0FCH,07H,0F5H,028H,0F9H,014H,0F4H,00H,0FBH,05H,037H,0AH,0FCH,013H,043H,0AH
DB 042H,0AH,040H,0AH,03EH,0AH,03CH,0AH,03BH,0AH,039H,0AH,037H,0AH,036H,0AH
DB 034H,0AH,032H,0AH,0FCH,02H,026H,0AH,0FCH,015H,032H,0AH,034H,0AH,036H,0AH
DB 037H,0AH,039H,0AH,03BH,0AH,03CH,0AH,03EH,0AH,040H,0AH,03EH,0AH,042H,0AH
DB 0FCH,02H,043H,0AH,0FCH,013H,03EH,0AH,03CH,0AH,03BH,0AH,039H,0AH,037H,0AH
DB 036H,0AH,037H,0AH,034H,0AH,032H,0AH,034H,0AH,031H,0AH,0FCH,02H,026H,0AH
DB 0FCH,015H,032H,0AH,034H,0AH,036H,0AH,037H,0AH,039H,0AH,03BH,0AH,03CH,0AH
DB 03EH,0AH,040H,0AH,03EH,0AH,042H,0AH,0FCH,02H,043H,0AH,0FCH,013H,040H,0AH
DB 03EH,0AH,03CH,0AH,03BH,0AH,039H,0AH,037H,0AH,039H,0AH,037H,0AH,035H,0AH
DB 034H,0AH,032H,0AH,0FCH,02H,024H,0AH,0FCH,015H,032H,0AH,034H,0AH,035H,0AH
DB 037H,0AH,039H,0AH,03BH,0AH,03CH,0AH,03EH,0AH,040H,0AH,0FCH,02H,030H,0AH
DB 030H,0AH,032H,0AH,0FCH,013H,03CH,0AH,03BH,0AH,039H,0AH,037H,0AH,036H,0AH
DB 034H,0AH,032H,0AH,037H,0AH,032H,0AH,02FH,0AH,02BH,0AH,028H,0AH,026H,0AH
DB 025H,0AH,026H,0AH,0FCH,02H,032H,0AH,032H,0AH,0FEH,00H,02BH,0AH,0FCH,019H
DB 032H,0AH,031H,0AH,032H,0AH,02FH,0AH,02BH,0AH,0FCH,02H,032H,0AH,0FCH,013H
DB 03EH,0AH,03DH,0AH,03EH,0AH,039H,0AH,036H,0AH,0FCH,02H,032H,0AH,0FCH,05H
DB 03EH,0AH,03DH,0AH,03EH,0AH,03BH,0AH,037H,0AH,0FCH,02H,032H,0AH,0FCH,013H
DB 03EH,0AH,040H,0AH,03EH,0AH,0FCH,02H,032H,0AH,032H,0AH,032H,0AH,0FCH,05H
DB 03EH,0AH,03DH,0AH,03EH,0AH,03BH,0AH,037H,0AH,0FCH,02H,032H,0AH,0FCH,013H
DB 03EH,0AH,040H,0AH,03EH,0AH,032H,0AH,032H,0AH,0FCH,02H,027H,0AH,0FCH,05H
DB 033H,0AH,033H,0AH,0FCH,02H,02FH,0AH,0FCH,05H,03BH,0AH,03BH,0AH,0FCH,02H
DB 028H,0AH,0FCH,05H,034H,0AH,034H,0AH,0FCH,02H,02DH,0AH,0FCH,05H,039H,0AH
DB 039H,0AH,0FCH,02H,032H,0AH,0FCH,05H,03EH,0AH,03EH,0AH,0FCH,02H,026H,0AH
DB 0FCH,05H,032H,0AH,032H,0AH,0FCH,02H,02BH,0AH,0FCH,013H,043H,0AH,041H,0AH
DB 040H,0AH,03EH,0AH,03CH,0AH,03BH,0AH,039H,0AH,037H,0AH,035H,0AH,034H,0AH
DB 032H,0AH,0FCH,02H,024H,0AH,0FCH,015H,032H,0AH,034H,0AH,035H,0AH,037H,0AH
DB 039H,0AH,03BH,0AH,03CH,0AH,03EH,0AH,040H,0AH,0FCH,02H,030H,0AH,030H,0AH
DB 026H,0AH,0FCH,013H,03CH,0AH,03BH,0AH,039H,0AH,037H,0AH,036H,0AH,034H,0AH
DB 032H,0AH,037H,0AH,032H,0AH,02FH,0AH,02BH,0AH,028H,0AH,026H,0AH,025H,0AH
DB 026H,0AH,0FCH,02H,032H,0AH,032H,0AH,0FEH,01H,0FCH,019H,02BH,014H,00H,00H
celebration_part4 LABEL BYTE
DB 0FCH,07H,0F4H,07FH,0F5H,028H,0F9H,02DH,0FBH,05H,02BH,0AH,00H,06EH,026H,0AH
DB 00H,06EH,037H,0AH,00H,06EH,026H,0AH,00H,06EH,037H,0AH,00H,06EH,024H,0AH
DB 00H,05AH,02BH,0AH,024H,0AH,026H,0AH,00H,096H,026H,0AH,026H,0AH,0FEH,00H
DB 01FH,0AH,00H,032H,026H,0AH,00H,032H,026H,0AH,00H,032H,026H,0AH,00H,01EH
DB 026H,0AH,026H,0AH,026H,0AH,00H,032H,026H,0AH,00H,01EH,026H,0AH,026H,0AH
DB 02FH,0AH,00H,014H,02DH,0AH,00H,014H,02BH,0AH,00H,014H,02DH,0AH,00H,014H
DB 02AH,0AH,00H,014H,032H,0AH,00H,08CH,024H,0AH,00H,05AH,024H,0AH,024H,0AH
DB 026H,0AH,00H,06EH,00H,028H,026H,0AH,026H,0AH,0FEH,01H,02BH,014H,00H,00H
celebration_part5 LABEL BYTE
DB 0FCH,07H,0F4H,07FH,0F5H,028H,0F9H,02DH,0FBH,05H,02FH,0AH,00H,06EH,02DH,0AH
DB 00H,06EH,02FH,0AH,00H,06EH,02DH,0AH,00H,06EH,02FH,0AH,00H,06EH,02BH,0AH
DB 00H,05AH,024H,0AH,02BH,0AH,02FH,0AH,00H,096H,02AH,0AH,02AH,0AH,0FEH,00H
DB 02BH,0AH,00H,032H,02DH,0AH,00H,032H,02FH,0AH,00H,032H,02DH,0AH,00H,01EH
DB 02DH,0AH,02DH,0AH,02FH,0AH,00H,032H,02DH,0AH,00H,01EH,02DH,0AH,02DH,0AH
DB 027H,0AH,00H,014H,023H,0AH,00H,014H,028H,0AH,00H,014H,021H,0AH,00H,014H
DB 026H,0AH,00H,014H,026H,0AH,00H,08CH,02BH,0AH,00H,05AH,02BH,0AH,02BH,0AH
DB 02DH,0AH,00H,06EH,00H,028H,02AH,0AH,02AH,0AH,0FEH,01H,01FH,014H,00H,00H
celebration_part6 LABEL BYTE
DB 0FCH,07H,0F4H,00H,0F5H,028H,0F9H,014H,0FBH,05H,02BH,0AH,0FCH,013H,037H,0AH
DB 036H,0AH,034H,0AH,032H,0AH,030H,0AH,02FH,0AH,02DH,0AH,02BH,0AH,02AH,0AH
DB 028H,0AH,026H,0AH,0FCH,07H,026H,0AH,0FCH,015H,032H,0AH,028H,0AH,02AH,0AH
DB 02BH,0AH,02DH,0AH,02FH,0AH,030H,0AH,032H,0AH,034H,0AH,032H,0AH,036H,0AH
DB 0FCH,07H,037H,0AH,0FCH,013H,032H,0AH,030H,0AH,02FH,0AH,02DH,0AH,02BH,0AH
DB 02AH,0AH,02BH,0AH,028H,0AH,026H,0AH,028H,0AH,025H,0AH,0FCH,07H,026H,0AH
DB 0FCH,015H,032H,0AH,028H,0AH,02AH,0AH,02BH,0AH,02DH,0AH,02FH,0AH,030H,0AH
DB 026H,0AH,028H,0AH,026H,0AH,02AH,0AH,0FCH,07H,02BH,0AH,0FCH,013H,028H,0AH
DB 026H,0AH,024H,0AH,023H,0AH,021H,0AH,01FH,0AH,02DH,0AH,02BH,0AH,029H,0AH
DB 028H,0AH,026H,0AH,0FCH,07H,024H,0AH,0FCH,015H,032H,0AH,034H,0AH,035H,0AH
DB 037H,0AH,039H,0AH,02FH,0AH,030H,0AH,032H,0AH,034H,0AH,0FCH,07H,024H,0AH
DB 024H,0AH,026H,0AH,0FCH,013H,030H,0AH,02FH,0AH,039H,0AH,037H,0AH,036H,0AH
DB 034H,0AH,032H,0AH,037H,0AH,032H,0AH,02FH,0AH,02BH,0AH,028H,0AH,026H,0AH
DB 025H,0AH,026H,0AH,0FCH,07H,032H,0AH,032H,0AH,0FEH,00H,02BH,0AH,0FCH,019H
DB 032H,0AH,031H,0AH,032H,0AH,02FH,0AH,02BH,0AH,0FCH,07H,026H,0AH,0FCH,015H
DB 032H,0AH,031H,0AH,032H,0AH,02DH,0AH,02AH,0AH,0FCH,07H,026H,0AH,0FCH,019H
DB 032H,0AH,031H,0AH,032H,0AH,02FH,0AH,02BH,0AH,0FCH,07H,026H,0AH,0FCH,015H
DB 032H,0AH,034H,0AH,032H,0AH,0FCH,07H,026H,0AH,026H,0AH,026H,0AH,0FCH,019H
DB 032H,0AH,031H,0AH,032H,0AH,02FH,0AH,02BH,0AH,0FCH,07H,026H,0AH,0FCH,015H
DB 032H,0AH,034H,0AH,032H,0AH,026H,0AH,026H,0AH,0FCH,07H,027H,0AH,0FCH,019H
DB 033H,0AH,033H,0AH,0FCH,07H,023H,0AH,0FCH,019H,02FH,0AH,02FH,0AH,0FCH,07H
DB 028H,0AH,0FCH,019H,034H,0AH,034H,0AH,0FCH,07H,021H,0AH,0FCH,019H,02DH,0AH
DB 02DH,0AH,0FCH,07H,026H,0AH,0FCH,019H,032H,0AH,032H,0AH,0FCH,07H,026H,0AH
DB 0FCH,019H,032H,0AH,032H,0AH,0FCH,07H,01FH,0AH,0FCH,013H,037H,0AH,035H,0AH
DB 034H,0AH,032H,0AH,030H,0AH,02FH,0AH,02DH,0AH,02BH,0AH,029H,0AH,028H,0AH
DB 026H,0AH,0FCH,07H,024H,0AH,0FCH,015H,032H,0AH,034H,0AH,035H,0AH,037H,0AH
DB 039H,0AH,02FH,0AH,030H,0AH,032H,0AH,034H,0AH,0FCH,07H,024H,0AH,024H,0AH
DB 026H,0AH,0FCH,013H,030H,0AH,02FH,0AH,039H,0AH,037H,0AH,036H,0AH,034H,0AH
DB 032H,0AH,037H,0AH,032H,0AH,02FH,0AH,02BH,0AH,028H,0AH,026H,0AH,025H,0AH
DB 026H,0AH,0FCH,07H,032H,0AH,032H,0AH,0FEH,01H,0FCH,019H,02BH,014H,00H,00H
celebration_part7 LABEL BYTE
DB 0FCH,0FH,0F5H,028H,0F9H,02DH,0F4H,07FH,0FBH,05H,01FH,0AH,00H,050H,01EH,0AH
DB 01CH,0AH,01AH,0AH,01AH,05H,00H,055H,028H,0AH,026H,0AH,02AH,0AH,02BH,05H
DB 00H,04BH,01CH,0AH,01AH,0AH,01CH,0AH,019H,0AH,01AH,05H,00H,04BH,026H,0AH
DB 028H,0AH,026H,0AH,02AH,0AH,02BH,05H,00H,055H,01DH,0AH,01CH,0AH,01AH,0AH
DB 018H,05H,00H,05FH,018H,0AH,018H,0AH,01AH,05H,00H,09BH,01AH,0AH,01AH,0AH
DB 0FEH,00H,01FH,05H,00H,019H,026H,0AH,023H,0AH,01FH,0AH,01AH,05H,00H,019H
DB 026H,0AH,021H,0AH,01EH,0AH,0FFH,00H,01AH,05H,00H,019H,026H,0AH,023H,0AH
DB 01FH,0AH,01AH,05H,00H,023H,01AH,0AH,01AH,0AH,0FFH,01H,01BH,0AH,027H,0AH
DB 027H,0AH,017H,0AH,023H,0AH,023H,0AH,01CH,0AH,028H,0AH,028H,0AH,015H,0AH
DB 021H,0AH,021H,0AH,01AH,0AH,026H,0AH,026H,0AH,01AH,0AH,026H,0AH,026H,0AH
DB 01FH,0AH,02BH,0AH,029H,0AH,028H,0AH,026H,0AH,024H,0AH,023H,0AH,021H,0AH
DB 01FH,0AH,01DH,0AH,01CH,0AH,01AH,0AH,018H,0AH,026H,0AH,028H,0AH,029H,0AH
DB 02BH,0AH,02DH,0AH,023H,0AH,024H,0AH,026H,0AH,028H,0AH,018H,0AH,018H,0AH
DB 01AH,0AH,030H,0AH,02FH,0AH,02DH,0AH,02BH,0AH,02AH,0AH,028H,0AH,026H,0AH
DB 02BH,0AH,026H,0AH,023H,0AH,01FH,0AH,01CH,0AH,01AH,0AH,019H,0AH,01AH,0AH
DB 026H,0AH,026H,0AH,0FEH,01H,01FH,014H,00H,00H
celebration_part8 LABEL BYTE
DB 0FCH,038H,0F4H,00H,0F5H,028H,0F9H,037H,0FFH,00H,07H,078H,02H,078H,0FFH,01H
DB 07H,078H,00H,064H,00H,0AH,00H,0AH,02H,0A0H,02H,0AH,02H,0AH,0FEH,00H
DB 07H,03CH,02H,03CH,02H,01EH,02H,01EH,02H,01EH,02H,014H,02H,0AH,02H,01EH
DB 02H,01EH,02H,01EH,02H,014H,02H,0AH,03H,01EH,0BH,01EH,04H,01EH,09H,01EH
DB 02H,01EH,02H,096H,00H,064H,00H,0AH,00H,0AH,02H,0A0H,02H,0AH,02H,0AH
DB 0FEH,01H,07H,014H,00H,00H
english_anthem_part1 LABEL BYTE
DB 0FCH,08H,0F5H,01EH,0F9H,028H,0F4H,07FH,0FBH,0AH,03EH,0AH,03CH,0AH,03AH,014H
DB 035H,014H,03AH,014H,03CH,014H,03EH,0AH,03FH,0AH,03EH,0AH,03CH,0AH,03AH,0AH
DB 03CH,0AH,03EH,0AH,03FH,0AH,041H,014H,0FBH,05H,041H,0AH,041H,0AH,041H,014H
DB 043H,014H,0FBH,0FFH,041H,04H,043H,03H,041H,017H,0FBH,0AH,03FH,0AH,03EH,014H
DB 03EH,0AH,03FH,0AH,041H,014H,046H,014H,041H,014H,03FH,014H,03EH,014H,03CH,0AH
DB 03EH,0AH,03FH,0AH,041H,0AH,03EH,0AH,03FH,0AH,03CH,014H,0FBH,05H,035H,0AH
DB 035H,0AH,035H,014H,035H,014H,035H,03CH,0FBH,0AH,03EH,0AH,03CH,0AH,03AH,014H
DB 035H,014H,03AH,014H,03CH,014H,03EH,0AH,03FH,0AH,03EH,0AH,03CH,0AH,03AH,0AH
DB 03CH,0AH,03EH,0AH,03FH,0AH,041H,014H,0FBH,05H,041H,0AH,041H,0AH,041H,014H
DB 043H,014H,0FBH,0FFH,041H,04H,043H,03H,041H,017H,0FBH,0AH,03FH,0AH,03EH,014H
DB 03EH,0AH,03FH,0AH,041H,014H,046H,014H,041H,014H,03FH,014H,03EH,014H,03CH,0AH
DB 03EH,0AH,03FH,0AH,041H,0AH,03EH,0AH,03FH,0AH,03CH,0AH,041H,0AH,03FH,0AH
DB 03EH,0AH,03CH,01EH,03AH,05H,00H,05H,03AH,03CH,0FCH,05H,039H,0AH,03AH,0AH
DB 03CH,014H,0FBH,05H,03CH,0AH,03CH,0AH,03CH,014H,03CH,014H,039H,014H,035H,014H
DB 0FBH,0AH,03AH,0AH,039H,0AH,03AH,0AH,03CH,0AH,03EH,014H,03CH,0AH,03EH,0AH
DB 03FH,0AH,03EH,0AH,03CH,0AH,03AH,0AH,03CH,03CH,039H,0AH,03AH,0AH,03CH,014H
DB 0FBH,05H,03CH,0AH,03CH,0AH,03CH,014H,03CH,014H,039H,014H,035H,014H,0FBH,0AH
DB 03AH,0AH,039H,0AH,03AH,0AH,03CH,0AH,03EH,014H,03CH,0AH,03EH,0AH,03FH,0AH
DB 03EH,0AH,03CH,0AH,03AH,0AH,041H,03CH,0FCH,03H,03EH,0AH,03CH,0AH,03AH,014H
DB 035H,014H,03AH,014H,03CH,014H,03EH,0AH,03FH,0AH,03EH,0AH,03CH,0AH,03AH,0AH
DB 03CH,0AH,03EH,0AH,03FH,0AH,041H,014H,0FBH,05H,041H,0AH,041H,0AH,0FBH,0AH
DB 041H,014H,043H,014H,0FBH,0FFH,041H,04H,043H,03H,041H,017H,03FH,0AH,0FBH,0AH
DB 03EH,014H,03EH,0AH,03FH,0AH,041H,014H,046H,014H,041H,014H,03FH,015H,0FBH,0FFH
DB 03EH,015H,03CH,0BH,03EH,0BH,03FH,0CH,041H,0CH,03EH,0CH,03FH,0DH,03CH,0DH
DB 041H,0DH,03FH,0EH,03EH,06H,03CH,04H,03EH,04H,03CH,02BH,03AH,08H,00H,08H
DB 03AH,060H,00H,00H
english_anthem_part2 LABEL BYTE
DB 0FCH,08H,0F5H,01EH,0F9H,028H,0F4H,07FH,0FBH,0AH,035H,0AH,033H,0AH,032H,014H
DB 032H,014H,035H,014H,039H,014H,03AH,0AH,03CH,0AH,03AH,0AH,039H,0AH,037H,0AH
DB 035H,0AH,033H,0AH,030H,0AH,032H,014H,0FBH,05H,032H,0AH,032H,0AH,032H,014H
DB 033H,014H,0FBH,0FFH,032H,01EH,030H,0AH,0FBH,0AH,02EH,014H,02EH,0AH,030H,0AH
DB 02EH,014H,032H,014H,035H,014H,039H,014H,03AH,014H,039H,0AH,03AH,0AH,03CH,0AH
DB 03EH,0AH,03AH,0AH,03CH,0AH,035H,014H,0FBH,05H,035H,0AH,035H,0AH,035H,014H
DB 035H,014H,035H,03CH,030H,014H,0FBH,0AH,032H,014H,032H,014H,035H,014H,039H,014H
DB 03AH,0AH,03CH,0AH,03AH,0AH,039H,0AH,037H,0AH,035H,0AH,033H,0AH,030H,0AH
DB 032H,014H,0FBH,05H,032H,0AH,032H,0AH,032H,014H,033H,014H,0FBH,0FFH,032H,04H
DB 033H,03H,032H,017H,0FBH,0AH,030H,0AH,02EH,014H,02EH,0AH,030H,0AH,02EH,014H
DB 032H,014H,035H,014H,039H,014H,03AH,014H,039H,0AH,03AH,0AH,037H,0AH,039H,0AH
DB 035H,0AH,037H,0AH,033H,0AH,035H,0AH,033H,0AH,032H,0AH,033H,01EH,032H,0AH
DB 032H,028H,0FCH,0BH,00H,028H,030H,014H,0FBH,05H,030H,0AH,030H,0AH,030H,014H
DB 030H,014H,030H,014H,030H,014H,030H,014H,030H,014H,0FBH,0AH,035H,014H,033H,0AH
DB 035H,0AH,037H,0AH,035H,0AH,033H,0AH,032H,0AH,030H,050H,0FCH,0BH,030H,014H
DB 0FBH,05H,030H,0AH,030H,0AH,030H,014H,030H,014H,030H,014H,030H,014H,035H,014H
DB 0FCH,08H,035H,014H,0FBH,0AH,03AH,014H,039H,0AH,03AH,0AH,03CH,0AH,03AH,0AH
DB 039H,0AH,037H,0AH,035H,03CH,0FCH,03H,032H,0AH,030H,0AH,02EH,014H,035H,014H
DB 03AH,014H,039H,014H,03AH,0AH,03CH,0AH,03AH,0AH,039H,0AH,037H,0AH,035H,0AH
DB 033H,0AH,030H,0AH,032H,014H,0FBH,05H,032H,0AH,032H,0AH,032H,014H,033H,014H
DB 0FBH,0FFH,032H,04H,033H,03H,032H,017H,030H,0AH,0FBH,0AH,02EH,014H,02EH,0AH
DB 030H,0AH,02EH,014H,032H,014H,035H,014H,039H,015H,0FBH,0FFH,03AH,015H,039H,0BH
DB 03AH,0BH,037H,0CH,039H,0CH,035H,0CH,037H,0DH,033H,0DH,035H,0DH,033H,0EH
DB 032H,0EH,033H,02BH,032H,08H,00H,08H,032H,060H,00H,00H
english_anthem_part4 LABEL BYTE
DB 0FCH,07H,0F5H,028H,0F9H,028H,0F4H,040H,00H,014H,0FBH,06H,026H,028H,026H,028H
DB 029H,050H,029H,050H,022H,050H,026H,028H,026H,028H,026H,050H,024H,050H,021H,050H
DB 026H,028H,026H,028H,029H,050H,029H,050H,022H,050H,026H,028H,026H,028H,022H,050H
DB 01DH,050H,022H,028H,0F9H,01FH,00H,028H,00H,050H,029H,050H,0FCH,01AH,022H,014H
DB 022H,0AH,022H,0AH,022H,014H,022H,014H,029H,050H,0FCH,07H,01DH,050H,029H,014H
DB 029H,0AH,029H,0AH,029H,014H,029H,014H,0F9H,027H,0FCH,019H,029H,014H,029H,0AH
DB 029H,0AH,029H,014H,029H,014H,01DH,050H,0FCH,07H,02EH,028H,02EH,028H,02EH,050H
DB 029H,050H,029H,050H,02EH,028H,02EH,029H,02EH,064H,029H,071H,02EH,060H,00H,00H
english_anthem_part5 LABEL BYTE
DB 0FCH,07H,0F5H,032H,0F9H,028H,0F4H,00H,00H,014H,0FBH,0BH,022H,014H,021H,014H
DB 01FH,014H,01DH,014H,022H,014H,024H,014H,026H,014H,027H,014H,029H,014H,027H,014H
DB 026H,014H,024H,014H,022H,014H,021H,014H,01DH,014H,01BH,014H,01AH,014H,01DH,014H
DB 022H,014H,024H,014H,026H,014H,022H,014H,01DH,014H,022H,014H,01DH,014H,018H,014H
DB 01DH,014H,024H,014H,018H,014H,01DH,014H,029H,0AH,027H,0AH,026H,0AH,024H,0AH
DB 0FBH,0BH,022H,014H,021H,014H,01FH,014H,01DH,014H,022H,014H,024H,014H,026H,014H
DB 027H,014H,01DH,014H,027H,014H,026H,014H,024H,014H,022H,014H,021H,014H,01DH,014H
DB 01BH,014H,01AH,014H,01DH,014H,022H,014H,024H,014H,026H,014H,022H,014H,01DH,014H
DB 022H,014H,01DH,014H,024H,014H,029H,014H,01DH,014H,022H,03CH,00H,014H,01DH,050H
DB 01DH,014H,0FBH,06H,01DH,0AH,01DH,0AH,01DH,014H,01DH,014H,022H,050H,01DH,050H
DB 01DH,050H,01DH,014H,01DH,0AH,01DH,0AH,01DH,014H,011H,014H,022H,014H,022H,0AH
DB 022H,0AH,022H,014H,022H,014H,01DH,014H,01DH,0AH,01DH,0AH,01DH,014H,01DH,014H
DB 0FBH,0BH,022H,014H,021H,014H,01FH,014H,01DH,014H,022H,014H,024H,014H,026H,014H
DB 027H,014H,029H,014H,027H,014H,026H,014H,024H,014H,022H,014H,021H,014H,01DH,014H
DB 01BH,014H,01AH,014H,01DH,014H,022H,014H,024H,015H,026H,015H,022H,016H,01DH,018H
DB 022H,019H,01DH,01AH,024H,01CH,029H,01CH,01DH,01FH,022H,060H,00H,00H
english_anthem_part3 LABEL BYTE
DB 0FCH,038H,0F5H,037H,0F9H,032H,0F4H,00H,00H,014H,00H,0F0H,00H,0F0H,05H,014H
DB 05H,0AH,05H,0AH,05H,014H,05H,014H,05H,014H,05H,0AH,05H,0AH,05H,014H
DB 05H,014H,00H,0F0H,00H,0F0H,05H,014H,05H,0AH,05H,0AH,05H,014H,05H,014H
DB 0AH,050H,05H,050H,05H,050H,0AH,050H,05H,014H,05H,0AH,05H,0AH,05H,014H
DB 05H,014H,05H,050H,05H,050H,0AH,050H,05H,014H,05H,0AH,05H,0AH,05H,014H
DB 05H,014H,0AH,028H,05H,028H,0AH,028H,0AH,028H,05H,014H,05H,0AH,05H,0AH
DB 05H,014H,05H,014H,0AH,050H,0AH,028H,05H,029H,0AH,02BH,05H,031H,05H,01AH
DB 05H,0EH,05H,0EH,0F9H,03CH,05H,01CH,05H,01FH,0AH,060H,00H,00H
english_motif_part1 LABEL BYTE
DB 0FCH,018H,0F5H,032H,0F9H,032H,0F4H,07FH,0FBH,0AH,039H,0AH,037H,0AH,035H,014H
DB 030H,014H,035H,014H,037H,014H,039H,05AH,00H,00H
english_motif_part2 LABEL BYTE
DB 0FCH,018H,0F5H,032H,00H,014H,0F9H,032H,0F4H,07FH,0FBH,05H,029H,014H,029H,0AH
DB 029H,0AH,029H,014H,029H,014H,029H,05AH,00H,00H
english_motif_part3 LABEL BYTE
DB 0FCH,018H,0F5H,032H,0F9H,032H,0F4H,00H,00H,014H,029H,050H,01DH,05AH,00H,00H
english_motif_part4 LABEL BYTE
DB 0FCH,01BH,0F5H,032H,0F9H,032H,0F4H,040H,00H,014H,0FBH,0AH,01DH,014H,018H,014H
DB 011H,014H,0CH,014H,011H,05AH,00H,00H
english_motif_part5 LABEL BYTE
DB 0FCH,038H,0F5H,03CH,0F9H,032H,0F4H,00H,00H,014H,0FBH,05H,05H,014H,05H,0AH
DB 05H,0AH,05H,014H,05H,014H,0FFH,00H,05H,0AH,0FFH,07H,05H,014H,00H,00H
french_anthem_part1 LABEL BYTE
DB 0FCH,05H,0F4H,00H,0FEH,00H,0F5H,05H,0F9H,05AH,0F8H,012H,01H,0FBH,04H,02EH
DB 018H,02EH,08H,033H,020H,033H,020H,035H,020H,035H,020H,0F8H,012H,0FFH,03AH,038H
DB 037H,08H,033H,018H,033H,08H,037H,018H,033H,08H,030H,020H,038H,040H,035H,018H
DB 032H,08H,033H,058H,02EH,08H,0FFH,01H,0F8H,00H,00H,0F9H,050H,0F8H,014H,01H
DB 033H,018H,035H,08H,037H,020H,037H,018H,037H,08H,037H,020H,038H,018H,037H,08H
DB 037H,010H,035H,010H,035H,040H,035H,018H,037H,08H,038H,020H,038H,018H,038H,08H
DB 038H,020H,03AH,018H,038H,08H,037H,058H,0F8H,00H,00H,03AH,08H,03AH,018H,03AH
DB 08H,0FFH,00H,03AH,020H,037H,018H,033H,08H,0FFH,01H,02EH,058H,0F9H,03CH,02EH
DB 08H,02EH,018H,032H,08H,035H,020H,035H,018H,035H,08H,038H,020H,035H,018H,032H
DB 08H,035H,010H,033H,010H,033H,040H,0FEH,04H,033H,010H,00H
sound_reset_unused_part1 LABEL BYTE
DB 00H,00H
french_anthem_part2 LABEL BYTE
DB 0FCH,05H,0F4H,07FH,0FEH,00H,0F5H,05H,0F9H,05AH,0F8H,012H,01H,0FBH,04H,029H
DB 018H,029H,08H,02EH,020H,02EH,020H,032H,020H,032H,020H,0F8H,012H,0FFH,033H,038H
DB 033H,08H,030H,018H,030H,08H,030H,018H,030H,08H,02CH,020H,030H,040H,02EH,018H
DB 02EH,08H,02EH,058H,02EH,08H,0FFH,01H,0F5H,0AH,0F8H,014H,01H,00H,03H,033H
DB 018H,035H,08H,037H,020H,037H,018H,037H,08H,037H,020H,038H,018H,037H,08H,037H
DB 010H,035H,010H,035H,040H,035H,018H,037H,08H,038H,020H,038H,018H,038H,08H,038H
DB 020H,03AH,018H,038H,08H,037H,058H,0F8H,00H,00H,03AH,08H,03AH,018H,03AH,08H
DB 0FFH,00H,03AH,020H,037H,018H,033H,08H,0FFH,01H,02EH,058H,0F9H,03CH,02EH,08H
DB 02EH,018H,032H,08H,035H,020H,035H,018H,035H,08H,038H,020H,035H,018H,032H,08H
DB 035H,010H,033H,010H,033H,03DH,0FEH,04H,033H,010H,00H,00H
french_anthem_part3 LABEL BYTE
DB 0FCH,05H,0F4H,00H,0FBH,04H,02EH,018H,02EH,08H,0FEH,00H,0F5H,0FH,0F9H,046H
DB 0FCH,05H,0F8H,0CH,01H,033H,020H,032H,020H,030H,020H,02EH,020H,0F8H,010H,0FFH
DB 030H,020H,02EH,020H,02CH,020H,02BH,020H,02CH,020H,0F8H,013H,01H,02CH,040H,02CH
DB 018H,029H,08H,027H,060H,0F5H,0AH,0F8H,00H,00H,00H,023H,033H,020H,032H,020H
DB 030H,020H,02EH,020H,02CH,020H,02BH,020H,029H,020H,027H,020H,02CH,020H,02CH,040H
DB 02CH,015H,02CH,08H,02BH,060H,00H,020H,0FBH,00H,0F9H,032H,0F5H,0FH,0FCH,02H
DB 0F8H,06H,01H,02BH,010H,02CH,010H,02EH,010H,030H,010H,0F8H,06H,0FFH,032H,010H
DB 033H,010H,032H,010H,02EH,010H,0F8H,06H,01H,02EH,010H,030H,010H,032H,010H,030H
DB 010H,0F8H,06H,0FFH,02EH,010H,02CH,010H,02BH,010H,02CH,010H,0F8H,06H,01H,02EH
DB 010H,030H,010H,032H,010H,033H,010H,0F8H,06H,0FFH,035H,010H,033H,010H,032H,010H
DB 035H,010H,0F8H,06H,01H,033H,010H,035H,010H,037H,010H,035H,010H,033H,018H,0F8H
DB 00H,00H,0F9H,05AH,0FBH,04H,02EH,08H,02EH,018H,02EH,08H,0FFH,00H,03AH,020H
DB 037H,018H,033H,08H,0FFH,01H,029H,060H,0F9H,01EH,02EH,020H,0F8H,06H,01H,030H
DB 010H,032H,010H,033H,010H,032H,010H,0F8H,06H,0FFH,030H,010H,02EH,010H,02CH,010H
DB 02EH,010H,0F8H,06H,01H,02BH,040H,00H,040H,0FEH,04H,00H,00H,00H
french_anthem_part4 LABEL BYTE
DB 0F4H,07FH,0FBH,04H,00H,020H,0FEH,00H,0F5H,0AH,0F9H,05AH,0FCH,05H,00H,080H
DB 00H,080H,00H,080H,02BH,060H,00H,020H,027H,020H,026H,020H,024H,020H,022H,020H
DB 020H,020H,01FH,020H,01DH,020H,01BH,020H,020H,020H,020H,040H,020H,018H,01DH,08H
DB 01BH,060H,00H,020H,00H,080H,00H,080H,00H,080H,00H,058H,0FCH,027H,035H,08H
DB 035H,018H,035H,08H,0FFH,00H,037H,020H,033H,018H,02EH,08H,0FFH,01H,02CH,060H
DB 00H,020H,00H,080H,00H,080H,0FEH,04H,00H,00H
french_anthem_part8 LABEL BYTE
DB 0F4H,00H,00H,020H,0FEH,00H,0FCH,01AH,0F5H,05H,0F9H,050H,0F8H,0CH,0FFH,027H
DB 080H,0F5H,05H,02BH,080H,0F5H,05H,029H,060H,0FBH,04H,02CH,018H,029H,08H,0F8H
DB 02H,01H,0FBH,08H,027H,014H,0FBH,04H,027H,06H,027H,06H,0FBH,08H,027H,010H
DB 027H,010H,027H,010H,027H,010H,027H,010H,027H,010H,0F8H,0CH,0FFH,0F5H,05H,027H
DB 080H,0F5H,05H,02BH,080H,0F5H,05H,030H,060H,0FBH,04H,02CH,018H,029H,08H,0F8H
DB 02H,01H,0FBH,08H,027H,014H,0FBH,04H,027H,06H,027H,06H,0FBH,08H,027H,010H
DB 027H,010H,027H,010H,027H,010H,027H,010H,027H,010H,0FCH,05H,0F5H,05H,0F8H,00H
DB 00H,0FBH,00H,027H,020H,02BH,020H,02EH,020H,027H,020H,026H,020H,029H,020H,02CH
DB 020H,029H,020H,027H,020H,026H,020H,029H,020H,026H,020H,027H,020H,02BH,020H,02EH
DB 020H,032H,010H,00H,08H,032H,08H,033H,020H,02EH,020H,02BH,020H,027H,060H,026H
DB 040H,022H,020H,026H,020H,029H,020H,026H,020H,0F5H,05H,0F8H,02H,01H,0FBH,08H
DB 027H,014H,0FBH,04H,027H,06H,027H,06H,0FBH,08H,027H,010H,027H,010H,027H,010H
DB 027H,010H,027H,010H,027H,010H,0FEH,04H,027H,010H,00H,00H
french_anthem_part6 LABEL BYTE
DB 0F4H,040H,00H,023H,0FEH,00H,0FCH,018H,0F5H,019H,0F9H,028H,0F8H,0CH,0FFH,027H
DB 080H,0F5H,019H,02BH,080H,0F5H,019H,029H,060H,0FBH,04H,02CH,018H,02CH,05H,0F8H
DB 02H,01H,0FBH,08H,01BH,014H,0FBH,04H,01BH,06H,01BH,06H,0FBH,08H,01BH,010H
DB 01BH,010H,01BH,010H,01BH,010H,01BH,010H,01BH,010H,0F8H,04H,0FFH,0F5H,019H,027H
DB 083H,0F5H,019H,02BH,080H,0F5H,019H,0F8H,08H,0FFH,030H,060H,0FBH,04H,01DH,018H
DB 022H,05H,0F8H,02H,01H,0FBH,08H,01BH,014H,0FBH,04H,01BH,06H,01BH,06H,0FBH
DB 08H,01BH,010H,01BH,010H,01BH,010H,01BH,010H,01BH,010H,01BH,013H,0FCH,05H,0F5H
DB 019H,0F8H,00H,00H,0FBH,00H,027H,020H,02BH,020H,02EH,020H,027H,020H,026H,020H
DB 029H,020H,02CH,020H,029H,020H,022H,020H,026H,020H,029H,020H,026H,020H,027H,020H
DB 02BH,020H,02EH,020H,032H,010H,00H,08H,032H,08H,033H,020H,02EH,020H,02BH,020H
DB 027H,060H,026H,040H,022H,020H,026H,020H,029H,020H,026H,01DH,0F5H,019H,0F8H,02H
DB 01H,0FBH,08H,01BH,014H,0FBH,04H,01BH,06H,01BH,06H,0FBH,08H,01BH,010H,01BH
DB 010H,01BH,010H,01BH,010H,01BH,010H,01BH,010H,0FEH,04H,01BH,010H,00H,00H,00H
french_anthem_part5 LABEL BYTE
DB 0F4H,07FH,00H,020H,0FEH,00H,0FCH,018H,0F5H,019H,0F9H,01EH,0F8H,0CH,0FFH,027H
DB 086H,0F5H,019H,02BH,080H,0F5H,019H,0F8H,00H,00H,029H,060H,0FBH,04H,02CH,018H
DB 02CH,02H,0F5H,019H,0F9H,03CH,0FCH,038H,03H,014H,03H,06H,03H,06H,03H,010H
DB 03H,010H,03H,010H,03H,010H,03H,010H,03H,010H,0F8H,04H,0FFH,0F5H,019H,0F9H
DB 01EH,0FCH,018H,027H,086H,0F5H,019H,02BH,080H,0F5H,019H,0F8H,00H,00H,030H,060H
DB 0FBH,04H,020H,018H,020H,02H,0F5H,019H,0F9H,03CH,0FCH,038H,03H,014H,03H,06H
DB 03H,06H,03H,010H,03H,010H,03H,010H,03H,010H,03H,010H,03H,010H,0FCH,05H
DB 0F5H,019H,0F9H,01EH,0F8H,00H,00H,0FBH,00H,027H,020H,02BH,020H,02EH,020H,027H
DB 020H,026H,020H,029H,020H,02CH,020H,029H,020H,022H,020H,026H,020H,029H,020H,026H
DB 020H,027H,020H,02BH,020H,02EH,018H,0F5H,037H,0FCH,038H,0AH,08H,0AH,018H,0AH
DB 08H,0FFH,00H,0AH,014H,0AH,06H,0AH,06H,0AH,010H,0AH,010H,0FFH,02H,0AH
DB 040H,0AH,080H,03H,014H,03H,06H,03H,06H,03H,010H,03H,010H,03H,010H,03H
DB 010H,03H,010H,03H,010H,0FEH,04H,0FH,010H,00H,00H,00H
french_anthem_part7 LABEL BYTE
DB 0FCH,01BH,0F9H,050H,00H,020H,0FEH,00H,0F5H,0FH,0FH,080H,0FH,080H,014H,040H
DB 016H,040H,0FH,080H,0FFH,01H,0FH,040H,018H,040H,016H,040H,01DH,040H,016H,040H
DB 01AH,040H,01BH,080H,016H,080H,016H,080H,01AH,040H,016H,040H,0FH,080H,0FEH,04H
DB 00H,00H
french_motif_part1 LABEL BYTE
DB 0FCH,05H,0F4H,00H,0F5H,0FH,0F9H,05AH,03AH,08H,03AH,018H,03AH,08H,0FFH,00H
DB 03AH,020H,037H,018H,033H,08H,0FFH,01H,02EH,050H,00H,00H
french_motif_part2 LABEL BYTE
DB 0FCH,05H,0F5H,0FH,0F4H,020H,0F9H,05AH,0FBH,04H,02EH,08H,02EH,018H,02EH,08H
DB 0FFH,00H,03AH,020H,037H,018H,033H,08H,0FFH,01H,027H,040H,026H,010H,00H,00H
french_motif_part3 LABEL BYTE
DB 0FCH,05H,0F5H,0FH,0F4H,040H,0F9H,05AH,035H,08H,035H,018H,035H,08H,0FFH,00H
DB 037H,020H,033H,018H,02EH,08H,0FFH,01H,022H,050H,00H,00H
french_motif_part4 LABEL BYTE
DB 0FCH,05H,0F5H,0FH,0F4H,060H,0F9H,05AH,00H,08H,032H,010H,00H,08H,032H,08H
DB 033H,020H,02EH,020H,02BH,020H,027H,020H,022H,050H,00H,00H
french_motif_part5 LABEL BYTE
DB 0FCH,01BH,0F4H,07FH,00H,028H,0F9H,05AH,0F5H,0FH,016H,080H,016H,050H,00H,00H
french_motif_part6 LABEL BYTE
DB 0F5H,0FH,0F9H,05AH,0FCH,038H,0F4H,040H,0AH,08H,0AH,018H,0AH,08H,0FFH,00H
DB 0AH,010H,0AH,08H,0AH,08H,0AH,010H,0AH,010H,0FFH,02H,0AH,010H,00H,00H
rapture_growth_part1 LABEL BYTE
DB 0FCH,016H,0F5H,037H,0F9H,0AH,0F4H,00H,030H,023H,00H,07H,030H,0EH,030H,069H
DB 037H,07H,0F8H,07H,01H,035H,02AH,034H,0EH,035H,02AH,037H,0EH,0F8H,05H,0FFH
DB 038H,070H,035H,038H,00H,038H,0F5H,037H,0F8H,0EH,01H,03CH,070H,03AH,014H,038H
DB 02AH,037H,014H,035H,02AH,0F5H,037H,0F8H,0EH,0FFH,038H,070H,037H,031H,00H,07H
DB 037H,023H,00H,07H,037H,0EH,0F8H,0EH,01H,037H,06EH,00H,02H,037H,02AH,038H
DB 0EH,03AH,02AH,03CH,0EH,03DH,08CH,0F8H,07H,0FFH,03DH,099H,0F5H,02DH,0F8H,00H
DB 00H,03AH,038H,038H,038H,037H,038H,038H,02AH,00H,0EH,038H,01CH,00H,0EH,038H
DB 07H,00H,07H,038H,038H,00H,00H,00H
rapture_growth_part2 LABEL BYTE
DB 0FCH,01AH,0F5H,02DH,0F9H,0AH,0F4H,07FH,00H,038H,0FBH,07H,0F5H,02DH,0F8H,0BH
DB 0FFH,024H,0E0H,0F5H,02DH,024H,0E0H,0F5H,02DH,0F8H,00H,00H,024H,070H,035H,03EH
DB 035H,038H,00H,06H,0F4H,00H,0FCH,016H,0F5H,02BH,035H,070H,034H,038H,00H,038H
DB 0F4H,07FH,0FCH,01AH,0F5H,02DH,0F8H,07H,0FFH,019H,070H,00H,070H,0F5H,02DH,019H
DB 070H,00H,0B5H,0F8H,00H,00H,0F4H,00H,0FCH,016H,0F5H,02DH,033H,0E0H,035H,01CH
DB 00H,0EH,035H,07H,00H,07H,033H,038H,00H,00H
rapture_growth_part3 LABEL BYTE
DB 0FCH,015H,0F5H,02DH,0F9H,0AH,0F4H,040H,00H,038H,0FBH,07H,0F8H,0BH,0FFH,024H
DB 0E0H,0F5H,02DH,024H,0E0H,0F5H,02DH,0F8H,00H,00H,024H,070H,029H,03EH,029H,03EH
DB 0FBH,0FFH,029H,038H,026H,038H,028H,038H,00H,038H,0F8H,07H,0FFH,028H,070H,00H
DB 070H,0F5H,02DH,028H,038H,0F5H,02DH,0F8H,0EH,0FFH,025H,0EDH,0F5H,02DH,0F8H,00H
DB 00H,025H,038H,024H,038H,022H,038H,0FBH,07H,024H,038H,025H,02AH,025H,0EH,024H
DB 038H,00H,00H,00H
rapture_growth_part4 LABEL BYTE
DB 0FCH,015H,0F5H,02DH,0F9H,0AH,0F4H,040H,00H,038H,0FBH,07H,0F8H,0BH,0FFH,02CH
DB 0E0H,0F5H,02DH,02CH,0E0H,0F5H,02DH,02CH,070H,0F5H,02DH,0F8H,00H,00H,030H,03EH
DB 030H,03EH,030H,038H,02CH,038H,0F8H,07H,0FFH,02BH,038H,00H,038H,0F5H,02DH,02BH
DB 070H,00H,070H,0F5H,02DH,02EH,070H,00H,0B5H,0F5H,02DH,0F8H,00H,00H,02EH,038H
DB 02CH,038H,02BH,038H,02CH,038H,02CH,02AH,02CH,0EH,02CH,038H,00H,00H
rapture_growth_part5 LABEL BYTE
DB 0FCH,01AH,0F9H,0AH,0F4H,07FH,00H,038H,0FBH,07H,0F5H,02DH,0F8H,0BH,0FFH,020H
DB 0E0H,0F5H,02DH,020H,0E0H,0F5H,02DH,020H,070H,0F5H,02DH,0F8H,00H,00H,020H,03EH
DB 024H,03EH,024H,038H,026H,038H,028H,038H,00H,038H,0F8H,0BH,0FFH,01CH,070H,00H
DB 070H,0F5H,02DH,01CH,070H,00H,07DH,0F5H,02DH,0F8H,00H,00H,027H,038H,02BH,038H
DB 02CH,038H,02EH,038H,02CH,038H,02CH,02AH,02CH,0EH,02CH,038H,00H,00H
rapture_growth_part6 LABEL BYTE
DB 0FCH,07H,0F5H,030H,0F9H,0AH,0F4H,07FH,0FBH,04H,00H,01CH,0FFH,00H,02CH,0AH
DB 02CH,09H,02CH,09H,02CH,038H,00H,08CH,0FFH,01H,02CH,0AH,02CH,09H,02CH,09H
DB 02CH,054H,02CH,0AH,02CH,09H,02CH,09H,02CH,060H,029H,0AH,029H,09H,029H,09H
DB 030H,038H,00H,01CH,032H,0AH,032H,09H,032H,09H,034H,054H,028H,0AH,028H,09H
DB 028H,09H,02BH,038H,00H,08CH,02EH,0AH,02EH,09H,02EH,09H,02EH,038H,00H,08CH
DB 031H,0BH,031H,0BH,031H,0DH,00H,03H,031H,03BH,00H,01CH,02EH,0AH,02EH,09H
DB 02EH,09H,030H,038H,00H,01CH,02EH,0AH,02EH,09H,02EH,09H,027H,038H,029H,038H
DB 027H,038H,00H,00H
rapture_growth_part7 LABEL BYTE
DB 0FCH,0DH,0F5H,030H,0F9H,0AH,0FBH,04H,0F4H,00H,00H,01CH,0FFH,00H,01DH,0AH
DB 01DH,09H,01DH,09H,01DH,038H,00H,08CH,0FFH,01H,01DH,0AH,01DH,09H,01DH,09H
DB 01DH,054H,01DH,0AH,01DH,09H,01DH,09H,01DH,060H,020H,0AH,020H,09H,020H,09H
DB 024H,038H,00H,01CH,018H,0AH,018H,09H,018H,09H,024H,054H,018H,0AH,018H,09H
DB 018H,09H,022H,038H,00H,08CH,01CH,0AH,01CH,09H,01CH,09H,01CH,038H,00H,08CH
DB 01CH,0BH,01CH,0BH,01CH,0DH,01BH,03BH,00H,01CH,01BH,0AH,01BH,09H,01BH,09H
DB 01BH,038H,00H,01CH,01BH,0AH,01BH,09H,01BH,09H,020H,038H,020H,038H,020H,038H
DB 00H,00H
rapture_growth_part8 LABEL BYTE
DB 0F5H,035H,0FCH,0DH,0F9H,014H,0F4H,020H,024H,024H,00H,07H,024H,0EH,024H,066H
DB 02BH,0AH,0F8H,010H,01H,029H,02AH,028H,0EH,029H,02AH,02BH,0EH,0F8H,015H,0FFH
DB 02CH,070H,029H,038H,00H,038H,0F5H,032H,0F8H,020H,01H,030H,070H,02EH,014H,02CH
DB 02AH,02BH,014H,029H,02AH,0F8H,010H,0FFH,02CH,070H,02BH,031H,00H,07H,0F8H,01CH
DB 01H,02BH,023H,00H,07H,02BH,0EH,02BH,070H,02BH,02AH,02CH,0EH,02EH,02AH,030H
DB 0EH,0F8H,0FH,0FFH,031H,0A8H,0F5H,034H,0F8H,038H,0FFH,030H,01CH,02EH,01BH,00H
DB 01H,02EH,01CH,02CH,01CH,02EH,01CH,02BH,01CH,029H,038H,02BH,02AH,028H,0EH,0FBH
DB 07H,029H,038H,029H,02AH,029H,0EH,029H,038H,00H,00H,00H
rapture_growth_part9 LABEL BYTE
DB 0FCH,019H,0FCH,09H,0F9H,014H,0F4H,07FH,00H,038H,0F8H,0BH,0FFH,0F5H,034H,024H
DB 070H,00H,070H,0F5H,034H,024H,070H,00H,070H,0F5H,034H,024H,062H,00H,0EH,0F5H
DB 034H,024H,06EH,00H,0EH,0F5H,034H,024H,038H,02FH,038H,030H,038H,00H,038H,0F5H
DB 034H,028H,070H,00H,070H,0F5H,034H,0F8H,029H,0FFH,025H,0A8H,024H,02AH,00H,0EH
DB 024H,038H,025H,02AH,00H,0EH,024H,02AH,00H,0EH,024H,02AH,00H,0EH,024H,038H
DB 025H,01CH,00H,0EH,025H,0EH,024H,038H,00H,00H
rapture_growth_part10 LABEL BYTE
DB 0FCH,019H,0FCH,09H,0F9H,014H,0F4H,040H,00H,038H,0F8H,0BH,0FFH,0F5H,034H,020H
DB 070H,00H,070H,0F5H,034H,020H,070H,00H,070H,0F5H,034H,020H,062H,00H,0EH,0F5H
DB 034H,020H,06EH,00H,0EH,0F5H,034H,020H,070H,01FH,038H,00H,038H,0F5H,034H,022H
DB 070H,00H,070H,0F5H,034H,0F8H,029H,0FFH,022H,0A8H,022H,01CH,024H,0EH,00H,0EH
DB 024H,038H,01FH,02AH,00H,0EH,020H,038H,022H,02AH,00H,0EH,020H,038H,022H,01CH
DB 00H,0EH,022H,0EH,020H,038H,00H,00H
rapture_growth_part11 LABEL BYTE
DB 0FCH,09H,0F9H,014H,0F4H,060H,00H,038H,0F5H,034H,0F8H,0BH,0FFH,01DH,070H,00H
DB 070H,0F5H,034H,01DH,070H,00H,070H,0F5H,034H,01DH,070H,0F5H,034H,01DH,07CH,0F5H
DB 034H,01DH,070H,01CH,038H,00H,038H,0F5H,034H,01CH,070H,00H,070H,0F8H,029H,0FFH
DB 01FH,0A8H,01FH,038H,01DH,02AH,00H,0EH,01DH,02AH,00H,0EH,01DH,02AH,00H,0EH
DB 01CH,02AH,01FH,07H,00H,07H,01DH,038H,022H,02AH,022H,0EH,01DH,038H,00H,00H
rapture_growth_part12 LABEL BYTE
DB 0FCH,0EH,0F5H,029H,0F9H,014H,0F4H,00H,00H,023H,0FFH,00H,018H,07H,01AH,07H
DB 01CH,07H,01DH,038H,00H,093H,0FFH,01H,018H,07H,01AH,07H,01CH,07H,01DH,038H
DB 00H,01CH,01DH,0EH,01FH,0EH,020H,03EH,00H,022H,020H,0EH,022H,0EH,024H,0A8H
DB 00H,01CH,022H,0EH,024H,0EH,025H,070H,00H,038H,022H,0EH,020H,0EH,01FH,0EH
DB 01DH,0EH,01CH,070H,00H,046H,018H,0EH,01AH,0EH,01CH,0EH,0F8H,021H,0FFH,01DH
DB 038H,00H,01CH,0AH,0AH,0AH,09H,0AH,09H,018H,038H,00H,01CH,018H,0AH,018H
DB 09H,018H,09H,01DH,038H,022H,01CH,00H,0EH,022H,0EH,01DH,038H,00H,00H,00H
battle_lost_heavy_part2 LABEL BYTE
DB 0FCH,024H,0F5H,032H,0F8H,04H,0FFH,01H,06EH,00H,00H,00H
battle_won_heavy_part1 LABEL BYTE
DB 0FCH,024H,0F8H,05H,0FFH,0F5H,039H,01H,06EH,00H,00H,00H
battle_lost_heavy_part1 LABEL BYTE
DB 00H,032H,0FCH,015H,0F5H,03FH,0FAH,0FDH,0F8H,01H,0FEH,012H,019H,00H,00H,00H
battle_lost_part2 LABEL BYTE
DB 0F5H,035H,0FCH,01FH,01H,0BH,01H,0BH,01H,0BH,00H,00H
battle_won_part1 LABEL BYTE
DB 0F5H,03FH,0FCH,01FH,01H,0BH,01H,0BH,01H,014H,00H,00H
battle_lost_part1 LABEL BYTE
DB 00H,021H,0FCH,015H,0F5H,03FH,0FAH,0FDH,0F8H,01H,0FEH,014H,019H,00H,00H,00H
nuclear_blast_part2 LABEL BYTE
DB 0F5H,03FH,0FCH,0EH,0F8H,09H,0FFH,019H,020H,0FCH,025H,0F5H,035H,01H,0FFH,00H
DB 00H,00H
nuclear_blast_part1 LABEL BYTE
DB 0FCH,025H,0F5H,03FH,0F8H,09H,0FFH,01H,0FFH,00H,00H,00H
bomber_hit_part1 LABEL BYTE
DB 0FCH,024H,0F5H,03FH,0F8H,03H,0FFH,0F5H,03FH,01H,013H,01H,0BDH,00H,00H,00H
bomber_hit_part2 LABEL BYTE
DB 0FCH,024H,0F5H,03FH,0F8H,03H,0FFH,01H,0BDH,00H,00H,00H
unit_move_part1 LABEL BYTE
DB 0F5H,032H,0FCH,026H,0FBH,0FH,0DH,015H,01H,019H,00H,00H
city_ambience_part1 LABEL BYTE
DB 00H,07H,0F4H,07FH,0FFH,01H
city_ambience_part2 LABEL BYTE
DB 00H,07H,0F4H,060H,0FFH,01H
city_ambience_part3 LABEL BYTE
DB 00H,07H,0F4H,020H,0FFH,01H
city_ambience_part4 LABEL BYTE
DB 0F5H,032H,0F8H,013H,01H,0FCH,010H,0FBH,03H,02FH,06H,02AH,06H,02FH,06H,034H
DB 06H,036H,06H,03BH,06H,032H,06H,037H,06H,03EH,06H,037H,06H,039H,06H,03EH
DB 06H,040H,06H,039H,06H,034H,06H,039H,06H,0F8H,02H,0FFH,03BH,06H,040H,06H
DB 043H,06H,03EH,06H,039H,06H,03EH,06H,045H,06H,040H,06H,03BH,06H,036H,06H
DB 03BH,06H,040H,06H,042H,06H,047H,06H,04AH,06H,04FH,06H,00H,00H,0AH,02H
DB 08H,03H,01H,00H,028H,00H,00H,01H,03H,00H,00H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,09H,02H,05H,09H,01H,01H,03FH,00H,00H,01H,03H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0AH,02H,03H,02H,01H,00H
DB 032H,00H,00H,01H,03H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 0BH,02H,0FH,06H,01H,01H,03FH,00H,00H,01H,03H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,05H,02H,01H,06H,01H,01H,024H,02H,00H,01H
DB 07H,01H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,07H,01H,01H,08H
DB 01H,00H,03FH,00H,00H,01H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,05H,02H,01H,05H,01H,01H,028H,02H,00H,01H,07H,01H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,07H,01H,01H,07H,01H,00H,03FH,00H
DB 00H,02H,07H,01H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,04H
DB 02H,07H,01H,01H,023H,00H,01H,01H,07H,01H,00H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,08H,01H,01H,08H,01H,00H,03FH,00H,00H,01H,07H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,03H,02H,07H,01H,01H
DB 01FH,00H,00H,01H,07H,01H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 08H,01H,01H,08H,01H,00H,03FH,00H,00H,01H,07H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,0AH,06H,03H,04H,01H,00H,030H,02H,00H,00H
DB 05H,01H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0DH,02H,0FH,0FH
DB 01H,00H,03FH,00H,00H,00H,06H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,07H,04H,01H,04H,01H,00H,030H,02H,01H,00H,06H,01H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,05H,01H,01H,05H,01H,01H,03FH,00H
DB 00H,01H,06H,01H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,07H,04H
DB 00H,03H,01H,00H,030H,02H,01H,00H,06H,00H,01H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,05H,02H,00H,07H,01H,01H,03FH,00H,00H,01H,06H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,04H,00H,03H,01H,00H
DB 024H,02H,00H,00H,07H,01H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 03H,01H,01H,04H,01H,01H,03FH,00H,00H,01H,07H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,0FH,06H,07H,05H,01H,01H,027H,00H,01H,01H
DB 05H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0FH,04H,0FH,08H
DB 01H,01H,03FH,00H,00H,00H,05H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,09H,02H,00H,05H,01H,01H,026H,02H,01H,01H,07H,01H,01H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,06H,02H,01H,08H,01H,00H,03FH,00H
DB 00H,01H,07H,00H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H,09H,02H
DB 00H,05H,01H,01H,029H,02H,01H,01H,07H,01H,01H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,06H,02H,01H,08H,01H,00H,03FH,00H,00H,01H,07H,00H
DB 01H,01H,00H,00H,00H,00H,00H,00H,00H,00H,07H,05H,01H,04H,01H,00H
DB 02CH,02H,01H,00H,07H,01H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 05H,01H,01H,05H,01H,01H,03FH,00H,00H,01H,06H,01H,01H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,07H,07H,02H,04H,01H,00H,035H,02H,01H,00H
DB 06H,00H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,01H,01H,05H
DB 01H,01H,03FH,00H,00H,01H,06H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,0FH,06H,07H,05H,01H,01H,01FH,00H,00H,01H,06H,00H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,0FH,05H,0FH,08H,01H,00H,03FH,00H
DB 01H,01H,06H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0FH,06H
DB 07H,05H,01H,01H,024H,02H,00H,01H,05H,00H,00H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,0FH,05H,0AH,04H,00H,00H,03FH,00H,01H,01H,05H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,04H,01H,03H,04H,01H,01H
DB 029H,00H,00H,02H,05H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 0DH,04H,02H,02H,00H,01H,03FH,00H,01H,01H,05H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,06H,05H,02H,06H,01H,00H,024H,00H,01H,01H
DB 02H,01H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H,07H,05H,04H,08H
DB 01H,00H,03FH,00H,00H,01H,02H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,05H,07H,01H,06H,01H,00H,026H,00H,01H,01H,02H,01H,01H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,07H,04H,05H,08H,01H,00H,03FH,00H
DB 00H,01H,02H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,03H
DB 02H,06H,01H,01H,035H,02H,01H,01H,04H,01H,01H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,06H,05H,02H,08H,01H,00H,03FH,00H,00H,02H,04H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,04H,02H,06H,01H,01H
DB 027H,02H,00H,02H,02H,01H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 05H,05H,00H,08H,01H,01H,035H,00H,00H,01H,02H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,04H,04H,01H,06H,01H,01H,01EH,01H,01H,04H
DB 04H,01H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,05H,00H,08H
DB 01H,01H,03FH,00H,01H,01H,02H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,06H,08H,01H,06H,01H,00H,034H,02H,01H,01H,05H,01H,01H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,06H,05H,01H,08H,01H,00H,03FH,00H
DB 00H,01H,05H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,05H
DB 01H,06H,01H,00H,028H,00H,00H,00H,06H,01H,01H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,05H,02H,01H,08H,01H,00H,03FH,00H,00H,00H,06H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,06H,02H,02H,05H,01H,00H
DB 029H,00H,00H,00H,05H,01H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 07H,01H,01H,0AH,01H,00H,03FH,00H,00H,01H,05H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,07H,08H,03H,0AH,01H,00H,02CH,02H,00H,01H
DB 05H,01H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H,07H,01H,01H,0AH
DB 01H,00H,03FH,00H,00H,01H,05H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,06H,06H,02H,0AH,01H,00H,026H,02H,00H,01H,05H,01H,01H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,07H,01H,00H,0AH,01H,00H,03FH,00H
DB 00H,01H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,05H,08H
DB 06H,06H,01H,01H,027H,00H,01H,01H,04H,01H,01H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,06H,02H,01H,08H,01H,00H,03FH,00H,00H,01H,04H,01H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0EH,08H,0AH,04H,00H,01H
DB 03FH,00H,03H,00H,03H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 0DH,04H,09H,05H,00H,00H,03FH,00H,00H,00H,03H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,0EH,08H,09H,06H,00H,01H,032H,00H,01H,01H
DB 04H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0DH,05H,08H,07H
DB 00H,00H,03FH,00H,00H,01H,04H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,0FH,05H,01H,05H,00H,00H,03CH,00H,00H,02H,07H,00H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,0FH,08H,04H,07H,00H,00H,03FH,00H
DB 00H,00H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,08H,0AH
DB 0BH,04H,00H,01H,03FH,00H,03H,01H,06H,00H,00H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,0CH,05H,04H,0AH,00H,01H,03FH,00H,00H,00H,06H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0AH,09H,0AH,05H,00H,01H
DB 03DH,00H,03H,01H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 0CH,04H,04H,03H,00H,01H,03FH,00H,00H,00H,07H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,0EH,0AH,05H,0AH,00H,00H,03FH,00H,02H,0AH
DB 07H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0DH,0AH,04H,09H
DB 00H,00H,03FH,00H,00H,01H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,0FH,02H,0FH,01H,00H,00H,025H,00H,01H,0FH,06H,00H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,0FH,01H,0FH,05H,00H,01H,03FH,00H
DB 03H,01H,06H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0DH,08H
DB 08H,0FH,01H,00H,018H,00H,00H,00H,05H,00H,00H,01H,06EH,00H,0FFH,03H
DB 00H,00H,00H,00H,0CH,03H,0FH,0FH,01H,00H,03FH,00H,00H,0CH,05H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0FH,01H,0BH,0BH,01H,00H
DB 02EH,00H,01H,02H,00H,00H,00H,01H,0FFH,00H,099H,09H,033H,03H,00H,00H
DB 0FH,01H,06H,0DH,01H,00H,03FH,00H,00H,05H,05H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,0AH,03H,02H,05H,00H,00H,02BH,00H,00H,06H
DB 05H,01H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0DH,00H,02H,04H
DB 01H,00H,03FH,00H,00H,01H,05H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,06H,02H,02H,0AH,01H,00H,022H,01H,01H,01H,05H,01H,01H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,07H,01H,01H,0AH,01H,00H,03FH,00H
DB 00H,01H,05H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0DH,08H
DB 08H,0FH,01H,00H,018H,00H,00H,00H,05H,00H,00H,01H,0C8H,00H,0FFH,03H
DB 00H,00H,00H,00H,0CH,03H,0FH,0FH,01H,00H,03FH,00H,00H,0CH,05H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0EH,08H,06H,06H,00H,01H
DB 02EH,00H,00H,01H,04H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 0DH,06H,03H,05H,00H,00H,03FH,00H,00H,01H,04H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,0EH,08H,09H,06H,00H,01H,034H,00H,00H,01H
DB 04H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0DH,05H,08H,07H
DB 00H,00H,03FH,00H,00H,01H,04H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,0EH,06H,0FH,03H,01H,00H,03CH,00H,00H,02H,07H,00H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,0EH,07H,0FH,09H,00H,01H,03FH,00H
DB 00H,00H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0FH,08H
DB 07H,0CH,01H,00H,029H,00H,01H,0AH,06H,00H,00H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,0FH,05H,0FH,0CH,01H,01H,03FH,00H,00H,02H,06H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0CH,0BH,0EH,08H,00H,00H
DB 035H,00H,01H,01H,00H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 0CH,08H,0DH,07H,00H,00H,03FH,00H,00H,01H,00H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,0AH,09H,09H,03H,00H,01H,032H,00H,00H,00H
DB 07H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0BH,06H,05H,06H
DB 00H,00H,03FH,00H,00H,01H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,0CH,08H,02H,06H,00H,01H,026H,00H,02H,04H,04H,00H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,0DH,06H,02H,03H,00H,01H,03FH,00H
DB 00H,01H,04H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,06H,08H
DB 02H,06H,01H,01H,021H,00H,01H,01H,04H,01H,00H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,06H,02H,01H,08H,01H,00H,03FH,00H,00H,01H,04H,01H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0CH,07H,02H,04H,00H,01H
DB 01DH,00H,00H,06H,02H,00H,01H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 0DH,05H,05H,03H,00H,00H,03FH,00H,01H,01H,02H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,0EH,00H,02H,01H,00H,01H,036H,02H,01H,00H
DB 01H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0EH,05H,02H,04H
DB 00H,00H,03FH,00H,02H,00H,01H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,0DH,07H,01H,05H,00H,00H,039H,00H,02H,00H,07H,00H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,0EH,09H,02H,07H,00H,00H,03FH,00H
DB 00H,02H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0FH,0BH
DB 01H,02H,01H,01H,026H,00H,00H,01H,07H,00H,00H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,0FH,04H,0FH,05H,01H,01H,03FH,00H,00H,02H,07H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0EH,04H,02H,01H,00H,00H
DB 030H,02H,00H,01H,04H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H
DB 0DH,05H,02H,03H,00H,01H,03FH,00H,00H,03H,04H,00H,00H,01H,00H,00H
DB 00H,00H,00H,00H,00H,00H,05H,02H,02H,07H,01H,01H,022H,00H,00H,01H
DB 07H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,08H,01H,01H,08H
DB 01H,00H,03FH,00H,00H,01H,07H,00H,00H,01H,00H,00H,00H,00H,00H,00H
DB 00H,00H,0EH,08H,04H,05H,00H,01H,03EH,00H,02H,01H,03H,00H,00H,01H
DB 00H,00H,00H,00H,00H,00H,00H,00H,0EH,03H,05H,06H,00H,01H,03FH,00H
DB 01H,00H,03H,00H,00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0CH,05H
DB 07H,0FH,01H,00H,024H,02H,01H,03H,04H,00H,01H,01H,00H,00H,00H,00H
DB 00H,00H,00H,00H,0DH,07H,02H,0FH,01H,00H,03FH,00H,00H,01H,04H,00H
DB 00H,01H,00H,00H,00H,00H,00H,00H,00H,00H,0D2H,04H
volume_levels DB 03FH,03FH,036H,031H,02DH,02AH,028H,026H,024H,022H,021H,020H,01FH,01EH,01DH,01CH ; Operator attenuation for each volume 0..127.
DB 01BH,01AH,019H,019H,018H,017H,017H,016H,016H,015H,015H,014H,014H,013H,012H,012H
DB 011H,011H,010H,010H,0FH,0FH,0EH,0EH,0DH,0DH,0CH,0CH,0BH,0BH,0AH,0AH
DB 0AH,09H,09H,09H,09H,09H,08H,08H,08H,08H,08H,07H,07H,07H,07H,07H
DB 06H,06H,06H,06H,06H,06H,05H,05H,05H,05H,05H,05H,05H,05H,05H,04H
DB 04H,04H,04H,04H,04H,04H,04H,04H,04H,04H,03H,03H,03H,03H,03H,03H
DB 03H,03H,03H,03H,03H,03H,02H,02H,02H,02H,02H,02H,02H,02H,02H,01H
DB 01H,01H,01H,01H,01H,01H,01H,01H,01H,01H,00H,00H,00H,00H,00H,00H
fnumbers DW 0200H,021EH,023FH,0261H,0285H,02ABH,02D4H,02FFH ; OPL F-numbers of the 12 semitones.
DW 032DH,035DH,0390H,03C7H
unused_words DW 3 DUP (01H)
voice_operators DB 00H,03H,01H,04H,02H,05H,06H,09H,07H,0AH,08H,0BH,0CH,0FH,0DH,010H ; Modulator and carrier slot of each of the 9 voices.
DB 0EH,011H
operator_regs DB 00H,01H,02H,03H,04H,05H,08H,09H,0AH,0BH,0CH,0DH,010H,011H,012H,013H ; Register offset of each of the 18 operator slots.
DB 014H,015H
rate_request DW 00H                      ; Timer rate request returned by sound_tick.
interlude_on DB 00H
interlude_saved DB 8 DUP (0)             ; Channel states saved during an interlude.
interlude_next DW 01H                    ; Next of interlude_tunes.
rate_now DW 00H                          ; Timer rate last requested.
spare_word DW 00H
init_argument DW 00H                     ; sound_init's argument (unused).
random_state DW 04D2H                    ; sound_random's state (starts at 1234).
spare_word2 DW 00H
card_found DW 00H                        ; Copy of card_kind.
DATA ENDS

BSS SEGMENT BYTE PUBLIC 'BSS'
DB 15 DUP (?)
sweep_a_freq DW 1 DUP (?)
sweep_b_freq DW 1 DUP (?)
sweep_a_step DW 1 DUP (?)
sweep_b_step DW 1 DUP (?)
update_lock DW 1 DUP (?)                 ; -1 while channels are being changed (the timer skips the step).
channel1 DB 30 DUP (?)                   ; Channel records: count, timers, sequence pointers, loop points, volume.
channel2 DB 30 DUP (?)
channel3 DB 30 DUP (?)
channel4 DB 30 DUP (?)
channel5 DB 30 DUP (?)
channel6 DB 30 DUP (?)
channel7 DB 30 DUP (?)
sweep_a_count DW 1 DUP (?)
channel8 DB 30 DUP (?)
sweep_b_count DW 1 DUP (?)
channel9 DB 30 DUP (?)
sweep_a_mask DW 1 DUP (?)
sweep_b_mask DW 1 DUP (?)
rhythm_mode DB 1 DUP (?)
note_pending DB 1 DUP (?)
sweep_a_voice DB 1 DUP (?)
sweep_b_voice DB 1 DUP (?)
opl_shadow DB 256 DUP (?)                ; Last value written to each OPL register.
voice_instrument DB 90 DUP (?)
instrument_base DW 1 DUP (?)
channel_now DW 1 DUP (?)                 ; Channel record being stepped.
instrument_now DW 1 DUP (?)              ; Instrument being loaded.
DB 10 DUP (?)
BSS ENDS

STACK SEGMENT PARA STACK 'STACK'
DB 16 DUP (?)
STACK ENDS
END start
