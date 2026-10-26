/* DRIVER.H: shared declarations of symbols defined in DRIVER.INC. */
#include "GRAPHICS.H"

extern int alloc_page_memory();
extern int free_paragraphs(void);
extern int cut_tile(int, int, int, int, int);
extern int char_width(int, int);
extern void sprite_draw_clipped();
/* Callers pass either a page pointer or a page handle. */
extern void recolour_rect();
extern void sprite_arena_release(void);
extern void page_dissolve();
extern int font_height(int);
extern char far *font_address();
extern void page_read_row(unsigned char *, struct page *, int, int, int);
extern int pixel_at();
extern int page_ready(int);
extern void sprite_arena_init(void);
extern void draw_scanline();
extern void page_string(struct page *, int, int, char *);
extern void set_page_memory();
extern void load_ega_palette();
extern void apply_palette(unsigned char *);
extern void set_page(int);
extern void sprite_end();
extern void sprite_encode_row();
extern void sprite_begin();
extern void video_init();
extern int read_key(void);
extern int key_echo(void);
extern void sound_init();
extern void sound_command(int, int);
extern void sound_shutdown(void);
extern int sound_quiet_ticks(void);
