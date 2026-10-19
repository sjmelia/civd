; Included by RESIDENT.ASM in F0000: driver gateway/table [0732,0A76).
PUBLIC cursor_set_vector
PUBLIC cursor_erase_vector
PUBLIC cursor_draw
PUBLIC raster_target
PUBLIC raster_page
PUBLIC raster_line
PUBLIC raster_colour
PUBLIC raster_flush
PUBLIC raster_spans
PUBLIC raster_plot
PUBLIC cursor_set
PUBLIC cursor_erase
; load_program/run_program patch far targets in seven-byte slots.
; Preserve direct and deferred-call paths and embedded mutable operands.
ASSUME CS:F0000
EXTRN mouse_driver_poll:NEAR
dr_jump_operand EQU driver_jump+3
dr_call_operand EQU driver_call+3
driver_dispatch:
    cmp BYTE PTR DS:[dr_deferred],00H
    jnz SHORT driver_deferred
    pop WORD PTR CS:dr_jump_operand
    jmp SHORT driver_jump
driver_jump:
    jmp DWORD PTR CS:[0]
driver_deferred:
    mov BYTE PTR DS:[dr_pending],01H
    pop WORD PTR CS:dr_call_operand
    pop WORD PTR DS:[dr_return_offset]
    pop WORD PTR DS:[dr_return_segment]
    jmp SHORT driver_call
driver_call:
    call DWORD PTR CS:[0]
driver_wait:
    dec BYTE PTR DS:[dr_pending]
    jz SHORT driver_return
    call NEAR PTR mouse_driver_poll
    jmp SHORT driver_wait
driver_return:
    jmp DWORD PTR DS:[dr_return_offset]
; A slot is either DW tag / far-JMP opcode / DD target, or CALL / DD target.
; The immediate far JMP is a runtime-written instruction template, not a
; link-time relocation. All pointers start at zero.
DRJUMP MACRO entry_name
    DW 0
entry_name LABEL FAR
    DB 0EAH
    DD 0
ENDM
DRCALL MACRO entry_name
entry_name LABEL FAR
    call NEAR PTR driver_dispatch
entry_name&_vector LABEL DWORD
    DD 0
ENDM
PUBLIC driver_slots
driver_slots LABEL BYTE
PUBLIC _alloc_page_memory
    DRJUMP _alloc_page_memory ; slot 0
PUBLIC _free_paragraphs
    DRJUMP _free_paragraphs ; slot 1
PUBLIC _cut_tile
    DRJUMP _cut_tile ; slot 2
PUBLIC _recolour_rect
    DRJUMP _recolour_rect ; slot 3
PUBLIC _char_width
    DRJUMP _char_width ; slot 4
    DRJUMP page_clear ; slot 5
PUBLIC _sprite_draw_clipped
    DRCALL _sprite_draw_clipped ; slot 6
PUBLIC _sprite_arena_release
    DRJUMP _sprite_arena_release ; slot 7
    DRJUMP page_free ; slot 8
    DRCALL cursor_draw ; slot 9
PUBLIC _page_dissolve
    DRJUMP _page_dissolve ; slot 10
PUBLIC _font_address
    DRJUMP _font_address ; slot 11
PUBLIC _font_height
    DRJUMP _font_height ; slot 12
PUBLIC _page_read_row
    DRJUMP _page_read_row ; slot 13
PUBLIC _pixel_at
    DRJUMP _pixel_at ; slot 14
    DRCALL cursor_erase ; slot 15
    DRJUMP font_copy ; slot 16
PUBLIC _page_ready
    DRJUMP _page_ready ; slot 17
PUBLIC _sprite_arena_init
    DRJUMP _sprite_arena_init ; slot 18
PUBLIC _draw_scanline
    DRJUMP _draw_scanline ; slot 19
PUBLIC _blit
    DRJUMP _blit ; slot 20
PUBLIC _page_string
    DRJUMP _page_string ; slot 21
    DRCALL sprite_draw_scaled ; slot 22
    DRJUMP screen_off ; slot 23
    DRJUMP screen_on ; slot 24
PUBLIC _set_page_memory
    DRJUMP _set_page_memory ; slot 25
    DRJUMP palette_reset_colour ; slot 26
PUBLIC _load_ega_palette
    DRJUMP _load_ega_palette ; slot 27
PUBLIC _apply_palette
    DRJUMP _apply_palette ; slot 28
    DRJUMP shake_start ; slot 29
    DRCALL cursor_set ; slot 30
PUBLIC _set_page
    DRCALL _set_page ; slot 31
PUBLIC _sprite_draw
    DRCALL _sprite_draw ; slot 32
