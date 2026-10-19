# The MCGA Graphics Driver (MGRAPHIC.EXE)

CIV.EXE draws nothing itself. Every pixel goes through a graphics driver,
a separate program loaded at start-up: `MGRAPHIC.EXE` for MCGA/VGA,
`EGRAPHIC.EXE` for EGA and `TGRAPHIC.EXE` for Tandy. `src/MGRAPHIC.ASM` is
the recovered MCGA driver. `just verify` rebuilds it with MASM 5.10 and
LINK 5.01 and checks it against the original byte for byte.

## Loading and the slot table

`load_program` (`LOADER.INC`) loads the driver with DOS EXEC function 4B03h
("load overlay"). `run_program` (`LOADTAIL.INC`) then copies the driver's entry
points into CIV's jump table, `driver_slots` in `DRIVER.INC`. CIV calls each
slot by its C name, a far call to a far jump that `run_program` has patched.

The driver begins with a header that `run_program` reads:

| Offset | Contents |
| --- | --- |
| 10h | Name and date, `MGRAPHIC.EXE09-19-91` |
| 28h | Segment of the slot routines (relocated by DOS) |
| 2Ah | Data segment; the loader appends `FONTS.CV` after it |
| 2Ch | Bytes of data before the fonts (`630h`) |
| 2Eh | First slot (0) |
| 30h | Slot count (52) |
| 32h | Offset of each slot routine |

The sound drivers (`*.CVL`, slots 100–110) and `MISC.EXE` (slots 90–97) use
the same header.

## Pages

A page is a 320×200 byte-per-pixel buffer in its own segment. `page_segs`
holds eight entries. All of them start as the screen (`A000h`); CIV then
gives pages 1 and 2 their own memory with `alloc_page_memory` and
`set_page_memory`. Drawing calls take a page record (`struct page` in
`GRAPHICS.H`), whose page number and origin the driver adds to the
coordinates. Calls that work on a whole page take its number.

| Slot | Name | Effect |
| ---: | --- | --- |
| 0 | `alloc_page_memory(n)` | n = 0: the screen segment; otherwise a fresh 64,000-byte block (0 on failure) |
| 25 | `set_page_memory(n, seg)` | Use `seg` for page n |
| 8 | `page_free(n)` | Free page n's block |
| 5 | `page_clear(n, colour)` | Fill page n |
| 31 | `set_page(n)` | Draw the mouse cursor on page n, then copy page n to the screen |
| 36 | `page_swap(n)` | Exchange page n with the screen |
| 10 | `page_dissolve(n, time)` | Copy page n to the screen in pseudo-random pixel order |
| 20 | `blit(src, sx, sy, w, h, dst, dx, dy)` | Copy a rectangle between pages, two bytes at a time |
| 3 | `recolour_rect(pg, x, y, w, h, from, to)` | Replace one colour with another in a rectangle (menu highlights) |
| 13, 19 | `page_read_row`, `draw_scanline` | Copy part of a row out of, or into, a page |
| 14 | `pixel_at(n, x, y)` | Read one pixel |
| 17 | `page_ready(n)` | Status word; always 0 on MCGA |

**The dissolve.** `page_dissolve` first waits for a BIOS timer tick. It sets
the diskette motor counter at `0040:0040` to 1 and waits for the BIOS to
count it down. It then visits all 64,000 pixels in an order given by a 16-bit
Galois LFSR: shift right, then XOR `B400h` if a bit fell out. Values above
64,000 are skipped, and value *v* copies pixel *v* − 1. Between pixels it
waits in a busy loop. On first use it measures how many steps fit in one
video frame, then sizes the delay so the whole dissolve takes roughly the
requested time. A port must use the same LFSR to show the same sequence.

## Sprites

Every tile, unit, face and cursor is a *sprite*: run-length rows in a
paragraph-aligned block. CIV refers to a sprite by its segment.

```
word  width
word  height
then per row:
  word  n      pixels stored for the row
  word  lead   transparent pixels before them
  byte  n pixels (0 is transparent)
```

