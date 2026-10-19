; Included by GRAPHICS.ASM in F0000: mouse [163E,182A).
; Preserve original register, segment, stack and interrupt behavior.
ASSUME CS:F0000
EXTRN cursor_erase:FAR
EXTRN cursor_set:FAR
EXTRN cursor_draw:FAR
EXTRN cursor_erase_vector:DWORD
EXTRN cursor_set_vector:DWORD
PUBLIC _mouse_close
PUBLIC _mouse_hide
PUBLIC _init_mouse
PUBLIC _mouse_poll
PUBLIC _set_mousecur
PUBLIC _mouse_show
PUBLIC _set_text_pos
PUBLIC mouse_driver_poll
_init_mouse:
    xor ax,ax
    mov es,ax
    mov ax,ES:[0CCH]
    or ax,ES:[0CEH]
    jz SHORT ms_init_done
    mov ax,00H
    int 033H
    or ax,ax
    jz SHORT ms_init_done
    mov ax,0CH
    mov cx,01FH
    mov dx,cs
    mov es,dx
; A code offset: no DGROUP frame for this explicit DS: operand.
ASSUME DS:NOTHING
    lea dx,DS:[mouse_event]
ASSUME DS:DGROUP
    int 033H
    mov ax,03H
    int 033H
    mov ax,cx
    mov cl,ch
    shr ax,cl
    mov DS:[f0_mouse_shift],cl
    mov DS:[f0_mouse_x],ax
    mov DS:[f0_mouse_y],dx
    mov DS:[f0_mouse_buttons],bx
    mov ax,0FFFFH
ms_init_done:
    mov DS:[f0_mouse_active],al
    retf
_mouse_close:
    xor al,al
    xchg DS:[f0_mouse_active],al
    or al,al
    jz SHORT ms_close_done
    mov ax,00H
    int 033H
ms_close_done:
    retf
_set_mousecur:
    push bp
    mov bp,sp
    mov ax,[bp+06H]
    mov DS:[f0_cursor_x],ax
    mov ax,[bp+08H]
    mov DS:[f0_cursor_y],ax
    mov ax,[bp+0AH]
    mov DS:[f0_cursor_picture],ax
    pop bp
    retf
_set_text_pos:
    push bp
    mov bp,sp
    mov ax,[bp+06H]
    mov dx,[bp+08H]
    mov DS:[f0_mouse_x],ax
    mov DS:[f0_mouse_y],dx
    cmp BYTE PTR DS:[f0_mouse_active],00H
    jz SHORT ms_pos_done
    mov cl,DS:[f0_mouse_shift]
    shl ax,cl
    mov cx,ax
    mov ax,04H
    int 033H
ms_pos_done:
    pop bp
    retf
_mouse_poll:
    xor ax,ax
    xchg DS:[f0_mouse_events],ax
    retf
_mouse_show:
    cmp WORD PTR DS:[f0_cursor_picture],00H
    jz SHORT ms_show_done
    cmp BYTE PTR DS:[f0_driver_deferred],00H
    jnz SHORT ms_show_done
    push WORD PTR DS:[f0_cursor_picture]
    mov ax,DS:[f0_mouse_y]
    sub ax,DS:[f0_cursor_y]
    push ax
    mov ax,DS:[f0_mouse_x]
    sub ax,DS:[f0_cursor_x]
    push ax
    call FAR PTR cursor_set
    add sp,06H
    mov BYTE PTR DS:[f0_driver_deferred],01H
ms_show_done:
    retf
_mouse_hide:
    cmp WORD PTR DS:[f0_cursor_picture],00H
    jz SHORT ms_hide_done
    cmp BYTE PTR DS:[f0_driver_deferred],00H
    jz SHORT ms_hide_done
    mov BYTE PTR DS:[f0_driver_deferred],00H
    call FAR PTR cursor_erase
ms_hide_done:
    retf
mc_saved_deferred DB 0 ; saved cursor/driver deferred state
mc_cursor_begin:
    push bp
    mov bp,sp
    cmp WORD PTR DS:[f0_cursor_picture],00H
    jz SHORT ms_begin_done
    cmp BYTE PTR DS:[f0_driver_deferred],00H
    jz SHORT ms_begin_done
    mov al,DS:[f0_driver_deferred]
    mov BYTE PTR DS:[f0_driver_deferred],00H
    mov CS:mc_saved_deferred,al
    mov ax,[bp+06H]
    call FAR PTR cursor_draw
ms_begin_done:
    pop bp
    retf
mc_cursor_end:
    cmp WORD PTR DS:[f0_cursor_picture],00H
    jz SHORT ms_end_done
    cmp BYTE PTR CS:mc_saved_deferred,00H
    jz SHORT ms_end_done
    call NEAR PTR mouse_driver_poll
    mov al,CS:mc_saved_deferred
    mov DS:[f0_driver_deferred],al
ms_end_done:
    retf
mc_call:
    push bp
    mov bp,sp
    mov ax,[bp+06H]
    mov bx,[bp+08H]
    mov cx,[bp+0AH]
    mov dx,[bp+0CH]
    int 033H
    mov DS:[f0_mouse_result],ax
    mov DS:[f0_mouse_result_bx],bx
    mov DS:[f0_mouse_result_cx],cx
    mov DS:[f0_mouse_result_dx],dx
    cmp WORD PTR [bp+06H],06H
    ja SHORT ms_call_done
    mov cl,DS:[f0_mouse_shift]
    shr WORD PTR DS:[f0_mouse_result_cx],cl
ms_call_done:
    lea ax,DS:[f0_mouse_result]
    pop bp
    retf
mouse_driver_poll:
    push ax
    push dx
    push es
    mov ax,SEG _F0000
    mov es,ax
    call DWORD PTR ES:[cursor_erase_vector]
    push WORD PTR DS:[f0_cursor_picture]
    mov ax,DS:[f0_mouse_y]
    sub ax,DS:[f0_cursor_y]
    push ax
    mov ax,DS:[f0_mouse_x]
    sub ax,DS:[f0_cursor_x]
    push ax
    mov ax,SEG _F0000
    mov es,ax
    call DWORD PTR ES:[cursor_set_vector]
    add sp,06H
    pop es
    pop dx
    pop ax
    ret
mc_set_callback:
    push bp
    mov bp,sp
    les ax,DWORD PTR [bp+06H]
    mov DS:[f0_mouse_callback],ax
    mov DS:[f0_mouse_callback_segment],es
    pop bp
    retf
mouse_event:
    mov bp,SEG _F2B01
    mov ds,bp
    mov bp,cx
    mov cl,DS:[f0_mouse_shift]
    shr bp,cl
    mov cx,bp
    or DS:[f0_mouse_events],bx
    mov DS:[f0_mouse_buttons],bx
    mov DS:[f0_mouse_x],cx
    mov DS:[f0_mouse_y],dx
    mov bp,DS:[f0_mouse_callback]
    or bp,DS:[f0_mouse_callback_segment]
    mov bp,ax
    jz SHORT ms_event_cursor
    call DWORD PTR DS:[f0_mouse_callback]
ms_event_cursor:
    test bp,01H
    jz SHORT ms_event_done
    cmp BYTE PTR DS:[f0_driver_deferred],00H
    jz SHORT ms_event_done
    mov al,02H
    xchg DS:[f0_driver_pending],al
    or al,al
    jnz SHORT ms_event_done
    call NEAR PTR mouse_driver_poll
    mov BYTE PTR DS:[f0_driver_pending],00H
ms_event_done:
    retf
ASSUME CS:NOTHING
