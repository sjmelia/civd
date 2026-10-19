; MGRAPHIC.EXE: CIV graphics driver (MCGA/VGA), recovered source.
; Built by MASM 5.10 and LINK 5.01 into the original byte for byte.
.8086
fonts EQU 0630H  ; FONTS.CV, appended after the data by CIV's loader (header word 2CH).
CODE SEGMENT BYTE PUBLIC 'CODE'
CODE ENDS
DATA SEGMENT PARA PUBLIC 'DATA'
DATA ENDS
DRIVER GROUP CODE, DATA

CODE SEGMENT
ASSUME CS:DRIVER, DS:NOTHING, ES:NOTHING
header_pad DB 16 DUP (0)
start:                                  ; DOS entry (never run: loaded as an overlay)
driver_id DB 'MGRAPHIC.EXE09-19-91',0,0,0,0 ; Name and date.
DW SEG CODE                     ; 28H: segment of the slot routines
DW SEG DATA                     ; 2AH: data paragraph (CIV may append a file)
DW fonts                        ; 2CH: CIV appends FONTS.CV after this many bytes
DW 0                            ; 2EH: first slot
DW 52                           ; 30H: slot count
DW OFFSET alloc_page_memory
DW OFFSET free_paragraphs
DW OFFSET cut_tile
DW OFFSET recolour_rect
DW OFFSET char_width
DW OFFSET page_clear
DW OFFSET sprite_draw_clipped
DW OFFSET sprite_arena_release
DW OFFSET page_free
DW OFFSET cursor_draw
DW OFFSET page_dissolve
DW OFFSET font_address
DW OFFSET font_height
DW OFFSET page_read_row
DW OFFSET pixel_at
DW OFFSET cursor_erase
DW OFFSET font_copy
DW OFFSET page_ready
DW OFFSET sprite_arena_init
DW OFFSET draw_scanline
DW OFFSET blit
DW OFFSET page_string
DW OFFSET sprite_draw_scaled
DW OFFSET screen_off
DW OFFSET screen_on
DW OFFSET set_page_memory
DW OFFSET palette_reset_colour
DW OFFSET load_ega_palette
DW OFFSET apply_palette
DW OFFSET shake_start
DW OFFSET cursor_set
DW OFFSET set_page
DW OFFSET sprite_draw
DW OFFSET sprite_end
DW OFFSET sprite_encode_row
DW OFFSET sprite_begin
DW OFFSET page_swap
DW OFFSET video_init
DW OFFSET raster_plot
DW OFFSET raster_spans
DW OFFSET raster_flush
DW OFFSET raster_colour
DW OFFSET raster_line
DW OFFSET text_clip_all
DW OFFSET text_clip_left
DW OFFSET text_draw
DW OFFSET text_clip_right
DW OFFSET text_clip_bottom
DW OFFSET raster_page
DW OFFSET raster_target
DW OFFSET raster_copy_spans
DW OFFSET shake_step

; Slot 3: in a page rectangle, replace one colour with another.
recolour_rect:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov si,[bp+06H]
    mov di,[bp+0AH]
    add di,[si+04H]
    shl di,1
    mov di,WORD PTR CS:row_offsets[di]
    add di,[bp+08H]
    add di,[si+02H]
    mov ah,[bp+010H]
    mov al,[bp+012H]
    mov bx,[bp+0CH]
    mov bp,[bp+0EH]
    mov dx,0140H
    sub dx,bx
    mov si,[si]
    shl si,1
    mov si,WORD PTR CS:page_segs[si]
    mov ds,si
    mov es,si
recolour_rect_loop:
    mov cx,bx
recolour_rect_loop2:
    cmp ah,[di]
    jnz SHORT recolour_rect_next
    stosb
    loop SHORT recolour_rect_loop2
recolour_rect_loop3:
    add di,dx
    dec bp
    jnz SHORT recolour_rect_loop
    pop ds
    pop di
    pop si
    pop bp
    retf
recolour_rect_next:
    inc di
    loop SHORT recolour_rect_loop2
    jmp SHORT recolour_rect_loop3

; Slot 26: reload one DAC colour from the default palette.
palette_reset_colour:
    push bp
    mov bp,sp
    mov ax,[bp+08H]
    mov bx,[bp+06H]
    mov cx,01H
    mov es,CS:data_seg
    lea dx,DS:default_palette
    add dx,ax
    add dx,ax
    add dx,ax
    call NEAR PTR dac_write
    pop bp
    retf

; Slot 27: set the 16 EGA colours (and the border) from an EGA palette.
load_ega_palette:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov si,[bp+06H]
    mov bx,0FFFFH
    mov al,[si+010H]
load_ega_palette_loop:
    inc bx
    cmp bx,0FH
    ja SHORT load_ega_palette_next
    cmp al,[bx+si]
    jnz SHORT load_ega_palette_loop
    mov bh,bl
    call NEAR PTR set_border
load_ega_palette_next:
    mov ax,CS:data_seg
    mov ds,ax
    mov es,ax
    mov bx,si
ASSUME DS:DATA
    lea di,ega_colours
    xor ah,ah
    mov cx,010H
load_ega_palette_loop2:
    mov al,SS:[bx]
    inc bx
    lea si,default_palette
    add si,ax
    add si,ax
    add si,ax
    movsb
    movsb
    movsb
    loop SHORT load_ega_palette_loop2
    mov bx,00H
    mov cx,010H
    lea dx,ega_colours
    call NEAR PTR dac_write
    pop ds
    pop di
    pop si
    pop bp
    retf

; Slot 28: load a block of DAC colours from an "M0" palette record.
apply_palette:
    push bp
    mov bp,sp
    mov ax,ds
    mov es,ax
ASSUME DS:NOTHING
    mov bx,[bp+06H]
    cmp BYTE PTR [bx],04DH
    jnz SHORT apply_palette_done
    cmp BYTE PTR [bx+01H],030H
    jnz SHORT apply_palette_done
    mov dx,bx
    add dx,06H
    mov cl,[bx+05H]
    xor ch,ch
    mov bl,[bx+04H]
    xor bh,bh
    sub cx,bx
    inc cx
    call NEAR PTR dac_write
apply_palette_done:
    pop bp
    retf

; Load the 128 default colours.
palette_reset:
    mov bx,00H
    mov cx,080H
    mov es,CS:data_seg
    lea dx,DS:default_palette
    call NEAR PTR dac_write
    ret

; Write CX colours from ES:DX to the DAC from index BX, a few per vertical retrace.
dac_write:
    push bp
    push si
    push ds
    mov si,es
    mov ds,si
    mov si,dx
dac_write_loop:
    mov dx,03DAH
dac_write_loop2:
    in al,dx
    test al,08H
    jz SHORT dac_write_loop2
    mov bp,00H
    jmp dac_write_loop4
dac_write_loop3:
    inc bp
    js SHORT dac_write_loop4
    jz SHORT dac_write_loop
    mov dl,0DAH
    in al,dx
    test al,08H
    jz SHORT dac_write_next5
dac_write_loop4:
    cli
    mov dl,0C8H
    mov al,bl
    inc bx
    out dx,al
    jmp SHORT dac_write_next
dac_write_next:
    jmp SHORT dac_write_next2
dac_write_next2:
    inc dx
    lodsb
    out dx,al
    jmp SHORT dac_write_next3
dac_write_next3:
    lodsb
    out dx,al
    jmp SHORT dac_write_next4
dac_write_next4:
    lodsb
    out dx,al
    sti
    loop SHORT dac_write_loop3
    pop ds
    pop si
    pop bp
    ret
dac_write_next5:
    mov ax,bp
    mov ah,0DH
    mul ah
    neg ax
    mov bp,ax
    jmp SHORT dac_write_loop4

; Set the overscan colour to BH.
set_border:
    mov dx,03DAH
set_border_loop:
    in al,dx
    test al,08H
    jz SHORT set_border_loop
    cli
    mov dl,0C0H
    mov al,011H
    out dx,al
    jmp SHORT set_border_next
set_border_next:
    mov al,bh
    out dx,al
    jmp SHORT set_border_next2
set_border_next2:
    mov ax,020H
    out dx,al
    sti
    ret
raster_ops DB 088H,020H,08H,030H         ; Opcodes for raster operations: MOV, AND, OR, XOR.

; Slot 41: set the raster colour (AL) and operation (AH: 0 set, 1 and, 2 or, 3 xor).
raster_colour:
    mov CS:raster_colour_value,al
    mov al,ah
    and al,03H
    mov CS:raster_op,al
    lea bx,DS:raster_ops
    DB 02EH,0D7H ; cs xlatb
    mov CS:raster_opcode,al
    retf

