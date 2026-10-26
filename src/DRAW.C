/* Drawing wrappers for the graphics driver. */
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "RASTER.H"


void draw_line(int x1, int y1, int x2, int y2, int colour)
{
    page_line(scrbuf, x1, y1, x2, y2, colour);
}

void draw_string_dim(char *text, int x, int y, int colour)
{
    int unused;
    scrbuf->colour = colour;
    page_string(scrbuf, draw_large_text ? 2 * x : x, y, text);
}

void draw_string(char *text, int x, int y, int colour)
{
    scrbuf->active = 0;
    draw_string_dim(text, x, y, colour);
    scrbuf->active = 1;
}

void draw_shadowed(char *text, int x, int y, int colour)
{
    draw_string(text, x, y + 1, 0);
    draw_string(text, x, y, colour);
}

void draw_text(char *text, int x, int y, int colour)
{
    x -= string_width(text) / 2;
    scrbuf->active = 0;
    draw_string_dim(text, x, y, colour);
    scrbuf->active = 1;
}

int string_width(char *text)
{
    unsigned char *ptr;
    int f, w;
    ptr = text;
    f = scrbuf->font;
    w = 0;
    while (*ptr)
        w += char_width(f, *ptr++);
    return w;
}

void flip(int ticks)
{
    timer_reset();
    while (timer_read() < ticks);
}

void fill_box(int x1, int y1, int x2, int y2, int colour)
{
    rect_fill(scrbuf, x1, y1, x2 - x1 + 1, y2 - y1 + 1, colour);
}
