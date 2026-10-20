/* Near storage: the game's uninitialized shared variables, in layout order.
 * They are communals; LINK places them in c_common by hash bucket, so these
 * names ascend in its order. See GLOBALS.H for the tie rule. */
#include <dos.h>
#include "CIV.H"
#define GLOBALS_STORAGE
#include "GLOBALS.H"

int palace_remote;              /* distance from the city to its palace */
int main_option_result;         /* title menu selection */
unsigned char intro_kept_pal[48];       /* saved palette entries 2..17 */
int map_view_changed;           /* city view replaced the screen */
int city_count[8];              /* cities per civ */
int troops_total[8];            /* units per civ */
int cur_tier;                   /* difficulty level (0..4) */
int best_camp_far;              /* distance to the best city site found */
int players;                    /* mask of civs still in play */
int draw_large_text;            /* strings drawn at double width */
int ai_ruler_refused;           /* last exchange ended in threat or refusal */
int civ_id;                     /* civ processed by ai_turn */
int human_counts;               /* human civs found this turn */
int int_reserved_one;           /* unused */
int ai_continent_towns[8][16];  /* cities per civ and continent */
int city_label_n;               /* city labels on the map */
int plant_waste_divisor;        /* production pollution divisor from plants */
int town_gap;                   /* distance found by the last nearest-city search */
int history_rating[8];          /* historian score per civ */
int defenders_island[8][16];    /* defence strength per civ and continent */
int map_label_xorg[32];         /* x of each city label on the map */
int small_space_check;          /* run the low-memory check */
int civ_research_tally[8];      /* research accumulated toward the next advance */
int terrain_coast_sprites[32];  /* river tiles by neighbour mask; GLOBALS.H names the parts */
int map_label_yorg[32];         /* y of each city label on the map */
int int_filler_b;               /* unused */
int map_rng_state;              /* map random generator state */
int military_table[8];          /* military strength per civ */
int mouse_cursor;               /* default mouse cursor shape */
int polluted;                   /* polluted tiles on the map */
int city_glad_glyphs[9];        /* citizen icons; GLOBALS.H names the parts */
int marvel_site[22];            /* city holding each wonder (-1 none) */
int in_at_war;                  /* current civ is at war with someone */
int view_xorg;                  /* world x at the overview map's left edge */
int see_fight_all;              /* show combat outside the player's view */
int town_res_sums[4];           /* city food, shields, trade and luxury */
int town_happy_ones_num;        /* happy citizens */
int town_unhappy_counts;        /* unhappy citizens */
int learning_yield;             /* city science output */
int town_human_unit_near;       /* human-owned unit in the work radius */
int view_yorg;                  /* world y at the overview map's top edge */
struct city townbuf[128];       /* city records */
int marks_pics[4];              /* marker tiles for the map */
int council_faces;              /* advisor face frame matching the leader (0..2) */
int custom_params[4];           /* land mass, temperature, climate and age */
int portrait_sprites[14][2];    /* leader eye [0] and mouth [1] tiles per frame */
int human_set;                  /* mask of human-controlled civilizations */
unsigned char coarse_ocean_map[20][13]; /* sea connectivity per coarse map cell */
int nation_ratings[8];          /* rank of each civ by score */
int max_rating;                 /* best value found so far */
int icons_council[4][3];        /* advisor bodies [i][0] and heads [i][1..2] */
int need_ok_key;                /* dialog shows an OK button */
int civ_settlers_n[8];          /* settlers per civ */
char civ_vis_grid[80][50];      /* per-civ visibility bits by map square */
int gangmode;                   /* AI civs gang up on the leader */
int civ_cont_attackers[8][16];  /* attack strength per civ and continent */
int houses_sprite[5][4];        /* city view house tiles by size and era */
int city_structure_glyphs[24];  /* city view building tiles */
int turn_tick;                  /* turns elapsed since the start */
struct unit armydata[8][128];   /* unit records per civ */
char takeover_seen;             /* conquests shown in the replay */
int money[8];                   /* gold per civ */
int throne_room;                /* throne room backdrop tile */
int show_city_display;          /* show the city screen after this city's turn */
int warm_levels;                /* global warming counter */
int entry_first;                /* first selectable line of a dialog */
int ai_zone_places[16];         /* free city sites per continent */
int egamode;                    /* EGA driver loaded */
int sci[8];                     /* science rate per civ, tenths */
int year;                       /* game year (negative BC) */
int section_grades[12];         /* palace section styles */
int advances_future;            /* Future Technology advances discovered */
int section_art[8][2];          /* palace section tiles */
int icon_x_draw;                /* x of the next city icon */
char royal_choices[8][2];       /* palace part frame per section */
char *file_read;                /* read pointer into the image buffer */
void (interrupt far * orig_break_isr) ();       /* saved Ctrl-Break vector */
int extra_3;                    /* unused */
int dialog_dim;                 /* disabled menu items mask */
int still;                      /* suppress messages and animation */
int map_paths_icon[16];         /* road tiles by direction */
int map_rail_shapes[16];        /* railroad tiles by direction */
int village_icons;              /* tribal hut tile */
int icon_y_draw;                /* y of the next city icon */
char aim_count[8][32];          /* AI target weight per civ and slot */
int forces_present[8][28];      /* units per civ and type */
int tally_final;                /* score entering the Hall of Fame */
char walk_cost[16][16];         /* path search costs around the origin */
int colour_shield_tiles;        /* civ colour shield tile */
int pop_smog_advances;          /* industrial advances adding population pollution */
int complete_flag;              /* game has ended */
int world_landscape_sprite[12][16];     /* terrain tiles per type and neighbour mask */
char msgtext[6144];             /* text, then palette buffers; GLOBALS.H names the parts */
int box_info_flags;             /* dialog is a plain message */
int menu_item_count;            /* dialog items */
int stk_plr;                    /* owner of the stack last found by map_stack */
int cleared[8];                 /* per-civ word, only ever cleared */
int bar_set[3][20];             /* stacked bar tiles */
int map_tile_edge[8][4];        /* coast edge tiles */
int mouth_deltas_sprites[4];    /* river mouth tiles */
int parley_tone;                /* set with style; otherwise unread */
int cost_penalty;               /* difficulty adjustment to research cost */
int city_leak;                  /* trade lost to corruption */
int refresh_pending;            /* redraw the whole dialog */
int last_met_turn[8];           /* turn of the last audience per civ (-1 none) */
int civ_total;                  /* civs in the game */
int city_keep;                  /* shields spent on unit support */
int citizens[8];                /* total city sizes per civ */
int civ_units_ordered[8][28];   /* cities building each unit type, per civ */
int linewide;                   /* widest dialog line */
int cmd_code;                   /* key pressed in the menu bar */
int map_x0;                     /* map view left column */
int statuspics[320];            /* sprites cut from the sprite sheet; GLOBALS.H names the parts */
int saved_found;                /* mask of saved game slots present */
union REGS bios_call_reg_block_io;      /* registers for BIOS calls */
int map_y0;                     /* map view top row */
int num_ai;                     /* AI civs found this turn */
int vga_modes;                  /* MCGA driver loaded (256 colours) */
int marvel_points;              /* score from wonders, 20 each */
int merit_people;               /* score from happy and content citizens */
struct _iobuf *credits_stream;  /* credits text file */
int beginner_help;              /* tutorial help enabled */
int setting_value;              /* value passed to the current dialog */
char spot_classes[8][16];       /* AI site kind per civ and slot (-1 free) */
int fight_enemy;                /* civ of the defending unit */
int civnow;                     /* civ whose turn it is */
int marked_mask;                /* marked menu items mask */
int civ_tribe_of[8];            /* tribe shown for each civ (+8 alternate) */
int audience_active;            /* audience under way; blocks re-entry */
int xray_on;                    /* debug view: show everything */
int player_peace_run;           /* peaceful turns since the last non-barbarian battle; counted in AD */
char nstr[12];                  /* number formatting buffer */
char block_steps[20][13];       /* path cost per coarse map cell */
int (*pic_reader)();            /* routine decoding the current image */
int floors_palace[12];          /* palace section heights */
unsigned char diskbuffer[512];  /* file input block */
int warming;                    /* global warming events so far */
int abort_flag;                 /* user aborted the operation */
int mse_held;                   /* mouse buttons held */
int ptr_x;                      /* mouse x */
int ptr_y;                      /* mouse y */
int int_unused_dd;              /* unused */
int margin_democracy;           /* drops with units abroad; >=0 lets AI pick democracy */
unsigned char block_ground_way[20][13]; /* land connectivity per coarse map cell */
int leave;                      /* quit requested */
unsigned char itemkeys[32];     /* dialog item hotkeys */
int ratio_score;                /* Hall of Fame score scale */
char camp_xpos[8][16];          /* AI site x per civ and slot */
int x_cap[8];                   /* x of the capital; centres map views */
int bare_flag;                  /* map view without units and improvements */
int continent_span[2][64];      /* land [0] and sea [1] tiles per area */
int civ_ratings[8];             /* score per civ */
int panel_frame_rim;            /* right edge of the current dialog */
int spaceship_arrive_dates[8];  /* spaceship arrival year per civ (-1 none) */
int live_civilizations;         /* civs counted this turn */
char civ_lot_y[8][16];          /* AI site y per civ and slot */
int rocket_module[10];          /* spaceship parts by kind */
int unused_support;             /* cleared with the support counters; never read */
int home_troops_maintained;     /* units homed in the city */
int found_floppy;               /* game disks found in the drive */
int free_fifth[23];             /* unused */
int pointer_hide_count;         /* mouse hide depth */
int space_ship_habitation[8];   /* spaceship habitation modules per civ */
int civ_gov[8];                 /* government per civ */
int banner_tile;                /* newspaper banner tile */
int civ_toll[8];                /* tax rate per civ, tenths (luxury is the remainder) */
int civ_flags_bits;             /* civs using their alternate tribe */
int label_no[32];               /* city of each map label */
int reload;                     /* turn resumes a loaded game */
int adviser_faces_glyphs[3];    /* advisor face tiles */
int slotsel;                    /* selected save slot */
int civ_held_cells[8];          /* coarse 4x4 cells holding own units or cities */
int taxes;                      /* city tax output (gold) */
int which_data;                 /* data file now open */
unsigned char unpack_raster[320];       /* image decoder line buffer */
void (interrupt far * int_23) ();       /* saved Ctrl-C vector */
char objectives_type[8][32];    /* AI target kind per civ and slot */
int troop_over;                 /* unit support over free units; >0 keeps AI government */
int rule_options;               /* game rule flags (all set at start) */
int colonist_homed;             /* settlers homed in the city */
int advances_known[8][5];       /* known advances per civ, bitmask */
int killed[8][8];               /* units destroyed per civ pair */
int treaties[8][8];             /* diplomatic state per civ pair */
char ai_camp_site_weights[8][16];       /* AI site weight per civ and slot */
char civ_targets_x[8][32];      /* AI target x per civ and slot */
int civ_area_plan[8][16];       /* AI continent policy: 0 none, 1 attack, 2 defend, 5 transport */
char civ_targets_y[8][32];      /* AI target y per civ and slot */
int tech_count[8];              /* advances known per civ */
int liftoff_when[8];            /* spaceship launch year per civ */
int city_experts_num;           /* specialist (entertainer) count */