; Slot 40: nothing to do on MCGA.
raster_flush:
    retf

; Slot 15: restore what was under the mouse cursor.
cursor_erase:
    call NEAR PTR cursor_restore
    mov WORD PTR CS:cursor_shown,00H
    retf

; Put back what the cursor covered.
cursor_restore:
    push si
    push di
    push ds
    push es
    mov ds,CS:data_seg
ASSUME DS:DATA
    cmp WORD PTR cursor_shown,00H
    jz SHORT cursor_restore_done
    lea si,under_cursor
    mov di,under_y
    shl di,1
    mov di,WORD PTR row_offsets[di]
    add di,under_x
    mov es,WORD PTR CS:page_segs
    mov ax,under_width
    mov bx,under_height
    mov dx,0140H
    sub dx,ax
cursor_restore_loop:
    mov cx,ax
    rep movsb
    add di,dx
    dec bx
    jnz SHORT cursor_restore_loop
cursor_restore_done:
    pop es
    pop ds
    pop di
    pop si
    ret

; Slot 30: set the mouse cursor sprite and position and draw it.
cursor_set:
    push bp
    mov bp,sp
    push ds
    mov ds,CS:data_seg
    mov ax,[bp+0AH]
    mov cursor_sprite,ax
    mov es,ax
    mov ax,[bp+06H]
    mov cursor_x,ax
    mov bx,ES:[00H]
    mov cursor_width,bx
    or bx,bx
    jz SHORT cursor_set_next6
    or ax,ax
    jns SHORT cursor_set_next
    neg ax
    sub bx,ax
    jna SHORT cursor_set_next6
    xor ax,ax
cursor_set_next:
    mov cx,0140H
    sub cx,ax
    jna SHORT cursor_set_next6
    cmp bx,cx
    jna SHORT cursor_set_next2
    mov bx,cx
cursor_set_next2:
    mov under_width,bx
    mov under_x,ax
    mov ax,0200H
    cwd
    div bx
    mov dx,ax
    mov ax,[bp+08H]
    mov cursor_y,ax
    mov bx,ES:[02H]
    mov cursor_height,bx
    or bx,bx
    jz SHORT cursor_set_next6
    or ax,ax
    jns SHORT cursor_set_next3
    neg ax
    sub bx,ax
    jna SHORT cursor_set_next6
    xor ax,ax
cursor_set_next3:
    mov cx,0C8H
    sub cx,ax
    jna SHORT cursor_set_next6
    cmp bx,cx
    jna SHORT cursor_set_next4
    mov bx,cx
cursor_set_next4:
    cmp bx,dx
    jna SHORT cursor_set_next5
    mov bx,dx
cursor_set_next5:
    mov under_height,bx
    mov under_y,ax
    mov WORD PTR cursor_shown,01H
    mov ax,00H
    nop
    push cs
    call NEAR PTR cursor_draw
cursor_set_loop:
    pop ds
    pop bp
    retf
cursor_set_next6:
    mov WORD PTR cursor_shown,00H
    jmp SHORT cursor_set_loop

; Slot 9: save what is under the mouse cursor on page AX and draw the cursor there.
cursor_draw:
    push si
    push di
    push ds
    push es
    mov ds,CS:data_seg
    mov cursor_page,ax
    cmp WORD PTR cursor_shown,00H
    jz SHORT cursor_draw_done
    mov si,under_y
    shl si,1
    mov si,WORD PTR row_offsets[si]
    add si,under_x
    mov bx,ax
    shl bx,1
    mov cx,WORD PTR CS:page_segs[bx]
    lea di,under_cursor
    mov es,CS:data_seg
    mov ax,under_width
    mov bx,under_height
    mov dx,0140H
    sub dx,ax
    mov ds,cx
cursor_draw_loop:
    mov cx,ax
    rep movsb
    add si,dx
    dec bx
    jnz SHORT cursor_draw_loop
    mov ds,CS:data_seg
    mov dx,0C8H
    dec dx
    push dx
    mov dx,0140H
    dec dx
    push dx
    xor dx,dx
    push dx
    push dx
    push WORD PTR cursor_page
    mov cx,sp
    push WORD PTR cursor_sprite
    push WORD PTR cursor_y
    push WORD PTR cursor_x
    push cx
    nop
    push cs
    call NEAR PTR sprite_draw_clipped
    add sp,012H
cursor_draw_done:
    pop es
    pop ds
    pop di
    pop si
    retf
DB 00H

; Slot 14: colour of one pixel.
pixel_at:
    push bp
    mov bp,sp
ASSUME DS:NOTHING
    mov cx,[bp+08H]
    mov dx,[bp+0AH]
    mov bx,[bp+06H]
    shl bx,1
    mov es,WORD PTR CS:page_segs[bx]
    mov bx,dx
    shl bx,1
    mov bx,WORD PTR CS:row_offsets[bx]
    add bx,cx
    xor ah,ah
    mov al,ES:[bx]
    pop bp
    retf

; Slot 13: copy part of a page row into a buffer.
page_read_row:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov ax,ds
    mov es,ax
    mov di,[bp+06H]
    mov si,[bp+0CH]
    shl si,1
    mov si,WORD PTR CS:row_offsets[si]
    add si,[bp+0AH]
    mov cx,[bp+0EH]
    mov bx,[bp+08H]
    shl bx,1
    mov ds,WORD PTR CS:page_segs[bx]
    rep movsb
    pop ds
    pop di
    pop si
    pop bp
    retf

; Slot 19: copy a buffer into a page row.
draw_scanline:
    push bp
    mov bp,sp
    push si
    push di
    mov si,[bp+06H]
    mov di,[bp+0CH]
    shl di,1
    mov di,WORD PTR CS:row_offsets[di]
    add di,[bp+0AH]
    mov cx,[bp+0EH]
    inc cx
    shr cx,1
    mov bx,[bp+08H]
    shl bx,1
    mov es,WORD PTR CS:page_segs[bx]
    rep movsw
    pop di
    pop si
    pop bp
    retf

; Slot 39: fill rows AX..CX between the left and right edge tables at BX.
raster_spans:
    or ax,ax
    js SHORT raster_spans_done
    cmp ax,cx
    jg SHORT raster_spans_done
    push es
    mov es,CS:raster_seg
    mov si,ax
    shl si,1
    mov dx,cx
    shl dx,1
    mov al,CS:raster_colour_value
    mov ah,al
raster_spans_loop:
    mov bp,[bx+si]
    mov cx,[bx+si+0190H]
    sub cx,bp
    jc SHORT raster_spans_next3
    ja SHORT raster_spans_next
    or bp,bp
    jz SHORT raster_spans_next3
    cmp bp,CS:raster_right
    jz SHORT raster_spans_next3
raster_spans_next:
    inc cx
    mov di,WORD PTR CS:row_offsets[si]
    add di,bp
    test di,01H
    jz SHORT raster_spans_next2
    stosb
    dec cx
    jz SHORT raster_spans_next3
raster_spans_next2:
    shr cx,1
    rep stosw
    adc cx,cx
    rep stosb
raster_spans_next3:
    add si,02H
    cmp si,dx
    jna SHORT raster_spans_loop
    pop es
raster_spans_done:
    retf

; Slot 17: status word for a page.
page_ready:
    push bp
    mov bp,sp
    mov bx,[bp+06H]
    mov ax,WORD PTR CS:page_status[bx]
    pop bp
    retf

; Slot 49: send raster output to the page record at BX (origin and right edge).
raster_target:
    mov ax,[bx+02H]
    mov CS:raster_left,ax
    mov ax,[bx+04H]
    mov CS:raster_top,ax
    mov ax,[bx+06H]
    mov CS:raster_right,ax
    mov ax,[bx]

; Slot 48: send raster output to page AX.
raster_page:
    xchg ax,bx
    shl bx,1
    mov bx,WORD PTR CS:page_segs[bx]
    xchg ax,bx
    mov CS:raster_seg,ax
    retf

; Slot 1: largest free DOS block, in paragraphs.
free_paragraphs:
    mov ah,048H
    mov bx,0FFFFH
    int 021H
    mov ax,bx
    retf

; Slot 25: set the segment behind page n.
set_page_memory:
    push bp
    mov bp,sp
    mov ax,[bp+08H]
    mov bx,[bp+06H]
    shl bx,1
    mov WORD PTR CS:page_segs[bx],ax
    pop bp
    retf

; Slot 0: segment for page n (0: the screen; else a fresh 64000-byte block, 0 on failure).
alloc_page_memory:
    push bp
    mov bp,sp
    cmp WORD PTR [bp+06H],00H
    jnz SHORT alloc_page_memory_next
    mov ax,WORD PTR CS:page_segs
    pop bp
    retf
