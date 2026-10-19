/* City turn processing, worker assignment and the interactive city screen. */
#include "CIV.H"
#include "RESIDENT.H"
#include "GAME.H"
#include "BATTLE.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CITYVIEW.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DIPLOMAT.H"
#include "DRAW.H"
#include "ECONOMY.H"
#include "MAP.H"
#include "NAMES.H"
#include "NEWGAME.H"
#include "NEWS.H"
#include "NOTES.H"
#include "SPACESHP.H"
#include "TURN.H"
#include "UNITS.H"
#include "DIPLOMAC.H"
#include "MESSAGE.H"
#include "UNITAI.H"

/* Keep the local data list: GLOBALS.H exhausts MSC 5.1 heap with LOADFIX. */
extern struct city townbuf[];
extern struct unit armydata[][128];
extern int radius_dx[];
extern int radius_dy[];
extern int city_view_mode;
extern int selected_city;
extern int radius_tiles;
extern int selected_owner;
extern int palace_remote;
extern int pop_smog_advances;
extern int year;
extern int city_experts_num;
extern int civnow;
extern int cur_tier;
extern int nation_ratings[];
extern int civ_gov[];
extern int map_x0;
extern int map_y0;
extern int advisor;
extern int rule_options;
extern char msgtext[];

extern char far map_visible_to[];

extern int abs(int);

extern char *strupr(char *);

extern int seen_map(int, int);
extern int map_owner(int, int);
extern void map_note(int, int);

extern void sound(int);

extern char *strcpy(char *, char *);
extern char *strcat(char *, char *);
extern struct unit_type utypes[];
extern int units_abroad;
extern int home_troops_maintained;
extern int city_keep;
extern int colonist_homed;
extern int town_human_unit_near;
extern int town_res_sums[];     /* city food, shields, trade and luxury */
#define city_revel_sums (town_res_sums[3])
extern int human_set;
extern int turn_tick;
extern int show_city_display;

extern int plant_waste_divisor;

extern void post_event(int, int, int, int);
extern int far advance_owners[];
extern int city_count[];

extern int town_gap;
extern int rand_scaled(int);
extern int terrain_at(int, int);
extern int treaties[8][8];
extern int forces_present[8][28];
extern int last_met_turn[];

extern int marvel_site[];
extern int debug;
extern int disk_insert(int, long);

extern int civ_units_ordered[8][28];
extern int game_flags, ai_ship_success, spaceship_arrive_dates[], money[], players;

extern int x_cap[];

extern int map_mark(int, int);
extern int civ_area_plan[8][16];

extern void recolour_rect(struct page *, int, int, int, int, int, int);
extern int city_leak;
extern int sci[], civ_toll[];
extern int taxes, learning_yield;

extern int town_happy_ones_num, town_unhappy_counts, excess_unhappy;

extern void label_with_style(int, int, int);
extern int map_stack(int, int), unused_support;

extern int gangmode, tech_count[], margin_democracy;
extern int troop_over, civ_tribe_of[];
struct civ_type {
    char strings[48];
    int f0, f1, f2, f3, f4;
};
extern struct civ_type civt[];

extern char *itoa(int, char *, int);
extern char nstr[];

extern int village_icons;
extern int troops_total[], mse_held, ptr_x, ptr_y;
extern void flip(int);
extern int kbhit(void);
extern int map_view_changed;

extern int human_counts;

extern char *leader_titles[];
extern int selected_index, civ_research_tally[], cost_penalty, civ_ratings[];
extern int map_flags(int, int);
extern void reveal(int, int, int);

extern int adj_grp(int, int);

int working_tiles = 0;
int city_view_mode = 0;
int radius_dx[21] = { 0, 1, 0, -1, 1, 1, -1, -1, 0, 2, 0, -2, -1, 1, 2, 2, 1, -1, -2, -2, 0 };  /* x offsets of the 21 city-radius tiles */
int radius_dy[21] = { -1, 0, 1, 0, -1, 1, 1, -1, -2, 0, 2, 0, -2, -2, -1, 1, 2, 2, 1, -1, 0 };  /* y offsets of the 21 city-radius tiles */

int radius_index(int first, int second)
{
    int index;
    for (index = 0; index <= 20; index++) {
        if (radius_dx[index] == first && radius_dy[index] == second)
            return index;
    }
    return -1;
}

