/* REPLAY.H: shared declarations of symbols defined in REPLAY.C. */

extern void conquest_replay(void);
extern void replay(void);
extern void replay_line(void);
extern void replay_map(void);
extern void conquest_scene(int, int);
extern void replay_area(int, int);
extern void replay_found(int, int, int);
extern void replay_names(int);
extern int replay_nearest(int, int);
extern void replay_owner(int, int, int);
extern int replay_year(int);
