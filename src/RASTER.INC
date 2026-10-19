; Included by GRAPHICS.ASM in F0000: raster [0BFA,108E).
; Preserve original register, segment, stack and interrupt behavior.
ASSUME CS:F0000
EXTRN raster_plot:FAR
EXTRN raster_spans:FAR
EXTRN raster_flush:FAR
EXTRN raster_colour:FAR
EXTRN raster_line:FAR
EXTRN raster_page:FAR
EXTRN raster_target:FAR
PUBLIC _rect_fill
PUBLIC _world_draw
PUBLIC _page_line
_rect_fill:
    push bp
    mov bp,sp
    push di
    push si
    xor ax,ax
    cmp [bp+0CH],ax
    jng SHORT rs_fill_done
    cmp [bp+0EH],ax
    jng SHORT rs_fill_done
    call NEAR PTR rs_reset_edges
    mov bx,[bp+06H]
    mov ax,[bx]
    call FAR PTR raster_page
    mov si,[bp+0AH]
    add si,[bx+04H]
    mov DS:[f0_first_row],si
    mov cx,[bp+0EH]
    shl si,1
    lea di,DS:[f0_left_edges]
    add di,si
    mov ax,[bp+08H]
    add ax,[bx+02H]
    mov dx,cx
    rep stosw
    mov cx,dx
    lea di,DS:[f0_right_edges]
    add di,si
    mov ax,[bp+08H]
    add ax,[bx+02H]
    add ax,[bp+0CH]
    dec ax
    rep stosw
    mov ax,[bp+010H]
    call FAR PTR raster_colour
    mov ax,DS:[f0_first_row]
    mov cx,ax
    add cx,[bp+0EH]
    dec cx
    mov DS:[f0_last_row],cx
    mov bx,05534H
    call FAR PTR raster_spans
    call FAR PTR raster_flush
rs_fill_done:
    pop si
    pop di
    pop bp
    retf
DB 1 DUP (0) ; original inter-function alignment
rs_flush_edges:
    push di
    push si
    push bp
    mov ax,DS:[f0_first_row]
    mov cx,DS:[f0_last_row]
    mov bx,05534H
    call FAR PTR raster_spans
    pop bp
    pop si
    pop di
    retf
rs_clear_edges:
    push di
    call NEAR PTR rs_reset_edges
    pop di
    retf
rs_reset_edges:
    mov ax,SEG _F2B01
    mov es,ax
    mov di,DS:[f0_first_row]
    or di,di
    js SHORT rs_reset_done
    mov cx,DS:[f0_last_row]
    inc cx
    sub cx,di
    shl di,1
    mov bx,cx
    mov dx,di
    add di,05534H
    mov ax,0FFFFH
    rep stosw
    mov DS:[f0_first_row],ax
    mov cx,bx
    mov di,dx
    add di,056C4H
    sub ax,ax
    rep stosw
    mov DS:[f0_last_row],ax
rs_reset_done:
    ret
rs_vertical_edge:
    mov ax,SEG _F2B01
    mov es,ax
    mov cx,di
    or si,si
    lea di,DS:[f0_left_edges]
    jz SHORT rs_vert_order
    cmp si,DS:[f0_clip_xmax]
    jnz SHORT rs_vert_done
    lea di,DS:[f0_right_edges]
rs_vert_order:
    cmp cx,dx
    jnl SHORT rs_vert_clip_top
    xchg cx,dx
rs_vert_clip_top:
    or dx,dx
    jns SHORT rs_vert_clip_bottom
    sub dx,dx
rs_vert_clip_bottom:
    cmp cx,DS:[f0_clip_ymax]
    jng SHORT rs_vert_origin
    mov cx,DS:[f0_clip_ymax]
rs_vert_origin:
    add si,DS:[f0_origin_x]
    add cx,DS:[f0_origin_y]
    add dx,DS:[f0_origin_y]
    cmp cx,DS:[f0_last_row]
    jna SHORT rs_vert_first
    mov DS:[f0_last_row],cx
rs_vert_first:
    cmp dx,DS:[f0_first_row]
    jnc SHORT rs_vert_store
    mov DS:[f0_first_row],dx
rs_vert_store:
    add di,dx
    add di,dx
    sub cx,dx
    inc cx
    mov ax,si
    rep stosw