/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
/* mode: 0 advances the turn, 1 opens the city screen; other values recalculate. */
int city_process(city, mode)
int city;
int mode;
{
    /* Letter suffixes preserve MSC stack slots; several scratch words are reused. */
    int raw_shld_m;             /* tile shields before factory/power bonuses */
    int best_idx_o;
    int choice_f;               /* reused: tie counter, menu selection, unit/build cost */
    int area_role_g;
    int auto_build_i;
    int unit_y_i;
    int next_bld;
    int draw_y_k;               /* reused for drawing, key codes and tile offsets */
    int unit_x_j;
    int factory_i;              /* factory/manufacturing bonus in quarters */
    int draw_x_l;
    int step_cost_m;            /* citizen spacing, later research cost factor */
    int unused_a_m;
    int key_h;
    /* Adjacent low/high words: long accesses below span both declarations. */
    int worked_lo_k;
    int worked_hi_e;
    int index_i;
    int scratch_i;
    int police_e;
    int city_y_p;               /* city coordinate, later improvement-list drawing Y */
    int dead_score_h;
    int pollution_k;
    int cost_h;
    int unused_b_l;
    int first_bld_h;
    int city_x;                 /* city coordinate, later resource-icon drawing X */
    int best_score_h;
    int score_e;
    int radius_y;
    int old_map_y_m;
    int best_food_l;
    int yield_buy_k;            /* tile yield, then AI rush-purchase shields */
    int bld_pages_p;            /* bit 1: MORE available; bit 0: toggled page state */
    int ai_result_i;            /* target score/distance, then spaceship result */
    int tile_x_l;
    int unused_c_k;
    int old_map_x_n;
    int flash_c;
    int cursor_y_k;
    int item_b;
    int unit;
    int tile_keys_h;
    int cursor_x_l;
    int bld_count_g;
    int distance_f;
    int dist_city_e;
    int worker_adj_i;           /* worker changes minus initial free workers */
    int power_d;                /* power bonus in quarters */
    int unused_d_j;
    int unhappy_l;
    /* Neighbouring cities block tiles; unit_list_f holds the clickable garrison units. */
    int blocked_m[5][5];
    int unit_list_f[18];
    int workers_d;
    int unit_count_i;
    int city_index;
    int cost_scale;             /* food and shields per population/cost unit */
    int sett_food_j;
    int trade;
    int scale_i;
    int happy_y_g;
    int wonders_o;
    int tile_taken[21];
    int unused_e_i;
    int unused_f_h;
    int unused_g_g;
    int unused_h_f;
    int shields_e;
    int raw_trade_h;
    int experts_f;
    int icon_x;
    int food_i;
    int stack_len_m;

    if (townbuf[city].status == -1)
        goto label_exit;

    city_view_mode = 0;
    tile_keys_h = 0;
    bld_pages_p = next_bld = first_bld_h = 0;
    mouse_off();

    for (city_x = 0; city_x < 5; city_x++) {
        for (city_y_p = 0; city_y_p < 5; city_y_p++) {
            blocked_m[city_x][city_y_p] = 0;
        }
    }

    city_x = townbuf[city].x;
    city_y_p = townbuf[city].y;
    selected_city = city;
    distance_f = 7;
    palace_remote = (townbuf[city].buildings & 1) ? 0 : 32;

    if (townbuf[city].shield_box > 999 || townbuf[city].shield_box < 0) {
        townbuf[city].shield_box = 0;
    }

    /* Reserve neighbours' worked tiles, including their city centres. */
    for (city_index = 0; city_index < 128; city_index++) {
        if (townbuf[city_index].status == -1 || city_index == city)
            continue;
        {
            dist_city_e = map_dist(city_x, townbuf[city_index].x, city_y_p, townbuf[city_index].y);
            if (dist_city_e < distance_f && dist_city_e != 0) {
                distance_f = dist_city_e;
            }
            if (townbuf[city_index].owner == townbuf[city].owner && (townbuf[city_index].buildings & 1)) {
                if (dist_city_e < palace_remote) {
                    palace_remote = dist_city_e;
                }
            }
            if (dist_city_e <= 5) {
                for (index_i = 0; index_i <= 20; index_i++) {
                    if ((townbuf[city_index].workers & (1L << index_i)) || index_i == 20) {
                        tile_x_l = townbuf[city_index].x + radius_dx[index_i];
                        radius_y = townbuf[city_index].y + radius_dy[index_i];
                        if (abs(tile_x_l - city_x) <= 2 && abs(radius_y - city_y_p) <= 2) {
                            blocked_m[tile_x_l - city_x + 2][radius_y - city_y_p + 2] = 1;
                        }
                    }
                }
            }
        }
    }
    blocked_m[2][2] = 0;

    selected_owner = townbuf[city].owner;
    radius_tiles = 21;
    old_map_x_n = map_x0;
    old_map_y_m = map_y0;

    /* Settlers eat one food under anarchy/despotism, two under later governments. */
    sett_food_j = (civ_gov[selected_owner] > 1) ? 2 : 1;
    pop_smog_advances = 0;
    if (!(townbuf[city].buildings & 0x1000)) {
        if (has_tech(selected_owner, 35))
            pop_smog_advances++;
        if (has_tech(selected_owner, 58))
            pop_smog_advances++;
        if (has_tech(selected_owner, 41))
            pop_smog_advances++;
        if (has_tech(selected_owner, 60))
            pop_smog_advances++;
    }

    /* The AI difficulty discount scales both food boxes and production costs. */
    cost_scale = 10;
    city_experts_num = (int) (townbuf[city].workers >> 26);
    if (selected_owner != civnow) {
        cost_scale = 16 - 2 * cur_tier;
        if (year >= 0 && nation_ratings[civnow] == 7 && cur_tier > 1) {
            cost_scale -= 2;
        }

        if (mode == 0L) {
            if (!(townbuf[city].status & 1)) {
                city_experts_num = 0;
            } else {
                townbuf[city].workers = 0;
            }
        }
    }

label_render_restart:
    city_x = townbuf[city].x;
    city_y_p = townbuf[city].y;

    /* Draw the city radius; outlined tiles belong to neighbouring cities. */
    if (mode == 1L) {
        unpack_garrison(city);
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
        framed_panel(2, 1, 208, 21);
        scratch_i = 0;
        for (index_i = 1; index_i <= townbuf[city].size; index_i++) {
            scratch_i += index_i;
        }
        *(int *) ((char *) scrbuf + 0x10) = 2;
        msgtext[0] = 0;
        append_place_name(city);
        strupr(msgtext);
        strcat(msgtext, " (Pop:");
        format_total(scratch_i);
        strcat(msgtext, ")");
        draw_text(msgtext, 104, 2, 15);
        framed_panel(127, 23, 208, 104);
        map_x0 = townbuf[city].x - 5;
        map_y0 = townbuf[city].y - 3;
        seen_map(townbuf[city].x, townbuf[city].y);

        for (index_i = 0; index_i < radius_tiles; index_i++) {
            tile_x_l = townbuf[city].x + radius_dx[index_i];
            radius_y = townbuf[city].y + radius_dy[index_i];
            if (selected_owner == civnow) {
                if (!(map_visible_to[tile_x_l * 50 + radius_y] & (1 << selected_owner)))
                    continue;
            }
            scratch_i = map_owner(tile_x_l, radius_y);
            if (scratch_i != -1 && scratch_i != selected_owner) {
                map_note(tile_x_l, radius_y);
                continue;
            }
            {
                seen_map(tile_x_l, radius_y);
                if (blocked_m[radius_dx[index_i] + 2][radius_dy[index_i] + 2] != 0) {
                    draw_box((tile_x_l - map_x0) * 16 + 80, (radius_y - map_y0) * 16 + 8, 15, 15, 12);
                }
            }
        }
    }

    /* Resolve last turn's food shortage or growth before calculating new yields. */
    if (mode == 0L) {
        townbuf[city].status &= 0x7f;
        if (townbuf[city].food_box < 0) {
            scratch_i = -1;
            for (index_i = 0; index_i < 128; index_i++) {
                if (armydata[selected_owner][index_i].type == 0 && armydata[selected_owner][index_i].home_city == city) {
                    scratch_i = index_i;
                    break;
                }
            }
            if (selected_owner == civnow) {
                strcpy(msgtext, "Food storage exhausted\nin ");
                append_place_name(city);
                strcat(msgtext, (scratch_i != -1) ? "!\nSettlers lost.\n" : "!\nFamine feared.\n");
                sound(36);
                news_box(city);
                sound(1);
            }
            if (scratch_i != -1) {
                drop_u_stack(selected_owner, scratch_i);
            } else {
                if (--townbuf[city].size == 0) {
                    vanish(city, townbuf[city].x, townbuf[city].y);
                    kill_civ(selected_owner, 0);
                    mouse_on();
                    goto label_exit;
                }
            }
            townbuf[city].food_box = 0;
            note_map(city_x, city_y_p);
        }
        if ((townbuf[city].size + 1) * cost_scale <= townbuf[city].food_box) {
            townbuf[city].size++;
            townbuf[city].food_box = (townbuf[city].buildings & 4) ? townbuf[city].size * 5 + 5 : 0;
            if (townbuf[city].size > 10) {
                if (!(townbuf[city].buildings & 0x100) && (rule_options & 4)) {
                    townbuf[city].size--;
                    if (selected_owner == civnow) {
                        advisor = 4;
                        msgtext[0] = 0;
                        append_place_name(city);
                        strcat(msgtext, " requires an AQUEDUCT\nfor further growth.\n");
                        show_msg(msgtext, 100, 80);
                    }
                }
            }
            note_map(city_x, city_y_p);
        }
    }

    /* Count the centre as a free worker; specialists consume the remaining slots. */
    workers_d = townbuf[city].size + 1;
    colonist_homed = city_keep = home_troops_maintained = units_abroad = 0;

    for (index_i = 0; index_i < 2; index_i++) {
        if (townbuf[city].garrison[index_i] != -1) {
            home_troops_maintained++;
        }
    }

    if (townbuf[city].size < home_troops_maintained) {
        city_keep = home_troops_maintained - townbuf[city].size;
    }

    for (index_i = 0; index_i < 128; index_i++) {
        if (armydata[selected_owner][index_i].type != -1) {
            if (armydata[selected_owner][index_i].home_city == city) {
                if (armydata[selected_owner][index_i].type < 26) {
                    if (townbuf[city].size < ++home_troops_maintained || (civ_gov[selected_owner] > 1 && (rule_options & 2))) {
                        city_keep++;
                    }
                    if (utypes[armydata[selected_owner][index_i].type].attack != 0 && (utypes[armydata[selected_owner][index_i].type].domain == 1 || armydata[selected_owner][index_i].x != townbuf[city].x || armydata[selected_owner][index_i].y != townbuf[city].y)) {
                        units_abroad++;
                    }
                }
                if (armydata[selected_owner][index_i].type == 0) {
                    colonist_homed++;
                }
            }
        }
    }

    /* Hidden, foreign-occupied and neighbour-worked tiles cannot be assigned. */
    town_human_unit_near = 0;
    for (index_i = 0; index_i < radius_tiles; index_i++) {
        tile_taken[index_i] = 0;
        tile_x_l = townbuf[city].x + radius_dx[index_i];
        radius_y = townbuf[city].y + radius_dy[index_i];
        if (selected_owner != 0) {
            if (!(map_visible_to[tile_x_l * 50 + radius_y] & (1 << selected_owner))) {
                tile_taken[index_i] = 1;
            }
        }
        scratch_i = map_owner(tile_x_l, radius_y);
        if (scratch_i != -1 && scratch_i != selected_owner) {
            tile_taken[index_i] = blocked_m[radius_dx[index_i] + 2][radius_dy[index_i] + 2] = 1;
            if (scratch_i == civnow) {
                town_human_unit_near = 1;
            }
        }
        if (blocked_m[radius_dx[index_i] + 2][radius_dy[index_i] + 2] != 0) {
            tile_taken[index_i] = 1;
        }
        if (tile_taken[index_i] != 0) {
            townbuf[city].workers &= ~(1L << index_i);
        }
    }

    for (index_i = 0; index_i < 4; index_i++) {
        town_res_sums[index_i] = 0;
    }
    worked_lo_k = worked_hi_e = 0;

    /* Preserve existing assignments; AI cities normally reconsider every fourth turn. */
    if ((human_set & (1 << selected_owner)) || ((city + turn_tick) & 3) != 0 || (townbuf[city].status & 1) || town_human_unit_near != 0) {

        for (index_i = 0; index_i < radius_tiles; index_i++) {
            if (townbuf[city].workers & (1L << index_i)) {
                workers_d--;
            }
        }

        if (workers_d >= 0) {
            workers_d = townbuf[city].size + 1;
            for (index_i = 0; index_i < radius_tiles; index_i++) {
                if (workers_d != 0 && (townbuf[city].workers & (1L << index_i))) {
                    tile_taken[index_i] = 1;
                    add_tile_yield(city, index_i, mode);
                    workers_d--;
                    *(long *) &worked_lo_k |= (1L << index_i);
                }
            }
        } else {
            if (mode == 0L && selected_owner == civnow) {
                strcpy(msgtext, "Population decrease\nin ");
                append_place_name(city);
                strcat(msgtext, ".\n");
                advisor = 4;
                show_msg(msgtext, 100, 80);
                show_city_display = 1;
            }
            workers_d = townbuf[city].size + 1;
        }

        if (workers_d == city_experts_num || workers_d == 0) {
            goto label_specialists;
        }

    }
label_assign_tiles:
    if (workers_d > 0 && tile_taken[20] == 0) {
        tile_taken[20] = 1;
        add_tile_yield(city, 20, mode);
        *(long *) &worked_lo_k |= (1L << 20);
        workers_d--;
    }

    /* Meet food needs first, breaking equal-food ties with twice shields plus trade.
       best_idx_o is read before initialization in the original; retain that quirk. */
label_food_loop:
    while (((townbuf[city].size * 2 + colonist_homed * sett_food_j > workers_d / 2 + town_res_sums[0]) || townbuf[city].size < 3) && workers_d > city_experts_num && best_idx_o != -1) {
        best_idx_o = -1;
        best_food_l = best_score_h = 0;
        for (index_i = 0; index_i < radius_tiles; index_i++) {
            if (tile_taken[index_i] == 0) {
                tile_x_l = townbuf[city].x + radius_dx[index_i];
                radius_y = townbuf[city].y + radius_dy[index_i];
                yield_buy_k = tile_yield(tile_x_l, radius_y, 0);
                score_e = tile_yield(tile_x_l, radius_y, 1) * 2;
                if (workers_d == 1 && town_res_sums[1] == 0 && score_e == 0)
                    continue;
                score_e += tile_yield(tile_x_l, radius_y, 2);
                if (yield_buy_k > best_food_l || (yield_buy_k == best_food_l && score_e > best_score_h)) {
                    best_food_l = yield_buy_k;
                    best_score_h = score_e;
                    best_idx_o = index_i;
                }
            }
        }
        if (best_idx_o != -1) {
            tile_taken[best_idx_o] = 1;
            add_tile_yield(city, best_idx_o, mode);
            *(long *) &worked_lo_k |= (1L << best_idx_o);
            workers_d--;
        } else {
            break;
        }
    }

label_prod_trade:
    if (selected_owner != civnow) {
        if (townbuf[city].status & 1) {
            city_experts_num++;
        }
    }

    /* Weight scarce resources more heavily when filling the remaining jobs. */
    while (workers_d > city_experts_num) {
        best_idx_o = -1;
        best_food_l = best_score_h = dead_score_h = 0;
        for (index_i = 0; index_i < radius_tiles; index_i++) {
            if (tile_taken[index_i] == 0) {
                tile_x_l = townbuf[city].x + radius_dx[index_i];
                radius_y = townbuf[city].y + radius_dy[index_i];

                score_e = (16 / clamp(town_res_sums[0] - townbuf[city].size * 2 - colonist_homed * sett_food_j, 1, 99)) * (food_i = tile_yield(tile_x_l, radius_y, 0));

                score_e += (shields_e = tile_yield(tile_x_l, radius_y, 1)) * ((townbuf[city].size * 3) / clamp(town_res_sums[1] - city_keep, 1, 99));

                score_e += (trade = tile_yield(tile_x_l, radius_y, 2)) * ((townbuf[city].size * 2) / clamp(town_res_sums[2], 1, 99));

                if (score_e > best_score_h) {
                    best_score_h = score_e;
                    best_idx_o = index_i;
                    dead_score_h = (food_i + shields_e) * 2 + trade;
                }
            }
        }
        if (best_idx_o != -1) {
            tile_taken[best_idx_o] = 1;
            add_tile_yield(city, best_idx_o, mode);
            *(long *) &worked_lo_k |= (1L << best_idx_o);
            workers_d--;
        } else {
            break;
        }
    }

    /* Low 21 bits select worked tiles; bits 26 onward store specialist count. */
label_specialists:
    townbuf[city].workers = ((long) workers_d << 26) + *(long *) &worked_lo_k;
    worker_adj_i = -workers_d;
label_citizens_refresh:
    city_x = townbuf[city].x;
    city_y_p = townbuf[city].y;
    experts_f = workers_d;

    for (index_i = 0; index_i < experts_f; index_i++) {
        if (get_specialist(index_i) == 0) {
            set_specialist(index_i, 3);
        }
    }

    for (index_i = experts_f; index_i < 8; index_i++) {
        set_specialist(index_i, 0);
    }

    /* Factory/manufacturing and power bonuses are quarters of base shields. */
label_governor_done:
    factory_i = power_d = 0;
    plant_waste_divisor = 1;

    if (townbuf[city].buildings & 0x4000L) {
        factory_i = 2;
    }
    if (townbuf[city].buildings & 0x8000L) {
        factory_i = 4;
    }
    if (townbuf[city].buildings & 0x40000L) {
        power_d = 2;
    }
    if (townbuf[city].buildings & 0x80000L) {
        plant_waste_divisor = 2;
        power_d = 2;
    }
    if (townbuf[city].buildings & 0x100000L) {
        plant_waste_divisor = 2;
        power_d = 2;
    }
    if (civ_wonder(selected_owner, 15)) {
        if (region_key(townbuf[city].x, townbuf[city].y) == region_key(townbuf[marvel_site[15]].x, townbuf[marvel_site[15]].y)) {
            plant_waste_divisor = 2;
            power_d = 2;
        }
    }
    if (townbuf[city].buildings & 0x20000L) {
        plant_waste_divisor = 3;
    }

    /* A power plant cannot exceed the factory bonus it supplies. */
    power_d = clamp(power_d, 0, factory_i);
    raw_shld_m = town_res_sums[1];
    raw_trade_h = town_res_sums[2];

    town_res_sums[1] += (factory_i * town_res_sums[1]) / 4 + (power_d * town_res_sums[1]) / 4;

    if (mode == 0L) {

        townbuf[city].food_box += town_res_sums[0] - townbuf[city].size * 2 - colonist_homed * sett_food_j;
        scratch_i = town_res_sums[1];
        scratch_i -= city_keep;
        if (townbuf[city].status & 1) {
            scratch_i = 0;
        }
        townbuf[city].shield_box += scratch_i;
        /* Positive production IDs are units; negative IDs select improvements. */
        if (townbuf[city].production >= 0) {
            if (utypes[townbuf[city].production].cost * cost_scale <= townbuf[city].shield_box) {
                if (townbuf[city].production == 0 && townbuf[city].size == 1 && cur_tier == 0)
                    goto label_ai_buy;
                townbuf[city].shield_box -= utypes[townbuf[city].production].cost * cost_scale;
                unit = -1;
                if (selected_owner == civnow || townbuf[city].production != 26)
                    unit = create_unit(selected_owner, townbuf[city].production, townbuf[city].x, townbuf[city].y);
                if (!(advance_owners[townbuf[city].production] & 8)) {
                    post_event(6, 2, selected_owner, townbuf[city].production);
                    advance_owners[townbuf[city].production] |= 8;
                }
                if ((townbuf[city].buildings & 2) && unit != -1)
                    armydata[selected_owner][unit].status |= 0x20;
                if (unit != -1 && townbuf[city].production == 0) {
                    if (townbuf[city].size > 1 || city_count[selected_owner] > 1) {
                        if (--townbuf[city].size == 0) {
                            vanish(city, townbuf[city].x, townbuf[city].y);
                            unit = create_unit(selected_owner, townbuf[city].production, townbuf[city].x, townbuf[city].y);
                            kill_civ(selected_owner, 0);
                            mouse_on();
                            goto label_exit;
                        }
                    }
                }
                if (selected_owner != civnow && unit != -1) {
                    if (townbuf[city].production == 27) {
                        ai_result_i = best_idx_o = -1;
                        for (city_index = 0; city_index < 128; city_index++) {
                            if (townbuf[city_index].owner != selected_owner && townbuf[city_index].status != -1 && townbuf[city_index].owner != civnow && townbuf[city_index].trade > ai_result_i) {
                                ai_result_i = townbuf[city_index].trade;
                                best_idx_o = city_index;
                            }
                        }
                        rpath(selected_owner, unit, best_idx_o);
                    }
                    /* AI diplomats are placed by an existing unit near a human city. */
                    if (townbuf[city].production == 26) {
                        best_idx_o = -1;
                        ai_result_i = 32767;
                        choice_f = 0;
                        for (city_index = 0; city_index < 128; city_index++) {
                            if (townbuf[city_index].owner != civnow || townbuf[city_index].status == -1)
                                continue;
                            scratch_i = nearest_record(selected_owner, unit, townbuf[city_index].x, townbuf[city_index].y);
                            town_gap = abs(town_gap - 3);
                            if (townbuf[city_index].status & 0x20)
                                town_gap = 3;
                            if (ai_result_i > town_gap || (ai_result_i == town_gap && rand_scaled(++choice_f) == 0)) {
                                if (terrain_at(armydata[selected_owner][scratch_i].x, armydata[selected_owner][scratch_i].y) != 10 && region_key(townbuf[city_index].x, townbuf[city_index].y) == region_key(armydata[selected_owner][scratch_i].x, armydata[selected_owner][scratch_i].y)) {
                                    if (ai_result_i == town_gap)
                                        choice_f = 1;
                                    else
                                        ai_result_i = town_gap;
                                    best_idx_o = scratch_i;
                                    dist_city_e = city_index;
                                }
                            }
                        }
                        if (ai_result_i <= 3 && !(treaties[selected_owner][civnow] & 2)) {
                            unit = create_unit(selected_owner, 26, armydata[selected_owner][best_idx_o].x, armydata[selected_owner][best_idx_o].y);
                            if (unit != -1) {
                                armydata[selected_owner][unit].goto_x = townbuf[dist_city_e].x;
                                armydata[selected_owner][unit].goto_y = townbuf[dist_city_e].y;
                            }
                        } else {
                            townbuf[city].shield_box += utypes[townbuf[city].production].cost * cost_scale;
                        }
                    }
                    if (townbuf[city].production == 25 && forces_present[selected_owner][25] == 1)
                        last_met_turn[civnow] = -1;
                    city_retarget(city);
                } else {
                    townbuf[city].shield_box = 0;
                    if (unit != -1 && (townbuf[city].production == 0 || townbuf[city].production >= 26)) {
                        show_city_display = 1;
                        msgtext[0] = 0;
                        append_place_name(city);
                        strcat(msgtext, " builds ");
                        strcat(msgtext, utypes[townbuf[city].production].name);
                        strcat(msgtext, ".\n");
                        advisor = 3;
                        show_msg(msgtext, 80, 80);
                    }
                }
            }
        } else {
            if (improvements[-townbuf[city].production].cost * cost_scale <= townbuf[city].shield_box) {
                scratch_i = -townbuf[city].production;
                if (scratch_i > 24) {
                    if ((marvel_site - 24)[scratch_i] == -1) {
                        (marvel_site - 24)[scratch_i] = city;
                        post_event(10, 2, selected_owner, scratch_i - 24);
                    } else {
                        scratch_i = -1;
                    }
                } else {
                    if (townbuf[city].buildings & (1L << (scratch_i - 1)))
                        scratch_i = -1;
                }
                if (scratch_i != -1) {
                    if (scratch_i == 41) {
                        strcpy(msgtext, "Sensors report a\nNUCLEAR WEAPONS test\nnear ");
                        append_place_name(city);
                        strcat(msgtext, "!\n");
                        sound(36);
                        news_box(-1);
                        sound(1);
                    }
                    if (scratch_i < 22)
                        townbuf[city].buildings |= 1L << (scratch_i - 1);
                    townbuf[city].shield_box -= improvements[-townbuf[city].production].cost * cost_scale;
                    if ((human_set & (1 << selected_owner)) || scratch_i > 24) {
                        if (human_set & (1 << selected_owner))
                            show_city_display = 1;
                        else
                            map_visible_to[townbuf[city].x * 50 + townbuf[city].y] = -1;
                        msgtext[0] = 0;
                        append_place_name(city);
                        strcat(msgtext, " builds\n");
                        strcat(msgtext, improvements[scratch_i].name);
                        strcat(msgtext, ".\n");
                        if (selected_owner == civnow && (debug & 8) && (scratch_i <= 21 || scratch_i > 24) && !(townbuf[city].status & 0x10) && scratch_i != 1 && disk_insert(1, 0L)) {
                            city_scene(city, scratch_i);
                        } else {
                            news_box(city);
                        }
                        townbuf[city].shield_box = 0;
                    } else {
                        if (scratch_i == 1)
                            townbuf[city].production = -2;
                        townbuf[city].production = find_slot(selected_owner, city);
                        if (townbuf[city].production >= 0)
                            civ_units_ordered[selected_owner][townbuf[city].production]++;
                    }
                    if (scratch_i >= 22 && scratch_i <= 24 && !(game_flags & (1 << selected_owner))) {
                        if (selected_owner == civnow) {
                            spaceship_build(selected_owner, scratch_i - 22);
                        } else {
                            ai_result_i = spaceship_ai_build(selected_owner, scratch_i - 22);
                            if (ai_result_i != 0 && scratch_i < 24) {
                                townbuf[city].shield_box += improvements[scratch_i].cost * cost_scale;
                                if (townbuf[city].production <= -22 && townbuf[city].production >= -24)
                                    townbuf[city].production = -scratch_i - 1;
                                ai_result_i--;
                            }
                            if (!(game_flags & (1 << selected_owner)) && ai_ship_success >= 40) {
                                if (ai_result_i != 0 || (has_tech(civnow, 40) && (ai_ship_success > 75 || nation_ratings[civnow] > nation_ratings[selected_owner] || ((game_flags & (1 << civnow)) && spaceship_arrive_dates[selected_owner] - year <= spaceship_arrive_dates[civnow] - year) || ((game_flags & (1 << (civnow + 8))) && money[players] > 1000)))) {
                                    spaceship_launch(selected_owner);
                                }
                            } else if (ai_result_i != 0) {
                                spaceship_launch(selected_owner);
                            }
                        }
                    }
                    if (scratch_i == 1) {
                        for (city_index = 0; city_index < 128; city_index++) {
                            if (townbuf[city_index].owner == selected_owner)
                                townbuf[city_index].buildings &= ~1L;
                        }
                        if (selected_owner == civnow || (treaties[civnow][selected_owner] & 0x40)) {
                            msgtext[0] = 0;
                            if (selected_owner != civnow) {
                                strcpy(msgtext, "Diplomats report:\n");
                                advisor = 1;
                            }
                            strcat(msgtext, civ_adjectives[selected_owner]);
                            strcat(msgtext, " capital\nmoved to ");
                            append_place_name(city);
                            strcat(msgtext, ".\n");
                            advisor = 5;
                            show_msg(msgtext, 80, 80);
                            x_cap[selected_owner] = townbuf[city].x;
                            townbuf[city].buildings |= 1L;
                        }
                    }
                    if (scratch_i == 38) {
                        mouse_force_on();
                        reduce_attitude(selected_owner, 0);
                        reduce_attitude(selected_owner, 0);
                        mouse_restore();
                    }
                    if (scratch_i == 43 && selected_owner == civnow) {
                        for (city_index = 0; city_index < 128; city_index++) {
                            if (townbuf[city_index].status != -1 && townbuf[city_index].owner != selected_owner) {
                                townbuf[city_index].visible_size = townbuf[city_index].size;
                                map_visible_to[townbuf[city_index].x * 50 + townbuf[city_index].y] |= 1 << selected_owner;
                                map_mark(townbuf[city_index].x, townbuf[city_index].y);
                            }
                        }
                    }

                }
            }

        }

        if (selected_owner == civnow && (townbuf[city].status & 0x10) && (townbuf[city].shield_box == 0 || (townbuf[city].production < 0 && scratch_i == -1))) {
            scratch_i = find_slot(selected_owner, city);
            *(int *) ((char *) scrbuf + 0x10) = 2;
            if (scratch_i != 99) {
                show_city_display = 0;
                if (townbuf[city].production >= 0)
                    civ_units_ordered[selected_owner][townbuf[city].production]--;
                townbuf[city].production = scratch_i;
                if (townbuf[city].production >= 0)
                    civ_units_ordered[selected_owner][townbuf[city].production]++;
            } else {
                townbuf[city].status &= 0xef;
            }
        }
        /* AI rush spending buys shields at two treasury coins per shield. */
label_ai_buy:
        if (!(human_set & (1 << selected_owner))) {
            if (townbuf[city].status & 0x10)
                city_retarget(city);
            yield_buy_k = 0;
            area_role_g = civ_area_plan[selected_owner][region_key(city_x, city_y_p)];
            if ((area_role_g == 1 || area_role_g == 2 || area_role_g == 5) && scratch_i != 0 && townbuf[city].production >= 0 && utypes[townbuf[city].production].role == area_role_g)
                yield_buy_k = money[selected_owner] / 64;
            if ((game_flags & (1 << (civnow + 8))) && -townbuf[city].production >= 22 && -townbuf[city].production <= 24)
                yield_buy_k = money[selected_owner] / 128;
            if ((townbuf[city].status & 1) && townbuf[city].production < 0 && townbuf[city].shield_box != 0)
                yield_buy_k = clamp(improvements[-townbuf[city].production].cost * cost_scale - townbuf[city].shield_box, 0, money[selected_owner] / 8);
            if ((map_owner(city_x, city_y_p) == -1 || (townbuf[city].status & 0x10)) && townbuf[city].production >= 0 && townbuf[city].shield_box != 0)
                yield_buy_k = clamp(utypes[townbuf[city].production].cost * cost_scale - townbuf[city].shield_box, 0, money[selected_owner] / 3);
            if (townbuf[city].production == -1 && townbuf[city].shield_box != 0)
                yield_buy_k = clamp(utypes[townbuf[city].production].cost * cost_scale - townbuf[city].shield_box, 0, money[selected_owner] / 3);
            if (townbuf[city].status == 25 && forces_present[selected_owner][25] == 0 && townbuf[city].shield_box != 0)
                yield_buy_k = clamp(utypes[townbuf[city].production].cost * cost_scale - townbuf[city].shield_box, 0, money[selected_owner] / 4);
            if (money[selected_owner] > 2000)
                yield_buy_k += money[selected_owner] / 512;
            townbuf[city].shield_box += yield_buy_k;
            money[selected_owner] -= yield_buy_k * 2;
            townbuf[city].status &= 0xef;
        }

    }
label_draw:
    if (mode == 1L) {
        framed_panel(2, 67, 124, 104);
        framed_panel(95, 106, 227, 197);
        draw_button(95, 106, 128, 114, "INFO", 9);
        draw_button(129, 106, 160, 114, "HAPPY", 9);
        draw_button(161, 106, 193, 114, "MAP", 9);
        draw_button(194, 106, 226, 114, "VIEW", 9);
        recolour_rect(scrbuf, city_view_mode * 33 + 96, 107, 32, 7, 9, 15);
        switch (city_view_mode) {
        case 2:
            draw_minimap();
            break;
        }
    }
    city_x = townbuf[city].x;
    city_y_p = townbuf[city].y;
    city_leak = 0;
    if (civ_gov[selected_owner] == 3)
        palace_remote = 10;
    city_leak = town_res_sums[2] * palace_remote * 3 / (civ_gov[selected_owner] * 20 + 80);
    if ((townbuf[city].buildings & 0x40) || (townbuf[city].buildings & 1))
        city_leak /= 2;
    if (civ_gov[selected_owner] == 5)
        city_leak = 0;
    townbuf[city].trade = town_res_sums[2] - city_leak;
    /* Foreign trade routes give twice the domestic route bonus. */
    for (item_b = 0; item_b < 3; item_b++) {
        city_index = (unsigned char) townbuf[city].routes[item_b];
        if (city_index != 255) {
            if (townbuf[city_index].owner != selected_owner)
                town_res_sums[2] += (townbuf[city_index].trade + town_res_sums[2] + 4) / 8;
            else
                town_res_sums[2] += (townbuf[city_index].trade + town_res_sums[2] + 4) / 16;
        }
    }
    city_leak = town_res_sums[2] * palace_remote * 3 / (civ_gov[selected_owner] * 20 + 80);
    if (civ_gov[selected_owner] == 5)
        city_leak = 0;
    if (townbuf[city].buildings & 0x40)
        city_leak /= 2;
    /* Split net trade before adding specialists and building multipliers. */
    city_revel_sums = clamp(((town_res_sums[2] - city_leak) * (10 - (sci[selected_owner] + civ_toll[selected_owner])) + 5) / 10, 0, town_res_sums[2]);

    taxes = clamp(((town_res_sums[2] - city_leak) * civ_toll[selected_owner] + 5) / 10, 0, town_res_sums[2] - city_revel_sums - city_leak);
    learning_yield = town_res_sums[2] - city_revel_sums - taxes - city_leak;
    taxes += count_specialists(1) * 2;
    learning_yield += count_specialists(2) * 2;
    city_revel_sums += count_specialists(3) * 2;
    if (townbuf[city].buildings & 0x10) {
        city_revel_sums += city_revel_sums / 2;
        taxes += taxes / 2;
    }
    if (townbuf[city].buildings & 0x200) {
        city_revel_sums += city_revel_sums / 2;
        taxes += taxes / 2;
    }
    scratch_i = learning_yield;
    if (townbuf[city].buildings & 0x20) {
        scratch_i += learning_yield / 2;
        if (civ_wonder(selected_owner, 12))
            scratch_i += learning_yield / 3;
    }
    if (townbuf[city].buildings & 0x800) {
        scratch_i += learning_yield / 2;
        if (civ_wonder(selected_owner, 12))
            scratch_i += learning_yield / 3;
    }
    if (active_wonder_city(10) == city)
        scratch_i += scratch_i;
    learning_yield = scratch_i;
    town_happy_ones_num = 0;
    if (selected_owner == civnow) {
        scratch_i = 14 - cur_tier * 2;
        scratch_i = (civ_gov[selected_owner] / 2 + 2) * (scratch_i / 2);
        unhappy_l = town_unhappy_counts = townbuf[city].size + (city % scratch_i + city_count[selected_owner] - scratch_i) / scratch_i + cur_tier - 6;
    } else {
        unhappy_l = town_unhappy_counts = townbuf[city].size - 3;
    }
    excess_unhappy = 0;
    if (townbuf[city].size < town_unhappy_counts) {
        excess_unhappy = town_unhappy_counts - townbuf[city].size;
        town_unhappy_counts = townbuf[city].size;
    }
    fit_citizens(city, experts_f);
    if (mode == 1L && city_view_mode == 1) {
        happy_y_g = 116;
        draw_city_view(city, 100, happy_y_g, experts_f, 92);
        happy_y_g += 16;
        draw_line(100, happy_y_g - 2, 222, happy_y_g - 2, 1);
    }
    town_happy_ones_num = city_revel_sums / 2;
    fit_citizens(city, experts_f);
    if (mode == 1L && city_view_mode == 1 && town_happy_ones_num != 0) {
        draw_city_view(city, 100, happy_y_g, experts_f, 92);
        label_with_style(14, 208, happy_y_g + 4);
        happy_y_g += 16;
        draw_line(100, happy_y_g - 2, 222, happy_y_g - 2, 1);
    }
    icon_x = 208;
    if (townbuf[city].buildings & 0x2000) {
        town_unhappy_counts -= 3;
        if (mode == 1L && city_view_mode == 1) {
            draw_icon(14, icon_x, happy_y_g);
            icon_x -= 16;
        }
    }
    if (has_tech(selected_owner, 29)) {
        town_unhappy_counts -= (townbuf[city].buildings & 0x400) ? (civ_wonder(selected_owner, 9) ? 6 : 4) : 0;
        if (mode == 1L && city_view_mode == 1 && (townbuf[city].buildings & 0x400)) {
            draw_icon(11, icon_x, happy_y_g);
            icon_x -= 16;
        }
    }
    if (townbuf[city].buildings & 8) {
        if (has_tech(selected_owner, 26))
            town_unhappy_counts -= 2;
        else if (has_tech(selected_owner, 25))
            town_unhappy_counts--;
        if (civ_wonder(selected_owner, 6))
            town_unhappy_counts -= has_tech(selected_owner, 26) ? 2 : 1;
        if (mode == 1L && city_view_mode == 1) {
            draw_icon(4, icon_x, happy_y_g);
            icon_x -= 16;
        }
    }
    fit_citizens(city, experts_f);
    if (mode == 1L && city_view_mode == 1 && (townbuf[city].buildings & 0x2408)) {
        draw_city_view(city, 100, happy_y_g, experts_f, 92);
        happy_y_g += 16;
        draw_line(100, happy_y_g - 2, 222, happy_y_g - 2, 1);
    }
    /* Up to three troops suppress unrest; republic/democracy penalize troops abroad. */
    icon_x = 208;
    if (civ_gov[selected_owner] < 4) {
        police_e = stack_len_m = 0;
        unit = scratch_i = map_stack(city_x, city_y_p);
        while (unit != -1 && ++stack_len_m < 32) {
            if (utypes[armydata[selected_owner][unit].type].attack != 0) {
                police_e++;
                if (mode == 1L && city_view_mode == 1 && police_e <= 3) {
                    label_with_style(armydata[selected_owner][unit].type + selected_owner * 32 + 64, icon_x, happy_y_g - 1);
                    icon_x -= 2;
                }
            }
            unit = armydata[selected_owner][unit].next;
            if (unit == scratch_i)
                unit = -1;
        }
        for (unit = 0; unit < 2; unit++) {
            if (town_unhappy_counts != 0 && townbuf[city].garrison[unit] != -1 && utypes[townbuf[city].garrison[unit] & 0x3f].attack != 0)
                police_e++;
        }
        if (police_e > 3)
            police_e = 3;
        town_unhappy_counts -= clamp(police_e, 0, town_unhappy_counts);
    } else {
        scratch_i = civ_wonder(selected_owner, 16) == 0;
        if (civ_gov[selected_owner] == 5)
            scratch_i++;
        if (scratch_i != 0) {
            town_unhappy_counts += scratch_i * units_abroad;
            if (mode == 1L && city_view_mode == 1) {
                for (unit = 0; unit < scratch_i * units_abroad; unit++) {
                    label_with_style(13, icon_x, happy_y_g + 4);
                    icon_x -= 2;
                }
            }
        }
    }
    fit_citizens(city, experts_f);
    if (mode == 1L && city_view_mode == 1) {
        draw_city_view(city, 100, happy_y_g, experts_f, 92);
        happy_y_g += 16;
        draw_line(100, happy_y_g - 2, 222, happy_y_g - 2, 1);
    }
    scratch_i = town_happy_ones_num - town_unhappy_counts;
    if (civ_wonder(selected_owner, 2))
        town_happy_ones_num++;
    if (civ_wonder(selected_owner, 21))
        town_happy_ones_num++;
    if (active_wonder_city(11) == city)
        town_unhappy_counts = 0;
    if (civ_wonder(selected_owner, 13)) {
        if (region_key(city_x, city_y_p) == region_key(townbuf[marvel_site[13]].x, townbuf[marvel_site[13]].y))
            town_unhappy_counts -= 2;
    }
    fit_citizens(city, experts_f);
    if (mode == 1L && city_view_mode == 1 && scratch_i != town_happy_ones_num - town_unhappy_counts) {
        draw_city_view(city, 100, happy_y_g, experts_f, 92);
        draw_string("WONDERS", 190, happy_y_g + 5, 15);
        happy_y_g += 16;
        draw_line(100, happy_y_g - 2, 222, happy_y_g - 2, 1);
    }
    if (!(rule_options & 2))
        town_happy_ones_num = town_unhappy_counts = 0;
    /* Recount support after each forced disband; remove the most distant unit. */
label_support_restart:
    city_keep = unused_support = home_troops_maintained = unit_count_i = 0;
    for (index_i = 0; index_i < 2; index_i++) {
        if (townbuf[city].garrison[index_i] != -1)
            home_troops_maintained++;
    }
    if (townbuf[city].size < home_troops_maintained)
        city_keep = home_troops_maintained - townbuf[city].size;
    draw_x_l = 8;
    draw_y_k = 69;
    unit_x_j = 100;
    unit_y_i = 116;
    for (index_i = 0; index_i < 128; index_i++) {
        if (armydata[selected_owner][index_i].type != -1 && armydata[selected_owner][index_i].home_city == city) {
            if (armydata[selected_owner][index_i].type < 26) {
                if (townbuf[city].size < ++home_troops_maintained || (civ_gov[selected_owner] > 1 && (rule_options & 2)))
                    city_keep++;
            }
            if (mode == 0L) {
                if (town_res_sums[1] < city_keep || ((townbuf[city].status & 1) && ((index_i + turn_tick) & 7) == 0 && selected_owner != civnow && civ_gov[selected_owner] >= 4)) {
                    ai_result_i = best_idx_o = -1;
                    for (choice_f = 0; choice_f < 128; choice_f++) {
                        if (armydata[selected_owner][choice_f].type != -1 && armydata[selected_owner][choice_f].home_city == city && armydata[selected_owner][choice_f].type < 26) {
                            distance_f = map_dist(townbuf[city].x, armydata[selected_owner][choice_f].x, townbuf[city].y, armydata[selected_owner][choice_f].y);
                            if (distance_f > ai_result_i) {
                                ai_result_i = distance_f;
                                best_idx_o = choice_f;
                            }
                        }
                    }
                    if (town_res_sums[1] >= city_keep) {
                        if (gangmode != 0 && tech_count[selected_owner] < tech_count[civnow]) {
                            margin_democracy = -999;
                        } else if (ai_result_i > 0 && utypes[armydata[selected_owner][best_idx_o].type].domain == 0 && armydata[selected_owner][best_idx_o].type != 0) {
                            drop_u_stack(selected_owner, best_idx_o);
                            townbuf[city].status &= 0xfe;
                        }
                    } else {
                        if (selected_owner == civnow) {
                            msgtext[0] = 0;
                            append_place_name(city);
                            strcat(msgtext, " can't support\n");
                            strcat(msgtext, utypes[armydata[selected_owner][best_idx_o].type].name);
                            strcat(msgtext, ".\n Unit Disbanded.\n");
                            advisor = 3;
                            show_msg(msgtext, 100, 80);
                        }
                        drop_u_stack(selected_owner, best_idx_o);
                        goto label_support_restart;
                    }
                }
            }
            if (mode == 1L) {
                if (armydata[selected_owner][index_i].type < 26) {
                    if (city_keep != 0)
                        label_with_style(9, draw_x_l + 8, draw_y_k + 12);
                    if (armydata[selected_owner][index_i].type == 0) {
                        label_with_style(8, draw_x_l, draw_y_k + 12);
                        if (civ_gov[selected_owner] >= 2)
                            label_with_style(8, draw_x_l + 2, draw_y_k + 12);
                    } else {
                        scratch_i = civ_wonder(selected_owner, 16) == 0;
                        if (civ_gov[selected_owner] == 5)
                            scratch_i++;
                        if (civ_gov[selected_owner] >= 4 && utypes[armydata[selected_owner][index_i].type].attack != 0 && scratch_i != 0 && (utypes[armydata[selected_owner][index_i].type].domain == 1 || armydata[selected_owner][index_i].x != townbuf[city].x || armydata[selected_owner][index_i].y != townbuf[city].y)) {
                            label_with_style(13, draw_x_l, draw_y_k + 12);
                            if (scratch_i > 1)
                                label_with_style(13, draw_x_l + 2, draw_y_k + 12);
                        }
                    }
                }
                label_with_style(armydata[selected_owner][index_i].type + selected_owner * 32 + 64, draw_x_l, draw_y_k);
                minimap_dot(armydata[selected_owner][index_i].x, armydata[selected_owner][index_i].y, 7);
                draw_x_l += 16;
                if (draw_x_l >= 112) {
                    draw_x_l = 8;
                    draw_y_k += 16;
                    if (draw_y_k > 85)
                        draw_y_k -= 24;
                }
            }
        }
        if (unit_count_i < 18 && city_view_mode == 0 && armydata[selected_owner][index_i].type != -1 && mode == 1L && armydata[selected_owner][index_i].x == city_x && armydata[selected_owner][index_i].y == city_y_p) {
            draw_unit(selected_owner, index_i, unit_x_j, unit_y_i);
            msgtext[0] = 0;
            append_place_name(armydata[selected_owner][index_i].home_city);
            msgtext[3] = '.';
            msgtext[4] = 0;
            draw_string(msgtext, unit_x_j, unit_y_i + 15, 0);
            unit_list_f[unit_count_i++] = index_i;
            unit_x_j += 18;
            if (unit_count_i % 6 == 0) {
                unit_x_j = 100;
                unit_y_i += 16;
            }
        }
    }
    if (mode == 0L) {
        if (town_res_sums[1] < home_troops_maintained)
            troop_over += (home_troops_maintained - town_res_sums[1]) * 5;
        troop_over += clamp(home_troops_maintained, 0, townbuf[city].size);
        margin_democracy -= (((townbuf[city].buildings & 0x10) ? 5 : 7) - civt[civ_tribe_of[selected_owner]].f2) * units_abroad;
    }
    if (mode == 1L) {
        *(int *) ((char *) scrbuf + 0x10) = 2;
        framed_panel(211, 1, 317, 97);
        city_y_p = choice_f = 2;
        bld_count_g = wonders_o = 0;
        /* Wonders precede saleable buildings and do not scroll onto later pages. */
        if (first_bld_h == 0) {
            for (item_b = 0; item_b < 21; item_b++) {
                if ((marvel_site + 1)[item_b] == city) {
                    scratch_i = item_b + 24;
                    strcpy(msgtext, improvements[scratch_i + 1].name);
                    strupr(msgtext);
                    truncate_text(msgtext, 63);
                    draw_string(msgtext, 253, city_y_p + 2, 15);
                    draw_icon(scratch_i + 1, (choice_f & 1) ? 213 : 233, city_y_p - 2);
                    city_y_p += 6;
                    choice_f++;
                    wonders_o++;
                }
            }
        }
        next_bld = 0;
        for (item_b = first_bld_h; item_b < 24; item_b++) {
            if (townbuf[city].buildings & (1L << item_b)) {
                if (choice_f >= 16) {
                    bld_pages_p |= 2;
                    bld_pages_p &= ~1;
                    next_bld = item_b;
                    break;
                }
                bld_count_g++;
                label_with_style(11, 309, city_y_p + 1);
                strcpy(msgtext, improvements[item_b + 1].name);
                truncate_text(msgtext, 56);
                draw_string(strupr(msgtext), 253, city_y_p + 2, 15);
                draw_icon(item_b + 1, (choice_f & 1) ? 213 : 233, city_y_p - 2);
                city_y_p += 6;
                choice_f++;
            }
        }
        draw_line(231, 0, 250, 0, 0);
        draw_line(231, 1, 250, 1, 1);
        if (bld_pages_p & 2)
            draw_button(287, 88, 315, 96, "MORE", 9);
        recolour_rect(scrbuf, 309, 2, 8, 96, 14, 12);
        if (townbuf[city].production >= 0)
            cost_h = utypes[townbuf[city].production].cost;
        else
            cost_h = improvements[-townbuf[city].production].cost;
        scale_i = clamp((cost_h - 1) / 10 + 1, (townbuf[city].shield_box - 1) / 100 + 1, 99);
        scratch_i = 80 / cost_scale;
        draw_x_l = cost_scale * scratch_i + (scale_i * 8 - 8) / scale_i;
        draw_y_k = ((cost_h - 1) / scale_i) * 8 + 8;
        rect_fill(scrbuf, 230, 99, draw_x_l + 3, draw_y_k + 19, 1);
        if (selected_owner == civnow) {
            draw_button(231, 106, 263, 114, (townbuf[city].status & 0x10) ? "AUTO." : "CHANGE", 9);
            draw_button(294, 106, 311, 114, "BUY", 9);
        }
        if (townbuf[city].production >= 0) {
            label_with_style(townbuf[city].production + selected_owner * 32 + 64, 264, 100);
        } else {
            strcpy(msgtext, improvements[-townbuf[city].production].name);
            truncate_text(msgtext, 86);
            draw_text(msgtext, 274, 100, 15);
        }
        tile_panel(231, 116, draw_x_l + 1, draw_y_k + 1);
        for (index_i = 0; index_i < townbuf[city].shield_box; index_i++) {
            label_with_style(9, (index_i % (cost_scale * scale_i)) * scratch_i / scale_i + 232, (index_i / (cost_scale * scale_i)) * 8 + 117);
        }
        scratch_i = clamp(80 / townbuf[city].size, 1, 8);
        draw_y_k = 80 / cost_scale;
        rect_fill(scrbuf, 2, 106, 91, cost_scale * draw_y_k + 12, 1);
        draw_string("Food Storage", 8, 108, 15);
        tile_panel(3, 115, townbuf[city].size * scratch_i + 9, cost_scale * draw_y_k + 2);
        if (townbuf[city].buildings & 4)
            draw_line(5, 155, townbuf[city].size * scratch_i + 9, 155, 1);
        for (index_i = 0; index_i < clamp(townbuf[city].food_box, 0, (townbuf[city].size + 1) * cost_scale); index_i++) {
            label_with_style(8, (index_i % (townbuf[city].size + 1)) * scratch_i + 4, (index_i / (townbuf[city].size + 1)) * draw_y_k + 116);
        }
        framed_panel(2, 23, 124, 65);
        draw_y_k = 25;
        rect_fill(scrbuf, 2, 23, 122, 9, 1);
        draw_string("City Resources", 8, draw_y_k, 15);
        draw_y_k += 8;
        for (item_b = 0; item_b < 3; item_b++) {
            city_x = 4;
            if (item_b == 0)
                scratch_i = clamp(116 / (clamp(town_res_sums[item_b], townbuf[city].size * 2 + colonist_homed * sett_food_j, 999) + 1), 1, 8);
            else
                scratch_i = clamp(116 / (town_res_sums[item_b] + 1), 1, 8);
            for (index_i = 0; index_i < town_res_sums[item_b]; index_i++) {
                if (item_b == 0 && townbuf[city].size * 2 + colonist_homed * sett_food_j == index_i)
                    city_x += 4;
                if (item_b == 1 && city_keep != 0 && index_i == city_keep)
                    city_x += 4;
                if (item_b == 2 && town_res_sums[2] - city_leak == index_i)
                    city_x += 2;
                label_with_style(item_b + 8, city_x, draw_y_k + item_b * 8);
                city_x += scratch_i;
            }
        }
        city_x = 8;
        scratch_i = clamp(224 / (city_revel_sums + learning_yield + taxes + city_leak + 2), 1, 16);
        for (index_i = 0; index_i < city_revel_sums; index_i++) {
            label_with_style(14, city_x / 2, draw_y_k + 24);
            city_x += scratch_i;
        }
        if (city_revel_sums != 0)
            city_x += 8;
        draw_x_l = city_x;
        for (index_i = 0; index_i < taxes; index_i++) {
            label_with_style(11, city_x / 2, draw_y_k + 24);
            city_x += scratch_i;
        }
        if (taxes != 0)
            city_x += 8;
        for (index_i = 0; index_i < learning_yield; index_i++) {
            label_with_style(12, city_x / 2, draw_y_k + 24);
            city_x += scratch_i;
        }
        if (townbuf[city].size * 2 + colonist_homed * sett_food_j > town_res_sums[0]) {
            scratch_i = clamp(116 / (townbuf[city].size * 2 + colonist_homed * sett_food_j + 1), 1, 8);
            for (index_i = town_res_sums[0]; index_i < townbuf[city].size * 2 + colonist_homed * sett_food_j; index_i++)
                label_with_style(8, index_i * scratch_i + 8, draw_y_k);
            recolour_rect(scrbuf, scratch_i * town_res_sums[0] + 8, draw_y_k, (townbuf[city].size * 2 + colonist_homed * sett_food_j - town_res_sums[0]) * scratch_i + 4, 8, 15, 0);
        }
        if (town_res_sums[1] < city_keep) {
            scratch_i = clamp(116 / (town_res_sums[1] + 1), 1, 8);
            for (index_i = town_res_sums[1]; index_i < city_keep; index_i++)
                label_with_style(9, index_i * scratch_i + 8, draw_y_k + 8);
            recolour_rect(scrbuf, scratch_i * town_res_sums[1] + 8, draw_y_k + 8, (city_keep - town_res_sums[1]) * scratch_i, 8, 15, 0);
        }
        if (city_leak != 0) {
            scratch_i = clamp(116 / (town_res_sums[2] + 1), 1, 8);
            recolour_rect(scrbuf, (town_res_sums[2] - city_leak) * scratch_i + 6, draw_y_k + 16, scratch_i * city_leak + 2, 8, 15, 0);
        }
        tile_panel(8, 8, 200, 13);
        step_cost_m = draw_city_view(city, 8, 8, experts_f, 192);
        scratch_i = clamp(townbuf[city].size * 8 + 24, 0, 128);
        for (item_b = 0; item_b < 3; item_b++) {
            city_index = (unsigned char) townbuf[city].routes[item_b];
            if (city_index != 255) {
                msgtext[0] = 0;
                append_place_name(city_index);
                strcat(msgtext, ":+");
                if (townbuf[city_index].owner != selected_owner)
                    strcat(msgtext, itoa((townbuf[city].trade + townbuf[city_index].trade + 4) / 8, nstr, 10));
                else
                    strcat(msgtext, itoa((townbuf[city].trade + townbuf[city_index].trade + 4) / 16, nstr, 10));
                strcat(msgtext, "} ");
                if (city_view_mode == 2)
                    minimap_dot(townbuf[city_index].x, townbuf[city_index].y, 10);
                if (city_view_mode == 0)
                    draw_string(msgtext, 98, item_b * 6 + 179, 10);
            }
        }
        if (city_view_mode == 2)
            minimap_dot(townbuf[selected_city].x, townbuf[selected_city].y, 15);
        if (city_view_mode == 0) {
            pollution_k = town_res_sums[1] / plant_waste_divisor - 20;
            pollution_k += townbuf[city].size * pop_smog_advances / 4;
            draw_y_k = clamp(100 / clamp(pollution_k, 1, 99), 1, 8);
            for (index_i = 0; index_i < pollution_k; index_i++)
                sprite_draw(scrbuf, index_i * draw_y_k + 98, (index_i & 1) + 161, village_icons);
        }
        if (selected_owner == civnow) {
            draw_button(284, 190, 316, 198, "EXIT", 12);
            draw_button(231, 190, 272, 198, "RENAME", 9);
            flash_c = 0;
            if (townbuf[city].production < 0 && townbuf[city].production >= -21 && (townbuf[city].buildings & (1L << (-townbuf[city].production - 1))))
                flash_c = 1;
            if (townbuf[city].production < -21 && townbuf[city].production >= -24 && (game_flags & (1 << selected_owner)))
                flash_c = 1;
            if (townbuf[city].production < -24 && (marvel_site - 24)[-townbuf[city].production] != -1)
                flash_c = 1;
            if (townbuf[city].production >= 0 && troops_total[selected_owner] >= 127)
                flash_c = 1;
            mouse_on();
            while (mse_held != 0)
                poll_mouse();
label_input:
            while (mse_held == 0 && kbhit() == 0 && tile_keys_h == 0) {
                poll_mouse();
                if (flash_c != 0) {
                    mouse_off();
                    if (++flash_c & 1)
                        recolour_rect(scrbuf, 231, 106, 32, 8, 14, 9);
                    else
                        recolour_rect(scrbuf, 231, 106, 32, 8, 9, 14);
                    mouse_on();
                    flip(10);
                }
            }
            key_h = -1;
            if (kbhit() != 0) {
                key_h = key_cmd();
                ptr_x = ptr_y = 0;
            } else if (tile_keys_h != 0) {
                key_h = 'p';
            }
            mouse_off();
            if ((mse_held == 2 && ptr_x >= 230 && ptr_x < 270 && ptr_y >= 106 && ptr_y <= 116) || key_h == 'A') {
                townbuf[city].status ^= 0x10;
                if (townbuf[city].status & 0x10) {
                    auto_build_i = find_slot(selected_owner, city);
                    *(int *) ((char *) scrbuf + 0x10) = 2;
                    if (auto_build_i != 99) {
                        show_city_display = 0;
                        if (townbuf[city].production >= 0)
                            civ_units_ordered[selected_owner][townbuf[city].production]--;
                        townbuf[city].production = auto_build_i;
                        if (townbuf[city].production >= 0)
                            civ_units_ordered[selected_owner][townbuf[city].production]++;
                        rect_fill(scrbuf, 230, 99, 90, 100, 0);
                    } else {
                        goto label_render_restart;
                    }
                }
                town_res_sums[1] = raw_shld_m;
                town_res_sums[2] = raw_trade_h;
                goto label_citizens_refresh;
            }
            if (key_h != -1 || (mse_held == 1 && ptr_y >= 106 && ptr_y <= 116)) {
                if (ptr_x >= 296 || key_h == 'b') {
                    /* Unit rush prices are quadratic; wonders and empty boxes cost extra. */
                    if (townbuf[city].production >= 0) {
                        choice_f = utypes[townbuf[city].production].cost;
                        scratch_i = clamp(10 * choice_f - townbuf[city].shield_box, 0, 999);
                        scratch_i = scratch_i * scratch_i / 20 + scratch_i * 2;
                    } else {
                        choice_f = improvements[-townbuf[city].production].cost;
                        scratch_i = clamp(10 * choice_f - townbuf[city].shield_box, 0, 999) * 2;
                        if (-townbuf[city].production > 21)
                            scratch_i *= 2;
                    }
                    if (townbuf[city].shield_box == 0)
                        scratch_i *= 2;
                    strcpy(msgtext, "Cost to complete\n");
                    if (townbuf[city].production >= 0)
                        strcat(msgtext, utypes[townbuf[city].production].name);
                    else
                        strcat(msgtext, improvements[-townbuf[city].production].name);
                    strcat(msgtext, ": $");
                    strcat(msgtext, itoa(scratch_i, nstr, 10));
                    strcat(msgtext, "\nTreasury: $");
                    strcat(msgtext, itoa(money[selected_owner], nstr, 10));
                    strcat(msgtext, "\n");
                    if (townbuf[city].production >= 0 && (townbuf[city].status & 1))
                        strcat(msgtext, "CIVIL DISORDER\n");
                    else if (money[selected_owner] >= scratch_i)
                        strcat(msgtext, " Yes\n No\n");
                    mouse_on();
                    *(int *) ((char *) scrbuf + 0x10) = 1;
                    if (show_menu(msgtext, 100, 80) == 0 && money[selected_owner] >= scratch_i) {
                        townbuf[city].shield_box = 10 * choice_f;
                        money[selected_owner] -= scratch_i;
                    }
                    mouse_off();
                    goto label_render_restart;
                }
                if ((ptr_x >= 230 && ptr_x < 270) || key_h == 'c') {
                    townbuf[city].status &= 0xef;
                    mouse_on();
                    if (townbuf[city].production >= 0)
                        civ_units_ordered[selected_owner][townbuf[city].production]--;
                    townbuf[city].production = find_slot(selected_owner, city);
                    if (townbuf[city].production >= 0)
                        civ_units_ordered[selected_owner][townbuf[city].production]++;
                    mouse_off();
                    goto label_render_restart;
                }
                if (key_h == 'i') {
                    city_view_mode = 0;
                    goto label_tab_selected;
                }
                if (key_h == 'h') {
                    city_view_mode = 1;
                    goto label_tab_selected;
                }
                if (key_h == 'm') {
                    city_view_mode = 2;
                    goto label_tab_selected;
                }
                if (key_h == 'v') {
                    city_view_mode = 3;
                    goto label_tab_selected;
                }
                if (ptr_x >= 96 && ptr_x < 224) {
                    city_view_mode = (ptr_x - 96) / 32;
label_tab_selected:
                    town_res_sums[1] = raw_shld_m;
                    town_res_sums[2] = raw_trade_h;
                    if (city_view_mode == 3) {
                        *(int *) ((char *) scrbuf + 0x10) = 1;
                        msgtext[0] = 0;
                        city_scene(city, -1);
                        map_view_changed = 1;
                        city_view_mode = 0;
                        goto label_render_restart;
                    }
                    goto label_citizens_refresh;
                }
            }
label_after_tabs:;
            if (key_h == 'a') {
                choice_f = 0;
                while (!(armydata[selected_owner][unit_list_f[choice_f]].status & 0xcf) && choice_f < unit_count_i)
                    choice_f++;
                if (choice_f == unit_count_i)
                    choice_f = 0;
                do {
                    unit_x_j = (choice_f % 6) * 18 + 100;
                    unit_y_i = (choice_f / 6) * 16 + 116;
                    do {
                        tile_panel(unit_x_j, unit_y_i, 16, 16);
                        flip(10);
                        draw_unit(selected_owner, unit_list_f[choice_f], unit_x_j, unit_y_i);
                        flip(10);
                    } while (kbhit() == 0);
                    draw_y_k = key_cmd();
                    switch (draw_y_k) {
                    case 0x4d00:
                        index_i = choice_f + 1;
                        break;
                    case 0x4b00:
                        index_i = choice_f - 1;
                        break;
                    case 0x5000:
                        index_i = choice_f + 6;
                        break;
                    case 0x4800:
                        index_i = choice_f - 6;
                        break;
                    default:
                        index_i = choice_f;
                    }
                    choice_f = clamp(index_i, 0, unit_count_i - 1);
                    if (draw_y_k == 13 || draw_y_k == 32) {
                        if (armydata[selected_owner][unit_list_f[choice_f]].status & 9)
                            armydata[selected_owner][unit_list_f[choice_f]].moves = utypes[armydata[selected_owner][unit_list_f[choice_f]].type].moves * 3;
                        armydata[selected_owner][unit_list_f[choice_f]].status &= 0x30;
                        armydata[selected_owner][unit_list_f[choice_f]].goto_x = -1;
                    }
                } while (draw_y_k != 27);
                town_res_sums[1] = raw_shld_m;
                town_res_sums[2] = raw_trade_h;
                goto label_citizens_refresh;
            }
            if (key_h == 's' && !(townbuf[city].status & 0x80)) {
                choice_f = 0;
                do {
                    do {
                        recolour_rect(scrbuf, 252, (choice_f + wonders_o) * 6 + 3, 56, 7, 9, 1);
                        flip(10);
                        recolour_rect(scrbuf, 252, (choice_f + wonders_o) * 6 + 3, 56, 7, 1, 9);
                        flip(10);
                    } while (kbhit() == 0);
                    draw_y_k = key_cmd();
                    switch (draw_y_k) {
                    case 0x5000:
                        index_i = choice_f + 1;
                        break;
                    case 0x4800:
                        index_i = choice_f - 1;
                        break;
                    }
                    choice_f = clamp(index_i, 0, bld_count_g - 1);
                    if (draw_y_k == 13 || draw_y_k == 32)
                        goto label_sell;
                } while (draw_y_k != 27);
            }
            if (key_h >= '1' && key_h <= '9') {
                index_i = key_h - '1';
                if (index_i < experts_f) {
                    if (townbuf[city].size >= 5) {
                        scratch_i = get_specialist(index_i);
                        if (scratch_i < 3)
                            set_specialist(index_i, scratch_i + 1);
                        else
                            set_specialist(index_i, 1);
                    } else {
                        specialist_warning();
                        goto label_render_restart;
                    }
                    town_res_sums[1] = raw_shld_m;
                    town_res_sums[2] = raw_trade_h;
                    goto label_citizens_refresh;
                }
                goto label_render_restart;
            }
            if (key_h == 'p') {
                if (tile_keys_h == 0) {
                    cursor_x_l = cursor_y_k = 0;
                    tile_keys_h = 1;
                }
                do {
                    do {
                        blit(scrbuf, cursor_x_l * 16 + 160, cursor_y_k * 16 + 56, 16, 16, back_page, 0, 0);
                        draw_box(cursor_x_l * 16 + 160, cursor_y_k * 16 + 56, 15, 15, 15);
                        flip(10);
                        blit(back_page, 0, 0, 16, 16, scrbuf, cursor_x_l * 16 + 160, cursor_y_k * 16 + 56);
                        flip(10);
                        map_view_changed = 1;
                    } while (kbhit() == 0);
                    draw_x_l = cursor_x_l;
                    draw_y_k = cursor_y_k;
                    scratch_i = key_cmd();
                    switch (scratch_i) {
                    case 0x4800:
                        cursor_y_k--;
                        break;
                    case 0x4900:
                        cursor_y_k--;
                        cursor_x_l++;
                        break;
                    case 0x4d00:
                        cursor_x_l++;
                        break;
                    case 0x5100:
                        cursor_x_l++;
                        cursor_y_k++;
                        break;
                    case 0x5000:
                        cursor_y_k++;
                        break;
                    case 0x4f00:
                        cursor_x_l--;
                        cursor_y_k++;
                        break;
                    case 0x4b00:
                        cursor_x_l--;
                        break;
                    case 0x4700:
                        cursor_x_l--;
                        cursor_y_k--;
                        break;
                    }
                    if (abs(cursor_x_l) > 2 || abs(cursor_y_k) > 2 || abs(cursor_x_l) + abs(cursor_y_k) >= 4 || !(map_visible_to[(townbuf[city].x + cursor_x_l) * 50 + townbuf[city].y + cursor_y_k] & (1 << civnow))) {
                        cursor_x_l = draw_x_l;
                        cursor_y_k = draw_y_k;
                    }
                    if (scratch_i == 13 || scratch_i == 32) {
                        draw_x_l = cursor_x_l;
                        draw_y_k = cursor_y_k;
                        goto label_tile_toggle;
                    }
                } while (scratch_i != 27);
                tile_keys_h = 0;
                goto label_render_restart;
            }
            if (key_h == 'M' && (bld_pages_p & 2)) {
                bld_pages_p ^= 1;
                first_bld_h = next_bld;
                town_res_sums[1] = raw_shld_m;
                town_res_sums[2] = raw_trade_h;
                goto label_citizens_refresh;
            }
            if (key_h == 'r' || (mse_held == 1 && ptr_x >= 231 && ptr_y > 190 && ptr_x < 270)) {
                repaint();
                *(int *) ((char *) scrbuf + 0x10) = 1;
                rename_city(city);
                map_view_changed = 1;
                goto label_render_restart;
            }
            if (mse_held == 1) {
                if (ptr_x >= 288 && ptr_y < 96 && ptr_y > 88 && (bld_pages_p & 2)) {
                    bld_pages_p ^= 1;
                    first_bld_h = next_bld;
                    town_res_sums[1] = raw_shld_m;
                    town_res_sums[2] = raw_trade_h;
                    goto label_citizens_refresh;
                }
                if (ptr_x >= 300 && ptr_y < 94) {
                    choice_f = (ptr_y - 2) / 6 - wonders_o;
label_sell:
                    for (item_b = first_bld_h; item_b < 24; item_b++) {
                        if ((townbuf[city].buildings & (1L << item_b)) && choice_f-- == 0 && !(townbuf[city].status & 0x80)) {
                            strcpy(msgtext, "Do you want to sell\nyour ");
                            strcat(msgtext, improvements[item_b + 1].name);
                            strcat(msgtext, " for ");
                            strcat(msgtext, itoa(improvements[item_b + 1].cost * 10, nstr, 10));
                            strcat(msgtext, "$?\n No.\n Yes.\n");
                            *(int *) ((char *) scrbuf + 0x10) = 1;
                            mouse_on();
                            if (menu_dialog(msgtext, 128, 80) == 1) {
                                money[selected_owner] += improvements[item_b + 1].cost * 10;
                                townbuf[city].buildings -= 1L << item_b;
                                townbuf[city].status |= 0x80;
                                if (item_b + 1 == 8)
                                    map_view_changed = 1;
                            }
                            mouse_off();
                            goto label_render_restart;
                        }
                    }
                }
                if (ptr_x < 200 && ptr_y < 20) {
                    if (townbuf[city].size >= 5) {
                        index_i = (ptr_x - 16) / step_cost_m;
                        index_i -= townbuf[city].size - workers_d;
                        if (index_i < 0 || index_i >= experts_f)
                            goto label_render_restart;
                        scratch_i = get_specialist(index_i);
                        if (scratch_i < 3)
                            set_specialist(index_i, scratch_i + 1);
                        else
                            set_specialist(index_i, 1);
                    } else {
                        specialist_warning();
                        goto label_render_restart;
                    }
                    town_res_sums[1] = raw_shld_m;
                    town_res_sums[2] = raw_trade_h;
                    goto label_citizens_refresh;
                }
                /* The original uses 9600 here, extending the unit panel hit area. */
                if (ptr_x >= 100 && ptr_x < 9600 && ptr_y >= 116) {
                    choice_f = (ptr_y - 116) / 16 * 6 + (ptr_x - 100) / 16;
                    if (choice_f < unit_count_i) {
                        if (armydata[selected_owner][unit_list_f[choice_f]].status & 9)
                            armydata[selected_owner][unit_list_f[choice_f]].moves = utypes[armydata[selected_owner][unit_list_f[choice_f]].type].moves * 3;
                        armydata[selected_owner][unit_list_f[choice_f]].status &= 0x30;
                        armydata[selected_owner][unit_list_f[choice_f]].goto_x = -1;
                        town_res_sums[1] = raw_shld_m;
                        town_res_sums[2] = raw_trade_h;
                        goto label_citizens_refresh;
                    }
                }
                if (ptr_y >= 24 && ptr_y < 104) {
                    draw_x_l = ptr_x / 16 - 10;
                    draw_y_k = (ptr_y - 24) / 16 - 2;
                    /* Preserve the worker bitfield while moving a citizen to or from a tile. */
label_tile_toggle:
                    index_i = radius_index(draw_x_l, draw_y_k);
                    if (index_i != -1 && index_i < 20) {
                        if (map_visible_to[(townbuf[city].x + radius_dx[index_i]) * 50 + townbuf[city].y + radius_dy[index_i]] & (1 << selected_owner)) {
                            town_res_sums[1] = raw_shld_m;
                            town_res_sums[2] = raw_trade_h;
                            if (!(townbuf[city].workers & (1L << index_i)) && blocked_m[radius_dx[index_i] + 2][radius_dy[index_i] + 2] != 0)
                                goto label_citizens_refresh;
                            townbuf[city].workers ^= 1L << index_i;
                            if (townbuf[city].workers & (1L << index_i)) {
                                worker_adj_i++;
                                add_tile_yield(city, index_i, mode);
                                if (city_experts_num != 0) {
                                    city_experts_num--;
                                    workers_d--;
                                }
                            } else {
                                worker_adj_i--;
                                for (item_b = 0; item_b < 3; item_b++) {
                                    yield_buy_k = tile_yield(townbuf[city].x + radius_dx[index_i], townbuf[city].y + radius_dy[index_i], item_b);
                                    town_res_sums[item_b] -= yield_buy_k;
                                }
                                seen_map(townbuf[city].x + radius_dx[index_i], townbuf[city].y + radius_dy[index_i]);
                                city_experts_num++;
                                workers_d++;
                            }
                            townbuf[city].workers = ((long) workers_d << 26) + (townbuf[city].workers & 0x3ffffffL);
                            if (worker_adj_i > 0)
                                goto label_render_restart;
                            else
                                goto label_citizens_refresh;
                        }
                    }
                    goto label_render_restart;
                }
            }
        } else {
            ad_wait();
        }
        map_x0 = old_map_x_n;
        map_y0 = old_map_y_m;
    }
    if (mode == 0L) {
        if (selected_owner != 0) {
            scratch_i = town_happy_ones_num - town_unhappy_counts;
            if (scratch_i < 0) {
                if (human_set & (1 << selected_owner)) {
                    strcpy(msgtext, "Civil Disorder in\n");
                    append_place_name(city);
                    strcat(msgtext, "! Mayor\nflees in panic.\n");
                    sound(36);
                    if ((debug & 8) && !(townbuf[city].status & 1) && human_counts == 0) {
                        city_scene(city, -2);
                        riot_anim();
                        disk_insert(0, 1L);
                        sel_player();
                        human_counts = 1;
                    } else {
                        advisor = 4;
                        show_msg(msgtext, 100, 80);
                    }
                    sound(1);
                    if (!(townbuf[city].status & 1)) {
                        show_city_display = 1;
                        if (debug & 1)
                            notice_a("*DISORDER");
                    }
                }
                if (!(townbuf[city].status & 1)) {
                    townbuf[city].status |= 1;
                    if (!(human_set & (1 << selected_owner))) {
                        if (townbuf[city].production >= 0)
                            civ_units_ordered[selected_owner][townbuf[city].production]--;
                        townbuf[city].production = find_slot(selected_owner, city);
                        if (townbuf[city].production >= 0)
                            civ_units_ordered[selected_owner][townbuf[city].production]++;
                    }
                    note_map(city_x, city_y_p);
                } else if (civ_gov[selected_owner] == 5) {
                    sound(36);
                    strcpy(msgtext, "Discontented citizens of\n");
                    append_place_name(city);
                    strcat(msgtext, " revolt:\nGovernment Collapses!\n");
                    if (selected_owner == civnow) {
                        news_box(city);
                        treaties[selected_owner][0] |= 4;
                    }
                    set_government(selected_owner, 0);
                    sound(1);
                }
            } else {
                if (scratch_i >= 0 && (townbuf[city].status & 1)) {
                    townbuf[city].status &= 0xfe;
                    if (human_set & (1 << selected_owner)) {
                        advisor = 4;
                        strcpy(msgtext, "Order restored\nin ");
                        append_place_name(city);
                        strcat(msgtext, ".\n");
                        show_msg(msgtext, 100, 80);
                    } else {
                        if (townbuf[city].production >= 0)
                            civ_units_ordered[selected_owner][townbuf[city].production]--;
                        townbuf[city].production = find_slot(selected_owner, city);
                        if (townbuf[city].production >= 0)
                            civ_units_ordered[selected_owner][townbuf[city].production]++;
                    }
                    note_map(city_x, city_y_p);
                }
                if (civ_gov[selected_owner] != 0)
                    money[selected_owner] += taxes;
            }
            /* Celebration requires no unhappy citizens and at least half happy. */
            if (town_unhappy_counts == 0 && townbuf[city].size > 2 && (townbuf[city].size + 1) / 2 <= town_happy_ones_num && civ_gov[selected_owner] != 0) {
                if (!(townbuf[city].status & 0x40)) {
                    if ((human_set & (1 << selected_owner)) && civ_gov[selected_owner] != 0) {
                        strcpy(msgtext, "'We love the ");
                        strcat(msgtext, leader_titles[civ_gov[selected_owner]]);
                        strcat(msgtext, "'\nday celebrated in\n");
                        append_place_name(city);
                        strcat(msgtext, "!\n");
                        sound(34);
                        if (debug & 8) {
                            city_scene(city, -2);
                            love_anim();
                            disk_insert(0, 1L);
                            sel_player();
                        } else {
                            news_box(city);
                        }
                        sound(1);
                    }
                    townbuf[city].status |= 0x40;
                } else if (civ_gov[selected_owner] >= 4 && townbuf[city].size * 2 + colonist_homed * sett_food_j < town_res_sums[0] && (townbuf[city].size < 10 || (townbuf[city].buildings & 0x100))) {
                    townbuf[city].size++;
                }
            } else if (scratch_i >= 0 && (townbuf[city].status & 0x40)) {
                if ((human_set & (1 << selected_owner)) && civ_gov[selected_owner] != 0) {
                    sound(35);
                    strcpy(msgtext, "'We love the ");
                    strcat(msgtext, leader_titles[civ_gov[selected_owner]]);
                    strcat(msgtext, "'\ncelebration canceled\nin ");
                    append_place_name(city);
                    strcat(msgtext, ".\n");
                    news_box(city);
                    sound(1);
                }
                townbuf[city].status &= 0xbf;
            }
        }
        scratch_i = learning_yield;
        if (selected_owner == civnow) {
            if ((selected_index != -1 && has_tech(selected_owner, selected_index)) || (selected_index == -1 && civ_research_tally[selected_owner] != 0)) {
                selected_index = -1;
                if (rule_options & 8) {
                    mouse_force_on();
                    reduce_attitude(selected_owner, 0);
                    mouse_restore();
                }
                civ_research_tally[selected_owner] = 0;
            }
        }
        if (civ_gov[selected_owner] == 0)
            scratch_i = 0;
        civ_research_tally[selected_owner] += scratch_i;
        if (selected_owner == civnow && cur_tier == 0 && (advance_owners[selected_index] & 7))
            civ_research_tally[selected_owner] += scratch_i;
        if (civ_wonder(selected_owner, 20))
            civ_research_tally[selected_owner] += scratch_i / 2;
        step_cost_m = (selected_owner != civnow) ? 14 - cur_tier : cur_tier * 2 + 6;
        step_cost_m += cost_penalty;
        if (selected_owner == civnow && 11 - tech_count[selected_owner] > step_cost_m)
            step_cost_m = 11 - tech_count[selected_owner];
        if (selected_owner != civnow || show_city_display == 0) {
            if (civ_research_tally[selected_owner] / ((year < 0) ? 1 : 2) > tech_count[selected_owner] * step_cost_m) {
                civ_research_tally[selected_owner] = 0;
                if (rule_options & 8) {
                    mouse_force_on();
                    reduce_attitude(selected_owner, 0);
                    mouse_restore();
                }
            }
        }
        civ_ratings[selected_owner] += townbuf[city].size + town_happy_ones_num - town_unhappy_counts;
        if (selected_owner == civnow) {
            pollution_k = town_res_sums[1] / plant_waste_divisor - 20;
            pollution_k += townbuf[city].size * pop_smog_advances / 4;
            /* A pollution roll chooses one outer-radius tile; unsuitable tiles are skipped. */
            if (pollution_k * 2 > rand_scaled(256 - tech_count[selected_owner] * cur_tier / 2)) {
                scratch_i = rand_scaled(20);
                tile_x_l = radius_dx[scratch_i] + city_x;
                radius_y = radius_dy[scratch_i] + city_y_p;
                if (!(map_flags(tile_x_l, radius_y) & 0x40) && terrain_at(tile_x_l, radius_y) != 10 && !(map_flags(tile_x_l, radius_y) & 1)) {
                    add_pollution(tile_x_l, radius_y);
                    reveal(civnow, tile_x_l - 8, radius_y - 6);
                    map_note(tile_x_l, radius_y);
                    advisor = 6;
                    strcpy(msgtext, "Pollution near ");
                    append_place_name(city);
                    strcat(msgtext, ".\n");
                    show_msg(msgtext, 80, 64);
                }
            }
            /* Disorder can cause a nuclear meltdown until Fusion Power is known. */
            if ((townbuf[city].status & 1) && (townbuf[city].buildings & 0x100000L) && rand_scaled(3) == 0 && has_tech(selected_owner, 66) == 0) {
                reveal(civnow, city_x - 8, city_y_p - 6);
                nuke_animation(city_x, city_y_p);
                nuke_fallout(city_x, city_y_p);
                townbuf[city].buildings &= ~0x100000L;
                sound(36);
                strcpy(msgtext, "Nuclear Catastrophe\nin ");
                append_place_name(city);
                strcat(msgtext, "!\nContamination feared!\n");
                news_box(city);
                sound(1);
            }
            for (item_b = 1; item_b < 24; item_b++) {
                if ((townbuf[city].buildings & (1L << item_b)) && civ_gov[selected_owner] != 0) {
                    money[selected_owner] -= improvements[item_b + 1].maintenance;
                    if (item_b == 1) {
                        if (cur_tier >= 2)
                            money[selected_owner]--;
                        if (cur_tier >= 4)
                            money[selected_owner]--;
                        if (has_tech(selected_owner, 34))
                            money[selected_owner]--;
                        if (has_tech(selected_owner, 37))
                            money[selected_owner]--;
                    }
                    if (money[selected_owner] < 0) {
                        money[selected_owner] = 0;
                        townbuf[city].buildings &= ~(1L << item_b);
                        msgtext[0] = 0;
                        append_place_name(city);
                        strcat(msgtext, "\ncan't maintain\n");
                        strcat(msgtext, improvements[item_b + 1].name);
                        strcat(msgtext, ".\n");
                        advisor = 4;
                        show_msg(msgtext, 100, 80);
                        money[selected_owner] += improvements[item_b + 1].cost * 10;
                    }
                }
            }
        }
    }
    if ((townbuf[city].status & 4) && adj_grp(townbuf[city].x, townbuf[city].y) == 0)
        garrison_in(city);
    *(int *) ((char *) scrbuf + 0x10) = 1;
    mouse_on();
    return town_happy_ones_num - town_unhappy_counts;
label_exit:;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

extern struct city townbuf[];
extern int improvement_words[], marvel_site[];
extern int selected_city, excess_unhappy, town_happy_ones_num, town_unhappy_counts, polluted;
extern int map_flags(int, int);
extern int map_mark(int, int);
#define CITY_STATE (townbuf[selected_city].specialists)

struct terrains {
    char raw[14];
    signed char value[3];
    char tail[2];
};
struct work_rule {
    int value[6];
};
extern struct terrains terrains[];
#define special_resources (terrains + 12)
extern struct work_rule work_rules[];
extern int rule_options, selected_owner, civ_gov[], working_tiles, town_res_sums[];
extern int troop_over, margin_democracy;
extern int terrain_at(int, int);

extern int radius_dx[], radius_dy[];
extern int seen_map(int, int);
extern void label_with_style(int, int, int);

/* yield_sum_f is only initialized in drawing mode. The original still reads it
   first in the final && test when mode differs; keep that observed ordering.
   The 1L comparisons also preserve the original widened int comparisons. */
void add_tile_yield(int city, int index, int mode)
{
    int yield_sum_f, icon_step_m, yield_left_p, tile_y_k, icon_dy_l, tile_x_l, icon_dx_m, resource_i;
    tile_x_l = townbuf[city].x + radius_dx[index];
    tile_y_k = townbuf[city].y + radius_dy[index];
    if (mode == 1L) {
        seen_map(tile_x_l, tile_y_k);
        yield_sum_f = 0;
        resource_i = 0;
        do {
            yield_sum_f += tile_yield(tile_x_l, tile_y_k, resource_i);
            resource_i++;
        } while (resource_i < 3);
        icon_step_m = yield_sum_f <= 4 ? 8 : yield_sum_f <= 6 ? 5 : 3;
    } else {
        working_tiles = 1;
        map_claim(tile_x_l + 80, tile_y_k, townbuf[city].owner, 2);
    }
    resource_i = icon_dx_m = icon_dy_l = 0;
    for (; resource_i < 3; resource_i++) {
        yield_left_p = tile_yield(tile_x_l, tile_y_k, resource_i);
        town_res_sums[resource_i] += yield_left_p;
        while (yield_left_p > 0 && mode == 1L) {
            label_with_style(resource_i + 8, 161 + (radius_dx[index] << 4) + icon_dx_m, 57 + (radius_dy[index] << 4) + icon_dy_l);
            if (icon_dx_m >= 8) {
                icon_dx_m = 0;
                icon_dy_l += 8;
            } else
                icon_dx_m += icon_step_m;
            yield_left_p--;
        }
    }
    if (yield_sum_f == 0 && mode == 1L)
        label_with_style(13, 165 + (radius_dx[index] << 4), 61 + (radius_dy[index] << 4));
    working_tiles = 0;
}

/* kind: 0 food, 1 shields, 2 trade; improvements precede government effects. */
int tile_yield(int x, int y, int kind)
{
    int terrain_l, tile_flags_g, yield_value_n;
    if (!map_in_bounds(x, y))
        return 0;
    terrain_l = terrain_at(x, y);
    if (patterned_cell(terrain_l, x, y))
        yield_value_n = special_resources[terrain_l].value[kind];
    else
        yield_value_n = terrains[terrain_l].value[kind];
    tile_flags_g = map_flags(x, y);
    if (!(rule_options & 2)) {
        tile_flags_g = terrain_l <= 2 ? 2 : 4;
        if (terrain_l != 1)
            tile_flags_g |= 8;
    }
    if (terrain_l != 10) {
        if (kind == 0 && (tile_flags_g & 2))
            yield_value_n += -1 - work_rules[terrain_l].value[0];
        if (kind == 1 && (tile_flags_g & 4))
            yield_value_n += -1 - work_rules[terrain_l].value[2];
        if (kind == 2 && (tile_flags_g & 8) && terrain_l <= 2)
            yield_value_n++;
    }
    if (kind == 1 && (terrain_l == 2 || terrain_l == 11) && ((x * 7 + y * 11) & 2))
        yield_value_n = 0;
    if (yield_value_n && kind == 2 && active_wonder_city(3) == selected_city)
        yield_value_n++;
    if (tile_flags_g & 16)
        yield_value_n += yield_value_n / 2;
    if (yield_value_n > 2 && !(townbuf[selected_city].status & 0x40)) {
        if (civ_gov[selected_owner] <= 1)
            yield_value_n--;
        if (working_tiles)
            troop_over -= 2;
    }
    if (yield_value_n && kind == 2) {
        if (working_tiles)
            margin_democracy++;
        if ((townbuf[selected_city].status & 0x40 ? 2 : 4) <= civ_gov[selected_owner])
            yield_value_n++;
    }
    if (tile_flags_g & 0x40)
        yield_value_n = (yield_value_n + 1) / 2;
    return yield_value_n >= 0 ? yield_value_n : 0;
}

/* A wonder expires when any non-barbarian civilization knows its obsoleting tech. */
int civ_wonder(int civ, int wonder)
{
    int city, i;
    for (i = 1; i < 8; i++)
        if (has_tech(i, improvement_words[wonder]))
            return 0;
    city = marvel_site[wonder];
    if (city == -1)
        return 0;
    if (townbuf[city].owner == civ)
        return 1;
    return 0;
}

int active_wonder_city(int wonder)
{
    int i;
    for (i = 1; i < 8; i++)
        if (has_tech(i, improvement_words[wonder]))
            return -1;
    return marvel_site[wonder];
}

void add_pollution(int x, int y)
{
    if (!(map_flags(x, y) & 0x40)) {
        flag_tile(0x40, x, y);
        map_mark(x, y);
        polluted++;
    }
}

/* Two bits per specialist: 0 unused, 1 taxman, 2 scientist, 3 entertainer. */
void set_specialist(int index, int value)
{
    if (index < 8) {
        index += index;
        CITY_STATE &= ~(3 << index);
        CITY_STATE |= value << index;
    }
}

int get_specialist(int index)
{
    if (index >= 8)
        return 1;
    index += index;
    return (CITY_STATE >> index) & 3;
}

int count_specialists(int value)
{
    int i, count;
    count = 0;
    i = 0;
    do {
        if (get_specialist(i) == value)
            count++;
        i++;
    } while (i < 8);
    return count;
}

void fit_citizens(int city, int delta)
{
    town_happy_ones_num = clamp(town_happy_ones_num, 0, townbuf[city].size);
    while (excess_unhappy != 0) {
        if (town_unhappy_counts < excess_unhappy) {
            excess_unhappy--;
            town_unhappy_counts++;
        } else
            break;
    }
    town_unhappy_counts = clamp(town_unhappy_counts, 0, townbuf[city].size);
    while (town_happy_ones_num + town_unhappy_counts > clamp(townbuf[city].size - delta, 0, 99)) {
        if (excess_unhappy != 0)
            excess_unhappy--;
        else
            town_happy_ones_num = clamp(--town_happy_ones_num, 0, townbuf[city].size);
        town_unhappy_counts = clamp(--town_unhappy_counts, 0, townbuf[city].size);
    }
}

/* Original reads unknown_gap_j before assigning it; preserve that observed quirk.
   unused_word_b reserves the other unused word in the ten-byte frame. */

extern int city_glad_glyphs[];
/* city_glad_glyphs is the nine citizen icons: 0-1 happy, 2-3 content, 4-5
 * unhappy, 6-8 the specialists, indexed by kind 1-3 from
 * specialist_people. */
#define quiet_persons (city_glad_glyphs + 2)
#define city_angry_images (city_glad_glyphs + 4)
#define specialist_people (city_glad_glyphs + 5)

extern void recolour_rect(struct page *, int, int, int, int, int, int);
int draw_city_view(int city, int x, int y, int special, int width)
{
    signed char population_f;
    int citizen_step_p, citizen_k, unused_word_b, unknown_gap_j;
    population_f = townbuf[city].size;
    if (7 * population_f > width)
        citizen_step_p = width / population_f;
    else
        citizen_step_p = 7;
    for (citizen_k = 0; citizen_k < town_happy_ones_num; citizen_k++) {
        sprite_draw(scrbuf, x, y, city_glad_glyphs[citizen_k & 1]);
        x += citizen_step_p;
    }
    if (unknown_gap_j)
        x += 2;
    for (citizen_k = 0; citizen_k < townbuf[city].size - special - town_happy_ones_num - town_unhappy_counts; citizen_k++) {
        sprite_draw(scrbuf, x, y, quiet_persons[citizen_k & 1]);
        x += citizen_step_p;
    }
    if (citizen_k)
        x += 2;
    for (citizen_k = 0; citizen_k < town_unhappy_counts; citizen_k++) {
        sprite_draw(scrbuf, x, y, city_angry_images[citizen_k & 1]);
        if (citizen_k < excess_unhappy)
            recolour_rect(scrbuf, x, y, 12, 14, 5, 12);
        x += citizen_step_p;
    }
    x += 4;
    for (citizen_k = 0; citizen_k < special; citizen_k++) {
        sprite_draw(scrbuf, x, y, specialist_people[get_specialist(citizen_k)]);
        x += citizen_step_p;
    }
    return citizen_step_p;
}

/* City-screen panels and improvement icons. */

extern int vga_modes;

void draw_icon(int icon, int x, int y)
{
    if (icon >= 22 && icon <= 24)
        return;
    icon--;
    if (icon >= 24)
        icon -= 3;
    if (icon < 40)
        blit(tile_page, 161 + (icon / 5) * 19, 50 + (icon % 5) * 10, 18, 10, scrbuf, x, y);
    else
        blit(tile_page, 161 + (icon & 1) * 19, 100, 18, 10, scrbuf, x, y);
}

void framed_panel(int x1, int y1, int x2, int y2)
{
    int width, height;
    width = x2 - x1;
    height = y2 - y1;
    tile_panel(x1, y1, width, height);
    draw_box(x1, y1, width, height, 1);
}

void tile_panel(int x, int y, int width, int height)
{
    int saved_width_o, dest_y_i, dest_x_j, tile_height_k, tile_width_d;
    if (!vga_modes)
        rect_fill(scrbuf, x, y, width, height, 9);
    else {
        saved_width_o = width;
        dest_y_i = y;
        do {
            tile_height_k = clamp(height, 0, 16);
            dest_x_j = x;
            width = saved_width_o;
            do {
                tile_width_d = clamp(width, 0, 16);
                blit(tile_page, 208, 100, tile_width_d, tile_height_k, scrbuf, dest_x_j, dest_y_i);
                dest_x_j += tile_width_d;
                width -= tile_width_d;
            } while (width > 0);
            dest_y_i += tile_height_k;
            height -= tile_height_k;
        } while (height > 0);
    }
}

void draw_button(int x1, int y1, int x2, int y2, char *label, int color)
{
    int width, height;
    width = x2 - x1;
    height = y2 - y1;
    rect_fill(scrbuf, x1, y1, width, height, color);
    draw_line(x1, y1, x2, y1, color < 8 ? color + 8 : 7);
    draw_line(x1, y1, x1, y2, color < 8 ? color + 8 : 7);
    draw_line(x1 + 1, y2, x2, y2, color < 8 ? 8 : color - 8);
    draw_line(x2, y1, x2, y2, color < 8 ? 8 : color - 8);
    draw_text(label, (x1 + x2) / 2 + 1, (y1 + y2) / 2 - 2, color ^ 8);
}

/* City-screen minimap, centred horizontally on the human capital. */

extern int civnow, x_cap[], city_view_mode;
extern char msgtext[];

extern int terrain_at(int, int), wrap_x(int);
extern char *strcpy(char *, char *);
signed char mini_x[4] = { 0, 2, 0, 1 }, mini_y[4] = { -1, 0, 1, 2 };

void draw_minimap(void)
{
    int pixel_part_i, screen_y_i, terrain_l, row_i, screen_x_j, map_y_k, column_c, map_x_l;
    rect_fill(scrbuf, 101, 117, 120, 75, 0);
    for (row_i = 0; row_i < 50; row_i++)
        for (column_c = 0; column_c < 80; column_c++) {
            map_x_l = wrap_x(x_cap[civnow] + column_c - 40);
            map_y_k = row_i;
            if (!(map_visible_to[map_x_l * 50 + map_y_k] & (1 << civnow)))
                continue;
            pixel_part_i = ((row_i & 1) << 1) + (column_c & 1);
            screen_x_j = column_c / 2 * 3 + mini_x[pixel_part_i] + 101;
            screen_y_i = row_i / 2 * 3 + mini_y[pixel_part_i] + 118;
            terrain_l = terrain_at(map_x_l, map_y_k);
            rect_fill(scrbuf, screen_x_j, screen_y_i, 2, 2, terrain_l == 10 ? 1 : 2);
        }
    draw_box(100, 117, 121, 75, 9);
}

void minimap_dot(int x, int y, int colour)
{
    if (city_view_mode == 2) {
        x = wrap_x(x - x_cap[civnow] + 40);
        x = 3 * x / 2 + 101;
        y = 3 * y / 2 + 118;
        rect_fill(scrbuf, x, y, 2, 2, colour);
    }
}

void specialist_warning(void)
{
    strcpy(msgtext, "A City must have five\npopulation units to support\ntaxmen or scientists.\n");
    menu_dialog(msgtext, 32, 32);
}
