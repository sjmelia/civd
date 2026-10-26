/* STATUS.H: shared declarations of symbols defined in STATUS.C. */

extern void kill_table(void);
extern void civ_info(int);
/* Calls use differing argument counts; retain the unprototyped interface. */
extern void unit_table();