rs_vert_done:
    ret
rs_choose_edge:
    sub si,si
    or bx,bx
    js SHORT rs_vertical_edge
    mov si,DS:[f0_clip_xmax]
    cmp bx,si
    jg SHORT rs_vertical_edge
    ret
rs_raster_edges:
    mov ax,DS:[f0_origin_x]
    add DS:[f0_line_x0],ax
    add DS:[f0_line_x1],ax
    mov ax,DS:[f0_origin_y]
    add DS:[f0_line_y0],ax
    add DS:[f0_line_y1],ax
    mov ax,DS:[f0_line_x1]
    cmp ax,DS:[f0_line_x0]
    jnc SHORT rs_edge_ordered
    mov bx,DS:[f0_line_x0]
    mov DS:[f0_line_x1],bx
    mov DS:[f0_line_x0],ax
    mov ax,DS:[f0_line_y1]
    mov bx,DS:[f0_line_y0]
    mov DS:[f0_line_y1],bx
    mov DS:[f0_line_y0],ax
rs_edge_ordered:
    mov bx,DS:[f0_line_x1]
    sub bx,DS:[f0_line_x0]
    mov DS:[f0_raster_dx],bx
    mov ax,DS:[f0_line_y1]
    mov dx,ax
    mov cx,DS:[f0_line_y0]
    sub ax,cx
    mov bp,02H
    jns SHORT rs_edge_down
    xchg cx,dx
    neg bp
    neg ax
rs_edge_down:
    mov DS:[f0_raster_dy],ax
    cmp dx,DS:[f0_last_row]
    jna SHORT rs_edge_first
    mov DS:[f0_last_row],dx
rs_edge_first:
    cmp cx,DS:[f0_first_row]
    jnc SHORT rs_edge_slope
    mov DS:[f0_first_row],cx
rs_edge_slope:
    cmp ax,bx
    jnc SHORT rs_ymajor
    mov di,DS:[f0_line_y0]
    shl di,1
    mov ax,DS:[f0_line_x0]
    mov dx,DS:[f0_raster_dx]
    mov cx,dx
    mov bx,cx
    inc bx
    shr bx,1
    neg bx
    mov si,DS:[f0_raster_dy]
    jmp SHORT rs_xmajor_left
    nop
rs_xmajor_right:
    cmp ax,[di+056C4H]
    jna SHORT rs_xmajor_step
    mov [di+056C4H],ax
rs_xmajor_step:
    inc ax
    sub bx,dx
    add di,bp
rs_xmajor_left:
    cmp ax,[di+05534H]
    jnc SHORT rs_xmajor_next
    mov [di+05534H],ax
rs_xmajor_next:
    dec cx
    js SHORT rs_xmajor_end
    add bx,si
    jns SHORT rs_xmajor_right
    inc ax
    jmp SHORT rs_xmajor_next
rs_xmajor_end:
    cmp ax,[di+056C4H]
    jna SHORT rs_xmajor_done
    mov [di+056C4H],ax
rs_xmajor_done:
    ret
rs_ymajor:
    mov di,DS:[f0_line_y0]
    shl di,1
    mov ax,DS:[f0_line_x0]
    mov dx,DS:[f0_raster_dy]
    mov cx,dx
    mov bx,cx
    inc bx
    shr bx,1
    neg bx
    mov si,DS:[f0_raster_dx]
rs_ymajor_left:
    cmp ax,[di+05534H]
    jnc SHORT rs_ymajor_right
    mov [di+05534H],ax
rs_ymajor_right:
    cmp ax,[di+056C4H]
    jna SHORT rs_ymajor_next
    mov [di+056C4H],ax
rs_ymajor_next:
    dec cx
    js SHORT rs_ymajor_done
    add di,bp
    add bx,si
    js SHORT rs_ymajor_left
    sub bx,dx
    inc ax
    jmp SHORT rs_ymajor_left
rs_ymajor_done:
    ret
DB 1 DUP (0) ; original inter-function alignment
rs_clip_far:
    call NEAR PTR rs_clip_line
    retf
rs_clip_saved:
    push bp
    push si
    push di
    push es
    call NEAR PTR rs_clip_line
    pop es
    pop di
    pop si
    pop bp
    retf