The encoder trims transparent pixels at both ends of each row. CIV builds
sprites in an *arena*: the largest free DOS block, taken by
`sprite_arena_init` (slot 18) and shrunk back to what was used by
`sprite_arena_release` (slot 7). `cut_tile(page, x, y, w, h)` (slot 2) runs
`sprite_begin`, `sprite_encode_row` and `sprite_end` (slots 35, 34, 33)
over a page rectangle and returns the new sprite's segment. A sprite may not
exceed 65,520 bytes.

| Slot | Name | Effect |
| ---: | --- | --- |
| 32 | `sprite_draw(pg, x, y, sprite)` | Draw without clipping |
| 6 | `sprite_draw_clipped(pg, x, y, sprite)` | Draw clipped to the page record; return a far pointer to the rectangle drawn |
| 22 | `sprite_draw_scaled(pg, x, y, w, h, sprite, c)` | Draw stretched to w×h; a negative size mirrors that axis (it patches the inner loop's `CLD`/`INC DI` into `STD`/`DEC DI`) |

## Mouse cursor

The cursor is a sprite drawn into whichever page is about to be shown.
`cursor_set(x, y, sprite)` (slot 30) records the sprite and position and clips
the area underneath to the screen. That area is limited to 512 bytes, so a wide
cursor saves fewer rows. `cursor_draw` (slot 9, page in AX) saves what is
underneath and draws the cursor. `cursor_erase` (slot 15) puts the saved
pixels back. `MOUSE.INC` calls these from the mouse handler.

## Palette

The data segment holds 128 default colours in 6-bit RGB. The first 16 are
the EGA colours. Writes to the DAC (`dac_write`) wait for vertical retrace
and then send colours until the retrace ends. The number of colours that fit
in one retrace is learnt as it goes.

| Slot | Name | Effect |
| ---: | --- | --- |
| 28 | `apply_palette(rec)` | Load colours `rec[4]..rec[5]` from an `"M0"` palette record (RGB from byte 6) |
| 27 | `load_ega_palette(pal)` | Map 16 EGA colour numbers to default colours; byte 16 picks the border colour |
| 26 | `palette_reset_colour(i, c)` | DAC entry i = default colour c |
| 23, 24 | `screen_off`, `screen_on` | Blank the display through the sequencer's clocking-mode register |
| 29, 51 | `shake_start(n)`, `shake_step` | For n frames, write a pseudo-random 0–3 (LCG ×5+1) into CRTC register 0Dh (start address low) |

## Raster primitives

`RASTER.INC` draws lines and filled shapes through register-call slots. The
first two choose the target and the colour:

- `raster_page` (48) or `raster_target` (49) selects the target page and origin.
- `raster_colour` (41) sets AL as the colour and AH as the operation: 0 set,
  1 AND, 2 OR, 3 XOR. It works by patching the opcode of the store
  instruction (`plot_store`, `line_store`, `hline_store`, `text_store`).

The drawing slots:

- `raster_plot` (38) plots one point.
- `raster_line` (42) draws a Bresenham line.
- `raster_spans` (39) fills rows between two edge tables: left x at
  `[BX + 2y]`, right x at `[BX + 400 + 2y]`.
- `raster_copy_spans` (50) copies the same spans from one page to another.
- `raster_flush` (40) does nothing on MCGA.

## Text

`FONTS.CV` is appended to the driver at data offset `630h` (`fonts`). It
starts with a font count and an offset for each font. Nine bytes before each
font's glyph data hold its metrics: first and last character, the row width
as a shift, the fixed width (0 means proportional, using the width table
just before the metrics), the height, the character spacing and extra rows.
Each glyph row is a big-endian bit row, most significant bit first.

`page_string` (slot 21) is the C entry. It calls the clipping variant
(slots 43–47) that fits, which leave out characters and glyph columns
beyond the edges. In strings, a byte of 80h or more changes the colour to
its low seven bits. `char_width` (4) and `font_height` (12) report metrics.
`font_address` (11) and `font_copy` (16) find and fill a font's glyph block.
`font_address` checks the font number against the word at data offset `20h`,
not `630h`. That puzzle is recorded but not explained.

## Video mode

`video_init(flags)` (slot 37) sets BIOS mode 13h; bit 7 of `flags` keeps video
memory. It then reads the mode back. If the mode did not take, it prints "MCGA
is not supported on your machine" and exits to DOS.