alloc_page_memory_next:
    mov ah,048H
    mov bx,0FA0H
    int 021H
    jnc SHORT alloc_page_memory_done
    xor ax,ax
alloc_page_memory_done:
    pop bp
    retf

; Unreferenced: exit with the out-of-memory message.
out_of_memory:
    lea dx,DS:[05CEH]
    jmp video_init_next2

; Slot 8: free the memory behind page n.
page_free:
    push bp
    mov bp,sp
    mov bx,[bp+06H]
    or bx,bx
    jz SHORT page_free_done
    shl bx,1
    mov es,WORD PTR CS:page_segs[bx]
    mov ah,049H
    int 021H
page_free_done:
    pop bp
    retf

; Slot 37: set mode 13h (bit 7 keeps video memory); exits to DOS without MCGA.
video_init:
    push bp
    mov bp,sp
    mov ax,[bp+06H]
    and ax,080H
    or ax,013H
    mov bl,al
    int 010H
    mov ah,0FH
    int 010H
    cmp al,bl
    jnz SHORT video_init_next
    call NEAR PTR palette_reset
    pop bp
    retf
video_init_next:
    lea dx,DS:no_mcga_text
video_init_next2:
    mov ds,CS:data_seg
    mov ax,03H
    int 010H
    mov ah,09H
    int 021H
    mov ax,04C00H
    int 021H

; Slot 23: blank the display.
screen_off:
    mov dx,03DAH
screen_off_loop:
    in al,dx
    test al,08H
    jz SHORT screen_off_loop
    mov dx,03D8H
    mov al,02H
    out dx,al
    mov dx,03C4H
    mov al,01H
    out dx,al
    inc dx
    in al,dx
    or al,020H
    out dx,al
    retf

; Slot 24: show the display again.
screen_on:
    mov dx,03DAH
screen_on_loop:
    in al,dx
    test al,08H
    jz SHORT screen_on_loop
    mov dx,03D8H
    mov al,0AH
    out dx,al
    mov dx,03C4H
    mov al,01H
    out dx,al
    inc dx
    in al,dx
    and al,0DFH
    out dx,al
    retf

; Slot 38: plot AX,BX with the raster colour and operation.
raster_plot:
    push ds
    push es
    mov ds,CS:data_seg
    mov es,CS:raster_seg
    add ax,CS:raster_left
    add bx,CS:raster_top
ASSUME DS:DATA
    mov line_x0,ax
    mov line_y0,bx
raster_plot_loop:
    call NEAR PTR raster_address
    mov BYTE PTR CS:plot_store+1,ah

; Opcode patched with the raster operation.
plot_store:
    mov ES:[di],al
    pop es
    pop ds
    retf

; ES:DI for the current point; AL colour, AH operation opcode.
raster_address:
    push cx
    push si
    push bp
    mov al,CS:raster_colour_value
    mov ah,CS:raster_opcode
    mov di,line_y0
    shl di,1
    mov di,WORD PTR row_offsets[di]
    add di,line_x0
    pop bp
    pop si
    pop cx
    ret

; Slot 42: draw a line from AX,BX to CX,DX.
raster_line:
    push ds
    push es
    mov ds,CS:data_seg
    mov es,CS:raster_seg
    mov di,CS:raster_left
    add ax,di
    add cx,di
    mov di,CS:raster_top
    add bx,di
    add dx,di
    cmp bx,dx
    jna SHORT raster_line_next
    xchg ax,cx
    xchg bx,dx
raster_line_next:
    mov line_x0,ax
    mov line_y0,bx
    mov line_x1,cx
    mov line_y1,dx
    jnz SHORT raster_line_next2
    cmp ax,cx
    jz SHORT raster_plot_loop
raster_line_next2:
    mov si,01H
    mov bp,0140H
    sub dx,bx
    sub cx,ax
    jns SHORT raster_line_next3
    neg cx
    neg si
raster_line_next3:
    mov bx,si
    add bx,bp
    cmp cx,dx
    jnc SHORT raster_line_next4
    xchg si,bp
    xchg cx,dx
raster_line_next4:
    jz SHORT line_store_next
    or cx,cx
    mov bx,bp
    jz SHORT line_store_next
    or dx,dx
    mov bx,si
    jz SHORT line_store_next
    call NEAR PTR raster_address
    mov BYTE PTR CS:line_store+1,ah
    mov line_minor,bp
    mov bp,cx
    inc bp
    shr bp,1
    neg bp
    mov bx,cx

; Opcode patched with the raster operation.
line_store:
    mov ES:[di],al
    dec cx
    js SHORT hline_store_done
    add di,si
    add bp,dx
    js SHORT line_store
    sub bp,bx
    add di,line_minor
    jmp SHORT line_store
line_store_next:
    call NEAR PTR raster_address
    mov BYTE PTR CS:hline_store+1,ah
    or cx,dx
    inc cx

; Opcode patched with the raster operation.
hline_store:
    mov ES:[di],al
    add di,bx
    loop SHORT hline_store
hline_store_done:
    pop es
    pop ds
    retf
DB 00H

; Slot 31: copy a page to the screen (redrawing the cursor on it first).
set_page:
    push bp
    mov bp,sp
    push si
    push di
    push ds
ASSUME DS:NOTHING
    mov si,[bp+06H]
    or si,si
    jz SHORT set_page_done
    mov ax,si
    nop
    push cs
    call NEAR PTR cursor_draw
    shl si,1
    mov ds,WORD PTR CS:page_segs[si]
    mov es,WORD PTR CS:page_segs
    xor si,si
    xor di,di
    mov cx,07D00H
    rep movsw
set_page_done:
    pop ds
    pop di
    pop si
    pop bp
    retf

; Slot 36: exchange a page with the screen.
page_swap:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov si,[bp+06H]
    shl si,1
    jz SHORT page_swap_done
    mov ds,WORD PTR CS:page_segs[si]
    mov es,WORD PTR CS:page_segs
    xor si,si
    xor di,di
    mov cx,07D00H
page_swap_loop:
    lodsw
    mov bx,ES:[di]
    mov [di],bx
    stosw
    loop SHORT page_swap_loop
page_swap_done:
    pop ds
    pop di
    pop si
    pop bp
    retf

; Slot 5: fill a whole page with one colour.
page_clear:
    push bp
    mov bp,sp
    push di
    mov al,[bp+08H]
    mov ah,al
    mov di,[bp+06H]
    shl di,1
    mov es,WORD PTR CS:page_segs[di]
    xor di,di
    mov cx,07D00H
    rep stosw
    pop di
    pop bp
    retf
dissolve_steps DW 00H                    ; Dissolve steps per frame at zero delay (0 until calibrated).
dissolve_scale DW 00H                    ; Delay units per step.

; Slot 10: copy a page to the screen pixel by pixel in pseudo-random order, taking about the given time.
page_dissolve:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    xor ax,ax
    mov es,ax
    mov al,01H
    mov ES:[0440H],al
page_dissolve_loop:
    cmp ES:[0440H],al
    jz SHORT page_dissolve_loop
    mov si,[bp+06H]
    shl si,1
    jz SHORT page_dissolve_loop3
    mov ds,WORD PTR CS:page_segs[si]
    mov es,WORD PTR CS:page_segs
    cmp WORD PTR CS:dissolve_steps,00H
    jnz SHORT page_dissolve_next
    call NEAR PTR dissolve_calibrate
page_dissolve_next:
    mov cx,[bp+08H]
    jcxz SHORT page_dissolve_next5
    mov ax,02260H
    mov dx,00H
    div cx
    mov bx,ax
    or bx,bx
    jnz SHORT page_dissolve_next2
    mov bx,01H
page_dissolve_next2:
    mov ax,CS:dissolve_steps
    sub ax,bx
    jna SHORT page_dissolve_next5
    mul WORD PTR CS:dissolve_scale
    mov cl,04H
page_dissolve_loop2:
    cmp dx,bx
    jc SHORT page_dissolve_next3
    shr dx,1
    rcr ax,1
    dec cl
    jmp SHORT page_dissolve_loop2
page_dissolve_next3:
    div bx
    mov bx,ax
    or cl,cl
    js SHORT page_dissolve_next4
    shr bx,cl
    adc bx,01H
    jmp page_dissolve_next6
page_dissolve_next4:
    shl bx,cl
    add bx,01H
    jmp page_dissolve_next6
page_dissolve_loop3:
    pop ds
    pop di
    pop si
    pop bp
    retf
page_dissolve_next5:
    xor bx,bx
page_dissolve_next6:
    mov si,01H
    mov cx,0FA00H
    cmp bx,01H
    ja SHORT page_dissolve_loop5