rs_clip_line:
    sub ax,ax
    mov es,ax
    push WORD PTR ES:[00H]
    push WORD PTR ES:[02H]
; A code offset: no DGROUP frame for this explicit DS: operand.
ASSUME DS:NOTHING
    lea ax,DS:[rs_divide_handler]
ASSUME DS:DGROUP
    mov ES:[00H],ax
    mov WORD PTR ES:[2],SEG _F0000
    push ds
    pop es
    jmp SHORT rs_clip_start
    nop
rs_clip_draw:
    sub ax,ax
    mov es,ax
    pop WORD PTR ES:[02H]
    pop WORD PTR ES:[00H]
    push ds
    pop es
    mov ax,DS:[f0_line_x0]
    mov bx,DS:[f0_line_y0]
    mov cx,DS:[f0_line_x1]
    mov dx,DS:[f0_line_y1]
    call FAR PTR raster_line
    clc
    ret
rs_clip_reject:
    sub ax,ax
    mov es,ax
    pop WORD PTR ES:[02H]
    pop WORD PTR ES:[00H]
    push ds
    pop es
    stc
    ret
rs_clip_dx_overflow:
    cmc
    rcr dx,1
    mov DS:[f0_clip_dx],dx
    sar dx,1
    mov DS:[f0_clip_dx_half],dx
    mov dx,di
    sub dx,bp
    jno SHORT rs_clip_dy_half
    cmc
    rcr dx,1
    jmp SHORT rs_clip_dy
    nop
rs_clip_dy_half:
    sar dx,1
    jmp SHORT rs_clip_dy
    nop
rs_clip_dy_overflow:
    cmc
    rcr dx,1
    sar WORD PTR DS:[f0_clip_dx],1
    sar WORD PTR DS:[f0_clip_dx_half],1
    jmp SHORT rs_clip_dy
    nop
rs_clip_reject_near:
    jmp SHORT rs_clip_reject
rs_clip_start:
    mov cx,DS:[f0_line_x0]
    mov dx,DS:[f0_line_y0]
    mov si,DS:[f0_line_x1]
    mov di,DS:[f0_line_y1]
    mov bx,cx
    mov bp,dx
    call NEAR PTR rs_outcode
    mov DS:[f0_clip_flags],al
    mov bx,si
    mov bp,di
    call NEAR PTR rs_outcode
    jnz SHORT rs_clip_test
    cmp BYTE PTR DS:[f0_clip_flags],00H
    jnz SHORT rs_clip_swap
    jmp NEAR PTR rs_clip_draw
rs_clip_swap:
    xchg cx,si
    xchg dx,di
    xchg DS:[f0_clip_flags],al
    mov DS:[f0_line_x0],cx
    mov DS:[f0_line_y0],dx
rs_clip_test:
    test DS:[f0_clip_flags],al
    jnz SHORT rs_clip_reject_near
    mov bp,dx
    mov dx,si
    sub dx,cx
    jo SHORT rs_clip_dx_overflow
    mov DS:[f0_clip_dx],dx
    sar dx,1
    mov DS:[f0_clip_dx_half],dx
    mov dx,di
    sub dx,bp
    jo SHORT rs_clip_dy_overflow
rs_clip_dy:
    mov DS:[f0_clip_dy],dx
    sar dx,1
    mov DS:[f0_clip_dy_half],dx
rs_clip_loop:
    test al,09H
    jz SHORT rs_clip_horizontal
    sub bx,bx
    or si,si
    js SHORT rs_clip_at_x
    mov bx,DS:[f0_clip_xmax]
rs_clip_at_x:
    mov ax,bx
    sub ax,cx
    imul WORD PTR DS:[f0_clip_dy]
    push bx
    mov bx,dx
    idiv WORD PTR DS:[f0_clip_dx]
    mov bl,bh
    xor bl,DS:[f0_clip_dx_sign]
    jns SHORT rs_clip_round_y
    neg dx
    dec ax
rs_clip_round_y:
    sub dx,DS:[f0_clip_dx_half]
    xor dh,bh
    js SHORT rs_clip_y_done
    inc ax
rs_clip_y_done:
    pop bx
    add ax,bp
    js SHORT rs_clip_top
    cmp ax,DS:[f0_clip_ymax]
    jng SHORT rs_clip_point
