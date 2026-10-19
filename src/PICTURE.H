/* PICTURE.H: shared declarations of symbols defined in PICTURE.INC. */

extern int image_begin(char *, int, int);
extern void image_byte(int);
extern int image_finish(void);
extern void begin_picture();
extern void copy_picture();
extern void decode_scanline();
extern void picture_target();