page_dissolve_loop4:
    shr si,1
    jnc SHORT page_dissolve_next7
    xor si,0B400H
page_dissolve_next7:
    cmp si,0FA00H
    ja SHORT page_dissolve_loop4
    dec si
    mov di,si
    movsb
    loop SHORT page_dissolve_loop4
    jmp SHORT page_dissolve_loop3
page_dissolve_loop5:
    shr si,1
    jnc SHORT page_dissolve_next8
    xor si,0B400H
page_dissolve_next8:
    cmp si,0FA00H
    ja SHORT page_dissolve_loop5
    dec si
    mov di,si
    movsb
    mov ax,bx
page_dissolve_loop6:
    dec ax
    jnz SHORT page_dissolve_loop6
    loop SHORT page_dissolve_loop5
    jmp SHORT page_dissolve_loop3

; Time the dissolve loop against one video frame.
dissolve_calibrate:
    mov bx,01H
    call NEAR PTR dissolve_time
    mov CS:dissolve_steps,cx
    mov bx,011H
    call NEAR PTR dissolve_time
    xor al,al
    mov ah,cl
    mov dl,ch
    xor dh,dh
    mov bx,CS:dissolve_steps
    sub bx,cx
    div bx
    shr bx,1
    cmp bx,dx
    adc ax,00H
    mov CS:dissolve_scale,ax
    ret

; Count dissolve steps (delay BX) in one frame.
dissolve_time:
    cli
    xor cx,cx
    mov si,01H
    mov dx,03DAH
dissolve_time_loop:
    in al,dx
    test al,08H
    jz SHORT dissolve_time_loop
dissolve_time_loop2:
    in al,dx
    test al,08H
    jnz SHORT dissolve_time_loop2
dissolve_time_loop3:
    shr si,1
    jnc SHORT dissolve_time_next
    xor si,0B400H
dissolve_time_next:
    cmp si,0FA00H
    ja SHORT dissolve_time_loop3
    dec si
    mov di,0FFFFH
    movsb
    mov ax,bx
dissolve_time_loop4:
    dec ax
    jnz SHORT dissolve_time_loop4
    inc cx
    in al,dx
    test al,08H
    jz SHORT dissolve_time_loop3
    sti
    ret
DB 00H

; Slot 20: copy a rectangle between pages.
blit:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov si,[bp+06H]
    mov di,[bp+010H]
    mov ax,[si+02H]
    mov bx,[si+04H]
    mov cx,[di+02H]
    mov dx,[di+04H]
    add ax,[bp+08H]
    add bx,[bp+0AH]
    add cx,[bp+012H]
    add dx,[bp+014H]
    mov si,[si]
    mov di,[di]
    shl si,1
    shl di,1
    mov ds,WORD PTR CS:page_segs[si]
    mov es,WORD PTR CS:page_segs[di]
    shl bx,1
    mov si,WORD PTR CS:row_offsets[bx]
    add si,ax
    mov bx,dx
    shl bx,1
    mov di,WORD PTR CS:row_offsets[bx]
    add di,cx
    mov ax,[bp+0CH]
    mov bx,[bp+0EH]
    or ax,ax
    jz SHORT blit_done
    or bx,bx
    jz SHORT blit_done
    mov dx,0140H
    sub dx,ax
blit_loop:
    mov cx,ax
    shr cx,1
    rep movsw
    adc cx,cx
    rep movsb
    add si,dx
    add di,dx
    dec bx
    jnz SHORT blit_loop
blit_done:
    pop ds
    pop di
    pop si
    pop bp
    retf
DB 00H

; Slot 50: copy rows AX..CX between edge tables at BX from page SI to page DI.
raster_copy_spans:
    or ax,ax
    js SHORT raster_copy_spans_done
    push ds
    push es
    shl si,1
    shl di,1
    mov ds,WORD PTR CS:page_segs[si]
    mov es,WORD PTR CS:page_segs[di]
    mov bp,bx
    mov bx,ax
    shl bx,1
    mov dx,cx
    shl dx,1
raster_copy_spans_loop:
    mov si,bx
    mov ax,[bp+si]
    mov cx,[bp+si+0190H]
    sub cx,ax
    jc SHORT raster_copy_spans_next3
    inc cx
    mov di,WORD PTR CS:row_offsets[si]
    add di,ax
    mov si,di
    test di,01H
    jz SHORT raster_copy_spans_next
    movsb
    dec cx
    jz SHORT raster_copy_spans_next3
raster_copy_spans_next:
    shr cx,1
    jz SHORT raster_copy_spans_next2
    rep movsw
raster_copy_spans_next2:
    jnc SHORT raster_copy_spans_next3
    movsb
raster_copy_spans_next3:
    add bx,02H
    cmp bx,dx
    jna SHORT raster_copy_spans_loop
    pop es
    pop ds
raster_copy_spans_done:
    retf
DB 00H
arena_next DW 00H                        ; Next free byte of the sprite arena (offset).
arena_seg DW 00H                         ; Its segment.
arena_base DW 00H                        ; First paragraph of the arena.
arena_end DW 00H                         ; Paragraph after the arena.
sprite_room DW 00H                       ; Bytes left for the sprite being encoded.
lead_skip DW 00H                         ; Leading transparent pixels in the row being encoded.
trail_skip DW 00H                        ; Trailing transparent pixels.
run_length DW 00H                        ; Pixels from the first opaque one.
sprite_rows DW 00H                       ; Rows left to encode.
sprite_width DW 00H                      ; Width of the sprite being encoded.
draw_rows DW 00H                         ; Rows left to draw.
draw_cols DW 00H                         ; Columns to draw.
src_width DW 00H                         ; Sprite width.
src_height DW 00H                        ; Sprite height.
dst_width DW 00H                         ; Drawn width.
dst_height DW 00H                        ; Drawn height.
mirror_x DW 00H                          ; -1 to mirror left-right.
mirror_y DW 00H                          ; -1 to mirror top-bottom.
drawn_rect DB 8 DUP (0)                  ; x, y, width and height actually drawn (returned to the caller).
clip_top DW 00H
clip_left DW 00H
clip_bottom DW 00H
clip_right DW 00H
scale_x DW 00H                           ; Horizontal scaling accumulator.
scale_y DW 00H                           ; Vertical scaling accumulator.
row_step DW 00H                          ; 320, or -320 when mirrored vertically.
sprite_seg DW 00H
page_seg DW 00H
scale_colour DB 00H                      ; Unused colour argument of sprite_draw_scaled.
DB 090H
row_buffer DB 320 DUP (0)                ; One unpacked sprite row.

; Slot 18: take the largest free DOS block as the sprite arena.
sprite_arena_init:
    nop
    push cs
    call NEAR PTR free_paragraphs
    mov bx,ax
    or bx,bx
    jz SHORT sprite_arena_init_next
    mov ah,048H
    int 021H
    jnc SHORT sprite_arena_init_next2
sprite_arena_init_next:
    xor ax,ax
    xor bx,bx
sprite_arena_init_next2:
    add bx,ax
    mov WORD PTR CS:arena_next,00H
    mov CS:arena_seg,ax
    mov CS:arena_base,ax
    mov CS:arena_end,bx
    retf

; Slot 7: give back the sprite arena (shrink to what was used, or free it).
sprite_arena_release:
    mov ax,CS:arena_base
    mov bx,CS:arena_seg
    sub bx,ax
    jc SHORT sprite_arena_release_next2
    mov es,ax
    mov ah,049H
    jz SHORT sprite_arena_release_next
    mov ah,04AH
sprite_arena_release_next:
    int 021H
    jnc SHORT sprite_arena_release_next3
sprite_arena_release_next2:
    xor ax,ax
    mov es,ax
sprite_arena_release_next3:
    xor ax,ax
    mov CS:arena_seg,ax
    mov CS:arena_end,ax
    mov CS:arena_base,ax
    mov ax,es
    retf

; Slot 35: start encoding a sprite of the given size in the arena.
sprite_begin:
    push bp
    mov bp,sp
    mov ax,CS:arena_end
    sub ax,CS:arena_seg
    jna SHORT sprite_begin_next2
    cmp ax,01000H
    jc SHORT sprite_begin_next
    mov ax,0FFFH
sprite_begin_next:
    shl ax,1
    shl ax,1
    shl ax,1
    shl ax,1
    mov CS:sprite_room,ax
    push di
    les di,DWORD PTR CS:arena_next
    mov ax,[bp+06H]
    stosw
    mov CS:sprite_width,ax
    mov ax,[bp+08H]
    stosw
    mov CS:sprite_rows,ax
    mov CS:arena_next,di
    mov ax,es
    pop di
    pop bp
    retf
