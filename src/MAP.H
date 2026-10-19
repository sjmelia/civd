/* MAP.H: shared declarations of symbols defined in MAP.C. */

extern int terrain_map[];
extern int redraw_stack(int, int);
extern int map_in_bounds(int, int);
extern int map_city(int, int);
extern int has_road(int, int);
extern int known_flags(int, int);
extern int get_city(int, int);
extern int map_random(int);
extern int patterned_cell(int, int, int);
extern int has_hut(int, int, int);
extern int region_key(int, int);
extern int map_region(int, int);
extern int draw_unit(int, int, int, int);
extern int flag_tile(int, int, int);
extern int map_refresh(int, int, int);
extern int occupy(int, int);
extern int unflag(int, int, int);
extern int map_claim(int, int, int, int);
extern int plot_pixel(int, int, int);
extern int put_unit(int, int, int, int);
