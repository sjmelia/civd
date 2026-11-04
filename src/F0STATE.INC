; Included by RESIDENT.ASM and GRAPHICS.ASM: shared DS declarations.
; Shared DS storage for the F0000 helpers. Initialized state lives in
; F0DATA.ASM; the remaining views name existing storage without allocating it,
; as offsets from its symbols so that LINK relocates them with the data.
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
; RESIDENT.ASM defines f0_bss and f0_common; other includers set F0_EXTERN_BSS.
IFDEF F0_EXTERN_BSS
_BSS SEGMENT WORD PUBLIC 'BSS'
EXTRN f0_bss:BYTE, f0_common:BYTE
_BSS ENDS
DGROUP GROUP _BSS
ENDIF
; The high bytes of the clip deltas hold their signs.
f0_clip_dx_sign EQU BYTE PTR f0_clip_dx+1
f0_clip_dy_sign EQU BYTE PTR f0_clip_dy+1
DGROUP GROUP _DATA
ASSUME DS:DGROUP
f0_driver_pending EQU dr_pending
f0_driver_deferred EQU dr_deferred
f0_picture_width EQU WORD PTR f0_bss+03B2H
f0_picture_height EQU WORD PTR f0_bss+03B4H
f0_picture_size EQU WORD PTR f0_bss+03B6H
f0_decode_stack EQU WORD PTR f0_bss+03B8H
f0_decode_count EQU WORD PTR f0_bss+03BAH
f0_repeat_count EQU BYTE PTR f0_bss+03BCH
f0_repeat_byte EQU BYTE PTR f0_bss+03BDH
f0_decode_bits EQU BYTE PTR f0_bss+03BEH
f0_decode_limit EQU BYTE PTR f0_bss+03BFH
f0_decode_mask EQU WORD PTR f0_bss+03C0H
f0_decode_next_code EQU WORD PTR f0_bss+03C2H
f0_decode_word EQU WORD PTR f0_bss+03C4H
f0_decode_word_bits EQU BYTE PTR f0_bss+03C6H
f0_decode_nibbles EQU BYTE PTR f0_bss+03C7H
f0_decode_previous EQU WORD PTR f0_bss+03C8H
f0_decode_first_byte EQU BYTE PTR f0_bss+03CAH
f0_decode_stack_top EQU BYTE PTR f0_bss+05CBH
f0_encode_next_code EQU WORD PTR f0_bss+05CCH
f0_image_handle EQU WORD PTR f0_bss+05CEH
f0_encode_string_end EQU WORD PTR f0_bss+05D0H
f0_encode_previous EQU WORD PTR f0_bss+05D2H
f0_encode_segment EQU WORD PTR f0_bss+05D4H
f0_encode_status EQU WORD PTR f0_bss+05D6H
f0_encode_bits EQU BYTE PTR f0_bss+05D8H
f0_encode_byte EQU BYTE PTR f0_bss+05D9H
f0_encode_width EQU BYTE PTR f0_bss+05DAH
f0_stream_pointer EQU WORD PTR f0_common+056DEH
f0_picture_table EQU BYTE PTR f0_common+05E76H
f0_picture_byte EQU BYTE PTR f0_common+05E78H
f0_stream_refill EQU DWORD PTR f0_common+07D8AH
f0_output_buffer EQU BYTE PTR f0_common+07DA6H