sprite_begin_next2:
    xor ax,ax
    mov CS:arena_next,ax
    mov CS:arena_seg,ax
    pop bp
    retf

; Slot 33: finish the sprite being encoded.
sprite_end:
    xor ax,ax
    xchg CS:arena_next,ax
    add ax,0FH
    shr ax,1
    shr ax,1
    shr ax,1
    shr ax,1
    add ax,CS:arena_seg
    xchg CS:arena_seg,ax
    retf

; Slot 34: encode one row of pixels into the sprite being built.
sprite_encode_row:
    push bp
    mov bp,sp
    push si
    push di
    mov si,[bp+06H]
    les di,DWORD PTR CS:arena_next
    mov ax,es
    or ax,ax
    jnz SHORT sprite_encode_row_next
    jmp NEAR PTR sprite_encode_row_done
sprite_encode_row_next:
    mov ax,CS:sprite_width
    add ax,04H
    add ax,di
    cmp ax,CS:sprite_room
    jna SHORT sprite_encode_row_next2
    dec WORD PTR ES:[02H]
    xor ax,ax
    mov CS:arena_end,ax
    jmp sprite_encode_row_done
sprite_encode_row_next2:
    xor ax,ax
    mov CS:lead_skip,ax
    mov CS:trail_skip,ax
    mov CS:run_length,ax
    mov bp,di
    stosw
    stosw
    mov cx,CS:sprite_width
sprite_encode_row_loop:
    lodsb
    or al,al
    jz SHORT sprite_encode_row_next3
    mov WORD PTR CS:trail_skip,00H
    jmp sprite_encode_row_next5
sprite_encode_row_next3:
    cmp WORD PTR CS:run_length,00H
    jnz SHORT sprite_encode_row_next4
    inc WORD PTR CS:lead_skip
    loop SHORT sprite_encode_row_loop
    jmp sprite_encode_row_next6
sprite_encode_row_next4:
    inc WORD PTR CS:trail_skip
sprite_encode_row_next5:
    inc WORD PTR CS:run_length
    stosb
    loop SHORT sprite_encode_row_loop
sprite_encode_row_next6:
    mov ax,CS:run_length
    mov bx,CS:lead_skip
    mov cx,CS:trail_skip
    or cx,cx
    jz SHORT sprite_encode_row_next7
    sub ax,cx
    sub di,cx
sprite_encode_row_next7:
    mov ES:[bp+00H],ax
    mov ES:[bp+02H],bx
    mov CS:arena_next,di
    mov ax,es
sprite_encode_row_done:
    pop di
    pop si
    pop bp
    retf

; Slot 2: encode a page rectangle (page, x, y, w, h) as a sprite in the arena; returns its segment.
cut_tile:
    push bp
    mov bp,sp
    push si
    push ds
    push WORD PTR [bp+0EH]
    push WORD PTR [bp+0CH]
    nop
    push cs
    call NEAR PTR sprite_begin
    add sp,04H
    or ax,ax
    jz SHORT cut_tile_next
    cmp WORD PTR CS:sprite_rows,00H
    jz SHORT cut_tile_next
    cmp WORD PTR CS:sprite_width,00H
    jz SHORT cut_tile_next
    mov si,[bp+0AH]
    shl si,1
    mov si,WORD PTR CS:row_offsets[si]
    add si,[bp+08H]
    mov ax,[bp+06H]
    cwd
    xor ax,dx
    sub ax,dx
    mov bp,dx
    mov bx,ax
    shl bx,1
    mov ds,WORD PTR CS:page_segs[bx]
cut_tile_loop:
    push si
    nop
    push cs
    call NEAR PTR sprite_encode_row
    add sp,02H
    add si,0140H
    dec WORD PTR CS:sprite_rows
    jnz SHORT cut_tile_loop
cut_tile_next:
    nop
    push cs
    call NEAR PTR sprite_end
    pop ds
    pop si
    pop bp
    retf

; Slot 22: draw a sprite scaled to a size (negative sizes mirror it), clipped.
sprite_draw_scaled:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov al,[bp+012H]
    mov CS:scale_colour,al
    mov ax,[bp+0CH]
    cwd
    xor ax,dx
    sub ax,dx
    mov CS:dst_width,ax
    mov CS:mirror_x,dx
    mov ax,[bp+0EH]
    cwd
    xor ax,dx
    sub ax,dx
    mov CS:dst_height,ax
    mov CS:mirror_y,dx
    mov dx,[bp+08H]
    mov di,[bp+0AH]
    mov ds,[bp+010H]
    xor si,si
    mov bp,[bp+06H]
    call NEAR PTR sprite_prepare
    jz SHORT sprite_draw_scaled_next
    call NEAR PTR sprite_clip
    jc SHORT sprite_draw_scaled_next
    call NEAR PTR sprite_scale_rows
sprite_draw_scaled_next:
    lea ax,DS:drawn_rect
    mov dx,cs
    pop ds
    pop di
    pop si
    pop bp
    retf

; Slot 6: draw a sprite clipped to a page; returns a far pointer to the drawn rectangle.
sprite_draw_clipped:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov dx,[bp+08H]
    mov di,[bp+0AH]
    mov ds,[bp+0CH]
    xor si,si
    mov bp,[bp+06H]
    mov ax,[si]
    mov CS:dst_width,ax
    mov ax,[si+02H]
    mov CS:dst_height,ax
    call NEAR PTR sprite_prepare
    jz SHORT sprite_draw_clipped_next
    call NEAR PTR sprite_clip
    jc SHORT sprite_draw_clipped_next
    call NEAR PTR sprite_blit_rows
sprite_draw_clipped_next:
    lea ax,DS:drawn_rect
    mov dx,cs
    pop ds
    pop di
    pop si
    pop bp
    retf

; Clip the sprite against the page; CF set when nothing is visible.
sprite_clip:
    or di,di
    jns SHORT sprite_clip_next
    mov ax,di
    neg ax
    cmp ax,CS:dst_height
    jnc SHORT sprite_clip_next4
    mov CS:clip_top,ax
sprite_clip_next:
    mov ax,di
    add ax,CS:dst_height
    dec ax
    mov bx,[bp+08H]
    or bx,bx
    js SHORT sprite_clip_next4
    cmp di,bx
    jg SHORT sprite_clip_next4
    sub ax,bx
    jng SHORT sprite_clip_next2
    mov CS:clip_bottom,ax
sprite_clip_next2:
    or dx,dx
    jns SHORT sprite_clip_next3
    mov ax,dx
    neg ax
    cmp ax,CS:dst_width
    jnc SHORT sprite_clip_next4
    mov CS:clip_left,ax
sprite_clip_next3:
    mov ax,dx
    add ax,CS:dst_width
    dec ax
    mov bx,[bp+06H]
    or bx,bx
    js SHORT sprite_clip_next4
    cmp dx,bx
    jg SHORT sprite_clip_next4
    sub ax,bx
    jng SHORT sprite_clip_next5
    mov CS:clip_right,ax
    jmp sprite_clip_next5
sprite_clip_next4:
    stc
    ret
sprite_clip_next5:
    mov ax,CS:clip_left
    add ax,dx
    mov WORD PTR CS:drawn_rect,ax
    mov ax,CS:clip_top
    add ax,di
    mov WORD PTR CS:drawn_rect+2,ax
    mov ax,CS:dst_width
    sub ax,CS:clip_left
    sub ax,CS:clip_right
    mov WORD PTR CS:drawn_rect+4,ax
    mov ax,CS:dst_height
    sub ax,CS:clip_top
    sub ax,CS:clip_bottom
    mov WORD PTR CS:drawn_rect+6,ax
    clc
    ret

; Clear the clip state; ZF set when the sprite is empty.
sprite_prepare:
    xor ax,ax
    mov CS:clip_top,ax
    mov CS:clip_left,ax
    mov CS:clip_bottom,ax
    mov CS:clip_right,ax
    mov WORD PTR CS:drawn_rect,ax
    mov WORD PTR CS:drawn_rect+2,ax
    mov WORD PTR CS:drawn_rect+4,ax
    mov WORD PTR CS:drawn_rect+6,ax
    mov ax,ds
    or ax,ax
    jz SHORT sprite_prepare_done
    mov ax,DS:[00H]
    or ax,ax
    jz SHORT sprite_prepare_done
    mov ax,DS:[02H]
    or ax,ax
sprite_prepare_done:
    ret

; Slot 32: draw a sprite, unclipped (fast path for sprites known to fit).
sprite_draw:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov dx,[bp+08H]
    mov di,[bp+0AH]
    mov ds,[bp+0CH]
    xor si,si
    mov bp,[bp+06H]
    call NEAR PTR sprite_prepare
    jz SHORT sprite_draw_done
    call NEAR PTR sprite_blit_rows
