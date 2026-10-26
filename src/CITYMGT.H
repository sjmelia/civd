/* CITYMGT.H: shared declarations of symbols defined in CITYMGT.C. */

extern int working_tiles;
extern int city_view_mode;
/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern int city_process();
extern int radius_dx[21];
extern int radius_dy[21];
extern int radius_index(int, int);
extern void add_tile_yield(int, int, int);
extern int tile_yield(int, int, int);
extern int civ_wonder(int, int);
extern int active_wonder_city(int);
extern void set_specialist(int, int);
extern int get_specialist(int);
extern int count_specialists(int);
extern void fit_citizens(int, int);
extern void draw_icon(int, int, int);
extern void framed_panel(int, int, int, int);
extern void tile_panel(int, int, int, int);
extern void draw_button(int, int, int, int, char *, int);
extern void draw_minimap(void);
extern void minimap_dot(int, int, int);
extern void specialist_warning(void);
extern void add_pollution(int, int);
extern int draw_city_view(int, int, int, int, int);
extern void truncate_text(char *, int);
extern int nuke_fallout(int, int);
