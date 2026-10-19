/* PALACE.H: shared declarations of symbols defined in PALACE.C. */

extern void palace_view(int);
/* Calls use differing argument counts; retain the unprototyped interface. */
extern void palpic();
/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern void calc_section_type();
extern void draw_section(int, int, int, int, int);
/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern void load_castle_tiles();
