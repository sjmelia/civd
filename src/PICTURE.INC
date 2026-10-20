; Included by GRAPHICS.ASM in F0000: picture [108E,163E).
; Preserve original register, segment, stack and interrupt behavior.
ASSUME CS:F0000
EXTRN _apply_palette:FAR
PUBLIC _image_begin
PUBLIC _image_byte
PUBLIC _image_finish
PUBLIC _begin_picture
PUBLIC _copy_picture
PUBLIC _decode_scanline
PUBLIC _picture_target
_picture_target:
    push bp
    mov bp,sp
    push si
    push di
    xor ax,ax
    push ax
    call FAR PTR _apply_palette
    add sp,02H
pic_next_chunk:
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_chunk_tag
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_chunk_tag:
    lodsw
    mov DS:[f0_stream_pointer],si
    cmp al,058H
    jz SHORT pic_image_chunk
    mov di,ds
    mov es,di
    lea di,DS:[f0_picture_table]
    cmp ax,0304DH
    jnz SHORT pic_table_target
    cmp WORD PTR [bp+06H],01H
    jc SHORT pic_second_table
    jz SHORT pic_table_target
    mov di,[bp+06H]
    jmp SHORT pic_table_target
    nop
pic_second_table:
    add di,02H
pic_table_target:
    push di
    stosw
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_table_length
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_table_length:
    lodsw
    mov DS:[f0_stream_pointer],si
    stosw
    mov cx,ax
    shr cx,1
    jcxz SHORT pic_table_done
pic_table_loop:
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_table_word
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_table_word:
    lodsw
    mov DS:[f0_stream_pointer],si
    stosw
    loop SHORT pic_table_loop
pic_table_done:
    pop di
    lea ax,DS:[f0_picture_table]
    cmp ax,di
    jnz SHORT pic_chunk_done
    push ax
    call FAR PTR _apply_palette
    add sp,02H
pic_chunk_done:
    jmp NEAR PTR pic_next_chunk
pic_image_chunk:
    and ah,01H
    mov DS:[f0_decode_nibbles],ah
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_image_size
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_image_size:
    lodsw
    mov DS:[f0_stream_pointer],si
    mov DS:[f0_picture_size],ax
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_image_width
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_image_width:
    lodsw
    mov DS:[f0_stream_pointer],si
    mov DS:[f0_picture_width],ax
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_image_height
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_image_height:
    lodsw
    mov DS:[f0_stream_pointer],si
    mov DS:[f0_picture_height],ax
    call NEAR PTR pc_decode_init
    pop di
    pop si
    pop bp
    retf
_begin_picture:
    push si
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_begin_mode
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_begin_mode:
    lodsw
    mov DS:[f0_stream_pointer],si
    and al,01H
    mov DS:[f0_decode_nibbles],al
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_begin_size
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_begin_size:
    lodsw
    mov DS:[f0_stream_pointer],si
    mov DS:[f0_picture_size],ax
    call NEAR PTR pc_decode_init
    pop si
    retf
_copy_picture:
    push bp
    mov bp,sp
    push si
    push di
    mov si,DS:[f0_stream_pointer]
    les di,DWORD PTR [bp+06H]
    mov cx,[bp+0AH]
    call NEAR PTR pc_decode_bytes
    mov DS:[f0_stream_pointer],si
    pop di
    pop si
    pop bp
    retf
_decode_scanline:
    push bp
    mov bp,sp
    push si
    push di
    mov ax,ds
    mov es,ax
    mov si,DS:[f0_stream_pointer]
    mov di,[bp+06H]
    mov cx,DS:[f0_picture_width]
    call NEAR PTR pc_decode_bytes
    mov DS:[f0_stream_pointer],si
    pop di
    pop si
    pop bp
    retf
pc_decode_init:
    mov ax,DS:[f0_picture_width]
    or ax,DS:[f0_picture_height]
    jnz SHORT pic_init_decoder
    ret
pic_init_decoder:
    mov BYTE PTR DS:[f0_repeat_count],00H
    mov BYTE PTR DS:[f0_repeat_byte],00H
    lea ax,DS:[f0_decode_stack_top]
    mov DS:[f0_decode_stack],ax
    mov si,DS:[f0_stream_pointer]
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_init_limit
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_init_limit:
    lodsw
    mov DS:[f0_stream_pointer],si
    cmp al,0BH
    jna SHORT pic_set_limit
    mov al,0BH
pic_set_limit:
    mov DS:[f0_decode_limit],al
    mov DS:[f0_decode_word],ax
    mov BYTE PTR DS:[f0_decode_word_bits],08H
