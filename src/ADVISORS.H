/* ADVISORS.H: shared declarations of symbols defined in ADVISORS.C. */

extern void advisor_open(char *, int);
extern void advisor_header(char *, int);
extern void advisor_close(void);
extern void sci_rpt(int);
extern void mil_rpt(int);
extern void military_losses(int);
extern void trade_rpt(int);
/* Calls use differing argument counts; retain the unprototyped interface. */
extern void spy_rpt();
extern void civ_dossier(int);
extern void mood_rpt(int);
extern void append_percent(int, int);
extern void city_rpt(int);
