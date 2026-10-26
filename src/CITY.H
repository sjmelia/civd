/* CITY.H: shared declarations of symbols defined in CITY.C. */

extern int city_create(int, int, int, int, int);
extern void city_disaster(int);
extern void place_icon(int);
/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern int civ_score();