pic_reset_codes:
    mov BYTE PTR DS:[f0_decode_bits],09H
    mov WORD PTR DS:[f0_decode_mask],01FFH
    mov dx,0100H
    mov DS:[f0_decode_next_code],dx
    mov ax,0FFFFH
    xor bx,bx
    mov cx,0800H
pic_clear_prefix:
    mov [bx-036DAH],ax
    add bx,03H
    loop SHORT pic_clear_prefix
    mov al,00H
    xor bx,bx
    mov cx,0100H
pic_set_suffix:
    mov [bx-036D8H],al
    inc al
    add bx,03H
    loop SHORT pic_set_suffix
    ret
pc_decode_bytes:
    cmp BYTE PTR DS:[f0_decode_nibbles],00H
    jz SHORT pic_bytes_count
    inc cx
    shr cx,1
pic_bytes_count:
    mov DS:[f0_decode_count],cx
    mov dx,DS:[f0_decode_next_code]
    xchg DS:[f0_decode_stack],sp
pic_bytes_loop:
    cmp BYTE PTR DS:[f0_repeat_count],00H
    jnz SHORT pic_rle_repeat
    call NEAR PTR pc_decode_next
    cmp al,090H
    jz SHORT pic_rle_escape
    mov DS:[f0_repeat_byte],al
    jmp SHORT pic_store
    nop
pic_rle_escape:
    call NEAR PTR pc_decode_next
    or al,al
    jnz SHORT pic_rle_count
    mov al,090H
    mov DS:[f0_repeat_byte],al
    jmp SHORT pic_store
    nop
pic_rle_count:
    dec al
    mov DS:[f0_repeat_count],al
pic_rle_repeat:
    mov al,DS:[f0_repeat_byte]
    dec BYTE PTR DS:[f0_repeat_count]
pic_store:
    cmp BYTE PTR DS:[f0_decode_nibbles],00H
    jz SHORT pic_store_byte
    mov ah,al
    and al,0FH
    shr ah,1
    shr ah,1
    shr ah,1
    shr ah,1
    stosw
    dec WORD PTR DS:[f0_decode_count]
    jnz SHORT pic_bytes_loop
    jmp SHORT pic_bytes_done
    nop
pic_store_byte:
    stosb
    dec WORD PTR DS:[f0_decode_count]
    jnz SHORT pic_bytes_loop
pic_bytes_done:
    mov DS:[f0_decode_next_code],dx
    xchg DS:[f0_decode_stack],sp
    ret
pc_decode_next:
    pop bp
    cmp sp,06A7DH
    jz SHORT pic_next_code
pic_next_pop:
    pop ax
    jmp bp
pic_next_code:
    mov bx,DS:[f0_decode_word]
    mov cl,010H
    mov ch,DS:[f0_decode_word_bits]
    sub cl,ch
    shr bx,cl
    mov cl,ch
pic_fill_bits:
    cmp cl,DS:[f0_decode_bits]
    jnl SHORT pic_have_code
    cmp si,DS:[f0_stream_end]
    jc SHORT pic_load_word
    push bx
    push cx
    push dx
    call DWORD PTR DS:[f0_stream_refill]
    pop dx
    pop cx
    pop bx
    mov si,DS:[f0_stream_pointer]
pic_load_word:
    lodsw
    mov DS:[f0_decode_word],ax
    shl ax,cl
    or bx,ax
    add cl,010H
    jmp SHORT pic_fill_bits
pic_have_code:
    sub cl,DS:[f0_decode_bits]
    mov DS:[f0_decode_word_bits],cl
    mov ax,bx
    and ax,DS:[f0_decode_mask]
    mov cx,ax
    cmp ax,dx
    jl SHORT pic_walk_chain
    mov cx,dx
    mov ax,DS:[f0_decode_previous]
    mov bl,DS:[f0_decode_first_byte]
    push bx
pic_walk_chain:
    mov bx,ax
    add bx,ax
    add bx,ax
    mov ax,[bx-036DAH]
    inc ax
    jz SHORT pic_chain_root
    dec ax
    mov bl,[bx-036D8H]
    push bx
    jmp SHORT pic_walk_chain
pic_chain_root:
    mov al,[bx-036D8H]
    mov DS:[f0_decode_first_byte],al
    push ax
    mov bx,dx
    add bx,dx
    add bx,dx
    mov [bx-036D8H],al
    mov ax,DS:[f0_decode_previous]
    mov [bx-036DAH],ax
    inc dx
    cmp dx,DS:[f0_decode_mask]
    jng SHORT pic_check_limit
    inc BYTE PTR DS:[f0_decode_bits]
    stc
    rcl WORD PTR DS:[f0_decode_mask],1
pic_check_limit:
    mov al,DS:[f0_decode_bits]
    cmp al,DS:[f0_decode_limit]
    jng SHORT pic_code_done
    call NEAR PTR pic_reset_codes
pic_code_done:
    mov DS:[f0_decode_previous],cx
    jmp NEAR PTR pic_next_pop
_image_begin:
    push bp
    mov bp,sp
    push di
    mov ah,03CH
    mov cx,00H
    mov dx,[bp+06H]
    int 021H
    mov DS:[f0_image_handle],ax
    jc SHORT pic_create_done
    mov ah,048H
    mov bx,05FFH
    int 021H
    mov DS:[f0_encode_segment],ax
    jc SHORT pic_create_done
    lea di,DS:[f0_output_buffer]
    mov ax,ds
    mov es,ax
    mov ax,03058H
    stosw
    mov ax,00H
    stosw
    mov ax,[bp+08H]
    stosw
    mov ax,[bp+0AH]
    stosw
    mov DS:[f0_stream_pointer],di
    xor ax,ax
    mov DS:[f0_encode_bits],al
    mov DS:[f0_encode_byte],al
    mov WORD PTR DS:[f0_encode_status],0FFFFH
pic_create_done:
    pop di
    pop bp
    retf
_image_finish:
    mov ax,DS:[f0_encode_previous]
    mov bx,ax
    add bx,ax
    add bx,ax
    shl bx,1
    mov es,DS:[f0_encode_segment]
    mov ax,ES:[bx+04H]
    call NEAR PTR pc_encode_bits
    call NEAR PTR pc_encode_flush
    mov WORD PTR DS:[f0_encode_status],00H
    mov ah,049H
    mov es,DS:[f0_encode_segment]
    int 021H
    jnc SHORT pic_finish_seek
    mov DS:[f0_encode_status],ax
pic_finish_seek:
    mov ah,042H
    mov al,01H
    mov bx,DS:[f0_image_handle]
    xor cx,cx
    xor dx,dx
    int 021H
    jnc SHORT pic_finish_rewind
    mov DS:[f0_encode_status],ax
pic_finish_rewind:
    sub ax,04H
    push ax
    mov ah,042H
    mov al,00H
    mov bx,DS:[f0_image_handle]
    xor cx,cx
    mov dx,02H
    int 021H
    jnc SHORT pic_finish_write
    mov DS:[f0_encode_status],ax
pic_finish_write:
    mov ah,040H
    mov bx,DS:[f0_image_handle]
    mov cx,02H
    mov dx,sp
    int 021H
    jnc SHORT pic_finish_close
    mov DS:[f0_encode_status],ax
pic_finish_close:
    pop ax
    mov ah,03EH
    mov bx,DS:[f0_image_handle]
    int 021H
    jnc SHORT pic_finish_done
    mov DS:[f0_encode_status],ax
pic_finish_done:
    mov ax,DS:[f0_encode_status]
    retf
_image_byte:
    push bp
    mov bp,sp
    push si
    push di
    mov ax,ds
    mov es,ax
    mov ax,[bp+06H]
    mov bp,DS:[f0_encode_previous]
    cmp WORD PTR DS:[f0_encode_status],-01H
    jz SHORT pic_first_byte
    mov di,DS:[f0_encode_string_end]
    stosb
    mov DS:[f0_encode_string_end],di
    call NEAR PTR pc_encode_find
    cmp bx,-01H
    jz SHORT pic_emit_code
    mov DS:[f0_encode_previous],bx
    pop di
    pop si
    pop bp
    retf
pic_emit_code:
    lea si,DS:[f0_picture_table]
    mov [si],al
    inc si
    mov DS:[f0_encode_string_end],si
    mov DS:[f0_encode_previous],ax
    call NEAR PTR pc_encode_insert
    mov bx,bp
    add bx,bp
    add bx,bp
    shl bx,1
    mov ax,ES:[bx+04H]
    call NEAR PTR pc_encode_bits
    mov ax,01H
    mov cl,DS:[f0_encode_width]
    shl ax,cl
    cmp DS:[f0_encode_next_code],ax
    jna SHORT pic_byte_done
    inc cl
    cmp cl,0BH
    mov DS:[f0_encode_width],cl
    jna SHORT pic_byte_done
    call NEAR PTR pc_encode_init
