/* GLOBALS.H: shared near storage defined in GLOBALS.C.
 * LINK orders communals by name hash, with the last first-seen name first
 * on ties. Declaration order here controls MSC's external emission order;
 * retain it when editing names so equal-hash objects keep their layout. */
struct _iobuf;

extern int warming;
extern int ptr_x;
extern int ptr_y;
extern int ratio_score;
extern char camp_xpos[8][16];
extern int x_cap[8];            /* x of the capital; centres map views */
extern int bare_flag;           /* map view without units and improvements */
extern int found_floppy;        /* game disks found in the drive */
extern int civ_toll[8];         /* tax rate per civ, tenths (luxury is the remainder) */
extern int civ_flags_bits;
extern int label_no[32];
extern int reload;
extern int adviser_faces_glyphs[3];
extern int slotsel;
extern int colonist_homed;      /* settlers homed in the city */
extern int advances_known[8][5];
extern char civ_targets_y[8][32];
extern int tech_count[8];       /* advances known per civ */
extern int rule_options;
extern int troop_over;          /* unit support over free units; >0 keeps AI government */
extern int city_experts_num;    /* specialist (entertainer) count */
extern int liftoff_when[8];
extern int palace_remote;       /* distance to nearest own palace */
extern int main_option_result;
extern unsigned char intro_kept_pal[48];
extern int marvel_points;       /* score from wonders, 20 each */
extern int city_count[8];       /* cities per civ */
extern int best_camp_far;
extern int players;
extern int town_gap;
extern int mouse_cursor;
extern int see_fight_all;
extern int view_yorg;           /* world y at the overview map's top edge */
extern int council_faces;       /* advisor face frame matching the leader (0..2) */
extern int human_set;           /* mask of human-controlled civilizations */
extern int civ_cont_attackers[8][16];   /* attack strength per civ and continent */
#ifdef GLOBALS_STORAGE
extern struct unit armydata[8][128];    /* unit records per civ */
#endif
extern char takeover_seen;
extern int money[8];
extern int sci[8];
extern int year;
extern int section_grades[12];
extern int advances_future;     /* Future Technology advances discovered */
extern int section_art[8][2];
extern int icon_x_draw;
extern char royal_choices[8][2];
extern int icon_y_draw;
extern char aim_count[8][32];
extern int forces_present[8][28];       /* units per civ and type */
extern int tally_final;
extern int pop_smog_advances;   /* industrial advances adding population pollution */
extern int refresh_pending;
extern int last_met_turn[8];
extern int civ_total;
extern int civ_units_ordered[8][28];    /* cities building each unit type, per civ */
extern int linewide;
extern int statuspics[320];
extern int map_y0;
extern int num_ai;
extern int vga_modes;           /* MCGA driver loaded (256 colours) */
extern int civ_tribe_of[8];
extern int audience_active;     /* audience under way; blocks re-entry */
extern char block_steps[20][13];
extern unsigned char diskbuffer[512];
#ifdef GLOBALS_STORAGE
extern union REGS bios_call_reg_block_io;       /* registers for BIOS calls */
#endif
extern int saved_found;
extern int marked_mask;
extern int map_view_changed;    /* city view replaced the screen */
extern int floors_palace[12];
extern int (*pic_reader)();
extern int mse_held;
extern int abort_flag;
extern unsigned char itemkeys[32];
extern int leave;
extern int civ_ratings[8];
extern int continent_span[2][64];
extern int spaceship_arrive_dates[8];   /* spaceship arrival year per civ (-1 none) */
extern int panel_frame_rim;
extern char civ_lot_y[8][16];
extern int live_civilizations;
extern int banner_tile;
extern int civ_gov[8];
extern char nstr[12];
extern int player_peace_run;
extern int xray_on;
#ifdef GLOBALS_STORAGE
extern unsigned char block_ground_way[20][13];  /* land connectivity per coarse map cell */
#endif
extern int margin_democracy;    /* drops with units abroad; >=0 lets AI pick democracy */
extern int int_unused_dd;
extern int home_troops_maintained;      /* units homed in the city */
extern int unused_support;      /* cleared with the support counters; never read */
extern int rocket_module[10];
extern int space_ship_habitation[8];
extern int pointer_hide_count;
extern int free_fifth[23];
extern int which_data;
extern int taxes;               /* city tax output (gold) */
extern int civ_held_cells[8];   /* coarse 4x4 cells holding own units or cities */
extern char objectives_type[8][32];     /* AI target kind per civ and slot */
extern void (interrupt far * int_23) ();
extern unsigned char unpack_raster[320];
extern int cur_tier;
extern int troops_total[8];
extern int ai_continent_towns[8][16];   /* cities per civ and continent */
extern int int_reserved_one;
extern int plant_waste_divisor; /* production pollution divisor from plants */
extern int city_label_n;
extern int city_glad_glyphs[9];
extern int polluted;
extern int marks_pics[4];
#ifdef GLOBALS_STORAGE
extern struct city townbuf[128];        /* city records */
#endif
extern int portrait_sprites[14][2];     /* leader eye [0] and mouth [1] tiles per frame */
extern int custom_params[4];
extern int nation_ratings[8];
#ifdef GLOBALS_STORAGE
extern unsigned char coarse_ocean_map[20][13];  /* sea connectivity per coarse map cell */
#endif
extern int colour_shield_tiles;
extern char walk_cost[16][16];
extern int cleared[8];          /* per-civ word, only ever cleared */
extern int stk_plr;             /* owner of the stack last found by map_stack */
extern int map_tile_edge[8][4];
extern int bar_set[3][20];
extern int parley_tone;         /* set with style; otherwise unread */
extern int mouth_deltas_sprites[4];
extern int city_leak;           /* trade lost to corruption */
extern int cost_penalty;
extern int citizens[8];         /* total city sizes per civ */
extern int city_keep;           /* shields spent on unit support */
extern int map_x0;
extern int cmd_code;
extern int terrain_coast_sprites[32];
extern int civ_research_tally[8];       /* research accumulated toward the next advance */
extern int small_space_check;
extern int map_label_xorg[32];
extern int defenders_island[8][16];     /* defence strength per civ and continent */
extern int history_rating[8];
extern int military_table[8];
extern int map_rng_state;
extern int int_filler_b;
extern int view_xorg;           /* world x at the overview map's left edge */
extern int in_at_war;           /* current civ is at war with someone */
extern int marvel_site[22];
extern int turn_tick;           /* turns elapsed since the start */
extern int city_structure_glyphs[24];
extern int houses_sprite[5][4];
extern int village_icons;
extern int map_rail_shapes[16]; /* railroad tiles by direction */
extern int map_paths_icon[16];  /* road tiles by direction */
extern int civnow;
extern int fight_enemy;
extern char spot_classes[8][16];        /* AI site kind per civ and slot (-1 free) */
extern int setting_value;
extern int beginner_help;
extern struct _iobuf *credits_stream;
extern int merit_people;        /* score from happy and content citizens */
extern int gangmode;
#ifdef GLOBALS_STORAGE
extern char civ_vis_grid[80][50];       /* per-civ visibility bits by map square */
#endif
extern int civ_settlers_n[8];   /* settlers per civ */
extern int need_ok_key;
extern int icons_council[4][3]; /* advisor bodies [i][0] and heads [i][1..2] */
extern int max_rating;
extern int egamode;             /* EGA driver loaded */
extern int ai_zone_places[16];
extern int entry_first;
extern int warm_levels;
extern int show_city_display;   /* show the city screen after this city's turn */
extern int throne_room;
extern int town_human_unit_near;        /* human-owned unit in the work radius */
extern int learning_yield;      /* city science output */
extern int town_unhappy_counts; /* unhappy citizens */
extern int town_happy_ones_num; /* happy citizens */
extern int town_res_sums[4];
extern int still;
extern int dialog_dim;
extern int extra_3;
extern void (interrupt far * orig_break_isr) ();
extern char *file_read;
extern int menu_item_count;
extern int box_info_flags;
extern char msgtext[6144];
extern int world_landscape_sprite[12][16];      /* terrain tiles per type and neighbour mask */
extern int complete_flag;
extern int civ_area_plan[8][16];        /* AI continent policy: 0 none, 1 attack, 2 defend, 5 transport */
extern char civ_targets_x[8][32];
extern char ai_camp_site_weights[8][16];
#ifdef GLOBALS_STORAGE
extern int treaties[8][8];      /* diplomatic state per civ pair */
#endif
extern int killed[8][8];
extern int human_counts;
extern int civ_id;
extern int ai_ruler_refused;    /* last exchange ended in threat or refusal */
extern int draw_large_text;
extern int map_label_yorg[32];

