/* SPACESHP.H: shared declarations of symbols defined in SPACESHP.C. */

extern int game_flags;
extern void spaceship_open(void);
extern void spaceship_close(void);
/* Calls use differing argument counts; retain the unprototyped interface. */
extern int spaceship_view();
extern int ship_connected(int, int, int);
extern void spaceship_stars(void);
extern int spaceship_ai_build(int, int);
extern void spaceship_build(int, int);
extern void spaceship_tile(int, int, int);
extern int spaceship_fits(int, int, int, int);
extern int spaceship_free(int, int, int);
extern void spaceship_inspect(void);
extern void spaceship_launch(int);
