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
f0_driver_pending EQU 053A4H
f0_driver_deferred EQU 053A5H
f0_picture_width EQU 06864H
f0_picture_height EQU 06866H
f0_picture_size EQU 06868H
f0_decode_stack EQU 0686AH
f0_decode_count EQU 0686CH
f0_repeat_count EQU 0686EH
f0_repeat_byte EQU 0686FH
f0_decode_bits EQU 06870H
f0_decode_limit EQU 06871H
f0_decode_mask EQU 06872H
f0_decode_next_code EQU 06874H
f0_decode_word EQU 06876H
f0_decode_word_bits EQU 06878H
f0_decode_nibbles EQU 06879H
f0_decode_previous EQU 0687AH
f0_decode_first_byte EQU 0687CH
f0_decode_stack_top EQU 06A7DH
f0_encode_next_code EQU 06A7EH
f0_image_handle EQU 06A80H
f0_encode_string_end EQU 06A82H
f0_encode_previous EQU 06A84H
f0_encode_segment EQU 06A86H
f0_encode_status EQU 06A88H
f0_encode_bits EQU 06A8AH
f0_encode_byte EQU 06A8BH
f0_encode_width EQU 06A8CH
f0_stream_pointer EQU 0C18EH
f0_picture_table EQU 0C926H
f0_picture_byte EQU 0C928H
f0_stream_refill EQU 0E83AH
f0_output_buffer EQU 0E856H
