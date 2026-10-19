/* CORE.H: shared declarations of symbols defined in CORE.C. */

extern void initialize_random(void);
extern int clamp(int, int, int);
extern int near_city(int, int);
extern int nearest_record(int, int, int, int);
extern int half_dist(int, int);
extern int distance_twice(int, int);
extern int map_dist(int, int, int, int);
extern void format_total(int);
extern void draw_backdrop(int, int, int, int);
extern void restore_palette();
extern void copy_to_far();
extern void copy_from_far();
extern void free_block();
extern void setup_cycles(void);
extern void cycle_on(void);
/* Calls use differing argument counts; retain the unprototyped interface. */
extern void cycle_off();
extern void set_dac(int, int, int, int);
extern int count_weighted_units(int);
