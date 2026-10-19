/* BATTLE.H: shared declarations of symbols defined in BATTLE.C. */

/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern int battle();
extern int stack_add(int, int, int);
extern int stack_remove(int, int);
extern int attack_ok(int);
extern int atom(int, int, int);
extern int nuke_fallout(int, int);
