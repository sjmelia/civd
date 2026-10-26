/* Unit planning, orders, targets, settlement sites and Barbarian movement. */
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UNITAI.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIPLOMAC.H"
#include "ECONOMY.H"
#include "MAP.H"
#include "PATH.H"
#include "TURN.H"
#include "UNITS.H"

#include "CIV.H"
#include "DIPLOMAT.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "NEWGAME.H"
#include "RESIDENT.H"
#include "BATTLE.H"
#include "DRIVER.H"


extern int treaties[8][8];
extern struct unit armydata[8][128];
extern struct unit_type utypes[28];
extern struct city townbuf[128];
extern char civ_vis_grid[];
extern char far fog_map[];

struct terr {
    char name[12];
    char cost;
    char defense;
    char food;
    char shields;
    char rest[3];
};





extern int work_rules[][6];
extern struct terr terrains[];
int ai_watch_player = -1;
int ai_watch_unit = -1;
int near_human = 0;             /* mask: civ shares a continent with the human's cities */

extern int treaties[8][8];

void unit_phase(int player)
{
    int danger;
    int cap;
    int max_trade;
    int rating;
    int slots[32];
    int v;
    int best_cost;
    int tot_a;
    int x;
    int i;
    int y;
    int j;
    int z;                      /* unused in the original too (frame slot 0x56) */
    int friends;
    int island;
    int flags;
    int prev;
    int strong;

    for (i = 0; i < 16; i++) {
        defenders_island[player][i] = civ_cont_attackers[player][i] = ai_continent_towns[player][i] = 0;
    }
    for (x = 0; x < 20; x++)
        for (y = 0; y < 13; y++)
            fog_map[x * 13 + y] = 0;
    troops_total[player] = civ_settlers_n[player] = military_table[player] = 0;
    for (i = 0; i < 128; i++) {
        if (armydata[player][i].type != -1 && armydata[player][i].type != 0x19) {
            if (armydata[player][i].type == 0)
                civ_settlers_n[player]++;
            else
                troops_total[player]++;
            military_table[player] += utypes[armydata[player][i].type].attack + utypes[armydata[player][i].type].defense;
            armydata[player][i].status &= ~0x10;
            if (unit_blocked(player, armydata[player][i].x, armydata[player][i].y))
                armydata[player][i].status |= 0x10;
            if (terrain_at(armydata[player][i].x, armydata[player][i].y) != 10) {
                island = region_key(armydata[player][i].x, armydata[player][i].y);
                defenders_island[player][island] += utypes[armydata[player][i].type].defense;
                civ_cont_attackers[player][island] += utypes[armydata[player][i].type].attack;
            } else if (utypes[armydata[player][i].type].domain == 0)
                armydata[player][i].status |= 0x10;
            fog_map[(armydata[player][i].x >> 2) * 13 + (armydata[player][i].y >> 2)]
                |= 2 << utypes[armydata[player][i].type].role;
        }
    }

    cap = clamp(troops_total[player] / 8, 3, 99);
    city_count[player] = citizens[player] = 0;
    for (i = 0; i < 28; i++)
        civ_units_ordered[player][i] = 0;
    for (i = 0; i < 128; i++) {
        if (townbuf[i].status == -1)
            continue;
        if (player != townbuf[i].owner) {
            if (turn_tick * cur_tier > 200 || (civ_vis_grid[townbuf[i].x * 50 + townbuf[i].y] & (1 << player))
                || civnow != townbuf[i].owner) {
                if ((treaties[player][townbuf[i].owner] & 0x102) != 2 && ((i + turn_tick) & 3) != 0)
                    add_target(player, townbuf[i].x, townbuf[i].y, 1, (townbuf[i].buildings & 0x80) ? 3 : 5);
            }
        } else {
            city_count[player]++;
            ai_continent_towns[player][region_key(townbuf[i].x, townbuf[i].y)]++;
            citizens[player] += townbuf[i].size;
            if (townbuf[i].production >= 0)
                civ_units_ordered[player][townbuf[i].production]++;
            x = townbuf[i].x >> 2;
            y = townbuf[i].y >> 2;
            fog_map[x * 13 + y] |= 1;
            for (v = 0; v < 2; v++) {
                if (townbuf[i].garrison[v] != -1) {
                    military_table[player] += utypes[townbuf[i].garrison[v] & 0x3f].attack + utypes[townbuf[i].garrison[v] & 0x3f].defense;
                    island = region_key(townbuf[i].x, townbuf[i].y);
                    defenders_island[player][island] += utypes[townbuf[i].garrison[v] & 0x3f].defense;
                    civ_cont_attackers[player][island] += utypes[townbuf[i].garrison[v] & 0x3f].attack;
                }
            }
        }
        if (map_stack(townbuf[i].x, townbuf[i].y) == -1 && townbuf[i].garrison[0] == -1)
            add_target(player, townbuf[i].x, townbuf[i].y, 2, townbuf[i].owner == player ? 4 : 2);
    }
    civ_held_cells[player] = 0;
    for (x = 0; x < 20; x++)
        for (y = 0; y < 13; y++)
            if (fog_map[x * 13 + y])
                civ_held_cells[player]++;
    civ_area_plan[player][0] = 0;
    civ_area_plan[player][15] = 0;
    near_human &= ~(1 << player);
    in_at_war = 0;
    for (j = 1; j < 8; j++)
        if ((treaties[player][j] & 3) == 1)
            in_at_war = 1;
    max_trade = -1;
    for (j = 1; j < 15; j++) {
        strong = 0;
        danger = 0;
        tot_a = friends = flags = 0;
        prev = civ_area_plan[player][j];
        for (i = 1; i < 8; i++) {
            if (defenders_island[i][j] != 0 && (civ_cont_attackers[player][j] / 4 < civ_cont_attackers[i][j] || ai_continent_towns[i][j] != 0)
                && ((treaties[player][i] & 3) == 1 || (treaties[player][i] & 0x100))) {
                if (defenders_island[i][j] < civ_cont_attackers[player][j]
                    || civ_cont_attackers[i][j] < defenders_island[player][j]
                    || ai_continent_towns[player][j] == 0)
                    strong++;
                else
                    danger++;
            }
            if (defenders_island[i][j] != 0 && civnow == i && (treaties[player][i] & 2)
                && civ_cont_attackers[player][j] / 2 + defenders_island[player][j] < civ_cont_attackers[i][j])
                danger++;
            tot_a += defenders_island[i][j];
            friends += ai_continent_towns[i][j];
            if (defenders_island[i][j] != 0)
                flags |= (treaties[player][i] & 2) ? 1 : 2;
        }
        if (ai_continent_towns[0][j] != 0)
            strong++;
        civ_area_plan[player][j] = ((defenders_island[player][j] + tot_a) * 2 > continent_span[0][j]
                                    || (ai_continent_towns[player][j] + friends) * 6 + 2 > ai_zone_places[j])
            && has_tech(player, 7) ? 5 : 0;
        if (strong)
            civ_area_plan[player][j] = 1;
        if (danger)
            civ_area_plan[player][j] = 2;
        if (defenders_island[player][j] == 0 && ai_continent_towns[player][j] == 0 && flags != 1)
            civ_area_plan[player][j] = 1;
        if (max_trade < ai_continent_towns[player][j])
            max_trade = ai_continent_towns[player][j];
        if (civ_area_plan[player][j] != prev && !((1 << player) & human_set))
            retarget(player, j);
        if (defenders_island[player][j] != 0 && ai_continent_towns[civnow][j] > 1)
            near_human |= 1 << player;
    }
    if (player == 0)
        return;
    if ((1 << player) & human_set)
        return;
    if (((player + turn_tick) & 7) == 0) {
        if (troop_over <= 0) {
            if (has_tech(player, 0x2c) && margin_democracy >= 0)
                set_government(player, 4);
            else if (has_tech(player, 0x2b) && city_count[player] > 10)
                set_government(player, 3);
            else if (has_tech(player, 5))
                set_government(player, 2);
        } else
            set_government(player, 1);
    }
    for (j = 0; j < 32; j++)
        slots[j] = cap;
    for (i = 0; i < 128; i++) {
        if (armydata[player][i].type == -1 || (armydata[player][i].status & 0x10)
            || armydata[player][i].type == 0x19)
            continue;
        best_cost = 9999;
        flags = -1;
        x = armydata[player][i].x;
        y = armydata[player][i].y;
        island = region_key(x, y);
        if (((i + turn_tick) & 15) == 0 && (has_tech(player, utypes[armydata[player][i].type].obsolete_by)
                                            || ((civ_gov[player] > 1 || civ_area_plan[player][island] == 5)
                                                && armydata[player][i].type == 1))
            && !unit_blocked(player, x, y)
            && !((map_flags(x, y) & 1) && armydata[player][i].next == -1)
            && terrain_at(x, y) != 10 && civ_area_plan[player][island] != 2) {
            drop_u_stack(player, i);
            continue;
        }
        for (j = 0; j < 32; j++) {
            if (objectives_type[player][j] != -1 && ((utypes[armydata[player][i].type].role == objectives_type[player][j]
                                                      && region_key(civ_targets_x[player][j], civ_targets_y[player][j]) == island)
                                                     || (armydata[player][i].type == 15 && (utypes[armydata[player][i].type].role == objectives_type[player][j]
                                                                                            || objectives_type[player][j] == 3)))) {
                if (armydata[player][i].type == 15) {
                    if ((aim_count[player][j] & 1) && ((j + (turn_tick >> 1)) & 1)) {
                        rating = map_dist(x, civ_targets_x[player][j], y, civ_targets_y[player][j]);
                        rating = (utypes[armydata[player][i].type].moves >= rating ? 2 : 4)
                            * rating / (aim_count[player][j] + 1);
                    } else
                        continue;
                } else
                    rating = slots[j] * map_dist(x, civ_targets_x[player][j], y, civ_targets_y[player][j])
                        / (aim_count[player][j] + 1);
                if (armydata[player][i].status & 0xc) {
                    if (aim_count[player][j] < 2 || (aim_count[player][j] * cap < rating && slots[j] != cap))
                        continue;
                    if (map_flags(armydata[player][i].x, armydata[player][i].y) & 1)
                        continue;
                }
                if (rating >= best_cost)
                    continue;
                if (4 * aim_count[player][j] < rating / cap)
                    continue;
                best_cost = rating;
                flags = j;
            }
        }
        if (flags == -1)
            continue;
        armydata[player][i].goto_x = civ_targets_x[player][flags];
        armydata[player][i].goto_y = civ_targets_y[player][flags];
        armydata[player][i].status |= 0x10;
        armydata[player][i].status &= 0xf0;
        slots[flags]++;
    }
}