pic_byte_done:
    pop di
    pop si
    pop bp
    retf
pic_first_byte:
    mov WORD PTR DS:[f0_encode_status],00H
    lea di,DS:[f0_picture_table]
    stosb
    mov DS:[f0_encode_string_end],di
    mov DS:[f0_encode_previous],ax
    mov al,0BH
    mov di,DS:[f0_stream_pointer]
    stosb
    mov DS:[f0_stream_pointer],di
    call NEAR PTR pc_encode_init
    pop di
    pop si
    pop bp
    retf
pc_encode_init:
    mov es,DS:[f0_encode_segment]
    xor di,di
    mov bx,0FFFFH
    xor cx,cx
pic_init_roots:
    mov ax,cx
    stosw
    mov ax,bx
    stosw
    mov ax,cx
    stosw
    inc cx
    cmp cx,0100H
    jc SHORT pic_init_roots
    mov ax,bx
pic_init_free:
    stosw
    add di,04H
    inc cx
    cmp cx,0FFBH
    jc SHORT pic_init_free
    mov BYTE PTR DS:[f0_encode_width],09H
    mov WORD PTR DS:[f0_encode_next_code],0101H
    ret
pc_encode_insert:
    stosw
    mov ax,bp
    stosw
    mov ax,DS:[f0_encode_next_code]
    stosw
    inc ax
    mov DS:[f0_encode_next_code],ax
    ret
pc_encode_find:
    xor bx,bx
    lea di,DS:[f0_picture_table]
    mov si,DS:[f0_encode_string_end]
    mov [si],bl
pic_hash_sum:
    add bx,[di]
    add di,02H
    cmp di,si
    jc SHORT pic_hash_sum
    and bx,07FFFH
    jnz SHORT pic_hash_step
    lea di,DS:[f0_picture_table]
    sub si,di
    mov cx,si
    mov dx,025H
pic_hash_spread:
    add bx,dx
    loop SHORT pic_hash_spread
    and bx,07FFFH
pic_hash_step:
    mov dx,bx
    mov si,0FF9H
pic_step_mod:
    sub dx,si
    jnc SHORT pic_step_mod
    neg dx
    mov si,0FFBH
    mov es,DS:[f0_encode_segment]
pic_probe_mod:
    sub bx,si
    jnc SHORT pic_probe_mod
    add bx,si
    mov di,bx
    add di,bx
    add di,bx
    shl di,1
    cmp WORD PTR ES:[di],-01H
    jnz SHORT pic_probe_match
    mov bx,0FFFFH
    ret
pic_probe_match:
    cmp ES:[di],ax
    jnz SHORT pic_probe_next
    cmp ES:[di+02H],bp
    jnz SHORT pic_probe_next
    ret
pic_probe_next:
    add bx,dx
    jmp SHORT pic_probe_mod
pc_encode_bits:
    mov cl,DS:[f0_encode_width]
    mov dl,DS:[f0_encode_bits]
    mov dh,DS:[f0_encode_byte]
pic_bits_loop:
    dec cl
    js SHORT pic_bits_done
    cmp dl,08H
    jc SHORT pic_bits_shift
    mov bx,DS:[f0_stream_pointer]
    mov [bx],dh
    inc bx
    xor dx,dx
    cmp bx,DS:[f0_stream_end]
    jc SHORT pic_bits_stored
    push ax
    push cx
    push dx
    mov ah,040H
    mov cx,bx
    mov bx,DS:[f0_image_handle]
    lea dx,DS:[f0_output_buffer]
    sub cx,dx
    int 021H
    pop dx
    pop cx
    pop ax
    lea bx,DS:[f0_output_buffer]
pic_bits_stored:
    mov DS:[f0_stream_pointer],bx
pic_bits_shift:
    shr ax,1
    rcr dh,1
    inc dl
    jmp SHORT pic_bits_loop
pic_bits_done:
    mov DS:[f0_encode_bits],dl
    mov DS:[f0_encode_byte],dh
    ret
pc_encode_flush:
    mov BYTE PTR DS:[f0_encode_width],08H
    xor ax,ax
    call NEAR PTR pc_encode_bits
    mov ah,040H
    mov bx,DS:[f0_image_handle]
    mov cx,DS:[f0_stream_pointer]
    lea dx,DS:[f0_output_buffer]
    sub cx,dx
    int 021H
    ret
DB 1 DUP (0) ; original inter-function alignment
ASSUME CS:NOTHING
