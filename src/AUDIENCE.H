/* AUDIENCE.H: shared declarations of symbols defined in AUDIENCE.C. */

/* Calls use differing argument counts; retain the unprototyped interface. */
extern void leader_audience();
extern int mouse_mode;
extern int style;
extern char leader_portrait[];
extern void audience_close(void);
extern int audience_open(int, int, int);
extern void leader_greeting(int);
extern int treaty_prompt(int);
extern void make_peace(int);
extern char face_eyes[];
extern char face_mouths[];
extern char face_next[];
extern void leader_animate(void);
extern int advisor_x[];
extern void load_leader(int, int, long);
extern void leader_backdrop(int);
extern void free_leader(void);
extern void draw_leader_face(int, int, int);
extern int leader_menu(char *, int, int);
