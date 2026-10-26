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

/* 16 tribes, 58 bytes each; entries 0 and 8 are empty. The first three
 * fields are personality traits from -1 to 1. */
struct civ_type {
    char leader[16];
    char plural[16];            /* empty: the adjective plus "s" */
    char adjective[16];
    int aggressive;             /* 1 Aggressive, -1 Friendly */
    int expansionist;           /* 1 Expansionistic, -1 Perfectionist */
    int civilized;              /* 1 Civilized, -1 Militaristic */
    int motif;                  /* tune 19-32: emissary, capture, discovery */
    int anthem;                 /* tune 5-18: leader portrait */
};
extern struct civ_type civt[];
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
/* Calls use differing argument counts; retain the unprototyped interface. */
extern int disk_insert();
/* Calls use differing argument counts; retain the unprototyped interface. */
extern int sound();
extern void game_menu(void);
extern void start_game(void);
extern void set_cursor(int, int);
extern void mouse_off(void);
extern void mouse_on(void);
extern char far city_site_x[], far city_site_y[];

#endif
