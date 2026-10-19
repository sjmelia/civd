; MISC.EXE: CIV keyboard and joystick overlay, recovered source.
; Built by MASM 5.10 and LINK 5.01 into the original byte for byte.
.8086
CODE SEGMENT BYTE PUBLIC 'CODE'
CODE ENDS
DATA SEGMENT PARA PUBLIC 'DATA'
DATA ENDS
DRIVER GROUP CODE, DATA

CODE SEGMENT
ASSUME CS:DRIVER, DS:NOTHING, ES:NOTHING
header_pad DB 16 DUP (0)
start:                                  ; DOS entry (never run: loaded as an overlay)
driver_id DB 'MISCOVR.EXE 02-20-89',0,0,0,0 ; Name and date.
DW SEG CODE                     ; 28H: segment of the slot routines
DW SEG DATA                     ; 2AH: data paragraph (CIV may append a file)
DW 090H ; bytes of data
DW 90                            ; 2EH: first slot
DW 8                           ; 30H: slot count
DW OFFSET key_none
DW OFFSET read_key
DW OFFSET bios_key
DW OFFSET locks_off
DW OFFSET locks_off
DW OFFSET stick_button
DW OFFSET stick_centre
DW OFFSET stick_read

; Slot 92: wait for a key and echo it (DOS function 1).
bios_key:
    mov ah,01H
    int 021H
    retf

; Slot 91: wait for a key; scan code and character (BIOS).
read_key:
    sub ah,ah
    int 016H
    retf

; Slot 90: 0 when a key is waiting, else -1 (BIOS).
key_none:
    mov ah,01H
    int 016H
    jz SHORT key_none_next
    sub ax,ax
    retf
key_none_next:
    sub ax,ax
    not ax
    retf

; Slots 93 and 94: turn off Caps, Num, Scroll Lock and Insert.
locks_off:
    push es
    sub ax,ax
    mov es,ax
    mov al,ES:[0417H]
    and al,0F0H
    mov ES:[0417H],al
    pop es
    retf
DB 00H

; Slot 95: joystick button n (0..3) held.
stick_button:
    mov bx,sp
    mov dx,0201H
    in al,dx
    not al
    mov cx,[bx+04H]
    add cx,04H
    shr al,cl
    and ax,01H
    retf

; Slot 96: take the joystick's position as its centre.
stick_centre:
    push ds
    mov ax,SEG DATA
    mov ds,ax
    call NEAR PTR stick_sample
    mov bx,00H
    call NEAR PTR stick_reset_axis
    mov bx,01H
    call NEAR PTR stick_reset_axis
    pop ds
    retf

; Centre axis BX at its current reading.
stick_reset_axis:
    shl bx,1
ASSUME DS:DATA
    mov ax,WORD PTR axis_raw[bx]
    mov WORD PTR axis_centre[bx],ax
    mov WORD PTR axis_low[bx],ax
    mov WORD PTR axis_high[bx],ax
    ret

; Slot 97: joystick deflection: AL x, AH y (signed).
stick_read:
    push ds
    mov ax,SEG DATA
    mov ds,ax
    call NEAR PTR stick_sample
    mov bx,00H
    call NEAR PTR stick_axis
    mov bx,01H
    call NEAR PTR stick_axis
    mov ax,stick_result
    pop ds
    retf

; Time both axes of joystick A (port 201h).
stick_sample:
    push bp
    xor bx,bx
    xor bp,bp
    mov cx,0FFFFH
    mov dx,0201H
    cli
    out dx,al
    jmp SHORT stick_sample_next
stick_sample_next:
    jmp SHORT stick_sample_loop
stick_sample_loop:
    in al,dx
    and al,03H
    jz SHORT stick_sample_next2
    shr al,1
    adc bx,00H
    shr al,1
    adc bp,00H
    loop SHORT stick_sample_loop
stick_sample_next2:
    sti
    mov WORD PTR axis_raw,bx
    mov WORD PTR axis_raw+2,bp
    pop bp
    ret

; Deflection of axis BX, scaled by the range seen so far.
stick_axis:
    shl bx,1
    mov ax,WORD PTR axis_raw[bx]
    mov dx,ax
    sub dx,WORD PTR axis_centre[bx]
    jc SHORT stick_axis_next
    ja SHORT stick_axis_next3
    mov ah,00H
    jmp stick_axis_next5
stick_axis_next:
    neg dx
    cmp ax,WORD PTR axis_low[bx]
    ja SHORT stick_axis_next2
    mov WORD PTR axis_low[bx],ax
    mov WORD PTR left_range[bx],dx
    mov ah,081H
    jmp stick_axis_next5
stick_axis_next2:
    xor ax,ax
    div WORD PTR left_range[bx]
    shr ax,1
    neg ax
    jmp stick_axis_next5
stick_axis_next3:
    cmp ax,WORD PTR axis_high[bx]
    jc SHORT stick_axis_next4
    mov WORD PTR axis_high[bx],ax
    mov WORD PTR right_range[bx],dx
    mov ah,07FH
    jmp stick_axis_next5
stick_axis_next4:
    xor ax,ax
    div WORD PTR right_range[bx]
    shr ax,1
stick_axis_next5:
    shr bx,1
    mov BYTE PTR stick_result[bx],ah
    ret
CODE ENDS

DATA SEGMENT
ASSUME DS:DATA
reserved DB 16 DUP (0)
copyright DB 'Copyright (C) 1989 by MicroProse Software, All Rights Reserved.',0
left_range DW 00H,00H                    ; Distance from centre to the furthest low reading, per axis.
DB 00H,00H,00H,00H
right_range DW 00H,00H                   ; Distance from centre to the furthest high reading.
DB 00H,00H,00H,00H
axis_low DW 00H,00H                      ; Lowest reading seen.
DB 00H,00H,00H,00H
axis_high DW 00H,00H                     ; Highest reading seen.
DB 00H,00H,00H,00H
axis_centre DW 00H,00H
DB 00H,00H,00H,00H
axis_raw DW 00H,00H                      ; Last timed reading of x and y.
DB 00H,00H,00H,00H
stick_result DW 00H                      ; Deflection of x (low byte) and y (high byte).
spare DW 00H
DATA ENDS

STACK SEGMENT PARA STACK 'STACK'
DB 16 DUP (?)
STACK ENDS
END start
