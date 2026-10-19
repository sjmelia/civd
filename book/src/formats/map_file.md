# The map save file

The companion `CIVIL?.MAP` saves graphics page 2, which holds the
[map database](../map/memory_layout.md). `read_save()` calls
`image_screen(2, file)`; `load_save()` restores it through `load_screen()`.
Despite its name, `read_save()` is the writing routine.

The decoded page has 320 columns and 200 rows. At that level, byte position
`y * 320 + x` corresponds to a page coordinate. The MAP file itself uses
the screen-image encoder, including framing and compression. It is not a
fixed 64,000-byte raw dump, and page offsets are not file offsets.

Terrain codes, region numbers, site data, occupancy and improvement nibbles
occupy separate areas of the decoded page. The per-civilization exploration
array is stored in the SVE file, separately from the remembered improvement
flags on this page.
