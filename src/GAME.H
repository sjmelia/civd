/* GAME.H: shared declarations of symbols defined in GAME.C. */
#ifndef GAME_H
#define GAME_H
#include "GRAPHICS.H"

/* Game rule tables, defined in GAME.C. */
struct advance {
    char name[20];
    signed char prereq1, prereq2;
};

struct improvement {
    char name[24];
    int cost, maintenance, prereq;
};

struct advance_attr {
    unsigned char a;
    signed char b;
};
extern struct advance advances[];
extern struct improvement improvements[];
#define building_names (improvements + 1)       /* from the Palace on */
#define achievement_names (improvements + 24)   /* the wonders */
extern struct advance_attr advance_attrs[];
extern char *civ_adjectives[];


extern struct page *back_page, *tile_page, *spare_page;


extern int improvement_words[];
extern int spiral_x[];
extern int spiral_y[];
extern int civ_colours[];
extern int civ_shadows[];
extern char *government_names[];
extern char *trade_goods[];
extern char *civ_names[];
extern char *leader_names[];
extern char *leader_titles[];
extern int selected_index;
extern char ega_palette[];
extern int has_mouse;
extern int low_memory;
extern void poll_mouse(void);
extern void mouse_force_on(void);
extern void mouse_restore(void);
extern int disk_swap;
extern void install_break_handlers(void);
extern void restore_break_handlers(void);
extern void game_menu(void);
extern void start_game(void);
extern void set_cursor(int, int);
extern void mouse_off(void);
extern void mouse_on(void);

#endif
