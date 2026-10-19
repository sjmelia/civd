# Graphics

Game drawing code calls a table of driver entry points. `MGRAPHIC.EXE`
implements MCGA/VGA drawing; EGA and Tandy have separate drivers. The
loader installs far jumps in `driver_slots` from each driver's header.
The [MCGA driver chapter](graphics_driver.md) describes its slots and data.

## Pages

The MCGA driver uses 320-by-200 byte-per-pixel pages. Page 0 is the screen;
allocated pages provide offscreen storage. Game code passes a `struct page`
with the page number, origin, clipping bounds and text settings:

```c
{{#include ../../../src/GRAPHICS.H:6:13}}
```

`blit()` copies rectangles between pages. `sprite_draw()` and its variants
draw stored sprites. Game code assembles map tiles, cities, units and menus
from those operations. Mouse visibility is managed around drawing through
`mouse_off()` and `mouse_on()`.

Page 2 also holds the [map database](../map/memory_layout.md). A page number
therefore identifies storage, not necessarily a picture intended for display.

## Video selection

`SETUP.C` offers VGA, MCGA, EGA and Tandy 1000. VGA and MCGA both select
driver M. `MGRAPHIC.ASM` initializes BIOS mode `13h`, a 320-by-200 display
with 256 palette indices. This repository builds the M driver; the setup
choices alone do not establish how the other drivers implement drawing.

## Palette animation

`PALETTE.INC` implements `set_cycle()`, `start_cycle()` and `stop_cycle()`.
`FADE.INC` implements fades. The MCGA driver writes palette values to the
VGA DAC. Changing a palette entry changes the displayed colour of every
pixel using that index without rewriting the pixels themselves.

Palette asset loading uses `load_palette()` in `FILEIO.C`, through the
picture-loading path. A file named as a palette must not be assumed to be
a headerless array of RGB values.
