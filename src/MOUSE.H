/* MOUSE.H: shared declarations of symbols defined in MOUSE.INC. */

extern int init_mouse(void);
extern int mouse_poll(void);
extern void set_mousecur(int, int, int);
extern void mouse_show(void);
extern void mouse_hide(void);
extern void mouse_close(void);
extern void set_text_pos(int, int);
