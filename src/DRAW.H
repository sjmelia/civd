/* DRAW.H: shared declarations of symbols defined in DRAW.C. */

extern void draw_line(int, int, int, int, int);
extern void draw_string_dim(char *, int, int, int);
extern void draw_text(char *, int, int, int);
extern int string_width(char *);
extern void fill_box(int, int, int, int, int);
extern void draw_shadowed(char *, int, int, int);
extern void draw_string(char *, int, int, int);
extern void flip(int);
