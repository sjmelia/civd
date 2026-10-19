/* CAPTURE.H: shared declarations of symbols defined in CAPTURE.C. */

extern int incited;
/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern void capunit();
extern void spaceship_lost(int);
extern int unrest_chance(int);
extern void steal_advance(int, int);
extern void ad_wait(void);
extern int rpath(int, int, int);
extern void append_place_name(int);
