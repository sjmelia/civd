/* DIPLOMAC.H: shared declarations of symbols defined in DIPLOMAC.C. */

extern void city_phase(int);
extern void set_government(int, int);
extern void diplomacy_event(int, int, int, int);
extern void relation_set(int, int, int);
extern int alliance_ok(int, int);
extern int trade_advances(int, int);
extern int nearest_unit_distance(int, int, int);
extern int score_of(int, int, int);
extern int value_of(int, int);
extern int power_of(int);
extern void accord(int, int, int);
