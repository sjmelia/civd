# Pictures, palettes and fonts

`FILEIO.C` loads resources either from individual files or through an
archive directory. Picture and palette loading share `LOAD_PIC()`.
`PICTURE.INC` handles picture chunks and decoding; the graphics driver
supplies scanline drawing and palette application.

File extensions alone do not identify the byte layout. Palette data can
travel through the picture loader, and `FONTS.CV` contains the fonts used
by the MCGA driver.

## Screen-image encoding

`IMAGE.C` writes a rectangular page area through `image_begin()`,
`image_byte()` and `image_finish()` in `PICTURE.INC`. Its run encoder uses
escape byte `0x90`:

- A literal `0x90` is written as `0x90, 0`.
- A run of at least four equal non-escape bytes is written as
  `value, 0x90, count`, with count at most 255.
- Short remaining runs are written literally.

This describes the run layer. The assembly routines handle the surrounding
stream encoding and framing, so these rules alone are not a complete PIC
or MAP decoder.

## Sprites and fonts

Pictures provide source pixels from which the game cuts sprites. In the
MCGA driver a sprite stores width, height and rows trimmed of leading and
trailing transparency. A row has a stored-pixel count, a leading transparent
count, and its pixel bytes. Zero pixels are transparent when drawn.

`FONTS.CV` is appended to the driver's data. The font table supplies offsets,
metrics and glyph data, with a width table for proportional fonts. See
[the MCGA driver](../engine/graphics_driver.md) for the rendering calls.
