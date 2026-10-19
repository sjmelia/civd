/* DIALOG.H: shared declarations of symbols defined in DIALOG.C. */

extern int busy_flag;
extern int editing;
extern int edit_mode;
extern int advisor;
extern int help_flag;
extern int used_flag;
extern char *advisor_title[];
extern int menu_choose(char *, int, int);
extern int menu_core(char *, int, int, int);
extern int menu_layout(char *, int, int, int);
extern int key_cmd(void);
extern int menu_dialog(char *, int, int);
extern int panel_frame(int, int, int, int, int);
extern int draw_box(int, int, int, int, int);
extern int draw_panel(int, int, int, int, int, int);