rs_clip_horizontal:
    mov bx,DS:[f0_clip_ymax]
    cmp di,bx
    jg SHORT rs_clip_at_y
rs_clip_top:
    sub bx,bx
rs_clip_at_y:
    mov ax,bx
    sub ax,bp
    imul WORD PTR DS:[f0_clip_dx]
    push bx
    mov bx,dx
rs_idiv_y:
    idiv WORD PTR DS:[f0_clip_dy]
    mov bl,bh
    xor bl,DS:[f0_clip_dy_sign]
    jns SHORT rs_clip_round_x
    neg dx
    dec ax
rs_clip_round_x:
    sub dx,DS:[f0_clip_dy_half]
    xor dh,bh
    js SHORT rs_clip_x_done
    inc ax
rs_clip_x_done:
    pop bx
    add ax,cx
    js SHORT rs_clip_outside
    cmp ax,DS:[f0_clip_xmax]
    jg SHORT rs_clip_outside
    xchg ax,bx
rs_clip_point:
    cmp BYTE PTR DS:[f0_clip_flags],00H
    jnz SHORT rs_clip_first
    mov DS:[f0_line_y1],ax
    mov DS:[f0_line_x1],bx
    jmp NEAR PTR rs_clip_draw
rs_clip_outside:
    jmp NEAR PTR rs_clip_reject
rs_clip_first:
    mov DS:[f0_line_y0],ax
    mov DS:[f0_line_x0],bx
    xchg cx,si
    xchg bp,di
    mov al,DS:[f0_clip_flags]
    mov BYTE PTR DS:[f0_clip_flags],00H
    jmp NEAR PTR rs_clip_loop
rs_outcode:
    mov al,0FH
    or bx,bx
    js SHORT rs_out_right
    and al,0F7H
rs_out_right:
    cmp bx,DS:[f0_clip_xmax]
    jg SHORT rs_out_top
    and al,0FEH
rs_out_top:
    or bp,bp
    js SHORT rs_out_bottom
    and al,0FBH
rs_out_bottom:
    cmp bp,DS:[f0_clip_ymax]
    jg SHORT rs_out_done
    and al,0FDH
rs_out_done:
    or al,al
    ret
rs_divide_handler:
    push bp
    mov bp,sp
    cmp WORD PTR [bp+2],OFFSET rs_idiv_y
    mov ax,DS:[f0_clip_dy]
    jz SHORT rs_div_sign
    mov ax,DS:[f0_clip_dx]
rs_div_sign:
    xor dx,ax
    mov ax,07F00H
    jns SHORT rs_div_skip
    neg ax
rs_div_skip:
    add WORD PTR [bp+02H],04H
    sub dx,dx
    pop bp
    iret
_page_line:
    push bp
    mov bp,sp
    push si
    push di
    mov bx,[bp+06H]
    mov ax,[bx+06H]
    mov DS:[f0_clip_xmax],ax
    mov ax,[bx+08H]
    mov DS:[f0_clip_ymax],ax
    call FAR PTR raster_target
    mov ax,[bp+08H]
    mov DS:[f0_line_x0],ax
    mov ax,[bp+0AH]
    mov DS:[f0_line_y0],ax
    mov ax,[bp+0CH]
    mov DS:[f0_line_x1],ax
    mov ax,[bp+0EH]
    mov DS:[f0_line_y1],ax
    mov ax,[bp+010H]
    call FAR PTR raster_colour
    call NEAR PTR rs_clip_line
    call FAR PTR raster_flush
    pop di
    pop si
    pop bp
    retf
_world_draw:
    push bp
    mov bp,sp
    push si
    push di
    mov bx,[bp+06H]
    mov cx,[bp+08H]
    or cx,cx
    js SHORT rs_world_done
    cmp cx,[bx+06H]
    jg SHORT rs_world_done
    mov dx,[bp+0AH]
    or dx,dx
    js SHORT rs_world_done
    cmp dx,[bx+08H]
    jg SHORT rs_world_done
    call FAR PTR raster_target
    mov ax,[bp+0CH]
    call FAR PTR raster_colour
    mov ax,cx
    mov bx,dx
    call FAR PTR raster_plot
    call FAR PTR raster_flush
rs_world_done:
    pop di
    pop si
    pop bp
    retf
ASSUME CS:NOTHING