sprite_draw_done:
    pop ds
    pop di
    pop si
    pop bp
    retf

; Draw the clipped sprite rows (0 is transparent).
sprite_blit_rows:
    push bp
    push es
    mov bx,[bp+00H]
    shl bx,1
    mov es,WORD PTR CS:page_segs[bx]
    add dx,[bp+02H]
    add di,[bp+04H]
    add di,CS:clip_top
    shl di,1
    mov bp,WORD PTR CS:row_offsets[di]
    add bp,dx
    lodsw
    sub ax,CS:clip_right
    jng SHORT sprite_blit_rows_done
    mov CS:draw_cols,ax
    lodsw
    sub ax,CS:clip_top
    sub ax,CS:clip_bottom
    jng SHORT sprite_blit_rows_done
    mov CS:draw_rows,ax
    mov cx,CS:clip_top
    jcxz SHORT sprite_blit_rows_loop2
sprite_blit_rows_loop:
    add si,[si]
    add si,04H
    loop SHORT sprite_blit_rows_loop
sprite_blit_rows_loop2:
    lodsw
    mov cx,ax
    mov dx,ax
    lodsw
    jcxz SHORT sprite_blit_rows_loop4
    mov bx,ax
    add cx,ax
    sub ax,CS:clip_left
    ja SHORT sprite_blit_rows_next
    sub si,ax
    sub bx,ax
    add dx,ax
sprite_blit_rows_next:
    mov ax,CS:draw_cols
    cmp ax,cx
    ja SHORT sprite_blit_rows_next2
    mov cx,ax
sprite_blit_rows_next2:
    sub cx,bx
    jna SHORT sprite_blit_rows_loop4
    mov di,bp
    add di,bx
    sub dx,cx
sprite_blit_rows_loop3:
    lodsb
    or al,al
    jz SHORT sprite_blit_rows_next3
    stosb
    loop SHORT sprite_blit_rows_loop3
sprite_blit_rows_loop4:
    add bp,0140H
    add si,dx
    dec WORD PTR CS:draw_rows
    jnz SHORT sprite_blit_rows_loop2
sprite_blit_rows_done:
    pop es
    pop bp
    ret
sprite_blit_rows_next3:
    inc di
    loop SHORT sprite_blit_rows_loop3
    jmp SHORT sprite_blit_rows_loop4
sprite_blit_rows_loop5:
    jmp NEAR PTR sprite_scale_rows_loop5

; Draw the clipped sprite scaled, a row at a time through row_buffer.
sprite_scale_rows:
    push bp
    push es
    mov bx,[bp+00H]
    shl bx,1
    mov ax,WORD PTR CS:page_segs[bx]
    mov CS:page_seg,ax
    add dx,[bp+02H]
    add di,[bp+04H]
    lodsw
    mov CS:src_width,ax
    lodsw
    mov CS:src_height,ax
    mov ax,ds
    mov es,ax
    mov CS:sprite_seg,ax
    mov ax,cs
    mov ds,ax
ASSUME DS:DRIVER
    mov ax,dst_width
    sub ax,clip_left
    sub ax,clip_right
    jng SHORT sprite_blit_rows_loop5
    mov draw_cols,ax
    mov ax,dst_height
    sub ax,clip_top
    sub ax,clip_bottom
    jng SHORT sprite_blit_rows_loop5
    mov draw_rows,ax
    mov ax,0140H
    mov row_step,ax
    mov cx,clip_top
    add di,cx
    cmp WORD PTR mirror_y,00H
    jz SHORT sprite_scale_rows_next
    neg WORD PTR row_step
    mov cx,clip_bottom
    add di,draw_rows
    dec di
sprite_scale_rows_next:
    mov ax,dst_height
    neg ax
sprite_scale_rows_loop:
    dec cx
    js SHORT sprite_scale_rows_next2
    add ax,src_height
    js SHORT sprite_scale_rows_loop
sprite_scale_rows_loop2:
    add si,ES:[si]
    add si,04H
    sub ax,dst_height
    jns SHORT sprite_scale_rows_loop2
    jmp SHORT sprite_scale_rows_loop
sprite_scale_rows_next2:
    mov scale_y,ax
    mov cx,clip_left
    add dx,cx
    mov al,BYTE PTR forward_ops
    mov bl,BYTE PTR forward_ops+1
    cmp WORD PTR mirror_x,00H
    jz SHORT sprite_scale_rows_next3
    mov al,BYTE PTR backward_ops
    mov bl,BYTE PTR backward_ops+1
    mov cx,clip_right
    add dx,draw_cols
    dec dx
sprite_scale_rows_next3:
    mov BYTE PTR scale_direction,al
    mov BYTE PTR scale_step,bl
    mov ax,dst_width
    neg ax
    xor bx,bx
sprite_scale_rows_loop3:
    dec cx
    js SHORT sprite_scale_rows_next4
    add ax,src_width
    js SHORT sprite_scale_rows_loop3
sprite_scale_rows_loop4:
    inc bx
    sub ax,dst_width
    jns SHORT sprite_scale_rows_loop4
    jmp SHORT sprite_scale_rows_loop3
sprite_scale_rows_next4:
    mov scale_x,ax
    mov clip_left,bx
    shl di,1
    mov di,[di+017D6H]
    add di,dx
    mov ds,sprite_seg
    jmp sprite_scale_rows_loop6
sprite_scale_rows_loop5:
    pop es
    pop bp
    ret
sprite_scale_rows_loop6:
    push si
    lodsw
    mov bx,ax
    or ax,ax
    jz SHORT scale_step_next2
    lodsw
    mov cx,ax
    add ax,bx
    cmp ax,CS:clip_left
    jna SHORT scale_step_next2
    push di
    mov bp,di
    mov ax,cs
    mov es,ax
    lea di,row_buffer
    mov dx,CS:src_width
    sub dx,bx
    sub dx,cx
    xor al,al
    rep stosb
    mov cx,bx
    rep movsb
    mov cx,dx
    rep stosb
    mov ax,cs
    mov ds,ax
    lea si,row_buffer
    mov di,bp
    mov es,page_seg
    mov bx,src_width
    mov dx,dst_width
    mov bp,scale_x
    add si,clip_left
    mov cx,draw_cols
sprite_scale_rows_loop7:
    lodsb
    or al,al
    jz SHORT scale_direction_loop2
sprite_scale_rows_loop8:
    dec cx
    js SHORT scale_step_next

; CLD, or STD when mirrored.
scale_direction:
    cld
    stosb
    cld
    add bp,bx
    js SHORT sprite_scale_rows_loop8
scale_direction_loop:
    sub bp,dx
    js SHORT sprite_scale_rows_loop7
    inc si
    jmp SHORT scale_direction_loop
scale_direction_loop2:
    dec cx
    js SHORT scale_step_next

; INC DI, or DEC DI when mirrored.
scale_step:
    inc di
    add bp,bx
    js SHORT scale_direction_loop2
scale_step_loop:
    sub bp,dx
    js SHORT sprite_scale_rows_loop7
    inc si
    jmp SHORT scale_step_loop
forward_ops DW 047FCH                    ; CLD, INC DI.
backward_ops DW 04FFDH                   ; STD, DEC DI.
scale_step_next:
    pop di
scale_step_next2:
    pop si
    dec WORD PTR CS:draw_rows
    jz SHORT scale_step_next4
    add di,CS:row_step
    mov ds,CS:sprite_seg
    mov ax,CS:scale_y
    add ax,CS:src_height
    js SHORT scale_step_next3
scale_step_loop2:
    add si,[si]
    add si,04H
    sub ax,CS:dst_height
    jns SHORT scale_step_loop2
scale_step_next3:
    mov CS:scale_y,ax
    jmp NEAR PTR sprite_scale_rows_loop6
scale_step_next4:
    jmp NEAR PTR sprite_scale_rows_loop5
DB 00H
scale_step_loop3:
    and al,07FH
ASSUME DS:DATA
    mov text_colour,al
    jmp text_render_loop
scale_step_loop4:
    pop bp
    mov al,text_colour
    mov [bp+0CH],al
    ret

; Draw the prepared string at text_x, text_y.
text_render:
    mov ax,[bp+0CH]
    mov text_colour,al
    nop
    push cs
    call NEAR PTR raster_colour
    mov BYTE PTR CS:text_store+1,al
    mov al,[bp+0EH]
    mov text_background,al
    mov ax,[bp+0AH]
    mov text_opaque,al
    push bp
    mov bp,glyph_bytes
    mov bx,es
    shl bx,1
    mov es,WORD PTR CS:page_segs[bx]
    mov di,text_y
    shl di,1
    mov di,WORD PTR row_offsets[di]
    add di,text_x
