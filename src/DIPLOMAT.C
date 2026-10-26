/* Overlay 22: Diplomat actions, bribery, nuclear animation and city revolts. */
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "ADVISORS.H"
#include "AUDIENCE.H"
#include "BATTLE.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CITYVIEW.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIPLOMAC.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "ECONOMY.H"
#include "FADE.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "TURN.H"
#include "UNITS.H"
#include "DIPLOMAT.H"
#include "MAP.H"
#include "PATH.H"

extern struct {
    char a, b;
} treaties[8][8];
extern struct city townbuf[];
extern struct unit armydata[8][128];
extern struct unit_type utypes[];
#define diplomat_improvements (improvements + 1)
extern int debug;
extern char civ_vis_grid[];





extern int kbhit(void);

#define tech unused_n
#define cost costz
#define dist unused_b
#define i unused_d
#define choice unused_e
#define target_civ unused_f
#define rand_tech unused_g
#define city_shields(c) (townbuf[c].shield_box)

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void dip_menu(int city_id, int player, int unit_id)
{
    int unused_m;
    int unused_n;
    int dummy2_5;
    int costz;
    int dummy3_6;
    int unused_b;
    int unused_c[10];
    int unused_d;
    int unused_e;
    int unused_f;
    int unused_g;

    target_civ = townbuf[city_id].owner;
    armydata[player][unit_id].goto_x = -1;

    strcpy(msgtext, civ_adjectives[player]);
    strcat(msgtext, " diplomat arrives\nin ");
    append_place_name(city_id);
    strcat(msgtext, "...\n Establish Embassy\n Investigate City\n Steal Technology\n Industrial Sabotage\n Incite a Revolt\n Meet with King\n");

    if (player == civnow) {
        if ((treaties[civnow][target_civ].a & 0x40) || target_civ == 0)
            dialog_dim = 1;
        if (townbuf[city_id].status & 0x20)
            dialog_dim |= 4;
        if (townbuf[city_id].buildings & HAS_PALACE)
            dialog_dim |= 0x10;
        if (target_civ == 0)
            dialog_dim |= 0x20;
        choice = show_msg(msgtext, 80, 80);
    } else {
        choice = 2;
        if (target_civ == civnow && (treaties[civnow][player].a & 2)) {
            drop_u_stack(player, unit_id);
            return;
        }
    }

    switch (choice) {
    case 1:
        kill(target_civ, city_id);
        drop_u_stack(player, unit_id);
        goto diplomat_done;

    case 0:
        treaties[civnow][target_civ].a |= 0x40;
        drop_u_stack(player, unit_id);
        civ_dossier(target_civ);
        return;

    case 2:
        if (player == civnow) {
            if (townbuf[city_id].status & 0x20)
                return;
            if (attack_ok(townbuf[city_id].owner) == -1)
                return;
            accord(civnow, townbuf[city_id].owner, 2);
        } else {
            if (townbuf[city_id].status & 0x20)
                goto sabotage;
        }
        msgtext[0] = 0;
        rand_tech = rand_scaled(72);
        for (i = 0; i < 72; i++) {
            tech = (rand_tech + i) % 72;
            if (player == 0)
                continue;
            if (!has_tech(target_civ, tech))
                continue;
            if (has_tech(player, tech))
                continue;
            give_advance(player, tech, target_civ);
            strcpy(msgtext, civ_names[player]);
            strcat(msgtext, " steal\n");
            strcat(msgtext, advances[tech].name);
            strcat(msgtext, ".\n");
            message_mode = 0;
            show_msg(msgtext, 80, 80);
            townbuf[city_id].status |= 0x20;
            break;
        }
        if (msgtext[0] != 0) {
            drop_u_stack(player, unit_id);
            return;
        }
        if (player != civnow) {
        } else {
            show_msg("No new technology found.\n", 80, 80);
            return;
        }

    case 3:
sabotage:
        rand_tech = 0;
        if (rand_scaled(2) != 0 || city_shields(city_id) == 0) {
            do {
                i = rand_scaled(24);
                rand_tech++;
            } while (rand_tech < 99 && (i + 1 == 1 || !(townbuf[city_id].buildings & (1 << i))));
            if (rand_tech < 99) {
                townbuf[city_id].buildings &= ~(1 << i);
                strcpy(msgtext, diplomat_improvements[i].name);
                strcat(msgtext, " destroyed\n");
            } else {
                rand_tech = 0;
            }
        }
        if (rand_tech == 0) {
            city_shields(city_id) = 0;
            if (townbuf[city_id].production >= 0)
                strcpy(msgtext, utypes[townbuf[city_id].production].name);
            else
                strcpy(msgtext, diplomat_improvements[-townbuf[city_id].production - 1].name);
            strcat(msgtext, " production sabotaged\n");
        }
        strcat(msgtext, "in ");
        append_place_name(city_id);
        strcat(msgtext, ".\n");
        message_mode = 0;
        show_msg(msgtext, 80, 80);
        drop_u_stack(player, unit_id);
        return;

    case 4:
        dist = nearest_unit_distance(target_civ, townbuf[city_id].x, townbuf[city_id].y);
        if (dist == 0)
            return;
        cost = (money[target_civ] + 1000) / (dist + 3) * townbuf[city_id].size;
        if (townbuf[city_id].status & 1)
            cost /= 2;
        strcpy(msgtext, "Dissidents in ");
        append_place_name(city_id);
        strcat(msgtext, "\nwill revolt for $");
        strcat(msgtext, itoa(cost, nstr, 10));
        strcat(msgtext, ".\n");
        if (cost <= money[player]) {
            strcat(msgtext, " Forget it.\n Incite revolt\n");
            if (cost * 2 <= money[player] && (treaties[player][target_civ].a & 2)) {
                strcat(msgtext, " Subvert city ($x2)\n");
            }
        }
        message_mode = 0;
        choice = show_msg(msgtext, 100, 80);
        if (choice < 1)
            return;
        if (choice == 1) {
            if (treaties[target_civ][player].a & 2) {
                accord(target_civ, player, 2);
                treaties[target_civ][player].a |= 8;
            }
        }
        money[player] -= cost * choice;
        city_revolt(city_id, player);
        map_note(townbuf[city_id].x, townbuf[city_id].y);
        drop_u_stack(player, unit_id);
        status_draw();
        return;

    case 5:
        if (target_civ != 0) {
            leader_audience(target_civ, armydata[player][unit_id].x, armydata[player][unit_id].y, 1L);
        }
        armydata[player][unit_id].moves = 0;
        return;
    }
diplomat_done:
    return;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

#undef tech
#undef cost
#undef dist
#undef i
#undef choice
#undef target_civ
#undef rand_tech

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void buy_u(int player, int unit_id, int bribing_player)
{
    int type;
    int cost;
    int unused_k;
    int distance;
    int new_u;
    int j;

    if (stack_len(player, unit_id, 2) > 1)
        return;

    distance = nearest_unit_distance(player, armydata[player][unit_id].x, armydata[player][unit_id].y);
    cost = (money[player] + 750) / (distance + 2) * utypes[armydata[player][unit_id].type].cost;
    if (armydata[player][unit_id].type != 0)
        cost /= 2;

    if (bribing_player != civnow) {
        if (money[bribing_player] / 2 < cost)
            return;
    }

    strcpy(msgtext, civ_adjectives[player]);
    strcat(msgtext, " ");
    strcat(msgtext, utypes[armydata[player][unit_id].type].name);
    strcat(msgtext, "\nwill desert for ");
    strcat(msgtext, itoa(cost, nstr, 10));
    strcat(msgtext, "$.\nTreasury: ");
    strcat(msgtext, itoa(money[bribing_player], nstr, 10));
    strcat(msgtext, "$\n");
    if (cost <= money[bribing_player])
        strcat(msgtext, " Forget it.\n Pay\n");

    if (bribing_player == civnow) {
        if (show_msg(msgtext, 100, 80) != 1)
            return;
    }

    if (player == civnow) {
        reveal(player, armydata[player][unit_id].x - 8, armydata[player][unit_id].y - 6);
        strcpy(msgtext, civ_adjectives[player]);
        strcat(msgtext, " ");
        strcat(msgtext, utypes[armydata[player][unit_id].type].name);
        strcat(msgtext, " unit\nbribed by ");
        strcat(msgtext, civ_names[bribing_player]);
        strcat(msgtext, "!\n");
        message_mode = 0;
        show_msg(msgtext, 80, 40);
    }

    type = armydata[player][unit_id].type;
    drop_u_stack(player, unit_id);
    forces_present[player][type]--;
    new_u = create_unit(bribing_player, type, armydata[player][unit_id].x, armydata[player][unit_id].y);
    if (new_u != -1) {
        forces_present[bribing_player][type]++;
        armydata[bribing_player][new_u].visibility |= (1 << player);
    }
    money[bribing_player] -= cost;

    if (player == civnow) {
        map_note(armydata[player][unit_id].x, armydata[player][unit_id].y);
        status_draw();
        flip(30);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void nuke_animation(int x, int y)
{
    int i;
    int tiles[28];

    if (!disk_insert(1, 0L))
        return;

    load_picture(1, "nuke1.pic");
    sprite_arena_init();
    for (i = 0; i < 28; i++) {
        tiles[i] = cut_tile(1, (i % 7) * 45 + 1, (i / 7) * 45 + 1, 44, 44);
    }
    sprite_arena_release();
    x = (wrap_x(x - map_x0) << 4) + 66;
    y = ((y - map_y0) << 4) - 6;
    x = clamp(x, 80, 276);
    y = clamp(y, 8, 156);
    sound(42);
    blit(scrbuf, x, y, 44, 44, back_page, x, y);
    for (i = 0; i < 35; i++) {
        sprite_draw_clipped(scrbuf, x, y, tiles[i < 28 ? i : i - 7]);
        flip(5);
        blit(back_page, x, y, 44, 44, scrbuf, x, y);
    }
    free_block(tiles[0]);
    disk_insert(0, 1L);
    load_player_palette();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void city_revolt(int city_id, int new_owner)
{
    int type, civ, c, d, min_d, best_score, new_unit, cx, score, cy, m, unused_k, best;

    civ = townbuf[city_id].owner;
    cx = townbuf[city_id].x;
    cy = townbuf[city_id].y;
    min_d = 999;
    unused_k = 0;
    best_score = 0;
    best = -1;

    if (new_owner == -1) {
        for (c = 0; c < 128; c++) {
            if (c == city_id || townbuf[c].status == -1 || townbuf[c].owner == civ)
                break;
            d = map_dist(townbuf[city_id].x, townbuf[c].x, townbuf[city_id].y, townbuf[c].y);
            city_process(c, -1);
            score = ((town_happy_ones_num - town_unhappy_counts) << 5) / d;
            if (score > best_score) {
                best_score = score;
                best = c;
            }
        }
        if (best_score != 0)
            new_owner = townbuf[best].owner;
    }

    if (new_owner == -1)
        return;

    civ_vis_grid[cx * 50 + cy] |= (1 << new_owner);
    for (m = 0; m < 20; m++)
        civ_vis_grid[(radius_dx[m] + cx) * 50 + radius_dy[m] + cy] |= (1 << new_owner);

    if (civ == civnow || new_owner == civnow || (treaties[civnow][civ].a & 0x40) || (treaties[civnow][new_owner].a & 0x40)) {
        if (civ_vis_grid[townbuf[city_id].x * 50 + townbuf[city_id].y] & (1 << civnow))
            reveal(civnow, townbuf[city_id].x - 8, townbuf[city_id].y - 5);
        strcpy(msgtext, civ_names[civ]);
        strcat(msgtext, new_owner == 0 ? " declare\nindependence in\n" : " rebel!\nCivil War in\n");
        append_place_name(city_id);
        if (new_owner != 0) {
            strcat(msgtext, ".\n");
            strcat(msgtext, civ_adjectives[new_owner]);
            strcat(msgtext, " influence\nsuspected.\n");
        } else {
            strcat(msgtext, "\n");
        }
        show_msg(msgtext, 100, 32);
    }

    for (m = 0; m < 128; m++) {
        d = map_dist(cx, armydata[civ][m].x, cy, armydata[civ][m].y);
        if (armydata[civ][m].type != -1 && d <= 1 && (d == 0 || !(map_flags(armydata[civ][m].x, armydata[civ][m].y) & 1))) {
            type = armydata[civ][m].type;
            drop_u_stack(civ, m);
            map_refresh(new_owner, armydata[civ][m].x, armydata[civ][m].y);
            occupy(armydata[civ][m].x, armydata[civ][m].y);
            new_unit = create_unit(new_owner, type, armydata[civ][m].x, armydata[civ][m].y);
            if (new_unit != -1) {
                armydata[new_owner][new_unit].visibility |= (1 << civ);
                armydata[new_owner][new_unit].home_city = city_id;
            }
        }
    }

    incited = 1;
    capunit(new_owner, city_id, 1L);
    incited = 0;
    map_refresh(new_owner, townbuf[city_id].x, townbuf[city_id].y);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void city_founded(int city_id)
{
    int tiles[4];
    int x;
    int j;
    int dummy;
    int ticks;

    if (!disk_insert(1, 0L))
        return;

    cycle_off();
    mouse_off();
    sprite_arena_init();
    load_screen(1, "settlers.pic");
    for (j = 0; j < 4; j++) {
        tiles[j] = cut_tile(1, 1, (j << 4) + 1, 47, 15);
    }
    sprite_arena_release();
    load_screen(1, "hill.pic");
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    load_back("hill.pic");
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    msgtext[0] = 0;
    append_place_name(city_id);
    strcat(msgtext, " founded: ");
    format_year();
    strcat(msgtext, ".");
    scrbuf->font = 6;
    draw_text(msgtext, 160, 4, 0);
    scrbuf->font = 1;
    blit(back_page, 0, 120, 320, 40, back_page, 0, 0);
    ticks = 0;
    repaint();
    for (x = 80; x < 120; x++) {
        timer_reset();
        blit(back_page, 0, 0, 320, 40, back_page, 0, 120);
        for (j = 0; j < 4; j++) {
            sprite_draw(back_page, x + 13 * j, ((10 * j) & 0xf) + 120, tiles[(j + x) & 3]);
        }
        blit(back_page, 0, 120, 320, 40, scrbuf, 0, 120);
        ticks++;
        while (timer_read() < 10);
        poll_mouse();
        if (kbhit() || mse_held != 0)
            break;
    }
    free_block(tiles[0], "set");
    disk_insert(0, 1L);
    if (vga_modes)
        set_palette(10, 0, 0, 0);
    msgtext[0] = 0;
    if (!kbhit()) {
        city_scene(city_id, -3);
    } else {
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
        load_player_palette();
    }
    mouse_on();
    repaint();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */
