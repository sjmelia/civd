/* Screen-image run-length encoding. */
#include "GLOBALS.H"
#include "RESIDENT.H"

struct page;
extern int image_begin(char *, int, int);
extern int image_finish(void);
extern void image_byte(int);
extern void page_read_row(unsigned char *, struct page *, int, int, int);
int image_rect(struct page *, int, int, int, int, char *);
void image_run(unsigned int, unsigned char);

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void image_screen(page, name)
struct page *page;
char *name;
{
    image_rect(page, 0, 0, 320, 200, name);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int image_rect(struct page *page, int x, int y, int width, int height, char *name)
{
    int last, run, value, alignment;
    unsigned char *begin, *pixels, *end;
    if (image_begin(name, width, height))
        return image_error;
    alignment = x & 7;
    begin = unpack_raster + alignment;
    end = begin + width;
    pixels = end;
    width += alignment;
    x -= alignment;
    run = 1;
    last = -1;
    for (;;) {
        if (pixels >= end) {
            if (height-- == 0)
                break;
            page_read_row(unpack_raster, page, x, y++, width);
            pixels = begin;
            if (last == -1)
                last = *pixels++;
        }
        value = *pixels++;
        if (value == last)
            ++run;
        else {
            image_run(run, last);
            last = value;
            run = 1;
        }
    }
    image_run(run, last);
    if (image_finish())
        return image_error;
    return 0;
}

void image_run(unsigned int count, unsigned char value)
{
    unsigned int n;
    if (value == 0x90) {
        while (count--) {
            image_byte(0x90);
            image_byte(0);
        }
    } else {
        if (count >= 4) {
            do {
                n = count < 255 ? count : 255;
                image_byte(value);
                image_byte(0x90);
                image_byte(n);
                count -= n;
            } while (count >= 4);
        }
        while (count--)
            image_byte(value);
    }
}
