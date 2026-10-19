; String and BIOS helpers, resident linear [01992,01A88).
; Instruction source recovered from the original; names are descriptive.
; Preserve register clobbers and original ABI quirks.
.8086
F0199 SEGMENT WORD PUBLIC 'CODE'
ASSUME CS:F0199
PUBLIC _string_upper, _string_to_far, _string_from_far, _string_far_length, _bytes_far_fill, _bytes_far_compare, _print_at, _dos_print
; Uppercase ASCII a..z in a near string; ES becomes DS.
_string_upper:
    push bp
    mov bp,sp
    push si
    push ds
    pop es
    mov si,[bp+06H]
sb_upper_loop:
    cmp BYTE PTR [si],00H
    jz SHORT sb_upper_done
    mov al,[si]
    cmp al,061H
    jl SHORT sb_upper_next
    cmp al,07AH
    jg SHORT sb_upper_next
    sub al,020H
    mov [si],al
sb_upper_next:
    inc si
    jmp SHORT sb_upper_loop
sb_upper_done:
    pop si
    pop bp
    retf
; Copy near source (BP+10) to far destination (BP+6), including NUL.
_string_to_far:
    push bp
    mov bp,sp
    push si
    push di
    push es
    les di,DWORD PTR [bp+06H]
    mov si,[bp+0AH]
sb_to_far_loop:
    lodsb
    stosb
    or al,al
    jnz SHORT sb_to_far_loop
    pop es
    pop di
    pop si
    pop bp
    retf
; Copy far source (BP+8) to near destination (BP+6), including NUL.
_string_from_far:
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds
    push ds
    pop es
    lds si,DWORD PTR [bp+08H]
    mov di,[bp+06H]
sb_from_far_loop:
    lodsb
    stosb
    or al,al
    jnz SHORT sb_from_far_loop
    pop ds
    pop es
    pop di
    pop si
    pop bp
    retf
; Return far-string length in AX; BX is also changed.
_string_far_length:
    push bp
    mov bp,sp
    push si
    push ds
    lds si,DWORD PTR [bp+06H]
    sub bx,bx
sb_length_loop:
    lodsb
    cmp al,00H
    jz SHORT sb_length_done
    inc bx
    jmp SHORT sb_length_loop
sb_length_done:
    mov ax,bx
    pop ds
    pop si
    pop bp
    retf
; Fill far destination with a byte; original saves SI but changes DI.
_bytes_far_fill:
    push bp
    mov bp,sp
    push si
    push es
    mov al,[bp+0AH]
    les di,DWORD PTR [bp+06H]
    mov cx,[bp+0CH]
    rep stosb
    pop es
    pop si
    mov sp,bp
    pop bp
    retf
; Return remaining CX after REPE CMPSB, rather than a strcmp-style sign.
_bytes_far_compare:
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds
    mov cx,[bp+0EH]
    les di,DWORD PTR [bp+06H]
    lds si,DWORD PTR [bp+0AH]
    repe cmpsb
    mov ax,cx
    pop ds
    pop es
    pop di
    pop si
    mov sp,bp
    pop bp
    retf
text_attribute DB 0FH
; FF in the string changes the persistent BIOS text attribute.
; Position cursor unless low row/column bytes form FFFF, then print near text.
_print_at:
    push bp
    mov bp,sp
    push si
    mov ah,02H
    xor bh,bh
    mov dh,[bp+06H]
    mov dl,[bp+08H]
    cmp dx,-1
    jz SHORT sb_print_text
    int 010H
sb_print_text:
    mov bl,CS:text_attribute
    xor bh,bh
    mov cx,01H
    mov si,[bp+0AH]
sb_print_loop:
    lodsb
    or al,al
    jz SHORT sb_print_done
    cmp al,0FFH
    jz SHORT sb_print_attr
    mov dl,al
    mov ah,09H
    mov al,020H
    int 010H
    mov ah,0EH
    mov al,dl
    int 010H
    jmp SHORT sb_print_loop
sb_print_attr:
    lodsb
    or al,al
    jz SHORT sb_print_done
    mov CS:text_attribute,al
    mov bl,al
    jmp SHORT sb_print_loop
sb_print_done:
    pop si
    pop bp
    retf
; DOS function 09 prints a dollar-terminated near string.
_dos_print:
    push bp
    mov bp,sp
    mov ah,09H
    mov dx,[bp+06H]
    int 021H
    pop bp
    retf
F0199 ENDS
END