/* Views into the tables above. Not communals: keep them after the
 * declarations, whose order sets LINK's tie order. */

/* statuspics is one table of sprite handles, drawn by number with
 * label_with_style: 0-5 status icons, 6 pollution, 7 title cursor, 8-15
 * cursor arrows, 16-31 city tiles by size and walls, 32-63 city tiles in
 * each civ colour, 64-319 unit tiles, 32 per civ colour. */
#define map_fumes_tile (statuspics[6])
#define splashhand (statuspics[7])
#define pointer_cursor_icon (statuspics + 8)
#define towns_icons_size (statuspics + 16)
#define town_tiles_2 (statuspics + 32)
#define colour_unit_glyph (statuspics + 64)

/* city_glad_glyphs is the nine citizen icons: 0-1 happy, 2-3 content, 4-5
 * unhappy, 6-8 the specialists, indexed by kind 1-3 from
 * specialist_people. */
#define quiet_persons (city_glad_glyphs + 2)
#define city_angry_images (city_glad_glyphs + 4)
#define specialist_people (city_glad_glyphs + 5)

/* terrain_coast_sprites is the river tiles by neighbour mask (sheet row
 * 64), then a second set (row 80) that seen_map adds 16 for. */
#define terrain_tracks_tiles (terrain_coast_sprites + 16)

/* msgtext is one 6144-byte block: the message text (1000 bytes), then
 * three 1000-byte palette buffers, the last followed by space nothing else
 * names. The resident code uses all of it as scratch: the picture decoder's
 * string table (2048 codes of three bytes) and the palette fade's three
 * palettes. A palette buffer holds a record: six header bytes, then three
 * per colour. */
#define colours_area ((unsigned char *) msgtext + 1000)
#define pic_colours_data ((unsigned char *) msgtext + 2000)
#define icon_colours_block ((unsigned char *) msgtext + 3000)
/* Colour 128 on of each palette record. */
#define pal_begin_colours (pic_colours_data + 6 + 128 * 3)
#define palette_to_pal (icon_colours_block + 6 + 128 * 3)

/* The city's luxury output: city_process clears all four sums. */
#define city_revel_sums (town_res_sums[3])
