/* SAVE.H: shared declarations of symbols defined in SAVE.C. */

extern int choose_drive(void);
extern void transfer_game(void);
extern int disk_ready(int);
extern int load_game(int);
extern int load_save(char *);
extern int read_save(char *);
extern void save_game(int);
extern void save_write(int, char *, int);
extern void transfer_far(char far *, int);
extern void transfer_near(char *, int);
extern int write_save(char *);
