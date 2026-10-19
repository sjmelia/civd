/* WORLDGEN.H: shared declarations of symbols defined in WORLDGEN.C. */

extern void make_world(void);
extern int land_corner(int, int);
extern void make_rivers(void);
extern void grow_land(void);
extern void walk_land(int, int);
extern void number_regions(void);
extern void score_sites(void);
/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern void link_land();
/* Calls use differing argument counts; retain the unprototyped interface. */
extern void link_seas();
extern int gen_terrain_at(int, int);
extern int gen_region_at(int, int);
extern int story_mode;
extern int story_step(void);
extern void global_warming(int);
