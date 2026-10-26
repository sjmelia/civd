/* Overlay 5: game setup, tribe selection, starting sites and cabinet. */
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "CAPTURE.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "ECONOMY.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "MAP.H"
#include "NAMES.H"
#include "NEWS.H"
#include "RESIDENT.H"
#include "TURN.H"
#include "UNITAI.H"
#include "UNITS.H"
#include "MESSAGE.H"
#include "NEWGAME.H"

extern int debug;
extern char sound_driver[];
extern struct city townbuf[];
extern int far advance_owners[];
extern char far map_visible_to[][50];
extern signed char far adv_source[][72];
extern int far unit_losses[][28];
extern struct unit armydata[8][128];

extern int treaties[8][8];




extern char far spaceships[][15][12];

signed char start_pos[16][2] = {
    0, 0, 36, 19, 45, 22, 38, 15, 41, 24, 12, 18, 39, 18, 57, 24,
    0, 0, 44, 12, 42, 42, 33, 16, 5, 23, 66, 19, 31, 14, 49, 19
};

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void new_game(void)
{
    int num, civ, x, i, y, j, flag, ret, len;

    mouse_off();
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    mouse_on();
    scrbuf->font = 7;
    strcpy(msgtext, "Difficulty Level...\n Chieftain (easiest)\n Warlord\n Prince\n King\n Emperor (toughest)\n");
    cur_tier = show_menu(msgtext, 160, 35);
    if (has_mouse && used_flag) {
        poll_mouse();
        if (ptr_x < 135)
            cur_tier = clamp((ptr_y - 12) / 35, 0, 4);
    }
    if (cur_tier == -1)
        cur_tier = 0;
    mouse_off();
    rect_fill(scrbuf, 0, 0, 150, 200, 0);
    blit(back_page, (cur_tier & 1) ? 80 : 21, cur_tier * 35 + 6, 53, 47, scrbuf, 20, 100);
    blit(back_page, 150, 0, 160, 200, scrbuf, 150, 0);
    mouse_on();
    strcpy(msgtext, "Level of Competition...\n ");
    for (i = 7; i > 2; i--) {
        strcat(msgtext, itoa(i, nstr, 10));
        strcat(msgtext, " Civilizations\n ");
    }
    do
        civ_total = show_menu(msgtext, 160, 35);
    while (civ_total == -1);
    civ_total = clamp(6 - civ_total, 2, 6);
    players = 255 >> (6 - civ_total);
    mouse_off();
    for (i = civ_total; i >= 0; i--)
        blit(back_page, (cur_tier & 1) ? 80 : 21, cur_tier * 35 + 6, 53, 47, scrbuf, i * 2 + 20, i * 3 + 100);
    year = -4000;
    rule_options = 15;
    for (i = 0; i < 128; i++)
        townbuf[i].status = townbuf[i].name_id = -1;
    for (i = 0; i < 256; i++)
        city_site_x[i] = -1;
    for (i = 0; i < 72; i++)
        advance_owners[i] = 0;
    for (i = 0; i <= 21; i++)
        marvel_site[i] = -1;
    for (i = 0; i < 8; i++) {
        civ_tribe_of[i] = -1;
        reset_spaceship(i);
    }
    for (x = 0; x < 80; x++)
        for (y = 0; y < 50; y++)
            map_visible_to[x][y] = 0;
    debug = 0xfa;
    if (*sound_driver == 'N')
        debug &= ~0x10;
    strcpy(msgtext, "Pick your tribe...\n ");
    flag = strlen(msgtext);
    for (i = 1; i < civ_total + 2; i++) {
        strcat(msgtext, civt[i].adjective);
        strcat(msgtext, "\n ");
    }
    for (i = 1; i < civ_total + 2; i++) {
        strcat(msgtext, civt[i + 8].adjective);
        strcat(msgtext, "\n ");
    }
    blit(back_page, 150, 0, 160, 200, scrbuf, 150, 0);
    mouse_on();
    civnow = show_menu(msgtext, 160, 35);
    if (civnow <= civ_total)
        civnow++;
    else
        civnow = civnow - civ_total + 8;
    if (cur_tier == 0)
        debug |= 1;
    selected_index = -1;
    next_event_turn += rand_scaled(50);
    for (i = 0; i < 12; i++) {
        floors_palace[i] = -1;
        section_grades[i] = 0;
    }
    for (i = 3; i <= 5; i++)
        floors_palace[i] = floors_palace[i + 6] = 0;
    if (civnow != 0) {
        flag = civnow > 7;
        civnow &= 7;
    } else {
        flag = -1;
        civnow = rand_scaled(civ_total) + 1;
    }
    *civ_adjectives[civnow] = 0;
    if (flag == -1) {
        rename_tribe();
        civ_tribe_of[civnow] = civnow;
        strcpy(leader_names[civnow], civt[civnow].leader);
    } else {
        civ_tribe_of[civnow] = flag ? civnow + 8 : civnow;
        strcpy(leader_names[civnow], civt[civ_tribe_of[civnow]].leader);
        strcpy(civ_adjectives[civnow], civt[civ_tribe_of[civnow]].adjective);
        strcpy(msgtext, civt[civ_tribe_of[civnow]].plural);
        if (*msgtext == 0) {
            strcpy(msgtext, civ_adjectives[civnow]);
            strcat(msgtext, "s");
        }
        strcpy(civ_names[civnow], msgtext);
    }
    draw_text(civ_names[civnow], 46, 92, 15);
    for (civ = 0; civ < 8; civ++) {
        init_civ(civ);
        for (j = 0; j < 8; j++)
            killed[i][j] = 0;
    }
    ret = rate_starts();
    human_set = 1 << civnow;
    sci[civnow] = civ_toll[civnow] = 5;
    if (cur_tier == 0)
        money[civnow] = 50;
    civ_flags_bits = 0;
    for (i = 0; i < 8; i++)
        if (civ_tribe_of[i] >= 8)
            civ_flags_bits |= 1 << i;
    mouse_off();
    rect_fill(scrbuf, 150, 0, 170, 200, 0);
    rename_leader();
    mouse_on();
    scrbuf->font = 1;
    strcpy(message_slot, leader_names[civnow]);
    strcpy(name_buf, civ_names[civnow]);
    *msgtext = 0;
    named_msg("*INIT");
    flag = strlen(msgtext);
    num = 0;
    for (i = 0; i < 72; i++) {
        if (has_tech(civnow, i)) {
            strcat(msgtext, advances[i].name);
            strcat(msgtext, ", ");
            if (++num == 2)
                strcat(msgtext, "\n");
        }
    }
    strcat(msgtext, "and Roads.\n");
    sound(civt[civ_tribe_of[civnow]].anthem, 3);
    mouse_off();
    rect_fill(scrbuf, 0, 0, 320, 200, 15);
    menu_clear();
    blit(back_page, (cur_tier & 1) ? 80 : 21, cur_tier * 35 + 6, 53, 47, scrbuf, 134, 20);
    news_effect(99, 88, 81, 0);
    mouse_on();
    ad_wait();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int init_civ(int civ)
{
    int p, dist, u, x, help, good, i, y, k, trial, counts[16];

    if (civ == civnow && turn_tick)
        return;
    money[civ] = civ_research_tally[civ] = tech_count[civ] = history_rating[civ] = space_ship_habitation[civ] = 0;
    civ_gov[civ] = 1;
    civ_toll[civ] = 4;
    sci[civ] = 4;
    for (i = 0; i < 16; i++) {
        civ_area_plan[civ][i] = 0;
        counts[i] = 0;
        for (trial = 1; trial < 8; trial++)
            counts[i] += ai_continent_towns[trial][i];
    }
    city_count[civ] = 0;
    troops_total[civ] = 0;
    civ_held_cells[civ] = 1;
    civ_settlers_n[civ] = 1;
    last_met_turn[civ] = -1;
    for (i = 0; i < 72; i++)
        adv_source[civ][i] = -1;
    for (i = 0; i < 128; i++) {
        armydata[civ][i].type = -1;
        armydata[civ][i].status = 0;
    }
    civ_ratings[civ] = cleared[civ] = 0;
    if (civ == civnow)
        rect_fill(tile_page, 240, 0, 80, 50, 0);
    site_clear(civ);
    for (i = 0; i < 8; i++)
        treaties[i][civ] = treaties[civ][i] = 0;
    for (i = 0; i < 5; i++)
        advances_known[civ][i] = 0;
    if (turn_tick) {
        for (i = 0; i < 72; i++)
            if (has_tech(civnow, i) && (advance_owners[i] & 7)
                && (advance_owners[i] & 7) != civnow && !rand_scaled(2))
                give_advance(civ, i, civ);
    }
    if (civ != 0 && (players & (1 << civ))) {
        if (civ != civnow) {
            if (!turn_tick) {
                do {
                    civ_tribe_of[civ] = rand_scaled(16);
                    good = 1;
                    if ((civ_tribe_of[civ] & 7) != civ)
                        good = 0;
                    for (i = 0; i < 8; i++)
                        if (i != civ && civ_tribe_of[i] == civ_tribe_of[civ])
                            good = 0;
                } while (!good);
            } else
                civ_tribe_of[civ] ^= 8;
        }
        trial = 0;
        do {
            x = rand_scaled(64) + 8;
            y = rand_scaled(34) + 8;
            u = near_city(x, y);
            dist = u == -1 ? 30 : map_dist(x, townbuf[u].x, y, townbuf[u].y);
            if (!turn_tick)
                for (p = 1; p < civ; p++) {
                    k = map_dist(x, armydata[p][0].x, y, armydata[p][0].y);
                    if (k < dist)
                        dist = k;
                }
            help = help_probe(2, x + 80, y);
            if (++trial > 2000 && (civ != civnow || turn_tick))
                break;
        } while (terrain_at(x, y) == 10 || help < 12 - trial / 32 || dist < 10 - trial / 64 || ai_zone_places[region_key(x, y)] < 32 - turn_tick / 16 - trial / 64 || year > 0 && counts[region_key(x, y)] != 0 || has_hut(terrain_at(x, y), x, y));
        if (!turn_tick && beginner_help) {
            x = start_pos[civ_tribe_of[civ]][0];
            y = start_pos[civ_tribe_of[civ]][1];
            trial = 0;
        }
        if (turn_tick) {
            if (civ_tribe_of[civ] >= 8 && (civ_flags_bits & (1 << civ)))
                trial = 2000;
            if (civ_tribe_of[civ] < 8 && !(civ_flags_bits & (1 << civ)))
                trial = 2000;
        }
        if (civ != civnow) {
            sci[civ] = civt[civ_tribe_of[civ]].civilized + 3;
            civ_toll[civ] = 9 - sci[civ];
        }
        if ((350 - cur_tier * 50 < turn_tick || trial >= 2000) && civ != civnow) {
            civ_tribe_of[civ] ^= 8;
            players &= ~(1 << civ);
            if (((1 << civnow) | 1) == players)
                complete_flag = leave = 1;
            return 0;
        }
        strcpy(leader_names[civ], civt[civ_tribe_of[civ]].leader);
        if (civ != civnow || *civ_adjectives[civ] == 0) {
            strcpy(civ_adjectives[civ], civt[civ_tribe_of[civ]].adjective);
            strcpy(msgtext, civt[civ_tribe_of[civ]].plural);
            if (*msgtext == 0) {
                strcpy(msgtext, civ_adjectives[civ]);
                strcat(msgtext, "s");
            }
            strcpy(civ_names[civ], msgtext);
        }
        map_refresh(civ, x, y);
        create_unit(civ, 0, x, y);
        x_cap[civ] = x;
        for (i = 0; i < 128; i++)
            if (armydata[0][i].type != -1 && map_dist(x, armydata[0][i].x, y, armydata[0][i].y) <= 8)
                drop_u_stack(0, i);
        tech_count[civ] = 1;
    }
    for (i = 0; i < 28; i++)
        civ_units_ordered[civ][i] = forces_present[civ][i] = unit_losses[civ][i] = 0;
    return 1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void kill_civ(int civ, int by)
{                               /* remove a destroyed civilization */
    int bits, x, i, y;

    if (civ == 0)
        return;
    if (civ == civnow)
        return;
    for (i = 0; i < 128; i++) {
        if (townbuf[i].status != -1 && townbuf[i].owner == civ)
            return;
    }
    strcpy(msgtext, civ_adjectives[civ]);
    strcat(msgtext, "\ncivilization\ndestroyed\nby ");
    strcat(msgtext, civ_names[by]);
    strcat(msgtext, "!\n");
    message_mode = (treaties[civnow][civ] & 0x40) ? 5 : 2;
    if (by == civnow)
        message_mode = 3;
    show_msg(msgtext, 100, 80);
    spaceship_lost(civ);
    post_event(13, 2, civ, by);
    for (i = 0; i < 128; i++)
        if (armydata[civ][i].type != -1)
            drop_u_stack(civ, i);
    bits = ~(1 << civ);
    for (x = 0; x < 80; x++)
        for (y = 0; y < 50; y++)
            map_visible_to[x][y] &= bits;
    init_civ(civ);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int rate_starts(void)
{
    int p, grass, val, dist, d, isolated, pts[8], river, i, best_score, y, oceans;
    int j, per_region[16], ret, where[8], ter;

    for (i = 0; i < 16; i++)
        per_region[i] = 0;
    i = 0;
    for (p = 1; p < 8; p++)
        if (armydata[p][i].type != -1) {
            where[p] = region_key(armydata[p][i].x, armydata[p][i].y);
            per_region[where[p]]++;
        }
    best_score = isolated = 0;
    for (p = 1; p < 8; p++) {
        if (armydata[p][i].type == -1)
            continue;
        y = p * 28 - 24;
        val = 0;
        dist = 999;
        for (j = 1; j < 8; j++) {
            if (j != p && armydata[j][0].type != -1 && region_key(armydata[p][i].x, armydata[p][i].y) == region_key(armydata[j][0].x, armydata[j][0].y)) {
                d = map_dist(armydata[p][i].x, armydata[j][0].x, armydata[p][i].y, armydata[j][0].y);
                if (d < dist)
                    dist = d;
            }
        }
        if (dist >= 10 && dist != 999)
            isolated |= 1 << p;
        strcpy(msgtext, "Nearest civilization: ");
        if (dist < 10)
            strcat(msgtext, "close");
        else if (dist < 20) {
            strcat(msgtext, "moderate");
            val++;
        } else if (dist != 999) {
            strcat(msgtext, "far away");
            val += 2;
        } else {
            strcat(msgtext, "isolated");
            val += 4;
        }
        river = grass = oceans = 0;
        for (j = 0; j <= 8; j++) {
            ter = terrain_at(armydata[p][i].x + spiral_x[j], armydata[p][i].y + spiral_y[j]);
            if (ter == 10)
                oceans++;
            if (ter == 2)
                grass++;
            if (ter == 11)
                river++;
        }
        strcpy(msgtext, oceans ? "Coastal " : "Inland ");
        if (river > 1) {
            strcat(msgtext, "river valley on a ");
            val += 2;
        } else if (grass >= 3) {
            strcat(msgtext, "grasslands on a ");
            val++;
        } else
            strcat(msgtext, "plains on a ");
        switch (map_region(armydata[p][i].x, armydata[p][i].y) / 50) {
        case 0:
            strcat(msgtext, "small island.");
            break;
        case 1:
            strcat(msgtext, "large island.");
            break;
        case 2:
            strcat(msgtext, "small continent.");
            val++;
            break;
        case 3:
            strcat(msgtext, "medium continent.");
            val++;
            break;
        default:
            strcat(msgtext, "large continent.");
            val += 2;
            break;
        }
        if (val > best_score)
            best_score = val;
        pts[p] = val;
    }
    ret = -1;
    for (p = 1; p < 8; p++) {
        if (armydata[p][i].type != -1) {
            y = p * 28 - 8;
            strcpy(msgtext, "Skills: ");
            if (best_score - pts[p] > 3) {
                pts[p] += 3;
                create_unit(p, 0, armydata[p][0].x, armydata[p][0].y);
                strcat(msgtext, "Settlers+");
            }
            for (j = 0; j < best_score - pts[p]; j++) {
                if (j)
                    strcat(msgtext, "+");
                strcat(msgtext, advances[reduce_attitude(p, 0)].name);
            }
            if (pts[p] == best_score) {
                strcat(msgtext, "NONE");
                if (ret == -1 || (isolated & (1 << p)))
                    ret = p;
            }
        }
    }
    return ret;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void setup_art(void)
{
    int p, i, j;

    disk_insert(7, 1L);
    sprite_arena_init();
    load_screen(1, "sp299.pic");
    blit(back_page, 160, 50, 160, 150, tile_page, 160, 50);
    for (i = 0; i < 3; i++)
        adviser_faces_glyphs[i] = cut_tile(1, i * 42 + 160, 142, 41, 58);
    if (vga_modes) {
        load_screen(1, "ter257.pic");
        for (i = 0; i < 10; i++)
            for (j = 0; j < 16; j++)
                if (i < 6 && !low_memory || j == 0)
                    world_landscape_sprite[i][j] = cut_tile(1, j * 16, i * 16, 16, 16);
                else
                    world_landscape_sprite[i][j] = world_landscape_sprite[i][j - 1];
        for (i = 0; i < 8; i++)
            for (j = 0; j < 4; j++)
                if (j < 2)
                    map_tile_edge[i][j] = cut_tile(1, (j & 1) * 8 + i * 16, 176, 8, 8);
                else
                    map_tile_edge[i][j] = cut_tile(1, i * 16 - (j & 1) * 8 + 8, 184, 8, 8);
        for (i = 0; i < 4; i++)
            mouth_deltas_sprites[i] = cut_tile(1, i * 16 + 128, 176, 16, 16);
    } else {
        LOAD_SPRITES("sprites.pic", 3);
        blit(spare_page, 0, 0, 320, 200, back_page, 0, 0);
        for (i = 0; i < 12; i++) {
            world_landscape_sprite[i][0] = cut_tile(3, i * 16, 0, 16, 16);
            world_landscape_sprite[i][1] = cut_tile(3, i * 16, 16, 16, 16);
        }
    }
    if (vga_modes) {
        load_screen(1, "sp257.pic");
        load_palette("sp256.pal", colours_area);
        for (i = 0; i < 48; i++)
            intro_kept_pal[i] = colours_area[i + 6];
    }
    for (i = 0; i < 8; i++) {
        map_paths_icon[i] = cut_tile(1, i * 16, 48, 16, 16);
        map_rail_shapes[i] = cut_tile(1, i * 16 + 128, 96, 16, 16);
    }
    village_icons = cut_tile(1, 48, 32, 16, 16);
    for (i = 1; i < 16; i++) {
        terrain_coast_sprites[i] = cut_tile(1, i * 16, 64, 16, 16);
        terrain_tracks_tiles[i] = cut_tile(1, i * 16, 80, 16, 16);
    }
    for (i = 0; i < 20; i++) {
        bar_set[0][i] = cut_tile(1, i * 7 + 160, 0, 7, 15);
        bar_set[1][i] = cut_tile(1, i * 7 + 160, 16, 7, 15);
        bar_set[2][i] = cut_tile(1, i * 7 + 160, 32, 7, 15);
    }
    for (i = 0; i < 4; i++)
        marks_pics[i] = cut_tile(1, i * 16 + 80, 128, 16, 16);
    for (i = 0; i < 7; i++)
        statuspics[i] = cut_tile(1, i * 16, 32, 16, 16);
    splashhand = mouse_cursor;
    for (i = 0; i < 8; i++)
        pointer_cursor_icon[i] = cut_tile(1, (i & 3) * 8 + 129, (i & 4) * 2 + 33, 7, 7);
    for (i = 0; i < 9; i++)
        city_glad_glyphs[i] = cut_tile(1, i * 8, 128, 8, 16);
    for (i = 0; i < 16; i++)
        towns_icons_size[i] = cut_tile(1, i * 16 + 1, 113, 15, 15);
    for (i = 0; i < 8; i++)
        town_tiles_2[i] = cut_tile(1, i * 16 + 1, 97, 15, 15);
    colour_shield_tiles = cut_tile(1, 153, 41, 7, 7);
    banner_tile = cut_tile(1, 176, 128, 20, 9);
    for (p = 0; p < 8; p++) {
        blit(back_page, 0, 160, 320, 32, back_page, 0, 128);
        if (civ_colours[p] == 15)
            recolour_rect(back_page, 0, 128, 320, 32, 15, 11);
        if (civ_colours[p] == 7)
            recolour_rect(back_page, 0, 128, 320, 32, 7, 3);
        recolour_rect(back_page, 0, 128, 320, 32, 10, civ_colours[p]);
        recolour_rect(back_page, 0, 128, 320, 32, 2, civ_shadows[p]);
        for (j = 0; j < 28; j++)
            colour_unit_glyph[p * 32 + j] = cut_tile(1, j % 20 * 16 + 1, j / 20 * 16 + 129, 15, 15);
    }
    sprite_arena_release();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void cabinet(void)
{                               /* load the advisor cabinet picture */
    int unused;
    char name[16];

    disk_insert(2, 1L);
    strcpy(name, "govt0a.pic");
    name[4] = civ_gov[civnow] / 2 + '0';
    if (civ_gov[civnow] == 3)
        name[4] = '3';
    else if (has_tech(civnow, 20))
        name[5] = 'm';
    load_screen(1, name);
    blit(back_page, 0, 0, 160, 60, tile_page, 160, 140);
    disk_insert(0, 1L);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void new_government(void)
{
    int i;

    if (civ_gov[civnow] == 0)
        return;
    backdrop_save();
    strcpy(msgtext, civ_adjectives[civnow]);
    strcat(msgtext, " government\nchanged to ");
    strcat(msgtext, government_names[civ_gov[civnow]]);
    strcat(msgtext, "!\n");
    news_box(-2);
    mouse_off();
    rect_fill(scrbuf, 0, 100, 320, 100, 15);
    scrbuf->font = 6;
    draw_text("New Cabinet:", 160, 102, 0);
    scrbuf->font = 2;
    for (i = 0; i < 4; i++) {
        blit(tile_page, i * 40 + 160, 140, 40, 60, scrbuf, i * 80 + 20, 118);
        strcpy(msgtext, advisor_title[i + 3]);
        msgtext[strlen(msgtext) - 1] = 0;
        draw_text(msgtext, i * 80 + 40, (i & 1) ? 186 : 180, 0);
    }
    scrbuf->font = 1;
    mouse_on();
    ad_wait();
    backdrop_load();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void reset_spaceship(int civ)
{
    int i, j;

    spaceship_arrive_dates[civ] = -1;
    for (i = 0; i < 15; i++)
        for (j = 0; j < 12; j++)
            spaceships[civ][i][j] = -1;
}
