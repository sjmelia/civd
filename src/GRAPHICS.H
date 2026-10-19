/* GRAPHICS.H: screen-page records used by the graphics helpers. */
#ifndef GRAPHICS_H
#define GRAPHICS_H

/* Nine 16-bit words; field offsets are part of the driver interface. */
struct page {
    int number;
    int left, top, right, bottom;
    int active, colour;
    int text_colour;
    int font;
};

/* Raster helper defined by GRAPHICS.ASM. */
extern void rect_fill(struct page *, int, int, int, int, int);

#endif