text_render_loop:
    mov ax,SS:[si]
    inc si
    or al,al
    jz SHORT scale_step_loop4
    js SHORT scale_step_loop3
    cmp al,last_char
    ja SHORT text_render_loop
    sub al,first_char
    jc SHORT text_render_loop
    push si
    push di
    mov si,ax
    and si,0FFH
    mov cl,fixed_width
    or cl,cl
    jnz SHORT text_render_next
    mov bx,width_table
    mov cl,[bx+si]
    add cl,char_spacing
text_render_next:
    or ah,ah
    jnz SHORT text_render_next2
    sub cl,cut_cols
text_render_next2:
    sub cl,skip_cols
    mov glyph_width,cl
    mov cl,glyph_shift
    shl si,cl
    add si,glyph_data
    mov cl,skip_cols
    mov ch,BYTE PTR glyph_rows
    mov dl,text_colour
    mov dh,text_background
    cmp BYTE PTR text_opaque,01H
    jz SHORT text_store_loop2
    mov al,glyph_width
    or ah,ah
    jz SHORT text_render_loop2
    sub al,char_spacing
text_render_loop2:
    push di
    mov ah,al
    mov bx,[si]
    xchg bh,bl
    shl bx,cl
text_render_loop3:
    shl bx,1
    jnc SHORT text_store_next

; Opcode patched with the raster operation.
text_store:
    mov ES:[di],dl
text_store_next:
    inc di
    dec ah
    jnz SHORT text_render_loop3
    pop di
    add di,0140H
    add si,bp
    dec ch
    jnz SHORT text_render_loop2
text_store_loop:
    pop di
    pop si
    xor ah,ah
    mov skip_cols,ah
    mov al,glyph_width
    add di,ax
    add text_x,ax
    jmp NEAR PTR text_render_loop
text_store_loop2:
    push di
    mov ah,glyph_width
    mov bx,[si]
    xchg bh,bl
    shl bx,cl
text_store_loop3:
    mov al,dl
    shl bx,1
    jc SHORT text_store_next2
    mov al,dh
text_store_next2:
    stosb
    dec ah
    jnz SHORT text_store_loop3
    pop di
    add di,0140H
    add si,bp
    dec ch
    jnz SHORT text_store_loop2
    jmp SHORT text_store_loop
shake_spare DB 0CH,04H,0CH,0EH
shake_seed DW 04D2H                      ; Random state for the shake offsets.
shake_frames DB 00H                      ; Frames of shaking left.

; Slot 29: shake the screen for the given number of frames.
shake_start:
    push bp
    mov bp,sp
ASSUME DS:NOTHING
    mov al,[bp+06H]
    mov CS:shake_frames,al
    pop bp
    retf

; Slot 51: per-frame screen shake (from the timer).
shake_step:
    mov ax,CS:shake_seed
    shl ax,1
    shl ax,1
    add ax,CS:shake_seed
    inc ax
    mov CS:shake_seed,ax
    cmp BYTE PTR CS:shake_frames,00H
    jz SHORT shake_step_done
    and ah,03H
    dec BYTE PTR CS:shake_frames
    jnz SHORT shake_step_next
    xor ah,ah
shake_step_next:
    mov dx,03D4H
    mov al,0DH
    out dx,ax
shake_step_done:
    retf

; Slot 11: far address of a font's glyph data.
font_address:
    push bp
    mov bp,sp
    mov es,CS:data_seg
    xor ax,ax
    mov bx,[bp+06H]
    cmp bx,ES:[020H]
    ja SHORT font_address_done
    shl bx,1
    jz SHORT font_address_done
    lea ax,DS:[fonts]
    DB 026H,03H,087H,020H,00H ; add ax,[es:bx+0x20]
    mov dx,es
font_address_done:
    pop bp
    retf

; Slot 16: copy glyph data into a font slot.
font_copy:
    push bp
    mov bp,sp
    push si
    push di
    push es
    mov es,CS:data_seg
    mov si,[bp+08H]
    mov di,[bp+06H]
    cmp di,ES:[020H]
    ja SHORT font_copy_done
    shl di,1
    jz SHORT font_copy_done
    DB 026H,08BH,0BDH,020H,00H ; mov di,[es:di+0x20]
    add di,0630H
    mov cl,[si-07H]
    sub cl,[si-08H]
    inc cl
    xor ch,ch
    mov ah,[si-04H]
    add ah,[si-02H]
    mov al,[si-06H]
    mul ah
    mul cl
    add cx,08H
    add ax,cx
    sub si,cx
    sub di,cx
    mov cx,ax
    shr cx,1
    rep movsw
font_copy_done:
    pop es
    pop di
    pop si
    pop bp
    retf

; Slot 4: advance width of a character in a font.
char_width:
    push bp
    mov bp,sp
    push ds
    mov ds,CS:data_seg
    xor ax,ax
ASSUME DS:DATA
    mov cl,[bp+08H]
    or cl,cl
    js SHORT char_width_next
    mov bx,[bp+06H]
    cmp bx,WORD PTR DS:[fonts]
    ja SHORT char_width_next
    shl bx,1
    jz SHORT char_width_next
    mov bx,WORD PTR DS:[bx+fonts]
    add bx,0630H
    cmp cl,[bx-08H]
    jc SHORT char_width_next
    cmp cl,[bx-07H]
    ja SHORT char_width_next
    mov ah,[bx-03H]
    mov al,[bx-05H]
    or al,al
    jnz SHORT char_width_next
    mov al,cl
    mov cl,[bx-07H]
    sub cl,al
    xor ch,ch
    add bx,-09H
    sub bx,cx
    mov al,[bx]
char_width_next:
    add al,ah
    cbw
    pop ds
    pop bp
    retf

; Slot 12: line height of a font.
font_height:
    push bp
    mov bp,sp
    push ds
    xor ax,ax
    mov ds,CS:data_seg
    mov bx,[bp+06H]
    cmp bx,WORD PTR DS:[fonts]
    ja SHORT font_height_done
    shl bx,1
    jz SHORT font_height_done
    mov bx,WORD PTR DS:[bx+fonts]
    add bx,0630H
    mov al,[bx-04H]
    add al,[bx-02H]
font_height_done:
    pop ds
    pop bp
    retf

; Slot 21: draw a string, clipped on all sides.
page_string:
    push bp
    mov bp,sp
    push si
    push di
ASSUME DS:NOTHING
    mov bx,[bp+0CH]
    mov cx,[bp+08H]
    mov dx,[bp+0AH]
    mov bp,[bp+06H]
    push cs
    call NEAR PTR text_clip_all
    pop di
    pop si
    pop bp
    retf

; Slot 43: draw the prepared string clipped right, left and bottom.
text_clip_all:
    call NEAR PTR text_prepare
    call NEAR PTR text_cut_right
    call NEAR PTR text_skip_left
    call NEAR PTR text_cut_bottom
    jmp NEAR PTR text_draw_next

; Slot 44: draw the prepared string clipped on the left.
text_clip_left:
    call NEAR PTR text_prepare
    call NEAR PTR text_skip_left
    jmp NEAR PTR text_draw_next

; Drop the characters left of the clip edge.
text_skip_left:
    xor cx,cx
ASSUME DS:DATA
    cmp [bp+06H],cx
    js SHORT text_skip_left_next2
    cmp text_x,cx
    jns SHORT text_skip_left_done
    xchg text_x,cx
    xor ah,ah
    dec bx
text_skip_left_loop:
    inc bx
    mov al,SS:[bx]
    or al,al
    jz SHORT text_skip_left_next2
    js SHORT text_skip_left_loop
    cmp al,last_char
    ja SHORT text_skip_left_loop
    sub al,first_char
    jc SHORT text_skip_left_loop
    mov di,ax
    mov al,fixed_width
    or al,al
    jnz SHORT text_skip_left_next
    add di,width_table
    mov al,[di]
    add al,char_spacing
text_skip_left_next:
    add cx,ax
    jng SHORT text_skip_left_loop
    sub ax,cx
    mov skip_cols,al
text_skip_left_done:
    ret
text_skip_left_next2:
    pop ax
    jmp NEAR PTR text_done

; Slot 46: draw the prepared string clipped on the right.
text_clip_right:
    call NEAR PTR text_prepare
    call NEAR PTR text_cut_right
    jmp NEAR PTR text_draw_next

; Cut the string at the right clip edge.
text_cut_right:
    mov cx,[bp+06H]
    or cx,cx
    js SHORT text_cut_right_next2
    sub cx,text_x
    jl SHORT text_cut_right_next2
    inc cx
    xor ah,ah
    mov si,bx
    dec si
