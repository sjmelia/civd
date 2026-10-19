/* UNITS.H: shared declarations of symbols defined in UNITS.C. */

extern int quiz_busy;
extern void unpack_garrison(int);
extern void garrison_in(int);
/* Callers pass long arguments to int parameters; preserve their stack layout. */
extern void unit_sight();
extern void blink_unit(int, int, int, int);
extern int select_location_record(int, int);
extern int stack_other(int, int);
extern void score_group_record(int, int);
extern int stack_len(int, int, int);
extern void add_group_value(int, int);
extern int count_u(int, int, int);
extern void count_matching_record(int, int);
extern int stack_strength(int, int, int);
extern void match_type_category(int, int);
extern int stack_cap(int, int);
extern void count_weighted_record(int, int);
extern void validate_record(int, int);
extern void remove_reset_kind_zero(int, int);
extern void kill_stack_cb(int, int);
extern void remove_reset_kind_one(int, int);
extern int if_open(int, int, int);
extern int unit_blocked(int, int, int);
/* Calls use differing argument counts; retain the unprototyped interface. */
extern int adj_grp();
extern void enter_hut(int, int);
extern void anim_unit(int, int, int);
extern int pick_unit(int, int);
extern int near_base(int, int);
/* Calls use differing argument counts; retain the unprototyped interface. */
extern void post_event();
extern void menu_begin(void);
extern void menu_end(void);
extern int help_probe(int, int, int);
extern void menu_clear(void);
extern int create_unit(int, int, int, int);
extern void reset_state(int, int);
extern void unit_reset(int, int);
extern void drop_u_stack(int, int);
extern void kill_cb(int, int);
extern void map_center(int, int, int);
