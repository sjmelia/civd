; Initialized DGROUP data of the F0000 assembly helpers, [5401,588A).
; Linked after every game C object, ahead of the runtime library data;
; its order follows the code: driver, loader, picture, raster, mouse.
; The original object/library boundary for this data remains unknown.
.8086
_DATA SEGMENT BYTE PUBLIC 'DATA'
_DATA ENDS
DGROUP GROUP _DATA
; ASSUME DS:DGROUP makes MASM give each PUBDEF the DGROUP base, so C
; references resolve group-relative rather than _DATA-relative.
ASSUME DS:DGROUP
_DATA SEGMENT BYTE PUBLIC 'DATA'
PUBLIC dr_pending, dr_deferred, dr_return_offset, dr_return_segment
PUBLIC ld_exec_block, ld_capacity, ld_error_prefix, ld_alloc_unexpected
PUBLIC ld_alloc_failed, ld_file_error, ld_memory_error, ld_exec_error
PUBLIC ld_capacity_error, ld_resize_error, ld_free_error
PUBLIC _image_buffer, f0_stream_end, _archive_handle
PUBLIC f0_line_x0, f0_line_x1, f0_line_y0, f0_line_y1
PUBLIC f0_left_edges, f0_right_edges, f0_raster_dx, f0_raster_dy
PUBLIC f0_first_row, f0_last_row, f0_origin_x, f0_origin_y
PUBLIC f0_clip_flags, f0_clip_dx, f0_clip_dy, f0_clip_dx_half, f0_clip_dy_half
PUBLIC f0_clip_xmax, f0_clip_ymax
PUBLIC _mouse_info_b, f0_mouse_x, _mouse_info_c, f0_mouse_y
PUBLIC _mouse_info_a, f0_mouse_buttons, f0_mouse_events
PUBLIC f0_cursor_picture, f0_cursor_x, f0_cursor_y, f0_mouse_shift
PUBLIC f0_mouse_active, f0_mouse_callback, f0_mouse_callback_segment
PUBLIC f0_mouse_result, f0_mouse_result_bx, f0_mouse_result_cx
PUBLIC f0_mouse_result_dx

; Driver gateway state (DRIVER.ASM).
    DB 0
dr_pending DB 0
dr_deferred DB 0
dr_return_offset DW 0
dr_return_segment DW 0

; Program/overlay loader state and DOS fatal messages (LOADER/LOADTAIL).
ld_exec_block DW 0, 0
ld_capacity DW 0
ld_error_prefix DB ' - $'
ld_alloc_unexpected DB 'Allocated 1Mb of space????$'
ld_alloc_failed DB 'MS-DOS lied to us about how much memory was available$'
ld_file_error DB 'Filename not found$'
ld_memory_error DB 'Not enough memory to load file$'
ld_exec_error DB 'Overlay load failed for some reason$'
ld_capacity_error DB 'Overlay has overrun allocated memory$'
ld_resize_error DB 'Unable to shrink overlay memory allocation$'
ld_free_error DB 'Error releasing overlay memory$'

; Picture input stream (PICTURE.ASM; C image_buffer/archive_handle). The
; read pointer starts at the end of the 512-byte input_block at DS:D936,
; so the first read refills it.
_image_buffer LABEL WORD
f0_stream_end DW 0D936H+0200H
_archive_handle DW -1

; Line rasterizer and clipping state (RASTER.ASM).
f0_line_x0 DW 0
f0_line_x1 DW 0
f0_line_y0 DW 0
f0_line_y1 DW 0
f0_left_edges DW 200 DUP (-1)
f0_right_edges DW 200 DUP (0)
f0_raster_dx DW 0
f0_raster_dy DW 0
f0_first_row DW -1
f0_last_row DW 0
f0_origin_x DW 0
f0_origin_y DW 0
f0_clip_flags DB 0
f0_clip_dx DW 0
f0_clip_dy DW 0
f0_clip_dx_half DW 0
f0_clip_dy_half DW 0
f0_clip_xmax DW 319
f0_clip_ymax DW 111
    DB 0

; Mouse and cursor state (MOUSE.ASM; C mouse_info_a/b/c).
_mouse_info_b LABEL WORD
f0_mouse_x DW 0
_mouse_info_c LABEL WORD
f0_mouse_y DW 0
_mouse_info_a LABEL WORD
f0_mouse_buttons DW 0
f0_mouse_events DW 0
f0_cursor_picture DW 0
f0_cursor_x DW 0
f0_cursor_y DW 0
f0_mouse_shift DB 0
f0_mouse_active DB 0
f0_mouse_callback DW 0
f0_mouse_callback_segment DW 0
f0_mouse_result DW 0
f0_mouse_result_bx DW 0
f0_mouse_result_cx DW 0
f0_mouse_result_dx DW 0
_DATA ENDS
END
