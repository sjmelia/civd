/* ECONOMY.H: shared declarations of symbols defined in ECONOMY.C. */

extern void civ_economy(int);
extern void vanish(int, int, int);
extern void backdrop_save(void);
extern void backdrop_load(void);
extern void kill(int, int);
extern int find_slot(int, int);
extern void turns_text(int, int);
extern void give_advance(int, int, int);
extern int has_tech(int, int);
extern int advance_value(int, int);
extern int reduce_attitude(int, int);
