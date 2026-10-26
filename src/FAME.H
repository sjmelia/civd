/* FAME.H: shared declarations of symbols defined in FAME.C. */

extern void fame_load(void);
extern void fame_save(void);
extern int fame_slot;
extern void fame_header(int, int);
extern void top_five(void);
extern int show_fame(int);
/* Calls use differing argument counts; retain the unprototyped interface. */
extern int fame_insert();