text_cut_right_loop:
    inc si
    mov al,SS:[si]
    or al,al
    jz SHORT text_cut_right_done
    js SHORT text_cut_right_loop
    cmp al,last_char
    ja SHORT text_cut_right_loop
    sub al,first_char
    jc SHORT text_cut_right_loop
    mov di,ax
    mov al,fixed_width
    or al,al
    jnz SHORT text_cut_right_next
    add di,width_table
    mov al,[di]
    add al,char_spacing
text_cut_right_next:
    sub cx,ax
    jg SHORT text_cut_right_loop
    neg cx
    mov cut_cols,cl
    inc si
    mov al,SS:[si]
    mov SS:[si],ah
    mov cut_char,al
    mov cut_at,si
text_cut_right_done:
    ret
text_cut_right_next2:
    pop ax
    jmp text_done

; Slot 47: draw the prepared string clipped at the bottom.
text_clip_bottom:
    call NEAR PTR text_prepare
    call NEAR PTR text_cut_bottom
    jmp text_draw_next

; Clip the glyph rows to the bottom edge.
text_cut_bottom:
    mov cx,[bp+08H]
    or cx,cx
    js SHORT text_cut_bottom_next2
    mov ax,text_y
    or ax,ax
    jns SHORT text_cut_bottom_next
    neg ax
    sub glyph_rows,ax
    jna SHORT text_cut_bottom_next2
    mov WORD PTR text_y,00H
    mul WORD PTR glyph_bytes
    add glyph_data,ax
text_cut_bottom_next:
    mov ax,text_y
    cmp ax,cx
    ja SHORT text_cut_bottom_next2
    add ax,glyph_rows
    dec ax
    sub ax,cx
    jna SHORT text_cut_bottom_done
    sub glyph_rows,ax
text_cut_bottom_done:
    ret
text_cut_bottom_next2:
    pop ax
    jmp text_done

; Slot 45: draw the prepared string unclipped.
text_draw:
    call NEAR PTR text_prepare
text_draw_next:
    mov ax,[bp+02H]
    add text_x,ax
    mov ax,[bp+04H]
    add text_y,ax
    mov si,bx
    call NEAR PTR text_render
text_done:
    mov bx,cut_at
    or bx,bx
    jz SHORT text_done_next
    mov al,cut_char
    mov SS:[bx],al
text_done_next:
    mov ax,text_x
    sub ax,[bp+02H]
    pop es
    pop ds
    retf

; Shared prologue: load font metrics and the string position (returns with DS at the driver data).
text_prepare:
    pop si
    push ds
    push es
    mov ds,CS:data_seg
    mov es,[bp+00H]
    mov text_x,cx
    mov text_y,dx
    xor ax,ax
    mov cut_at,ax
    mov skip_cols,al
    mov cut_cols,al
    cmp SS:[bx],al
    jz SHORT text_done
    mov di,[bp+010H]
    cmp di,WORD PTR DS:[fonts]
    ja SHORT text_done
    shl di,1
    jz SHORT text_done
    mov di,WORD PTR DS:[di+fonts]
    add di,0630H
    mov glyph_data,di
    mov al,[di-04H]
    test BYTE PTR [bp+0AH],01H
    jz SHORT text_prepare_next
    add al,[di-02H]
text_prepare_next:
    cbw
    mov glyph_rows,ax
    mov ah,[di-03H]
    mov char_spacing,ah
    mov al,[di-05H]
    or al,al
    jz SHORT text_prepare_next2
    add al,ah
text_prepare_next2:
    mov fixed_width,al
    mov ah,[di-08H]
    mov first_char,ah
    mov al,[di-07H]
    mov last_char,al
    sub al,ah
    xor ah,ah
    mov cl,[di-06H]
    dec cl
    mov glyph_shift,cl
    add di,-09H
    sub di,ax
    mov width_table,di
    inc ax
    shl ax,cl
    mov glyph_bytes,ax
    jmp si
CODE ENDS

DATA SEGMENT
ASSUME DS:DATA
reserved DB 16 DUP (0)
copyright DB 'Copyright (c) 1991 by MicroProse Software, All Rights Reserved.',0
default_palette DB 00,00,00, 00,00,42, 00,42,00, 00,42,42 ; The 128 default colours (6-bit RGB).
DB 42,00,00, 42,00,42, 42,21,00, 42,42,42
DB 21,21,21, 21,21,63, 21,63,21, 21,63,63
DB 63,21,21, 63,21,63, 63,63,21, 63,63,63
DB 00,00,00, 10,10,10, 21,21,21, 31,31,31
DB 42,42,42, 53,53,53, 63,63,63, 63,63,63
DB 63,63,63, 63,63,63, 63,63,63, 63,63,63
DB 63,63,63, 63,63,63, 63,63,63, 63,63,63
DB 21,00,00, 31,10,10, 42,21,21, 42,00,00
DB 42,10,00, 53,10,10, 53,21,10, 53,31,31
DB 63,21,21, 63,42,42, 63,63,63, 63,63,63
DB 63,63,63, 63,63,63, 63,63,63, 63,63,63
DB 21,10,00, 31,21,10, 42,31,21, 42,21,00
DB 53,31,10, 53,42,31, 63,42,21, 63,63,63
DB 21,21,00, 31,31,10, 21,31,00, 42,42,21
DB 53,53,31, 53,42,10, 63,63,42, 63,63,21
DB 00,21,00, 21,31,21, 10,31,10, 31,42,31
DB 21,42,21, 31,42,10, 00,42,00, 00,42,21
DB 31,53,31, 31,53,10, 10,53,10, 10,53,31
DB 42,63,42, 42,63,21, 21,63,21, 21,63,42
DB 00,21,21, 10,31,31, 21,42,42, 00,42,42
DB 31,53,53, 10,53,53, 42,63,63, 21,63,63
DB 63,63,63, 63,63,63, 63,63,63, 63,63,63
DB 63,63,63, 63,63,63, 63,63,63, 63,63,63
DB 00,00,21, 10,10,31, 21,21,42, 00,21,42
DB 00,00,42, 21,00,42, 10,10,53, 10,31,53
DB 31,10,53, 31,31,53, 21,21,63, 21,42,63
DB 42,21,63, 42,42,63, 63,63,63, 63,63,63
DB 21,10,21, 21,00,21, 31,21,31, 31,10,31
DB 42,21,42, 42,00,42, 53,31,53, 53,10,53
DB 63,21,63, 63,42,63, 63,63,63, 42,10,21
DB 42,00,21, 53,21,31, 53,10,31, 63,21,42
ega_colours DB 48 DUP (0)                ; The 16 colours of the current EGA palette.
cursor_sprite DW 00H                     ; Segment of the cursor sprite.
cursor_x DW 00H
cursor_y DW 00H
cursor_width DW 00H
cursor_height DW 00H
under_x DW 00H                           ; Saved rectangle (clipped to the screen).
under_y DW 00H
under_width DW 00H
under_height DW 00H
cursor_shown DW 00H
cursor_page DW 00H
under_cursor DB 512 DUP (0)              ; Pixels the cursor covers.
row_offsets LABEL WORD                   ; y * 320 for each row.
value = 0
REPT 200
    DW value
    value = value + 320
ENDM
page_status DW 00H,01H,00H,00H,00H       ; page_ready(n) reads the word at byte n; CIV only asks for page 0 (0 here).
page_segs DW 8 DUP (0A000H)              ; Segment of each page; all the screen until set.
raster_seg DW 0A000H                     ; Page that raster output goes to.
raster_left DW 00H
raster_top DW 00H
raster_right DW 013FH
data_seg DW SEG DATA                ; This segment (relocated by DOS).
raster_colour_value DB 00H
raster_op DB 00H
raster_opcode DB 00H
spare DB 00H
no_memory_text DB 'Out of memory$'
no_mcga_text DB 'MCGA is not supported on your machine$'
line_x0 DW 00H
line_y0 DW 032H
line_x1 DW 064H
line_y1 DW 00H
line_minor DW 00H                        ; Step along the minor axis.
text_colour DB 00H
text_background DB 00H
text_opaque DB 00H
glyph_width DB 00H
glyph_data DW 00H
width_table DW 00H
glyph_bytes DW 00H
text_x DW 00H
text_y DW 00H
fixed_width DB 00H
char_spacing DB 00H
glyph_shift DB 00H
first_char DB 00H
last_char DB 00H
skip_cols DB 00H
cut_cols DB 00H
glyph_rows DW 00H
cut_char DB 00H
cut_at DW 00H
DATA ENDS

STACK SEGMENT PARA STACK 'STACK'
DB 16 DUP (?)
STACK ENDS
END start
