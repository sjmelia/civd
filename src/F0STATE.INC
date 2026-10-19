; Included by RESIDENT.ASM and GRAPHICS.ASM: shared DS declarations.
; Shared DS storage for the F0000 helpers. Initialized state lives in
; F0DATA.ASM; the remaining views name existing storage without allocating it.
_DATA SEGMENT WORD PUBLIC 'DATA'
EXTRN dr_pending:BYTE, dr_deferred:BYTE, dr_return_offset:WORD
EXTRN dr_return_segment:WORD, ld_exec_block:WORD, ld_capacity:WORD
EXTRN ld_error_prefix:BYTE, ld_alloc_unexpected:BYTE
EXTRN ld_alloc_failed:BYTE, ld_file_error:BYTE, ld_memory_error:BYTE
EXTRN ld_exec_error:BYTE, ld_capacity_error:BYTE, ld_resize_error:BYTE
EXTRN ld_free_error:BYTE, f0_stream_end:WORD, f0_line_x0:WORD
EXTRN f0_line_x1:WORD, f0_line_y0:WORD, f0_line_y1:WORD
EXTRN f0_left_edges:WORD, f0_right_edges:WORD, f0_raster_dx:WORD
EXTRN f0_raster_dy:WORD, f0_first_row:WORD, f0_last_row:WORD
EXTRN f0_origin_x:WORD, f0_origin_y:WORD, f0_clip_flags:BYTE
EXTRN f0_clip_dx:WORD, f0_clip_dy:WORD
EXTRN f0_clip_dx_half:WORD, f0_clip_dy_half:WORD
EXTRN f0_clip_xmax:WORD, f0_clip_ymax:WORD, f0_mouse_x:WORD
EXTRN f0_mouse_y:WORD, f0_mouse_buttons:WORD, f0_mouse_events:WORD
EXTRN f0_cursor_picture:WORD, f0_cursor_x:WORD, f0_cursor_y:WORD
EXTRN f0_mouse_shift:BYTE, f0_mouse_active:BYTE, f0_mouse_callback:WORD
EXTRN f0_mouse_callback_segment:WORD, f0_mouse_result:WORD
EXTRN f0_mouse_result_bx:WORD, f0_mouse_result_cx:WORD
EXTRN f0_mouse_result_dx:WORD
_DATA ENDS
; The high bytes of the clip deltas hold their signs.
f0_clip_dx_sign EQU BYTE PTR f0_clip_dx+1
f0_clip_dy_sign EQU BYTE PTR f0_clip_dy+1
DGROUP GROUP _DATA
ASSUME DS:DGROUP
f0_driver_pending EQU 05402H
f0_driver_deferred EQU 05403H
f0_picture_width EQU 068E2H
f0_picture_height EQU 068E4H
f0_picture_size EQU 068E6H
f0_decode_stack EQU 068E8H
f0_decode_count EQU 068EAH
f0_repeat_count EQU 068ECH
f0_repeat_byte EQU 068EDH
f0_decode_bits EQU 068EEH
f0_decode_limit EQU 068EFH
f0_decode_mask EQU 068F0H
f0_decode_next_code EQU 068F2H
f0_decode_word EQU 068F4H
f0_decode_word_bits EQU 068F6H
f0_decode_nibbles EQU 068F7H
f0_decode_previous EQU 068F8H
f0_decode_first_byte EQU 068FAH
f0_decode_stack_top EQU 06AFBH
f0_encode_next_code EQU 06AFCH
f0_image_handle EQU 06AFEH
f0_encode_string_end EQU 06B00H
f0_encode_previous EQU 06B02H
f0_encode_segment EQU 06B04H
f0_encode_status EQU 06B06H
f0_encode_bits EQU 06B08H
f0_encode_byte EQU 06B09H
f0_encode_width EQU 06B0AH
f0_stream_pointer EQU 0B26EH
f0_picture_table EQU 0BA06H
f0_picture_byte EQU 0BA08H
f0_stream_refill EQU 0D91AH
f0_output_buffer EQU 0D936H
