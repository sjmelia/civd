/* REPORTS.H: shared declarations of symbols defined in REPORTS.C. */

extern void cont_table(void);
extern void empty_report(void);
extern void historian_report(void);
extern void power_graph(void);
extern void wonders(void);
extern void demographic_rank(int *, int, char *);
extern void demographics(int);
/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern void world_map();