int ai_order(int player, int unit)
{
    int freem;
    int odds_f;
    int cowner;
    int rolel;
    int count_f;
    int attack_f;
    int pbitn;
    int unused_j;
    int blocked_i;
    int regn;
    int unused_i;
    int unused_y;
    int j2;
    int terrain_g;
    int k;
    int foundn;
    int ok;
    int sc_d;
    int tern;
    int uyk;
    int home;
    int near_dist;
    int uxk;
    int ny;
    int result_i;
    int city_distj;
    int best_scorem;
    int nx;
    int stk_d;
    int jk;
    int own_b;
    int roles;
    int enemy_g;
    int region;
    int unused1n;
    int mym;
    int best_d_c;
    int dn;
    int vl;
    int mxm;
    int cn;
    int adiff_h;
    int utypej;
    int divisor;
    int best_c;
    int rstatem;

    if (player == ai_watch_player && unit == ai_watch_unit)
        ai_break();
    if (player == 0)
        return unit_ai_move(player, unit);
    if (armydata[player][unit].type == 15 && armydata[player][unit].fuel <= 0)
        return 0x68;
    if (armydata[player][unit].type == 14 && armydata[player][unit].goto_x == -1) {
        if (utypes[armydata[player][unit].type].moves / 2 * 3 >= armydata[player][unit].moves)
            return 0x68;
        best_d_c = 999;
        best_c = -1;
        for (k = 0; k < 128; k++) {
            if (armydata[civnow][k].type == 15 && (armydata[civnow][k].visibility & (1 << player))) {
                dn = map_dist(armydata[player][unit].x, armydata[civnow][k].x, armydata[player][unit].y, armydata[civnow][k].y);
                if (dn < best_d_c) {
                    best_d_c = dn;
                    best_c = k;
                }
            }
        }
        if (best_c != -1 && dn > 1) {
            if (utypes[armydata[player][unit].type].moves > best_d_c) {
                armydata[player][unit].goto_x = armydata[civnow][best_c].x;
                armydata[player][unit].goto_y = armydata[civnow][best_c].y;
            } else {
                best_d_c = 999;
                home = -1;
                for (k = 0; k < 128; k++) {
                    if (townbuf[k].status == -1 || townbuf[k].owner != player)
                        continue;
                    if (utypes[armydata[player][unit].type].moves < map_dist(armydata[player][unit].x, townbuf[k].x, armydata[player][unit].y, townbuf[k].y))
                        continue;
                    dn = map_dist(armydata[civnow][best_c].x, townbuf[k].x, armydata[civnow][best_c].y, townbuf[k].y);
                    if (dn < best_d_c) {
                        best_d_c = dn;
                        home = k;
                    }
                }
                if (home != -1) {
                    armydata[player][unit].goto_x = townbuf[home].x;
                    armydata[player][unit].goto_y = townbuf[home].y;
                } else
                    return 0x20;
            }
        }
    }
    blocked_i = if_open(player, armydata[player][unit].x, armydata[player][unit].y);
    result_i = 0;
    uxk = armydata[player][unit].x;
    uyk = armydata[player][unit].y;
    utypej = armydata[player][unit].type;
    home = near_city(uxk, uyk);
    city_distj = town_gap;
    armydata[player][unit].type = -1;
    best_c = nearest_record(player, unit, uxk, uyk);
    near_dist = town_gap;
    armydata[player][unit].type = utypej;
    terrain_g = terrain_at(uxk, uyk);
    rolel = utypes[armydata[player][unit].type].role;
    region = region_key(uxk, uyk);
    rstatem = civ_area_plan[player][region];
    pbitn = 1 << player;

    if (utypej == 25) {
        best_scorem = -1;
        for (cn = 0; cn < 128; cn++) {
            sc_d = townbuf[cn].owner;
            if (townbuf[cn].status != -1 && !(townbuf[cn].buildings & 0x10000L)
                && (military_table[player] * 3 < military_table[sc_d] * 2 || (treaties[player][sc_d] & 8)
                    || (forces_present[sc_d][25] == 0 && military_table[player] < military_table[sc_d] * 2))
                && (treaties[player][sc_d] & 0x82) == 0x80 && townbuf[cn].size > 4) {
                sc_d = 0;
                for (k = 0; k <= 8; k++) {
                    nx = townbuf[cn].x + spiral_x[k];
                    ny = townbuf[cn].y + spiral_y[k];
                    own_b = map_owner(nx, ny);
                    if (own_b != -1) {
                        if ((treaties[player][own_b] & 0x82) == 0x80)
                            sc_d++;
                        else if (own_b == player)
                            sc_d -= 2;
                        else
                            sc_d -= 99;
                    }
                }
                sc_d += townbuf[cn].size / 2;
            }
            if (sc_d > best_scorem) {
                foundn = 0;
                for (jk = 0; jk < 128; jk++) {
                    if (townbuf[jk].status != -1 && townbuf[jk].owner == player && map_dist(townbuf[cn].x, townbuf[jk].x, townbuf[cn].y, townbuf[jk].y) <= 16) {
                        foundn = 1;
                        break;
                    }
                }
                if (foundn) {
                    best_scorem = sc_d;
                    home = cn;
                }
            }
        }
        if (best_scorem >= 10 && forces_present[player][25] > 1) {
            cn = home;
            armydata[player][unit].goto_x = townbuf[cn].x;
            armydata[player][unit].goto_y = townbuf[cn].y;
            for (k = 1; k <= 8; k++) {
                nx = (unsigned char) armydata[player][unit].goto_x + spiral_x[k];
                ny = (unsigned char) armydata[player][unit].goto_y + spiral_y[k];
                if (map_owner(nx, ny) == -1) {
                    lift_unit(player, unit, uxk, uyk);
                    armydata[player][unit].x = nx;
                    armydata[player][unit].y = ny;
                    put_unit(player, unit, nx, ny);
                    if (townbuf[home].owner == civnow)
                        last_met_turn[player] = -2;
                    return ((k - 1) ^ 4) + 1;
                }
            }
        }
        return 0x20;
    }
    if (utypej == 26) {
        if ((unsigned char) armydata[player][unit].goto_x != 0xff)
            return 0;
        if (townbuf[home].owner != player) {
            armydata[player][unit].goto_x = townbuf[home].x;
            armydata[player][unit].goto_y = townbuf[home].y;
            return 0;
        }
        drop_u_stack(player, unit);
    }
    if (rolel == 0)
        clear_sites_near(player, 0, uxk, uyk, 0);
    best_scorem = help_probe(2, uxk + 80, uyk);
    if (best_scorem != 0 && rolel < 3) {
        result_i = 0;
        for (k = 1; k <= 8; k++) {
            nx = wrap_x(spiral_x[k] + uxk);
            ny = spiral_y[k] + uyk;
            vl = help_probe(2, nx + 80, ny);
            if (vl > best_scorem) {
                best_scorem = vl;
                result_i = k;
            }
            if (has_hut(terrain_at(nx, ny), nx, ny) && rolel != 0)
                return k;
        }
        if (turn_tick == 0 && beginner_help != 0) {
            best_scorem = 15;
            result_i = 0;
        }
        if (best_scorem >= 8 && 15 - city_distj <= best_scorem && blocked_i == 0) {
            if (rolel == 0) {
                if (result_i == 0)
                    return 0x62;
                else
                    return result_i;
            } else
                site_add(player, uxk, uyk, 0, 2);
        }
    }
    if (rolel == 0 && cur_tier != 0 && townbuf[home].owner == civnow && blocked_i == 0 && city_distj > 1 && player != townbuf[home].owner && tech_count[player] < tech_count[civnow]) {
        sc_d = help_probe(2, uxk + 80, uyk + 50);
        if (sc_d >= 9 && 14 - city_distj <= sc_d)
            return 0x62;
    }
    if (map_flags(uxk, uyk) & 1) {
        if (rolel == 2) {
            if (armydata[player][unit].next == -1)
                return 0x66;
            else {
                sc_d = armydata[player][unit].type;
                armydata[player][unit].type = 26;
                own_b = select_location_record(player, unit);
                armydata[player][unit].type = sc_d;
                if (own_b != -1 && utypes[armydata[player][own_b].type].defense < utypes[armydata[player][unit].type].defense)
                    return 0x66;
            }
            divisor = unit_blocked(player, uxk, uyk) ? 3 : 4;
            if (townbuf[home].buildings & 1)
                divisor--;
            else if (rstatem == 0)
                divisor++;
            sc_d = stack_strength(player, unit, 2);
            if (townbuf[home].size / divisor + 1 > sc_d)
                add_target(player, uxk, uyk, 2, 2);
            if (townbuf[home].size / divisor + 1 >= sc_d)
                return 0x66;
            sc_d = armydata[player][unit].status;
            armydata[player][unit].status |= 8;
            if (select_location_record(player, unit) == unit && townbuf[home].size / divisor + 1 > stack_len(player, unit, 4)) {
                armydata[player][unit].status = sc_d;
                return 0x66;
            } else {
                armydata[player][unit].status = sc_d;
                if (((unit + turn_tick) & 7) == 0)
                    armydata[player][unit].status &= 0xf2;
            }
        } else if (near_dist > 0 && rolel != 0) {
            add_target(player, uxk, uyk, 2, 4);
            return 0x20;
        }
    }
    if (rolel == 5 || rolel == 3) {
        count_f = roles = result_i = 0;
        own_b = armydata[player][unit].next;
        while (own_b != -1 && own_b != unit) {
            if (!(armydata[player][own_b].status & 8)) {
                roles |= 1 << utypes[armydata[player][own_b].type].role;
                if (utypes[armydata[player][own_b].type].domain == 0)
                    count_f++;
            } else if (rstatem == 5 && townbuf[cn].size / 5 < result_i++) {
                armydata[player][own_b].status &= ~8;
                count_f++;
            }
            own_b = armydata[player][own_b].next;
        }
        if (((city_distj ? 1 : 3) > count_f || roles == 1) && rolel == 5) {
            best_d_c = 999;
            for (cn = 0; cn < 128; cn++) {
                if (townbuf[cn].status == -1 || townbuf[cn].owner != player || !(townbuf[cn].status & 2))
                    continue;
                dn = map_dist(uxk, townbuf[cn].x, uyk, townbuf[cn].y);
                regn = region_key(townbuf[cn].x, townbuf[cn].y);
                if (defenders_island[player][regn] < 16)
                    continue;
                stk_d = map_stack(townbuf[cn].x, townbuf[cn].y);
                if (stk_d != -1 && stack_strength(player, stk_d, 5) > !dn)
                    dn += 16;
                if (civ_area_plan[player][regn] != 5)
                    dn += 16;
                else
                    dn -= civ_cont_attackers[player][regn] / 4;
                if (dn < best_d_c) {
                    best_d_c = dn;
                    armydata[player][unit].goto_x = townbuf[cn].x;
                    armydata[player][unit].goto_y = townbuf[cn].y;
                    sc_d = civ_area_plan[player][regn];
                }
            }
            dn = sc_d == 5 ? 4 : 2;
            if (3 * dn >= best_d_c) {
                if (sc_d != 0)
                    add_target(player, (unsigned char) armydata[player][unit].goto_x, (unsigned char) armydata[player][unit].goto_y, 0, dn);
                if (sc_d != 2)
                    add_target(player, (unsigned char) armydata[player][unit].goto_x, (unsigned char) armydata[player][unit].goto_y, 2, dn);
                if (sc_d != 1)
                    add_target(player, (unsigned char) armydata[player][unit].goto_x, (unsigned char) armydata[player][unit].goto_y, 1, dn);
            }
        } else {
            if (rolel == 5) {
                for (k = 1; k <= 8; k++) {
                    nx = wrap_x(spiral_x[k] + uxk);
                    ny = spiral_y[k] + uyk;
                    own_b = map_owner(nx, ny);
                    if (terrain_at(nx, ny) != 10 && map_in_bounds(nx, ny) && (own_b == -1 || own_b == player)) {
                        if (own_b == player && stack_len(player, map_stack(nx, ny), 2) >= 2)
                            continue;
                        if ((treaties[player][civnow] & 2) && map_city(nx, ny) == civnow && (map_flags(nx, ny) & 0xf))
                            continue;
                        regn = region_key(nx, ny);
                        ok = 0;
                        if ((defenders_island[player][regn] < 16 && civ_area_plan[player][regn] != 5)
                            || ((1 << civ_area_plan[player][regn]) & roles & 6))
                            ok = 1;
                        if ((unsigned char) armydata[player][unit].goto_x != 0xff && region_key((unsigned char) armydata[player][unit].goto_x, (unsigned char) armydata[player][unit].goto_y) == regn)
                            ok = 1;
                        if (ok && ai_zone_places[regn] >= 5 && ny > 1 && ny < 48) {
                            if (armydata[player][unit].moves > 3) {
                                sc_d = ocean_edge(player, uxk, uyk);
                                if (sc_d > 0)
                                    return sc_d;
                            }
                            if (civ_area_plan[player][regn] == 1)
                                add_target(player, uxk, uyk, 3, 5);
                            armydata[player][unit].moves = 0;
                            armydata[player][unit].goto_x = -1;
                            return 0x75;
                        } else {
                            if (!(turn_tick & 3))
                                armydata[player][unit].goto_x = -1;
                        }
                        break;
                    }
                }
                if (armydata[player][unit].moves <= 3) {
                    nx = ((unsigned char) armydata[player][unit].x + (unsigned char) armydata[player][unit].goto_x) / 2;
                    ny = ((unsigned char) armydata[player][unit].y + (unsigned char) armydata[player][unit].goto_y) / 2;
                    if ((unsigned char) armydata[player][unit].goto_x == 0xff || terrain_at(nx, ny) != 10) {
                    } else
                        add_target(player, nx, ny, 3, 3);
                }
            }
            if (blocked_i == 0 || rolel == 5) {
                if ((unsigned char) armydata[player][unit].goto_x == 0xff && ((roles & 2) || !(roles & 1))
                    && armydata[player][unit].type != 16) {
                    best_d_c = 999;
                    for (k = turn_tick & 7; k < 128; k += 8) {
                        sc_d = townbuf[k].owner;
                        if (townbuf[k].status != -1 && sc_d != player && (treaties[player][sc_d] & 0x102) != 2 && (nation_ratings[civnow] < 7 || sc_d == civnow)) {
                            dn = map_dist(uxk, townbuf[k].x, uyk, townbuf[k].y);
                            if (dn < best_d_c) {
                                best_d_c = dn;
                                armydata[player][unit].goto_x = townbuf[k].x;
                                armydata[player][unit].goto_y = townbuf[k].y;
                            }
                        }
                    }
                }
                if ((unsigned char) armydata[player][unit].goto_x == 0xff && armydata[player][unit].type != 16) {
                    best_d_c = 999;
                    vl = -1;
                    if (terrain_at(uxk, uyk) != 10)
                        vl = region;
                    for (k = 0; k < 128; k++) {
                        sc_d = townbuf[k].owner;
                        if (townbuf[k].status != -1 && sc_d == player) {
                            dn = map_dist(uxk, townbuf[k].x, uyk, townbuf[k].y);
                            if (dn < 8)
                                continue;
                            regn = region_key(townbuf[k].x, townbuf[k].y);
                            if (regn == vl)
                                continue;
                            if (civ_area_plan[player][regn] == 5)
                                continue;
                            if (!((1 << civ_area_plan[player][regn]) & roles & 7))
                                dn *= 2;
                            if (dn < best_d_c) {
                                best_d_c = dn;
                                armydata[player][unit].goto_x = townbuf[k].x;
                                armydata[player][unit].goto_y = townbuf[k].y;
                            }
                        }
                    }
                }
                if ((unsigned char) armydata[player][unit].goto_x == 0xff) {
                    for (k = 2; k < 24; k++) {
                        sc_d = rand_scaled(9);
                        nx = wrap_x(spiral_x[sc_d] * k + armydata[player][unit].x);
                        ny = spiral_y[sc_d] * k + armydata[player][unit].y;
                        if (ny > 2 && ny < 47 && terrain_at(nx, ny) != 10 && ai_continent_towns[player][region_key(nx, ny)] == 0) {
                            armydata[player][unit].goto_x = nx;
                            armydata[player][unit].goto_y = ny;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (utypes[armydata[player][unit].type].moves < 2 && city_distj < 4 && townbuf[home].owner == civnow && !(treaties[civnow][player] & 2)
        && (map_flags(uxk, uyk) & 6)
        && townbuf[home].owner != player)
        return 0x50;
    if (rolel == 0) {
        if (terrain_g != 10) {
            if (near_dist >= 2)
                add_target(player, uxk, uyk, 2, 2);
            if ((work_rules[terrain_g][has_tech(player, 5) ? 5 : 4] != 0 || rstatem == 5)
                && city_distj > 0 && city_distj <= 2 && townbuf[home].owner == player && !(townbuf[home].size < 3 && terrain_g == 4 && !patterned_cell(terrain_g, uxk, uyk))
                && (rule_options & 2)) {
                switch (work_tile(uxk, uyk)) {
                case 1:
                    armydata[player][unit].goto_x = -1;
                    return 0x69;
                case 2:
                    armydata[player][unit].goto_x = -1;
                    return 0x6d;
                }
                sc_d = map_flags(uxk, uyk);
                if ((sc_d & 6) && !(sc_d & 8) && terrain_g <= 2) {
                    armydata[player][unit].goto_x = -1;
                    return 0x72;
                }
                if ((sc_d & 0x18) != 0x18 && has_tech(player, 0x31)
                    && tile_yield(uxk, uyk, 1) >= ((sc_d & 8) ? 1 : 2)) {
                    armydata[player][unit].goto_x = -1;
                    return 0x72;
                }
            }
            if (map_flags(uxk, uyk) & 0x40)
                return 0x70;
            if ((unsigned char) armydata[player][unit].goto_x != 0xff && (civt[civ_tribe_of[player]].expansionist + 1) * ai_zone_places[region]
                > ai_continent_towns[player][region] << 4)
                return 0;
            if (city_distj <= 1 && townbuf[home].owner == player && (rule_options & 2)) {
                for (k = 1; k <= 8; k++) {
                    sc_d = ((k + turn_tick) & 7) + 1;
                    nx = spiral_x[sc_d] + uxk;
                    ny = spiral_y[sc_d] + uyk;
                    own_b = map_stack(nx, ny);
                    if (own_b != -1 && (player != fight_enemy || count_u(player, own_b, 0)))
                        continue;
                    if (work_rules[terrain_at(nx, ny)][has_tech(player, 5) ? 5 : 4] != 0 && work_tile(nx, ny)
                        && !(townbuf[home].size < 3 && terrain_at(nx, ny) == 4 && !patterned_cell(4, nx, ny))
                        && half_dist(nx - townbuf[home].x, ny - townbuf[home].y) <= 2) {
                        armydata[player][unit].goto_x = nx;
                        armydata[player][unit].goto_y = ny;
                        return 0;
                    }
                }
            }
            if (!(map_flags(uxk, uyk) & 8)
                && (terrain_g != 11 || has_tech(player, 0x13))
                && townbuf[home].owner == player) {
                if (city_distj <= 2 && (terrain_g == 1 || terrain_g == 2))
                    return 0x72;
                vl = count_f = sc_d = 0;
                for (k = 1; k <= 8; k++) {
                    if (map_flags(spiral_x[k] + uxk, spiral_y[k] + uyk) & 8) {
                        vl |= 1 << (k - 1);
                        count_f++;
                        if (map_flags(spiral_x[k] * 2 + uxk, spiral_y[k] * 2 + uyk) & 8)
                            sc_d = 1;
                    }
                }
                vl &= vl >> 4;
                if ((count_f < 4 && (vl || (city_distj == 1 && terrain_g <= 2)))
                    || (count_f == 1 && sc_d && terrains[terrain_g].cost == 1))
                    return 0x72;
            }
        }
        if ((unsigned char) armydata[player][unit].goto_x != 0xff)
            return 0;
        sc_d = rand_scaled(20) + 1;
        nx = armydata[player][unit].x / 4 + spiral_x[sc_d];
        ny = armydata[player][unit].y / 4 + spiral_y[sc_d];
        if (!(fog_map[nx * 13 + ny] & 3)) {
            armydata[player][unit].goto_x = spiral_x[sc_d] * 4 + armydata[player][unit].x;
            armydata[player][unit].goto_y = spiral_y[sc_d] * 4 + armydata[player][unit].y;
            if (map_in_bounds((unsigned char) armydata[player][unit].goto_x, (unsigned char) armydata[player][unit].goto_y)
                && terrains[terrain_at((unsigned char) armydata[player][unit].goto_x, (unsigned char) armydata[player][unit].goto_y)].shields != 0 && region_key((unsigned char) armydata[player][unit].goto_x, (unsigned char) armydata[player][unit].goto_y) == region)
                return 0;
            else
                armydata[player][unit].goto_x = -1;
        }
    }
    if ((unsigned char) armydata[player][unit].goto_x != 0xff) {
        if (rolel != 3 || !adj_grp(player, uxk, uyk))
            return 0;
    }
    if (rolel == 2 && terrain_g != 10) {
        if ((near_dist > 1 && city_distj <= 3 && townbuf[home].owner == player) || (blocked_i && near_dist))
            return 0x66;
    }
    if (rolel == 0 && ai_continent_towns[player][region] > ai_zone_places[region] / 8) {
        if (city_distj == 0 && townbuf[home].size < 10)
            return 0x62;
        if (townbuf[home].owner == player && townbuf[home].size < 10) {
            armydata[player][unit].goto_x = townbuf[home].x;
            armydata[player][unit].goto_y = townbuf[home].y;
            return 0;
        }
    }
    best_scorem = -999;
    result_i = 0;
    freem = 0;
    if (unit_blocked(player, uxk, uyk))
        armydata[player][unit].f8 = -1;
    else
        freem = 1;
    if (utypej == 1 && rstatem == 1)
        rolel = 1;
    for (k = 1; k <= 8; k++) {
        nx = wrap_x(spiral_x[k] + uxk);
        ny = spiral_y[k] + uyk;
        if (!map_in_bounds(nx, ny))
            continue;
        cowner = map_city(nx, ny);
        tern = terrain_at(nx, ny);
        if (tern == 10 && utypes[armydata[player][unit].type].domain == 0)
            continue;
        stk_d = map_stack(nx, ny);
        if (stk_d != -1 && armydata[fight_enemy][stk_d].type == 26) {
            own_b = stk_d;
            do {
                own_b = armydata[fight_enemy][own_b].next;
            } while (own_b != -1 && own_b != stk_d && armydata[fight_enemy][own_b].type == 26);
            if (own_b != -1)
                stk_d = own_b;
        }
        if (blocked_i && utypes[armydata[player][unit].type].domain == 0 && stk_d == -1 && if_open(player, nx, ny) && armydata[player][unit].type < 26 && terrain_g != 10)
            continue;
        if (terrain_g == 10 && utypes[armydata[player][unit].type].domain == 0 && stk_d != -1 && cowner != player)
            continue;
        if (utypes[armydata[player][unit].type].domain == 2 && tern != 10 && (stk_d == -1 || cowner == player || utypej == 22 || terrain_g != 10))
            continue;
        if (stk_d != -1 && cowner == player && stack_len(player, stk_d, 2) >= 2)
            continue;
        if (rolel == 0) {
            vl = 0;
            if (terrain_g == 10 && unit_blocked(player, nx, ny))
                continue;
        } else {
            if (armydata[player][unit].visibility == 0 && terrain_g != 10) {
                if (rolel == 1)
                    vl = rand_scaled(3) - terrains[tern].cost * 2;
                else
                    vl = rand_scaled(3) - terrains[tern].defense;
            } else {
                vl = rand_scaled(5);
                if (stk_d != -1 && cowner == player) {
                    if (rolel == 1)
                        vl += stack_len(player, stk_d, 1) * 4 / (stack_len(player, stk_d, 2) + 1);
                    if (rolel == 0)
                        vl += (terrains[tern].defense + stack_len(player, stk_d, 1)) * 2;
                    if (rolel == 2)
                        vl += stack_len(player, stk_d, 3) * 2 / (stack_len(player, stk_d, 1) + 1);
                } else
                    vl += terrains[tern].defense * 4;
            }
            if (utypes[armydata[player][unit].type].domain == 1)
                vl = rand_scaled(3);
        }
        if (armydata[player][unit].f8 != -1) {
            adiff_h = abs(armydata[player][unit].f8 - k);
            if (adiff_h > 4)
                adiff_h = 8 - adiff_h;
            vl -= adiff_h * adiff_h * 2;
        }
        enemy_g = 0;
        if (stk_d != -1) {
            if (cowner != player) {
                enemy_g = 1;
                if (rolel == 0)
                    continue;
                if (!(treaties[player][cowner] & 2)) {
                    stk_d = stack_other(cowner, stk_d);
                    if (utypes[armydata[cowner][stk_d].type].domain == 1 && armydata[player][unit].type != 14 && !(map_flags(nx, ny) & 1))
                        continue;
                    odds_f = battle(player, unit, cowner, stk_d, 0L);
                    odds_f = (stack_len(cowner, stk_d, 0) + 1) * odds_f;
                    odds_f = odds_f / utypes[armydata[player][unit].type].cost;
                    if (map_flags(nx, ny) & 1)
                        odds_f = 3 * odds_f;
                    if (rolel == 1 && rstatem == 1)
                        odds_f = 3 * odds_f;
                    if (rolel == 1 && utypes[armydata[player][unit].type].attack * 2 < stack_len(player, unit, 3))
                        odds_f <<= 1;
                    if ((player ? 6 : 12) <= odds_f) {
                        vl += odds_f * 4;
                    } else {
                        vl -= 999;
                        if (rolel == 1 && rstatem == 1 && utypes[armydata[player][unit].type].domain == 0 && (armydata[cowner][stk_d].status & 0x28)
                            && terrains[tern].defense >= 4 && stack_len(cowner, stk_d, 2) < 2 && !rand_scaled(4)
                            && !(map_flags(nx, ny) & 1))
                            buy_u(cowner, stk_d, player);
                    }
                } else {
                    if (rolel == 1 && rstatem == 1 && utypes[armydata[player][unit].type].domain == 0 && (armydata[cowner][stk_d].status & 8)
                        && (treaties[player][civnow] & 3) == 1 && (treaties[cowner][civnow] & 3) != 1 && ai_continent_towns[civnow][region] != 0 && stack_len(cowner, stk_d, 2) < 2 && (!rand_scaled(8) || townbuf[home].owner == civnow)
                        && !(map_flags(nx, ny) & 1))
                        buy_u(cowner, stk_d, player);
                    continue;
                }
            } else
                vl -= utypes[armydata[player][unit].type].defense;
        } else {
            if ((map_flags(nx, ny) & 1) && cowner != player)
                vl = 999;
            if (has_hut(tern, nx, ny))
                vl += 20;
        }
        if (freem) {
            nx = spiral_x[k] * 4 + uxk;
            ny = spiral_y[k] * 4 + uyk;
            if (fog_map[nx / 4 * 13 + ny / 4] == 0 && terrain_at(nx, ny) != 10 && map_in_bounds(nx, ny))
                vl += 8;
            for (j2 = 1; j2 <= 8; j2++) {
                mxm = wrap_x(spiral_x[j2] + nx);
                mym = spiral_y[j2] + ny;
                if (!map_in_bounds(mxm, mym))
                    continue;
                if (!(civ_vis_grid[mxm * 50 + mym] & pbitn)) {
                    if (terrain_at(mxm, mym) != 10 || utypes[armydata[player][unit].type].domain == 2)
                        vl += 2;
                }
                if (map_stack(mxm, mym) != -1)
                    vl -= 2;
                if (rolel == 0)
                    vl += terrains[terrain_at(mxm, mym)].food;
            }
        }
        if (enemy_g && armydata[player][unit].moves < 3)
            vl = vl * stack_len(player, unit, 1) / clamp(stack_len(player, unit, 3), 1, 99);
        if (vl > best_scorem) {
            best_scorem = vl;
            result_i = k;
            attack_f = enemy_g;
        }
    }
    if (attack_f && armydata[player][unit].moves < 3)
        result_i = 0;
    armydata[player][unit].f8 = result_i;
    return result_i;
}

void ai_break(void)
{
}

void unit_cleanup(int player)
{
    int i;

    for (i = 0; i < 32; i++) {
        objectives_type[player][i] = -1;
        aim_count[player][i] = 0;
    }
    for (i = 0; i < 16; i++) {
        if (spot_classes[player][i] != -1 && spot_classes[player][i] <= 2)
            add_target(player, camp_xpos[player][i], civ_lot_y[player][i], spot_classes[player][i], ai_camp_site_weights[player][i]);
    }
}

void add_target(int player, int x, int y, int kind, int count)
{
    int i;
    int d;

    for (i = 0; i < 32; i++) {
        if (civ_targets_x[player][i] == x && civ_targets_y[player][i] == y && objectives_type[player][i] == kind && aim_count[player][i] <= count)
            return;
    }
    if (player == civ_id && player != civnow && (kind == 3 || kind == 4)) {
        for (i = 0; i < 128; i++) {
            if (armydata[player][i].type != -1 && armydata[player][i].moves != 0 && utypes[armydata[player][i].type].role == kind && region_key(armydata[player][i].x, armydata[player][i].y) == region_key(x, y)) {
                d = map_dist(armydata[player][i].x, x, armydata[player][i].y, y);
                if (armydata[player][i].moves >= d * 2) {
                    armydata[player][i].goto_x = x;
                    armydata[player][i].goto_y = y;
                    armydata[player][i].status |= 0x10;
                    armydata[player][i].status &= 0xF0;
                    break;
                }
            }
        }
    }
    for (i = 0; i < 32; i++) {
        if (objectives_type[player][i] == -1 || aim_count[player][i] < count) {
            target_shift(player, i);
            civ_targets_x[player][i] = x;
            civ_targets_y[player][i] = y;
            objectives_type[player][i] = kind;
            aim_count[player][i] = count;
            return;
        }
    }
}

void target_shift(int player, int from)
{
    int i;

    for (i = 30; i >= from; i--) {
        if (aim_count[player][i]) {
            civ_targets_x[player][i + 1] = civ_targets_x[player][i];
            civ_targets_y[player][i + 1] = civ_targets_y[player][i];
            objectives_type[player][i + 1] = objectives_type[player][i];
            aim_count[player][i + 1] = aim_count[player][i];
        }
    }
}

void site_clear(int player)
{
    int i;

    for (i = 0; i < 16; i++) {
        spot_classes[player][i] = -1;
        ai_camp_site_weights[player][i] = 0;
    }
}

void site_add(int player, int x, int y, int kind, int count)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (camp_xpos[player][i] == x && civ_lot_y[player][i] == y && spot_classes[player][i] == kind && ai_camp_site_weights[player][i] <= count)
            return;
    }
    for (i = 0; i < 16; i++) {
        if (spot_classes[player][i] == -1 || ai_camp_site_weights[player][i] < count) {
            site_shift(player, i);
            camp_xpos[player][i] = x;
            civ_lot_y[player][i] = y;
            spot_classes[player][i] = kind;
            ai_camp_site_weights[player][i] = count;
            return;
        }
    }
}

void site_shift(int player, int from)
{
    int i;

    for (i = 14; i >= from; i--) {
        if (ai_camp_site_weights[player][i]) {
            camp_xpos[player][i + 1] = camp_xpos[player][i];
            civ_lot_y[player][i + 1] = civ_lot_y[player][i];
            spot_classes[player][i + 1] = spot_classes[player][i];
            ai_camp_site_weights[player][i + 1] = ai_camp_site_weights[player][i];
        }
    }
}

int site_nearest(int player, int kind, int x, int y, int region)
{
    int slot;
    int val;
    int dist;
    int min;
    int i;

    slot = -1;
    min = best_camp_far = 999;
    i = 0;
    do {
        if (spot_classes[player][i] == kind && (region == -1 || region_key(camp_xpos[player][i], civ_lot_y[player][i]) == region)) {
            dist = map_dist(x, camp_xpos[player][i], y, civ_lot_y[player][i]);
            val = dist / (ai_camp_site_weights[player][i] + 1);
            if (val < min) {
                slot = i;
                min = val;
                best_camp_far = dist;
            }
        }
    } while (++i < 16);
    return slot;
}

void clear_sites_near(int player, int kind, int x, int y, int range)
{
    int i;
    int d;

    for (i = 0; i < 16; i++) {
        if (spot_classes[player][i] == kind) {
            d = map_dist(x, camp_xpos[player][i], y, civ_lot_y[player][i]);
            if (d <= range)
                spot_classes[player][i] = -1;
        }
    }
}

void retarget(int owner, int region)
{                               /* re-choose production for a civ's cities on a continent */
    int i;

    for (i = 0; i < 128; i++) {
        if (townbuf[i].owner == owner && townbuf[i].status != -1 && (region == -1 || region_key(townbuf[i].x, townbuf[i].y) == region))
            city_retarget(i);
    }
}

void city_retarget(int c)
{
    int owner;

    owner = townbuf[c].owner;
    if (townbuf[c].production >= 0)
        civ_units_ordered[owner][townbuf[c].production]--;
    if ((townbuf[c].production = find_slot(owner, c)) >= 0)
        civ_units_ordered[owner][townbuf[c].production]++;
}

int ocean_edge(int player, int x, int y)
{
    int available;
    int neighbor_x;
    int land_count;
    int cell_y;
    int i;
    int neighbour_index;
    int map_col;
    int col;
    int dir;

    land_count = dir = -1;
    for (i = 0; i <= 8; i++) {
        available = 0;
        map_col = x + spiral_x[i];
        col = y + spiral_y[i];
        if (terrain_at(map_col, col) != 10 || (i != 0 && map_owner(map_col, col) != -1))
            continue;
        for (neighbour_index = 1; neighbour_index <= 8; neighbour_index++) {
            neighbor_x = map_col + spiral_x[neighbour_index];
            cell_y = col + spiral_y[neighbour_index];
            if (terrain_at(neighbor_x, cell_y) != 10 && map_owner(neighbor_x, cell_y) == -1) {
                available++;
                if ((treaties[player][civnow] & 2) && pixel_at(2, neighbor_x + 80, cell_y) == civnow)
                    available--;
            }
        }
        if (available > land_count) {
            land_count = available;
            dir = i;
        }
    }
    return dir;
}

int unit_ai_move(int player, int unit)
{
    /* 0x54-byte frame: 41 declared words (23 never touched) + MSC temp at BP-54.
     * The padN names only reproduce slot order (tools/msc_locals.py rule);
     * the real identifiers are unknown. */
    int pad47;
    int pad38;
    int pad29;
    int pad49;
    int type;
    int pad4;
    int home;
    int pad53;
    int best_value3;
    int tx1;
    int st6;
    int rec3;
    int pad44;
    int pad35;
    int pad26;
    int pad17;
    int pad8;
    int value;
    int uy;
    int city5;
    int pad9;
    int ux1;
    int ty1;
    int best;
    int pad118;
    int pad109;
    int pad91;
    int pad82;
    int pad73;
    int pad64;
    int fy;
    int enemy1;
    int pad55;
    int fx1;
    int pad46;
    int pad37;
    int pad28;
    int n1;
    int pad19;
    int dist29;
    int land;

    if (armydata[player][unit].y < 2 || armydata[player][unit].y >= 0x30) {
        drop_u_stack(player, unit);
        return;
    }
    enemy1 = if_open(player, armydata[player][unit].x, armydata[player][unit].y);
    best = 0;
    ux1 = armydata[player][unit].x;
    uy = armydata[player][unit].y;
    type = armydata[player][unit].type;
    if (utypes[armydata[player][unit].type].role == 5) {
        if (armydata[player][unit].next == -1) {
            drop_u_stack(player, unit);
            return 0x20;
        }
        home = near_city(ux1, uy);
        for (n1 = 1; n1 <= 8; n1++) {
            tx1 = wrap_x(spiral_x[n1] + ux1);
            ty1 = spiral_y[n1] + uy;
            if (town_gap <= 8 && terrain_at(tx1, ty1) != 10 && !(map_flags(tx1, ty1) & 1) && map_owner(tx1, ty1) == -1 && ty1 > 1 && ty1 < 0x30) {
                if ((civ_vis_grid[tx1 * 50 + ty1] & (1 << civnow)) && townbuf[home].owner == civnow && region_key(tx1, ty1) == region_key(townbuf[home].x, townbuf[home].y)) {
                    armydata[player][unit].visibility |= 1 << civnow;
                    map_center(civnow, tx1, ty1);
                    message_mode = 3;
                    strcpy(msgtext, "Barbarian raiding party\nlands near ");
                    home = near_city(tx1, ty1);
                    append_place_name(home);
                    strcat(msgtext, "!\nCitizens are alarmed.\n");
                    show_msg(msgtext, 100, 32);
                }
                armydata[player][unit].visibility |= civ_vis_grid[tx1 * 50 + ty1];
                armydata[player][unit].moves = 0;
                return 0x75;
            }
            if (map_owner(tx1, ty1) > 0)
                return n1;
        }
        if (armydata[player][unit].goto_x == -1) {
            best_value3 = 0;
            home = 0;
            do {
                if (townbuf[home].status != -1) {
                    dist29 = (unrest_chance(home) + 50) / map_dist(ux1, townbuf[home].x, uy, townbuf[home].y);
                    if (dist29 > best_value3) {
                        best_value3 = dist29;
                        city5 = home;
                    }
                }
            } while (++home < 128);
            if (best_value3 == 0) {
                drop_u_stack(player, unit);
                return 0x20;
            }
            armydata[player][unit].goto_x = townbuf[city5].x;
            armydata[player][unit].goto_y = townbuf[city5].y;
        }
        return 0;
    }
    city5 = near_city(ux1, uy);
    if (town_gap == 0 && (armydata[player][unit].next == -1 || utypes[armydata[player][unit].type].role == 2))
        return 0x66;
    fx1 = fy = -2;
    if (((unit + turn_tick) & 3) == 0) {
        if (region_key(ux1, uy) != region_key(townbuf[city5].x, townbuf[city5].y)) {
            drop_u_stack(player, unit);
            return 0x20;
        }
        if (town_gap > 8) {
            drop_u_stack(player, unit);
            return 0x20;
        }
    }
    if (city5 != -1) {
        if (region_key(ux1, uy) == region_key(townbuf[city5].x, townbuf[city5].y) && (human_set & (1 << townbuf[city5].owner))) {
            fx1 = map_random(townbuf[city5].x - ux1);
            fy = map_random(townbuf[city5].y - uy);
        }
    }
    if (player == townbuf[city5].owner || (city_count[townbuf[city5].owner] < 2 && money[townbuf[city5].owner] < 100))
        fx1 = fy = -2;
    if (fx1 != -2) {
        armydata[player][unit].goto_x = townbuf[city5].x;
        armydata[player][unit].goto_y = townbuf[city5].y;
        armydata[player][unit].f8 = unit_goal(player, unit);
    }
    armydata[player][unit].goto_x = -1;
    if (armydata[player][unit].type == 0x1a) {
        land = terrain_at(armydata[player][unit].x, armydata[player][unit].y);
        if (armydata[player][unit].next != -1 && land != 10 && armydata[player][armydata[player][unit].next].type != 0x1a)
            return 0x20;
        dist29 = town_gap;
        if (land != 10) {
            rec3 = nearest_record(player, unit, armydata[player][unit].x, armydata[player][unit].y);
            if (town_gap <= 3 && armydata[player][rec3].type != 0x1a) {
                armydata[player][unit].goto_x = armydata[player][rec3].x;
                armydata[player][unit].goto_y = armydata[player][rec3].y;
                return 0;
            }
            if ((turn_tick & 7) + 4 <= dist29) {
                drop_u_stack(player, unit);
                return 0x20;
            }
        } else {
            if (dist29 >= 3)
                goto next;
            drop_u_stack(player, unit);
            return 0x20;
        }
    }
next:
    best = 0;
    best_value3 = -999;
    for (n1 = 1; n1 < 9; n1++) {
        tx1 = wrap_x(armydata[player][unit].x + spiral_x[n1]);
        ty1 = armydata[player][unit].y + spiral_y[n1];
        land = terrain_at(tx1, ty1);
        if ((land == 10) != (utypes[armydata[player][unit].type].domain == 2))
            continue;
        if (!map_in_bounds(tx1, ty1))
            continue;
        st6 = map_stack(tx1, ty1);
        if (armydata[player][unit].type == 0x1a) {
            value = 0;
            if (st6 != -1)
                value += (player == fight_enemy) ? 99 : -99;
            near_city(tx1, ty1);
            value += terrains[land].cost + rand_scaled(4) + town_gap * 4;
        } else {
            if ((map_flags(tx1, ty1) & 1) && map_city(tx1, ty1) != player) {
                if (terrain_at(ux1, uy) != 10)
                    return n1;
                continue;
            }
            value = rand_scaled(6) + ((map_flags(tx1, ty1) & 6) ? 6 : 0) + (map_city(tx1, ty1) == player ? 0 : 4);
            if (st6 != -1) {
                if (player == fight_enemy) {
                    if (terrain_at(ux1, uy) != 10)
                        continue;
                    value -= 20;
                } else {
                    if (terrain_at(ux1, uy) == 10)
                        continue;
                    if (armydata[fight_enemy][st6].type != 0x1a)
                        value += 99;
                }
            }
            if (enemy1 && utypes[armydata[player][unit].type].domain == 0 && st6 == -1 && if_open(player, tx1, ty1))
                continue;
            if (armydata[player][unit].f8 == n1)
                value += 6;
            if (spiral_x[n1] == fx1)
                value += 2;
            if (spiral_y[n1] == fy)
                value += 2;
        }
        if (value > best_value3) {
            best_value3 = value;
            best = n1;
        }
    }
    if ((map_flags(ux1, uy) & 1) && armydata[player][unit].next == -1 && best_value3 < 99)
        return 0x20;
    armydata[player][unit].f8 = best;
    return best;
}

void city_captured(int city)
{
    int p;
    int cx;
    int cy;
    int n;

    cx = townbuf[city].x;
    cy = townbuf[city].y;
    for (p = 1; p < 8; p++) {
        if (p == civnow || (treaties[p][civnow] & 2))
            continue;
        for (n = 0; n < 128; n++) {
            if (armydata[p][n].type != -1 && utypes[armydata[p][n].type].capacity != 0 && armydata[p][n].next != -1 && terrain_at(armydata[p][n].x, armydata[p][n].y) == 10 && utypes[armydata[p][n].type].moves * 3 > map_dist(armydata[p][n].x, cx, armydata[p][n].y, cy)) {
                armydata[p][n].goto_x = cx;
                armydata[p][n].goto_y = cy;
            }
        }
    }
}
