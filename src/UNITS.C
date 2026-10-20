/* Unit stacks, creation, movement, exploration and animation. */
#include "CIV.H"
#include "AUDIENCE.H"
#include "DIALOG.H"
#include "DIPLOMAC.H"
#include "DRIVER.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "UNITAI.H"
#include "CITY.H"
#include "CONTROL.H"
#include "CORE.H"
#include "ECONOMY.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "NOTES.H"
#include "UNITS.H"
#include "CAPTURE.H"
#include "DRAW.H"
#include "TURN.H"

extern struct unit_type utypes[];

int quiz_busy = 0;
/* City garrison transfer. Letter suffixes preserve MSC stack-slot ordering. */

extern struct city townbuf[];
extern struct unit armydata[][128];

void unpack_garrison(int city)
{                               /* turn stored city units back into units */
    int new_unit_i, slot;
    struct unit *unit_ptr_o;
    if (townbuf[city].owner != civnow)
        return;
    quiz_busy = 1;
    for (slot = 0; slot < 2; slot++) {
        if (townbuf[city].garrison[slot] != -1) {
            new_unit_i = create_unit(townbuf[city].owner, townbuf[city].garrison[slot] & 0x3f, townbuf[city].x, townbuf[city].y);
            if (new_unit_i != -1) {
                unit_ptr_o = &armydata[townbuf[city].owner][new_unit_i];
                unit_ptr_o->status |= 8;
                if (townbuf[city].garrison[slot] & 0x40)
                    unit_ptr_o->status |= 0x20;
                townbuf[city].garrison[slot] = -1;
            }
        }
    }
    quiz_busy = 0;
    townbuf[city].status |= 4;
}

void garrison_in(int city)
{                               /* store fortified units into the city record */
    int owner_g, kept_unit_p, unit_index_m, saved_fight_view_g;
    owner_g = townbuf[city].owner;
    if (owner_g != civnow)
        return;
    saved_fight_view_g = see_fight_all;
    see_fight_all = kept_unit_p = 0;
    quiz_busy = 1;
    /* Keep the first fortified unit on the map; pack up to two more into the city. */
    for (unit_index_m = 0; unit_index_m < 128; unit_index_m++) {
        if (armydata[owner_g][unit_index_m].type != -1 && armydata[owner_g][unit_index_m].x == townbuf[city].x && armydata[owner_g][unit_index_m].y == townbuf[city].y && armydata[owner_g][unit_index_m].home_city == city && (armydata[owner_g][unit_index_m].status & 8)) {
            if (kept_unit_p) {
                if (townbuf[city].garrison[0] == -1) {
                    townbuf[city].garrison[0] = armydata[owner_g][unit_index_m].type;
                    if (armydata[owner_g][unit_index_m].status & 0x20)
                        townbuf[city].garrison[0] |= 0x40;
                } else if (townbuf[city].garrison[1] == -1) {
                    townbuf[city].garrison[1] = armydata[owner_g][unit_index_m].type;
                    if (armydata[owner_g][unit_index_m].status & 0x20)
                        townbuf[city].garrison[1] |= 0x40;
                } else
                    break;
                drop_u_stack(owner_g, unit_index_m);
            }
            kept_unit_p = 1;
        }
    }
    quiz_busy = 0;
    see_fight_all = saved_fight_view_g;
    townbuf[city].status &= ~4;
}

/* Exploration and contact around a unit. */

extern struct unit armydata[][128];
extern struct terrain_type terrains[];
extern char civ_vis_grid[80][50];
extern int terrain_at(int, int), wrap_x(int);
extern void map_mark(int, int);
extern struct city townbuf[];
extern struct {
    char a, b;
} treaties[8][8];
extern int map_flags(int, int), city_at(int, int);
extern int map_stack(int, int);

extern void map_note();         /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

extern void each_stack(int, int, void (far *) (int, int));

