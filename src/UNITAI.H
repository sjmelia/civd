/* UNITAI.H: shared declarations of symbols defined in UNITAI.C. */

extern int near_human;
extern void unit_phase(int);
extern int ai_order(int, int);
extern void ai_break(void);
extern void unit_cleanup(int);
extern void add_target(int, int, int, int, int);
extern void target_shift(int, int);
extern void site_clear(int);
extern void site_add(int, int, int, int, int);
extern void site_shift(int, int);
extern void clear_sites_near(int, int, int, int, int);
extern void retarget(int, int);
extern void city_retarget(int);
extern int ocean_edge(int, int, int);
extern int unit_ai_move(int, int);
extern void city_captured(int);