PUBLIC _sprite_end
    DRJUMP _sprite_end ; slot 33
PUBLIC _sprite_encode_row
    DRJUMP _sprite_encode_row ; slot 34
PUBLIC _sprite_begin
    DRJUMP _sprite_begin ; slot 35
    DRJUMP page_swap ; slot 36
PUBLIC _video_init
    DRJUMP _video_init ; slot 37
    DRJUMP raster_plot ; slot 38
    DRJUMP raster_spans ; slot 39
    DRJUMP raster_flush ; slot 40
    DRJUMP raster_colour ; slot 41
    DRJUMP raster_line ; slot 42
    DRJUMP text_clip_all ; slot 43
    DRJUMP text_clip_left ; slot 44
    DRJUMP text_draw ; slot 45
    DRJUMP text_clip_right ; slot 46
    DRJUMP text_clip_bottom ; slot 47
    DRJUMP raster_page ; slot 48
    DRJUMP raster_target ; slot 49
    DRJUMP raster_copy_spans ; slot 50
    DRJUMP shake_step ; slot 51
    DB 7 DUP (0) ; unused driver slot 52
    DB 7 DUP (0) ; unused driver slot 53
    DB 7 DUP (0) ; unused driver slot 54
    DB 7 DUP (0) ; unused driver slot 55
    DB 7 DUP (0) ; unused driver slot 56
    DB 7 DUP (0) ; unused driver slot 57
    DB 7 DUP (0) ; unused driver slot 58
    DB 7 DUP (0) ; unused driver slot 59
    DB 7 DUP (0) ; unused driver slot 60
    DB 7 DUP (0) ; unused driver slot 61
    DB 7 DUP (0) ; unused driver slot 62
    DB 7 DUP (0) ; unused driver slot 63
    DB 7 DUP (0) ; unused driver slot 64
    DB 7 DUP (0) ; unused driver slot 65
    DB 7 DUP (0) ; unused driver slot 66
    DB 7 DUP (0) ; unused driver slot 67
    DB 7 DUP (0) ; unused driver slot 68
    DB 7 DUP (0) ; unused driver slot 69
    DB 7 DUP (0) ; unused driver slot 70
    DB 7 DUP (0) ; unused driver slot 71
    DB 7 DUP (0) ; unused driver slot 72
    DB 7 DUP (0) ; unused driver slot 73
    DB 7 DUP (0) ; unused driver slot 74
    DB 7 DUP (0) ; unused driver slot 75
    DB 7 DUP (0) ; unused driver slot 76
    DB 7 DUP (0) ; unused driver slot 77
    DB 7 DUP (0) ; unused driver slot 78
    DB 7 DUP (0) ; unused driver slot 79
    DB 7 DUP (0) ; unused driver slot 80
    DB 7 DUP (0) ; unused driver slot 81
    DB 7 DUP (0) ; unused driver slot 82
    DB 7 DUP (0) ; unused driver slot 83
    DB 7 DUP (0) ; unused driver slot 84
    DB 7 DUP (0) ; unused driver slot 85
    DB 7 DUP (0) ; unused driver slot 86
    DB 7 DUP (0) ; unused driver slot 87
    DB 7 DUP (0) ; unused driver slot 88
    DB 7 DUP (0) ; unused driver slot 89
    DRJUMP key_none ; slot 90
PUBLIC _read_key
    DRJUMP _read_key ; slot 91
PUBLIC _key_echo
    DRJUMP _key_echo ; slot 92
    DRJUMP locks_off ; slot 93
    DRJUMP locks_off_too ; slot 94
    DRJUMP stick_button ; slot 95
    DRJUMP stick_centre ; slot 96
    DRJUMP stick_read ; slot 97
    DB 7 DUP (0) ; unused driver slot 98
    DB 7 DUP (0) ; unused driver slot 99
PUBLIC _sound_init
    DRJUMP _sound_init ; slot 100
PUBLIC _sound_command
    DRJUMP _sound_command ; slot 101
PUBLIC _sound_shutdown
    DRJUMP _sound_shutdown ; slot 102
    DRJUMP sound_tick ; slot 103
    DRJUMP sound_fast_tick ; slot 104
PUBLIC _sound_quiet_ticks
    DRJUMP _sound_quiet_ticks ; slot 105
    DRJUMP driver_entry_106 ; slot 106
    DRJUMP driver_entry_107 ; slot 107
    DRJUMP driver_entry_108 ; slot 108
    DRJUMP driver_entry_109 ; slot 109
    DRJUMP driver_entry_110 ; slot 110
ASSUME CS:NOTHING