/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void unit_sight(x, y, owner, index, redraw)     /* reveal and make contact around a unit */
int x, y, owner, index, redraw;
{
    int a, b, c, d, e, f, g, h, i, j, k;
    f = terrain_at(x, y);
    if (owner == civnow) {
        e = 0;
        do {
            h = wrap_x(x + spiral_x[e]);
            i = y + spiral_y[e];
            if (e >= 9 && (!(utypes[armydata[owner][index].type].sight & 2) || (utypes[armydata[owner][index].type].domain == 2 && terrain_at(h, i) != 10)))
                goto next_radius;
            if (map_in_bounds(h, i)) {
                civ_vis_grid[h][i] |= (char) (1 << owner);
                map_mark(h, i);
            }
next_radius:
            e++;
        } while (e < 25);
    }
    g = map_flags(x, y);
    for (e = 1; e < 9; e++) {
        h = wrap_x(x + spiral_x[e]);
        i = y + spiral_y[e];
        if (!map_in_bounds(h, i))
            continue;
        if (owner != 0)
            civ_vis_grid[h][i] |= (char) (1 << owner);
        k = map_flags(h, i);
        if ((k & 1) && map_city(h, i) != owner) {
            j = map_city(h, i);
            d = city_at(h, i);
            unpack_garrison(d);
            if (owner == civnow)
                townbuf[d].visible_size = townbuf[d].size;
            if (utypes[armydata[owner][index].type].domain == 0) {
                armydata[owner][index].visibility |= 1 << j;
                if (!(treaties[owner][j].a & 2)) {
                    add_target(j, h, i, 1, 4);
                    add_target(j, h, i, 2, 2);
                }
            }
        }
        a = map_stack(h, i);
        if (a != -1 && owner != fight_enemy) {
            if (!(g & 1))
                armydata[owner][index].visibility |= 1 << fight_enemy;
            each_stack(fight_enemy, a, validate_record);
            if (terrain_at(h, i) == 10 || f != 10)
                armydata[fight_enemy][a].goto_x = -1;
            if (!(terrain_at(h, i) == 10 && f != 10 && utypes[armydata[owner][index].type].domain != 1))
                armydata[owner][index].goto_x = -1;
            if (!(k & 1))
                armydata[fight_enemy][a].visibility |= 1 << owner;
            if (utypes[armydata[owner][index].type].domain == 0 && terrain_at(x, y) != 10 && terrain_at(h, i) != 10 && utypes[armydata[fight_enemy][a].type].domain != 1) {
                diplomacy_event(owner, fight_enemy, x, y);
                if (!(treaties[owner][fight_enemy].a & 2)) {
                    c = 1;
                    if (fight_enemy != 0 && armydata[fight_enemy][a].type != 26 && (!(armydata[fight_enemy][a].status & 8) || utypes[armydata[fight_enemy][a].type].role != 2))
                        c = 3;
                    add_target(owner, h, i, 1, c);
                }
                if (armydata[owner][index].type != 26) {
                    if (owner != 0)
                        add_target(fight_enemy, x, y, 2, 2);
                    else
                        add_target(fight_enemy, x, y, 2, 1);
                }
            }
            if (terrain_at(h, i) == 10 && fight_enemy != 0 && !(treaties[owner][fight_enemy].a & 2))
                add_target(owner, h, i, 3, 2);
        }
        if (redraw && (owner == civnow || xray_on)) {
            map_note(h, i);
            map_claim(h + 240, i, help_probe(2, h, i) == 1 ? 1 : 2, 2);
        }
    }
    for (e = 9; e < 25; e++) {
        h = wrap_x(x + spiral_x[e]);
        i = y + spiral_y[e];
        if (!map_in_bounds(h, i))
            continue;
        a = map_stack(h, i);
        k = map_flags(h, i);
        if ((utypes[armydata[owner][index].type].sight & 2) && (utypes[armydata[owner][index].type].domain != 2 || terrain_at(h, i) == 10)) {
            if (owner != 0)
                civ_vis_grid[h][i] |= (char) (1 << owner);
            if (a != -1 && owner != fight_enemy && armydata[fight_enemy][a].type != 22) {
                if (!(k & 1))
                    armydata[fight_enemy][a].visibility |= 1 << owner;
                if (terrain_at(h, i) == 10 && fight_enemy != 0 && !(treaties[owner][fight_enemy].a & 2))
                    add_target(owner, h, i, 3, 2);
            }
            if (redraw && (owner == civnow || xray_on)) {
                map_note(h, i);
                map_claim(h + 240, i, help_probe(2, h, i) == 1 ? 1 : 2, 2);
            }
        }
        if (a != -1 && owner != fight_enemy && armydata[owner][index].type != 22) {
            b = armydata[fight_enemy][a].type;
            if ((utypes[b].sight & 2) && (utypes[b].domain != 2 || terrain_at(x, y) == 10)) {
                armydata[fight_enemy][a].status &= 0xfe;
                armydata[fight_enemy][a].goto_x = -1;
                if (!(g & 1))
                    armydata[owner][index].visibility |= 1 << fight_enemy;
                if (terrain_at(h, i) == 10 && fight_enemy != 0 && !(treaties[owner][fight_enemy].a & 2))
                    add_target(fight_enemy, x, y, 3, 2);
            } else if (fight_enemy == civnow && (map_flags(h, i) & 1) && (civ_vis_grid[x][y] & (1 << civnow))) {
                armydata[owner][index].visibility |= 1 << fight_enemy;
                map_note(x, y);
            }
        }
        if ((k & 1) && map_city(h, i) == civnow && (civ_vis_grid[x][y] & (1 << civnow))) {
            armydata[owner][index].visibility |= 1 << civnow;
            map_note(x, y);
        }
    }
    if (owner == civnow && redraw) {
        map_note(x, y);
        for (e = 9; e < 49; e++) {
            h = wrap_x(x + spiral_x[e]);
            i = y + spiral_y[e];
            if (!map_in_bounds(h, i))
                continue;
            if (civ_vis_grid[h][i] & (1 << owner))
                map_note(h, i);
        }
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Unit/end-turn blink, 0866:[0AD6,0CF5); isolated diagnostic. */

extern struct unit armydata[][128];
extern int wrap_x(int), kbhit(void), terrain_at(int, int);

extern int seen_map(int, int);

extern void flip(int);
extern void unit_redraw(int, int);

void blink_unit(int owner, int index, int x, int y)
{
    int a, b, c, d, e, f, g;
    c = -1;
    if (x != -1) {
        c = (wrap_x(x - map_x0) << 4) + 80;
        d = ((y - map_y0) << 4) + 8;
        if (c < 80 || c >= 320 || d < 8 || d > 192)
            c = -1;
    }
    g = view_yorg < 0 ? 0 : view_yorg;
    e = g - view_yorg;
    a = wrap_x(armydata[owner][index].x - view_xorg);
    b = armydata[owner][index].y - g + e;
    do {
        mouse_off();
        if (index < 128) {
            seen_map(armydata[owner][index].x, armydata[owner][index].y);
            if (b >= 0 && b < 50)
                plot_pixel(a, b + 8, 15);
        } else
            draw_string("End of Turn", 4, 124, 7);
        if (c != -1)
            draw_box(c, d, 15, 15, 15);
        mouse_on();
        flip(kbhit()? 1 : 10);
        mouse_off();
        if (index < 128) {
            unit_redraw(owner, index);
            if (b >= 0 && b < 50)
                plot_pixel(a, b + 8, terrain_at(armydata[owner][index].x, armydata[owner][index].y) == 10 ? 1 : 2);
        } else
            draw_string("End of Turn", 4, 124, 0);
        if (c != -1)
            draw_box(c, d, 15, 15, 0);
        mouse_on();
        flip(kbhit()? 1 : 10);
        poll_mouse();
    } while (!kbhit() && !mse_held);
}

/* Genuine unit-creation body, 0866:[0CF5,0F10).
 * Isolated diagnostics only: its 01DC same-TU call needs the full unit. */

extern struct unit armydata[][128];
extern struct city townbuf[];
extern char civ_vis_grid[80][50];

extern void map_note(int, int, int);
extern void unit_sight();

int create_unit(int owner, int type, int x, int y)
{
    int index;
    char bit;
    for (index = 0; index < 127; index++) {
        if (armydata[owner][index].type == -1) {
            armydata[owner][index].x = -1;
            armydata[owner][index].next = -1;
            map_refresh(owner, x, y);
            put_unit(owner, index, x, y);
            bit = 1 << owner;
            civ_vis_grid[x][y] |= bit;
            armydata[owner][index].status = 0;
            armydata[owner][index].x = x;
            armydata[owner][index].y = y;
            armydata[owner][index].type = type;
            armydata[owner][index].visibility = bit;
            armydata[owner][index].goto_x = -1;
            armydata[owner][index].f8 = -1;
            armydata[owner][index].fuel = utypes[armydata[owner][index].type].fuel;
            armydata[owner][index].home_city = near_city(x, y);
            if (townbuf[armydata[owner][index].home_city].owner != owner)
                armydata[owner][index].home_city = -1;
            if (armydata[owner][index].fuel != 0)
                armydata[owner][index].fuel--;
            forces_present[owner][type]++;
            if (!quiz_busy) {
                if (xray_on || owner == civnow || (armydata[owner][index].visibility & (1 << civnow)))
                    map_note(armydata[owner][index].x, armydata[owner][index].y, owner);
                unit_sight(x, y, owner, index, 1L);
            }
            if (owner == civnow && turn_tick != 0 && quiz_busy == 0) {
                if (troops_total[owner] == 0)
                    notice_a("*FIRSTUNIT1");
                if (troops_total[owner] == 1)
                    notice_a("*FIRSTUNIT2");
            }
            return index;
        }
    }
    if (owner == civnow && num_ai == 0) {
        advisor = 3;
        msgtext[0] = 0;
        named_msg("*UMAX");
        show_msg(msgtext, 80, 64);
        num_ai = 1;
    }
    return -1;
}

extern struct unit armydata[][128];
extern int far unit_losses[][28];

extern int far terrain_at(int x, int y);
extern void far lift_unit(int group, int index, int x, int y);
extern void far map_note(int x, int y, int group);
void each_stack(int group, int index, void (far * callback) (int, int));

void drop_u_stack(int group, int index)
{
#define record_type armydata[group][index].type
    if (see_fight_all)
        unit_losses[group][record_type]++;
    if (utypes[record_type].capacity && armydata[group][index].next != -1 && terrain_at(armydata[group][index].x, armydata[group][index].y) == 10)
        each_stack(group, index, remove_reset_kind_zero);
    if (record_type == 23 && armydata[group][index].next != -1 && terrain_at(armydata[group][index].x, armydata[group][index].y) == 10)
        each_stack(group, index, remove_reset_kind_one);
    forces_present[group][record_type]--;
    armydata[group][index].type = -1;
    armydata[group][index].moves = 0;
    lift_unit(group, index, armydata[group][index].x, armydata[group][index].y);
    if (!quiz_busy)
        if (style == -1)
            if (xray_on || group == civnow || (armydata[group][index].visibility & (1 << civnow)))
                map_note(armydata[group][index].x, armydata[group][index].y, group);
#undef record_type
}

extern struct unit armydata[][128];
extern int far terrain_at(int value_a, int value_b);
void each_stack(int group, int index, void (far * callback) (int, int));

int select_location_record(int group, int index)
{
    int candidate;
    if (armydata[group][index].next == -1)
        return index;
    selected_stack_index = index;
    selected_score = 0;
    for (candidate = 0; candidate < 128; candidate++)
        if (armydata[group][candidate].type != -1 && armydata[group][candidate].x == armydata[group][index].x && armydata[group][candidate].y == armydata[group][index].y) {
            each_stack(group, candidate, score_group_record);
            break;
        }
    return selected_stack_index;
}

int stack_other(int group, int index)
{
    if (armydata[group][index].next == -1)
        return index;
    selected_stack_index = index;
    selected_score = -1;
    each_stack(group, index, score_group_record);
    return selected_stack_index;
}

void score_group_record(int group, int index)
{
    int type, classification, score;
    type = armydata[group][index].type;
    classification = terrain_at(armydata[group][index].x, armydata[group][index].y);
    if (classification == 10 && utypes[type].domain != 2)
        return;
    if (utypes[type].domain == 0)
        score = ((armydata[group][index].status & 8) ? 3 : 2) * utypes[type].defense * terrains[classification].defense << 3;
    else
        score = utypes[type].defense << 4;
    if (armydata[group][index].status & 0x20)
        score += score / 2;
    if (score > selected_score) {
        selected_score = score;
        selected_stack_index = index;
    }
}

extern struct unit armydata[][128];
void each_stack(int group, int index, void (far * callback) (int, int));

int stack_len(int group, int index, int mode)
{
    group_total = 0;
    total_mode = mode;
    total_limit = index;
    each_stack(group, index, add_group_value);
    return group_total;
}

void add_group_value(int group, int index)
{
    switch (total_mode) {
    case 0:
        group_total += utypes[armydata[group][index].type].cost;
        break;
    case 1:
        group_total += utypes[armydata[group][index].type].defense;
        break;
    case 2:
        ++group_total;
        break;
    case 3:
        group_total += utypes[armydata[group][index].type].attack;
        break;
    case 4:
        if (index < total_limit && utypes[armydata[group][index].type].role == 2)
            ++group_total;
    }
}

extern struct unit armydata[][128];
extern int far terrain_at(int value_a, int value_b);
void each_stack(int group, int index, void (far * callback) (int, int));

int count_u(int group, int index, int value)
{
    target_value = value;
    match_count = 0;
    each_stack(group, index, count_matching_record);
    return match_count;
}

void count_matching_record(int group, int index)
{
    if (armydata[group][index].type == target_value)
        ++match_count;
}

int stack_strength(int group, int index, int value)
{
    target_value = value;
    match_count = 0;
    each_stack(group, index, match_type_category);
    return match_count;
}

void match_type_category(int group, int index)
{
    if (utypes[armydata[group][index].type].role == target_value)
        ++match_count;
}

int stack_cap(int group, int index)
{
    match_count = 0;
    each_stack(group, index, count_weighted_record);
    return match_count;
}

void count_weighted_record(int group, int index)
{
    if (utypes[armydata[group][index].type].domain == 0)
        --match_count;
    if (utypes[armydata[group][index].type].role == 5)
        match_count += utypes[armydata[group][index].type].capacity;
}

void each_stack(int group, int index, void (far *callback) (int, int))
{
    int state[5];
#define count state[0]
#define next_link state[1]
#define original state[2]
    count = 0;
    original = index;
    do {
        next_link = armydata[group][index].next;
        callback(group, index);
        index = next_link;
    } while (index != -1 && index - original != 0 && ++count < 10);
#undef original
#undef next_link
#undef count
}

void validate_record(int group, int index)
{
    if (terrain_at(armydata[group][index].x, armydata[group][index].y) == 10 && utypes[armydata[group][index].type].domain == 0)
        return;
    armydata[group][index].status &= 0xfe;
}

extern struct unit armydata[][128];
void unit_reset(int group, int index)
{
    if (armydata[group][index].status & 1) {
        armydata[group][index].status &= ~1;
        armydata[group][index].moves = 3 * utypes[armydata[group][index].type].moves;
        if (utypes[armydata[group][index].type].fuel)
            armydata[group][index].fuel = utypes[armydata[group][index].type].fuel - 1;
    }
    armydata[group][index].goto_x = armydata[group][index].f8 = -1;
}

extern struct unit armydata[][128];
extern void unit_redraw(int group, int index);

void reset_state(int group, int index)
{
    if (utypes[armydata[group][index].type].domain == 2)
        kill_cb(group, index);
}

void kill_cb(int group, int index)
{
    if (armydata[group][index].status & 0xc2)
        armydata[group][index].fuel = 0;
    if (armydata[group][index].status & 0xcb)
        armydata[group][index].moves = 3 * utypes[armydata[group][index].type].moves;
    armydata[group][index].status &= 0x30;
    armydata[group][index].goto_x = -1;
    if (group == civnow)
        unit_redraw(group, index);
}

extern struct unit armydata[][128];

void remove_reset_kind_zero(int group, int index)
{
    if (utypes[armydata[group][index].type].domain == 0)
        drop_u_stack(group, index);
}

void kill_stack_cb(int group, int index)
{
    if (utypes[armydata[group][index].type].domain != 0)
        drop_u_stack(group, index);
}

void remove_reset_kind_one(int group, int index)
{
    if (utypes[armydata[group][index].type].domain == 1)
        drop_u_stack(group, index);
}

extern struct unit armydata[][128];
extern int far wrap_x(int value);
extern void far reveal(int group, int x, int y);
extern int far map_flags(int x, int y);
extern int far terrain_at(int x, int y);
extern int far map_owner(int x, int y);
extern int far map_stack(int x, int y);
extern int far city_at(int x, int y);
extern void far map_mark(int x, int y);
extern void far map_note(int x, int y);

int adj_grp(int group, int x, int y);
void map_center(int group, int x, int y)
{
    int refresh = 0;
    if (x < 16 && map_x0 > 64)
        x += 80;
    if (x < map_x0 + 2 || x > map_x0 + 13)
        refresh = 1;
    if (y < map_y0 + 2 || y > map_y0 + 9)
        refresh = 1;
    if (refresh)
        reveal(group, wrap_x(x - 8), y - 6);
}

int if_open(int group, int x, int y)
{
    if (map_flags(x, y) & 1)
        return 0;
    return unit_blocked(group, x, y);
}

int unit_blocked(int group, int x, int y)
{
    int state[8];
#define unit_index state[7]
#define record_index state[6]
#define candidate_x state[5]
#define found_group state[4]
#define candidate_y state[3]
#define direction state[2]
#define classification state[1]
#define visibility_bit (*(unsigned char *)&state[0])
    classification = terrain_at(x, y);
    for (direction = 1; direction < 9; direction++) {
        candidate_x = wrap_x(spiral_x[direction] + x);
        candidate_y = spiral_y[direction] + y;
        if ((classification == 10) != (terrain_at(candidate_x, candidate_y) == 10))
            continue;
        found_group = map_owner(candidate_x, candidate_y);
        if (found_group == -1 || found_group == group)
            continue;
        if (group == civnow) {
            record_index = map_stack(candidate_x, candidate_y);
            if (!(armydata[found_group][record_index].visibility & (1 << civnow))) {
                armydata[found_group][record_index].visibility |= visibility_bit = 1 << civnow;
                civ_vis_grid[candidate_x][candidate_y] |= visibility_bit;
                map_note(candidate_x, candidate_y);
            }
            if (map_flags(candidate_x, candidate_y) & 1) {
                unit_index = city_at(candidate_x, candidate_y);
                townbuf[unit_index].visible_size = townbuf[unit_index].size;
                map_mark(candidate_x, candidate_y);
                civ_vis_grid[candidate_x][candidate_y] |= 1 << civnow;
                map_note(candidate_x, candidate_y);
            }
        }
        return 1;
    }
#undef visibility_bit
#undef classification
#undef direction
#undef candidate_y
#undef found_group
#undef candidate_x
#undef record_index
#undef unit_index
    return 0;
}

int adj_grp(int group, int x, int y)
{
    int state[4];
#define adjacent_x state[3]
#define adjacent_group state[2]
#define adjacent_y state[1]
#define adjacent_direction state[0]
    for (adjacent_direction = 1; adjacent_direction < 9; adjacent_direction++) {
        adjacent_x = wrap_x(spiral_x[adjacent_direction] + x);
        adjacent_y = spiral_y[adjacent_direction] + y;
        adjacent_group = map_owner(adjacent_x, adjacent_y);
        if (adjacent_group != -1 && adjacent_group != group)
            return 1;
    }
#undef adjacent_direction
#undef adjacent_y
#undef adjacent_group
#undef adjacent_x
    return 0;
}

/* Hut rewards and Barbarian encounters. */
#include <string.h>

extern struct unit armydata[][128];
extern struct city townbuf[];
extern struct terrain_type terrains[];
extern int rand_scaled(int);

extern int map_owner(int, int), map_flags(int, int);
extern int terrain_at(int, int);

extern void map_note();         /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

void enter_hut(int owner, int index)
{
    int a, b, c, d, e, f, g, h, i;
    if (owner == 0)
        return;
    if (utypes[armydata[owner][index].type].domain == 1)
        return;
    f = near_city(armydata[owner][index].x, armydata[owner][index].y);
    d = map_dist(armydata[owner][index].x, townbuf[f].x, armydata[owner][index].y, townbuf[f].y);
    switch (rand_scaled(4)) {
    case 0:
        g = help_probe(2, armydata[owner][index].x + 80, armydata[owner][index].y) & 7;
        if (d < 4)
            goto mercenaries;
        if (g <= 4)
            goto gold;
        strcpy(msgtext, "You have discovered\nan advanced tribe.\n");
        if (owner == civnow)
            show_msg(msgtext, 100, 80);
        city_create(owner, 0, armydata[owner][index].x, armydata[owner][index].y, 1);
        if (owner == civnow)
            map_note(armydata[owner][index].x, armydata[owner][index].y);
        break;
    case 1:
        if (!turn_tick || year > 1000)
            goto gold;
        strcpy(msgtext, "You have discovered\nscrolls of ancient wisdom.\n");
        if (owner == civnow)
            show_msg(msgtext, 100, 80);
        a = 0;
        i = rand_scaled(72);
        do {
            if (!has_tech(owner, i) && has_tech(owner, advances[i].prereq1) && has_tech(owner, advances[i].prereq2)) {
                give_advance(owner, i, 0);
                i = -1;
            } else {
                i++;
                i = (int) i % 72;
            }
            a++;
        } while (a < 999 && i != -1);
        break;
    case 2:
gold:
        strcpy(msgtext, "You have discovered\nvaluable metal deposits\nworth 50$\n");
        if (owner == civnow)
            show_msg(msgtext, 100, 80);
        money[owner] += 50;
        break;
    case 3:
        if (d < 4 || city_count[owner] == 0)
            goto mercenaries;
        strcpy(msgtext, "You have unleashed\na horde of barbarians!\n");
        if (owner == civnow)
            show_msg(msgtext, 100, 80);
        for (h = 1; h <= 8; h += clamp(4 - city_count[owner], 1, 4)) {
            c = armydata[owner][index].x + spiral_x[h];
            e = armydata[owner][index].y + spiral_y[h];
            if (map_owner(c, e) == -1 && !(map_flags(c, e) & 1) && terrain_at(c, e) != 10) {
                b = (signed char) terrains[terrain_at(c, e)].move_cost > 2 ? 3 : 6;
                armydata[0][create_unit(0, b, c, e)].visibility |= 1 << owner;
                if (owner == civnow)
                    map_note(c, e);
            }
        }
        break;
    case 4:
mercenaries:
        strcpy(msgtext, "You have discovered\na friendly tribe of\nskilled mercenaries.\n");
        if (owner == civnow)
            show_msg(msgtext, 100, 80);
        create_unit(owner, rand_scaled(2) ? 3 : 6, armydata[owner][index].x, armydata[owner][index].y, 1);
        if (owner == civnow)
            map_note(armydata[owner][index].x, armydata[owner][index].y);
        break;
    }
}

/* Exact animation [1D55,1F69). The SI local is reused for group,
 * record and direction offsets; inner direction_offset spills at BP-12.
 * Refresh computes its group offset inside the argument expression. */

extern struct unit armydata[][128];

extern int far seen_map(int x, int y);
extern int far in_view(int x, int y);
extern int far terrain_at(int x, int y);
extern void far unit_redraw(int group, int index);

extern int far wrap_x(int value);
extern void far label_with_style(int, int, int);
extern void far flip(int ticks);

void anim_unit(int group, int index, int direction)
{
    int screen_x, screen_y;
    if (still == 1) {
        seen_map(armydata[group][index].x, armydata[group][index].y);
        return;
    }
    mouse_off();
    {
        register int offset;
        int frame;

        {
            register int record_offset;
            int direction_offset;
            offset = group * 1536;
            record_offset = index * 12 + offset;
#define record (*(struct unit *)((char *)armydata+record_offset))
            if (!seen_map(record.x, record.y))
                goto done;
            if (!in_view(record.x, record.y))
                goto done;
            direction_offset = direction * 2;
            if (!in_view(record.x + *(int *) ((char *) spiral_x + direction_offset), record.y + *(int *) ((char *) spiral_y + direction_offset)))
                goto done;
            if (record.next != -1 && (terrain_at(record.x, record.y) != 10 || utypes[((struct unit *) ((char *) armydata + offset))
                                                                                     [record.next].type].domain == 2))
                goto refresh_link;
            goto draw_frame;
#undef record
        }
refresh_link:
        unit_redraw(group, ((struct unit *) ((char *) armydata + index * 12 + (offset = group * 1536)))->next);
draw_frame:;
        {
            blit(scrbuf, 80, 0, 240, 200, back_page, 80, 0);
            offset = group * 1536 + index * 12;
            screen_x = (wrap_x(((struct unit *) ((char *) armydata + offset))->x - map_x0) << 4) + 80;
            screen_y = ((((struct unit *) ((char *) armydata + offset))->y - map_y0) << 4) + 8;
            frame = 0;
            do {
                offset = direction * 2;
                label_with_style((signed char) armydata[group][index].type + (group << 5) + 64, screen_x + *(int *) ((char *) spiral_x + offset) * frame + 1, screen_y + *(int *) ((char *) spiral_y + offset) * frame + 1);
                flip(1);
                blit(back_page, screen_x + *(int *) ((char *) spiral_x + offset) * frame, screen_y + *(int *) ((char *) spiral_y + offset) * frame, 16, 16, scrbuf, screen_x + *(int *) ((char *) spiral_x + offset) * frame, screen_y + *(int *) ((char *) spiral_y + offset) * frame);
                frame++;
            } while (frame <= 16);
        }
    }
done:
    mouse_on();
}

/* Stack selection dialog, 0866:[1F69,226D). */
#include <string.h>
extern struct unit armydata[][128];

extern int kbhit(void);

extern void recolour_rect(int, int, int, int, int, int, int);
int pick_unit(int owner, int index)
{                               /* choose a unit from a stack */
    int a, b[12], c, d, e, f, g, h, i;
    mouse_off();
    menu_begin();
    h = select_location_record(owner, index);
    i = 0;
    d = index;
    a = 0;
    do {
        b[i] = d;
        d = armydata[owner][d].next;
        i++;
    } while (i < 12 && d != -1 && d != index);
    f = 96 - (i << 3);
redraw_list:
    panel_frame(100, f, 120, (i << 4) + 5, 3);
    i = 0;
    d = index;
    e = f + 5;
    do {
        draw_unit(owner, d, 106, e);
        strcpy(msgtext, utypes[armydata[owner][d].type].name);
        if (armydata[owner][d].status & 0x20)
            strcat(msgtext, " (V)");
        draw_string(msgtext, 128, e, 15);
        msgtext[0] = 0;
        append_place_name(armydata[owner][d].home_city);
        draw_string(msgtext, 128, e + 8, 14);
        d = armydata[owner][d].next;
        e += 16;
        i++;
    } while (i < 12 && d != -1 && d != index);
    mouse_on();
    repaint();
    if (has_mouse) {
        do {
            poll_mouse();
        } while (!mse_held && !kbhit());
    } else {
        do {
            recolour_rect(scrbuf, 105, (a << 4) + f + 4, 112, 17, 3, 11);
            c = a;
            g = key_cmd();
            switch (g) {
            case 0x4800:
                if (a != 0)
                    a--;
                break;
            case 0x5000:
                if (a < i - 1)
                    a++;
                break;
            }
            recolour_rect(scrbuf, 105, (c << 4) + f + 4, 112, 17, 11, 3);
            if (g == 13 || g == 32) {
                d = a;
                goto activate;
            }
        } while (g != 27);
    }
    mouse_off();
    if (mse_held == 1 && ptr_x > 100 && ptr_x < 220) {
        d = (ptr_y - f - 5) / 16;
activate:
        if (d >= 0 && d < i) {
            h = b[d];
            if (armydata[owner][h].status & 0xc2)
                armydata[owner][h].moves = 3 * utypes[armydata[owner][h].type].moves;
            armydata[owner][h].status &= 0x30;
            armydata[owner][h].goto_x = -1;
            if (i > 1)
                goto redraw_list;
        }
    }
    menu_end();
    mouse_on();
    repaint();
    return h;
}

/* Nearest city/transport direction, 0866:[226D,250E). */
#include <stdlib.h>

int near_base(int owner, int index)
{                               /* nearest city or carrier for a unit */
    int a, b, c, d, e, f, g, h, i, j, k;
    c = 999;
    f = -1;
    for (a = 0; a < 128; a++) {
        if (townbuf[a].status == -1 || townbuf[a].owner != owner)
            continue;
        {
            e = abs(armydata[owner][index].x - townbuf[a].x);
            if (e > 40)
                e = 80 - e;
            g = abs(armydata[owner][index].y - townbuf[a].y);
            b = e < g ? g : e;
            if (b < c) {
                c = b;
                j = townbuf[a].x;
                k = townbuf[a].y;
                f = a;
            }
        }
    }
    d = 0;
    do {
        if (armydata[owner][d].type != 23)
            goto unit_next;
        {
            e = abs(armydata[owner][index].x - armydata[owner][d].x);
            if (e > 40)
                e = 80 - e;
            g = abs(armydata[owner][index].y - armydata[owner][d].y);
            b = e < g ? g : e;
            if (b < c) {
                c = b;
                j = armydata[owner][d].x;
                k = armydata[owner][d].y;
            }
        }
unit_next:
        d++;
    } while (d < 128);
    h = 0;
    c = distance_twice(armydata[owner][index].x - j, armydata[owner][index].y - k);
    i = 1;
    do {
        e = armydata[owner][index].x + spiral_x[i];
        g = armydata[owner][index].y + spiral_y[i];
        a = map_owner(e, g);
        if (a == -1 || a == owner) {
            if (!(map_flags(e, g) & 1) || map_city(e, g) == owner) {
                b = distance_twice(e - j, g - k);
                if (b < c) {
                    c = b;
                    h = i;
                }
            }
        }
        i++;
    } while (i <= 8);
    return h;
}

/* Event stream append, 0866:[250E,25B6). */
extern int replay_length;
extern char far replay_log[];
/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void post_event(type, count, arg1, arg2, arg3, arg4)
int type, count, arg1, arg2, arg3, arg4;
{
    if (count + replay_length + 2 < 4096) {
        replay_log[replay_length++] = (type << 4) + (turn_tick >> 8);
        replay_log[replay_length++] = turn_tick;
        if (count > 0)
            replay_log[replay_length++] = arg1;
        if (count > 1)
            replay_log[replay_length++] = arg2;
        if (count > 2)
            replay_log[replay_length++] = arg3;
        if (count > 3)
            replay_log[replay_length++] = arg4;
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

extern void far draw_values(int, int, int);
void menu_begin(void)
{
    blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
}

void menu_end(void)
{
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int help_probe(int a, int b, int c)
{
    if (draw_large_text)
        pixel_at(a, b * 2, c);
    else
        pixel_at(a, b, c);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

extern int far rand_scaled(int limit);

void menu_clear(void)
{
    int state[3];
#define border_y state[0]
#define border_x state[1]
#define index state[2]
    border_x = (rand_scaled(2) << 5) + 192;
    border_y = 120;
    blit(tile_page, border_x, border_y, 8, 8, scrbuf, 0, 0);
    blit(tile_page, border_x + 8, border_y, 8, 8, scrbuf, 312, 0);
    blit(tile_page, border_x + 24, border_y, 8, 8, scrbuf, 312, 192);
    blit(tile_page, border_x + 16, border_y, 8, 8, scrbuf, 0, 192);
    for (index = 1; index < 24; index++) {
        blit(tile_page, border_x + 24, border_y + 8, 8, 8, scrbuf, 0, index << 3);
        blit(tile_page, border_x + 8, border_y + 8, 8, 8, scrbuf, 312, index << 3);
    }
    for (index = 1; index < 39; index++) {
        blit(tile_page, border_x, border_y + 8, 8, 8, scrbuf, index << 3, 0);
        blit(tile_page, border_x + 16, border_y + 8, 8, 8, scrbuf, index << 3, 192);
    }
#undef index
#undef border_x
#undef border_y
}
