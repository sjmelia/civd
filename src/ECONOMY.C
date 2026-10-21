/* City processing, production selection and technology handling. */
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CAPTURE.H"
#include "CORE.H"
#include "ECONOMY.H"
#include "FILEIO.H"
#include "NEWGAME.H"
#include "NEWS.H"
#include "NOTES.H"
#include "TURN.H"

#include "CIV.H"
#include "ADVISORS.H"
#include "AUDIENCE.H"
#include "CITYMGT.H"
#include "DIALOG.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "PEDIA.H"
#include "RESIDENT.H"
#include "SPACESHP.H"
#include "UNITAI.H"
#include "UNITS.H"

struct civ_type {
    char strings[48];
    int f0, f1, f2, f3, f4;
};

#define CITY_SCI(i) (townbuf[i].shield_box)

extern int debug;

extern int treaties[8][8];

extern struct civ_type civt[];
extern struct unit armydata[][128];
extern struct city townbuf[];
extern struct unit_type utypes[];
extern int far advance_owners[];
extern char far adv_source[];
extern char far map_visible_to[];

extern int city_process();

extern int map_stack(int, int);
extern int rand_scaled(int);

extern void map_mark(int, int);

extern void each_stack();
extern int wrap_x(int);

extern void cycle_off(void);
extern void sound(int);
extern int disk_insert(int, long);

extern void load_palette(char *, char *);
extern void install_palette(int, char *);

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void civ_economy(int civ)
{
    int i, j, rate, flags, n;

    i = flags = troop_over = margin_democracy = 0;
    for (; i < 128; i++) {
        if (townbuf[i].status != (char) 0xff && townbuf[i].owner == civ) {
            for (;;) {
                if (!city_process(i, 0L) && townbuf[i].size > 5)
                    flags |= 1;
                if (townbuf[i].status & 1)
                    flags |= 2;
                if (show_city_display) {
                    city_process(i, 1L);
                    sel_player();
                    if (show_city_display == 2) {
                        show_city_display = 0;
                        continue;
                    }
                    show_city_display = 0;
                }
                break;
            }
        }
    }
    if (civ != civnow && (rule_options & 4)) {
        rate = 10 - (sci[civ] + civ_toll[civ]);
        if ((flags & 2) && sci[civ] + civ_toll[civ] > 6)
            rate++;
        if (!(turn_tick & 3) && flags == 0 && civ_toll[civ] + sci[civ] < 8)
            rate--;
        rate = clamp(rate, 0, 4);
        sci[civ] = civt[civ_tribe_of[civ]].f2 + (10 - rate) / 2;
        if (money[civ] > turn_tick + 100)
            sci[civ]++;
        if (has_tech(civ, 63))
            sci[civ] = 0;
        civ_toll[civ] = -(sci[civ] + rate - 10);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void vanish(int idx, int x, int y)
{
    int dist, u, nx, ny, j;

    post_event(1, 4, -1, (unsigned char) townbuf[idx].name_id, x, y);
    if (townbuf[idx].production >= 0)
        civ_units_ordered[townbuf[idx].owner][townbuf[idx].production]--;
    city_count[townbuf[idx].owner]--;
    townbuf[idx].status = 0xff;
    flag_tile(0, x, y);
    if (map_visible_to[50 * x + y] & (1 << civnow))
        map_mark(x, y);
    for (dist = 0; dist < 128; dist++)
        for (j = 0; j < 3; j++)
            if (idx == *(unsigned char *) &townbuf[dist].routes[j])
                townbuf[dist].routes[j] = 0xff;
    for (j = 1; j <= 21; j++)
        if (marvel_site[j] == idx)
            marvel_site[j] = 0x80;
    for (j = 0; j < 128; j++) {
        if (armydata[townbuf[idx].owner][j].type != -1 && armydata[townbuf[idx].owner][j].home_city == idx)
            drop_u_stack(townbuf[idx].owner, j);
    }
    u = map_stack(x, y);
    if (u != -1)
        each_stack(townbuf[idx].owner, u, kill_stack_cb);
    for (j = 0; j < 45; j++) {
        nx = wrap_x(spiral_x[j] + x);
        ny = spiral_y[j] + y;
        dist = near_city(nx, ny);
        if (j <= 20 || town_gap > 3) {
            map_claim(nx + 80, ny, help_probe(2, nx + 80, ny + 50), 2);
        }
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void backdrop_save(void)
{
    mouse_off();
    blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void backdrop_load(void)
{
    mouse_off();
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void kill(int a, int b)
{
    int unused;

    backdrop_save();
    map_view_changed = 0;
    city_process(b, 1L);
    if (map_view_changed)
        sel_player();
    else
        backdrop_load();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int find_slot(int civ, int city)
{
    /* Smaller ranks win; candidate IDs are positive units or negative improvements.
       Letter suffixes preserve MSC local allocation buckets. */
    int food_surplus_l, danger, hoover_power_c, warned, what, have[28], best_u2, ncand, cand[73], best_all, f, best_i, units_ordered_c[29], i;
    char choice_text_l[16];
    int best_unit_rank_c, j, save_n, rank, military_build_c, is, best_any_index_d, area_role_f, save, land, military_area_m, menu_page_o, len;

    choice_text_l[0] = '*';
    warned = menu_page_o = 0;
    city_process(city, -1);
    for (;;) {
        ncand = 1;
        cand[0] = townbuf[city].production;
        strcpy(msgtext, "What shall we build in ");
        append_place_name(city);
        strcat(msgtext, "?\n ");
        if (menu_page_o != 2) {
            for (i = 0; i < 28; i++) {
                if (i == 25 && (marvel_site[17] == -1 || !has_tech(civ, 27)))
                    continue;
                if (has_tech(civ, utypes[i].prereq) && !has_tech(civ, utypes[i].obsolete_by) && ((townbuf[city].status & 2) || utypes[i].domain != 2)) {
                    strcat(msgtext, utypes[i].name);
                    turns_text(city, utypes[i].cost);
                    strcat(msgtext, ", ADM:");
                    strcat(msgtext, itoa(utypes[i].attack, nstr, 10));
                    strcat(msgtext, "/");
                    strcat(msgtext, itoa(utypes[i].defense, nstr, 10));
                    strcat(msgtext, "/");
                    strcat(msgtext, itoa(utypes[i].moves, nstr, 10));
                    strcat(msgtext, ")\n ");
                    cand[ncand++] = i;
                }
            }
        }
        save_n = ncand;
        len = strlen(msgtext);
        /* Hoover supplies power only to cities on its own continent. */
        hoover_power_c = 0;
        if (civ_wonder(civ, 15)) {
            if (region_key(townbuf[city].x, townbuf[city].y) == region_key(townbuf[marvel_site[15]].x, townbuf[marvel_site[15]].y))
                hoover_power_c = 1;
        }
        if (rule_options & 4) {
            for (i = 1; i <= 24; i++) {
                long bld;

                if (!has_tech(civ, improvements[i].prereq) || ((bld = townbuf[city].buildings), (bld & (1L << (i - 1)))) || (i == 7 && (bld & 1)) || (i == 15 && (townbuf[city].buildings & 0x8000L)) || (i == 10 && !(townbuf[city].buildings & 0x10L) && has_tech(civ, improvements[5].prereq)) || (i == 12 && !(townbuf[city].buildings & 0x20L) && has_tech(civ, improvements[6].prereq)) || (i == 20 && !(townbuf[city].status & 8)) || (i == 19 && !(townbuf[city].buildings & 0x4000L)) || ((i == 19 || i == 20 || i == 21) && (hoover_power_c || (townbuf[city].buildings & 0x1c0000L))))
                    continue;
                if (i > 21 && (marvel_site[19] == -1 || (game_flags & (1 << civ))))
                    continue;
                {
                    strcat(msgtext, improvements[i].name);
                    turns_text(city, improvements[i].cost);
                    strcat(msgtext, ")\n ");
                    cand[ncand++] = -i;
                }
            }
        }
        for (i = 25; i <= 45; i++) {
            if (has_tech(civ, improvements[i].prereq) && (marvel_site - 24)[i] == -1) {
                if ((improvement_words - 24)[i] != 127 && (advance_owners[(improvement_words - 24)[i]] & 7))
                    strcat(msgtext, "*");
                strcat(msgtext, improvements[i].name);
                turns_text(city, improvements[i].cost);
                strcat(msgtext, ")\n ");
                cand[ncand++] = -i;
            }
        }
        if (civ == civnow && !(townbuf[city].status & 0x10) && ncand > 30) {
            if (menu_page_o == 0) {
                menu_page_o = 1;
                continue;
            }
            strcpy(msgtext + len, "more...\n");
            cand[save_n] = 99;
        }
        if (menu_page_o == 2) {
            strcat(msgtext, "more...\n");
            cand[ncand++] = 98;
        }
        land = region_key(townbuf[city].x, townbuf[city].y);
        area_role_f = civ_area_plan[civ][land];
        military_area_m = (area_role_f == 1 || area_role_f == 2);
        if (gangmode && area_role_f == 5)
            military_area_m = 1;
        food_surplus_l = town_res_sums[0] - (civ_gov[civ] > 1 ? 2 : 1) * colonist_homed - townbuf[city].size * 2;
        /* Seed at one so repeated unit orders incur an increasing rank penalty. */
        for (i = 0; i < 28; i++) {
            units_ordered_c[i] = 1;
            have[i] = 0;
        }
        save = troops_total[civ];
        troops_total[civ] = 0;
        for (f = 0; f < 128; f++) {
            if (townbuf[f].status != (char) 0xff && townbuf[f].owner == civ && region_key(townbuf[f].x, townbuf[f].y) == land) {
                if (townbuf[f].production >= 0 && f != city)
                    units_ordered_c[townbuf[f].production]++;
                for (j = 0; j < 2; j++) {
                    char c;

                    c = townbuf[f].garrison[j];
                    if (c != -1) {
                        troops_total[civ]++;
                        have[c & 0x3f]++;
                    }
                }
            }
        }
        for (i = 0; i < 128; i++) {
            if (armydata[civ][i].type != -1) {
                troops_total[civ]++;
                if (region_key(armydata[civ][i].x, armydata[civ][i].y) == land)
                    have[armydata[civ][i].type]++;
            }
        }
        if (troops_total[civ] == 0 && civ != civnow && civ_gov[civ] <= 1)
            return 1;
        if (civ == 0) {
            if (!(advance_owners[56] & 7))
                return 3;
            return 7;
        }
        danger = 0;
        i = map_stack(townbuf[city].x, townbuf[city].y);
        if (i == -1)
            danger = 2;
        else if (townbuf[city].size / 4 + military_area_m > stack_strength(civ, i, 2))
            danger = 1;
        /* Keep best overall, building/special-unit and ordinary-unit choices separately. */
        best_all = is = best_unit_rank_c = 999;
        best_i = best_u2 = -1;
        for (i = 1; i < ncand; i++) {
            what = cand[i];
            if (what < 0 || what >= 26) {
                rank = 99;
                military_build_c = 0;
                switch (-what) {
                case 4:
                    if (townbuf[city].status & 1)
                        rank = -4;
                    break;
                case 7:
                    rank = 12 - city_leak * 2;
                    break;
                case 11:
                    if (townbuf[city].status & 1)
                        rank = -2;
                    break;
                case 14:
                    if (townbuf[city].status & 1)
                        rank = -1;
                    break;
                case 2:
                    rank = 5;
                    military_build_c = 1;
                    break;
                case 3:
                    rank = town_res_sums[1] >= 3 ? 4 : 8;
                    break;
                case 15:
                case 20:
                    rank = 13 - town_res_sums[1] / 2;
                    break;
                case 19:
                    rank = 12 - town_res_sums[1] / 5;
                    break;
                case 16:
                case 18:
                case 21:
                    rank = 12 - town_res_sums[1] / 8;
                    break;
                case 5:
                    rank = 10 - town_res_sums[2] / 2;
                    if (townbuf[city].status & 1)
                        rank = -3;
                    break;
                case 6:
                case 10:
                    rank = 10 - town_res_sums[2] / 3;
                    break;
                case 12:
                    rank = 10 - town_res_sums[2] / 4;
                    break;
                case 8:
                    rank = 10 - townbuf[city].size / 2;
                    if (townbuf[city].buildings & 1)
                        rank -= 4;
                    military_build_c = 1;
                    break;
                case 9:
                    if (10 - food_surplus_l / 2 <= townbuf[city].size)
                        rank = 14 - townbuf[city].size - food_surplus_l / 2;
                    break;
                case 22:
                    if (!(game_flags & ((1 << civ) + (1 << civnow))))
                        rank = (game_flags & (1 << (civnow + 8))) ? 2 : 4;
                    break;
                case -27:
                    if (townbuf[city].routes[2] == -1) {
                        rank = 10 - town_res_sums[2] / 2;
                        if (townbuf[city].routes[0] != -1)
                            rank += 2;
                        if (townbuf[city].routes[1] != -1)
                            rank += 2;
                        if (rank < 3)
                            rank = 3;
                    }
                    if (civ == civnow && (townbuf[city].status & 0x10))
                        rank = 99;
                    break;
                case -26:
                    if (civnow <= 0)
                        break;
                    if (treaties[civ][civnow] & 2)
                        break;
                    if (civ_units_ordered[civ][26] > 0)
                        break;
                    f = rand_scaled(5);
                    if (!(near_human & (1 << civ)))
                        break;
                    if ((advances_known[civnow][f] | advances_known[civ][f]) == advances_known[civ][f])
                        break;
                    rank = clamp(tech_count[civ] - tech_count[civnow] + 10, 5, 10);
                    break;
                }
                rank *= (military_area_m != military_build_c) ? 20 : 10;
                rank = 3 * rank / (civt[civ_tribe_of[civ]].f2 + 3);
                if (townbuf[city].buildings & 2)
                    rank += rank / 4;
            } else {
                if (what == 23)
                    continue;
                if (what == 0) {
                    if ((civ_gov[civ] > 1 ? 2 : 1) >= food_surplus_l) {
                        if (forces_present[civ][0] != 0)
                            continue;
                        if (units_ordered_c[0] > 1)
                            continue;
                    }
                }
                if (what == 1 && civ_gov[civ] > 1)
                    continue;
                if (what == 25) {
                    if (forces_present[civ][25] >= 4 || civ_units_ordered[civ][25] >= 2)
                        continue;
                }
                if (what == 14) {
                    if (forces_present[civ][what] + civ_units_ordered[civ][what] >= forces_present[civnow][15] / 2 || (treaties[civ][civnow] & 2))
                        continue;
                }
                if (utypes[what].domain == 0) {
                    rank = units_ordered_c[what] * 2 + have[what];
                } else {
                    if (defenders_island[civ][land] < 8)
                        continue;
                    rank = defenders_island[civ][land] * (civ_units_ordered[civ][what] * 2 + forces_present[civ][what]) / (troops_total[civ] + 1);
                }
                if (danger != 0) {
                    if (utypes[what].role != 2)
                        continue;
                    rank = 2 / danger;
                }
                if (utypes[what].role != 2 && utypes[what].role != 0) {
                    rank <<= 1;
                    if (area_role_f != utypes[what].role)
                        rank <<= 1;
                }
                if (utypes[what].role == 5) {
                    if (area_role_f != 5)
                        continue;
                    rank = 3 * rank / 2;
                }
                rank = (what == 0 ? ((townbuf[city].buildings & 4) ? 4 : 6) - civt[civ_tribe_of[civ]].f2 : 16) * (utypes[what].cost + 2) * rank / ((utypes[what].attack + utypes[what].defense) * (utypes[what].moves + 1));
                if (utypes[what].role == 3)
                    rank *= military_area_m ? 4 : 2;
                if (utypes[what].domain == 1)
                    rank <<= 1;
                if (troops_total[civ] > 120)
                    rank <<= 2;
                if ((military_area_m == 0 && town_res_sums[1] <= home_troops_maintained) || in_at_war == 0)
                    rank <<= 1;
            }
            if (rank < best_all) {
                best_any_index_d = i;
                best_all = rank;
            }
            if (what < 0 || what >= 26) {
                if (rank >= is)
                    continue;
                is = rank;
                best_i = i;
            } else {
                if (rank >= best_unit_rank_c)
                    continue;
                best_unit_rank_c = rank;
                best_u2 = i;
            }
        }
        troops_total[civ] = save;
        if (!(human_set & (1 << civ))) {
            if (townbuf[city].production == -1)
                return -1;
            return cand[best_any_index_d];
        }
        if (townbuf[city].status & 0x10) {
            if (best_i != -1 && is <= 500)
                return cand[best_i];
            return 99;
        }
        if ((debug & 1) && !warned) {
            scrbuf->font = 2;
            rank = cand[best_u2];
            if (rank >= 0)
                strcpy(choice_text_l + 1, utypes[rank].name);
            else
                strcpy(choice_text_l + 1, improvements[-rank].name);
            strcpy(msgtext, "Military Advisor:\n");
            news_name("produce.txt", choice_text_l);
            strcpy(message_slot, choice_text_l + 1);
            help_open();
            news_wrap(50);
            if (best_u2 != -1) {
                abort_flag = 1;
                show_menu(msgtext, 8, 160);
            }
            rank = cand[best_i];
            if (rank >= 0)
                strcpy(choice_text_l + 1, utypes[rank].name);
            else
                strcpy(choice_text_l + 1, improvements[-rank].name);
            strcpy(msgtext, "Domestic Advisor:\n");
            news_name("produce.txt", choice_text_l);
            strcpy(message_slot, choice_text_l + 1);
            help_open();
            news_wrap(50);
            if (best_i != -1) {
                abort_flag = 1;
                show_menu(msgtext, 164, 160);
            }
            warned = 1;
            scrbuf->font = 1;
            continue;
        }
        if (ncand > 20 || menu_page_o != 0)
            scrbuf->font = 2;
        help_flag = 1;
        rank = cand[show_menu(msgtext, 80, 8) + 1];
        if (rank == 99) {
            menu_page_o = 2;
            continue;
        }
        if (rank == 98) {
            menu_page_o = 1;
            continue;
        }
        scrbuf->font = 1;
        if (edit_mode) {
            mouse_off();
            menu_begin();
            no_pictures = 1;
            if (rank >= 0)
                pedia_show(rank, 2);
            else
                pedia_show(-rank, 1);
            menu_end();
            mouse_on();
            map_view_changed = 1;
            continue;
        }
        return rank;
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void turns_text(int a, int b)
{
    int n;

    n = clamp((10 * b - CITY_SCI(a) - 1) / clamp(town_res_sums[1] - city_keep, 1, 99) + 1, 1, 999);
    strcat(msgtext, " (");
    strcat(msgtext, itoa(n, nstr, 10));
    strcat(msgtext, n != 1 ? " turns" : " turn");
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int reduce_attitude(int civ, int src)
{
    int i;
    int idx;
    int gfx;
    int rating;
    int pad;
    int bs;
    int menu[17];
    int jj;
    int forced;
    int top;
    int nopt;
    char name[16];

    if (civ == 0)
        return -1;
    forced = 0;
    bs = top = -1;
    for (i = 0; i < 72; i++) {
        if (!has_tech(civ, i) && has_tech(civ, advances[i].prereq1) && has_tech(civ, advances[i].prereq2)) {
            rating = rand_scaled(advance_value(civ, i) << 2);
            if (rating > bs && (i != selected_index || civ != civnow)) {
                bs = rating;
                top = i;
            }
        }
    }
retry:
    if (civ != civnow || !turn_tick) {
        if (turn_tick && !cur_tier && !has_tech(civnow, top) && rand_scaled(2))
            return -1;
        if (bs != -1) {
            give_advance(civ, top, src);
            retarget(civ, -1);
        }
        return top;
    }
    if (selected_index != -1) {
        idx = pedia_advance_index(selected_index);
        gfx = 0;
        if (selected_index != 71 && found_floppy > 3 && disk_insert(1, 0L) && (debug & 8))
            gfx = 1;
        if (gfx) {
            mouse_off();
            strcpy(name, "discovr1.pic");
            nopt = 0;
            if (has_tech(civnow, 54)) {
                name[7] = '2';
                nopt = 1;
            }
            load_screen(1, name);
            rect_fill(scrbuf, 0, 0, 320, 200, 0);
            cycle_off();
            load_back(name);
            blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
        }
        strcpy(msgtext, civ_adjectives[civ]);
        strcat(msgtext, has_tech(civnow, 20) ? " scientists" : " wise men");
        strcat(msgtext, "\ndiscover the secret\nof ");
        strcat(msgtext, advances[selected_index].name);
        if (selected_index == 71) {
            strcat(msgtext, " ");
            strcat(msgtext, itoa(++advances_future, nstr, 10));
        }
        strcat(msgtext, "!\n");
        sound(civt[civ_tribe_of[civ]].f3);
        if (gfx) {
            if (!nopt)
                scrbuf->font = 6;
            abort_flag = 1;
            show_menu(msgtext, 96, nopt ? 25 : 1);
            scrbuf->font = 1;
            strcpy(msgtext, "iconpg1.pic");
            msgtext[6] += idx / 9;
            disk_insert(3, 1L);
            load_screen(1, msgtext);
            disk_insert(0, 1L);
            blit_centred(idx % 3 * 111 + 1, idx % 9 / 3 * 69 + 1, 110, 68, 174, nopt ? 87 : 96);
            if (vga_modes) {
                strcpy(name + 8, ".pal");
                load_palette(name, pic_colours_data);
                load_palette("iconpg1.pal", icon_colours_block);
                memcpy(icon_colours_block + 6, pic_colours_data + 6, 255);
                install_palette(30, icon_colours_block);
            }
            ad_wait();
            mouse_on();
            cycle_on();
        } else
            news_box(-1);
        give_advance(civ, selected_index, src);
        sound(1);
        selected_index = -1;
    }
    if (tech_count[civ] <= 1)
        notice_a("*FIRSTCIV");
    mouse_off();
    menu_begin();
    mouse_on();
    if ((debug & 1) && top != -1 && !cur_tier) {
        strcpy(msgtext, "Science Advisor:\nI recommend we\ndevelop ");
        strcat(msgtext, advances[top].name);
        strcat(msgtext, ".\n");
        rating = strlen(msgtext);
        nopt = 0;
        strcat(msgtext, "So we can build:\n-");
        for (jj = 0; jj < 28; jj++) {
            if (utypes[jj].prereq == top) {
                strcat(msgtext, utypes[jj].name);
                strcat(msgtext, "\n-");
                nopt++;
            }
        }
        for (jj = 0; jj < 24; jj++) {
            if (improvements[jj].prereq == top) {
                strcat(msgtext, improvements[jj].name);
                strcat(msgtext, "\n-");
                nopt++;
            }
        }
        if (nopt == 0)
            msgtext[rating] = 0;
        scrbuf->font = 2;
        abort_flag = 1;
        show_menu(msgtext, 8, -(nopt * 8 - 168));
        scrbuf->font = 1;
    }
    strcpy(msgtext, "Which discovery should our\n");
    strcat(msgtext, has_tech(civnow, 20) ? "scientists" : "wise men");
    strcat(msgtext, " be pursuing, sire?\nPick one...\n ");
    nopt = 0;
    for (i = 0; i < 72; i++) {
        if (!has_tech(civ, i) && has_tech(civ, advances[i].prereq1) && has_tech(civ, advances[i].prereq2)) {
            if (!((forced || (nopt > 1 && (i - tech_count[civ]) % 3 == 0 && cur_tier)) && (!forced || menu[nopt] != i))) {
                strcat(msgtext, advances[i].name);
                strcat(msgtext, "\n ");
                menu[nopt++] = i;
            }
        }
    }
    message_mode = 6;
    if (nopt) {
        help_flag = 1;
        i = show_menu(msgtext, 80, 64);
        mouse_off();
        menu_end();
        mouse_on();
        if (i == -1)
            i = 0;
        i = menu[i];
        if (edit_mode) {
            pedia_show(i, 0);
            sel_player();
            forced = 1;
            goto retry;
        }
        selected_index = i;
    } else {
        strcat(msgtext, " Future Technology ");
        strcat(msgtext, itoa(advances_future + 1, nstr, 10));
        strcat(msgtext, "\n");
        show_msg(msgtext, 100, 80);
        selected_index = 71;
    }
    return i;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void give_advance(int civ, int adv, int src)
{
    int i, j, k, m;

    if (has_tech(civ, adv))
        return;
    if (adv != 71) {
        advances_known[civ][adv / 16] |= 1 << (adv % 16);
        adv_source[72 * civ + adv] = src;
    }
    if (turn_tick)
        ++tech_count[civ];
    else
        return;
    if (civ == civnow || (treaties[civnow][civ] & 0x40)) {
        msgtext[0] = 0;
        strcat(msgtext, civ_names[civ]);
        strcat(msgtext, (civ == src || src == 0) ? " develop\n" : " acquire\n");
        strcat(msgtext, advances[adv].name);
        if (civ != src && src != 0) {
            strcat(msgtext, " from\nthe ");
            if (src == -2)
                strcat(msgtext, "Great Library");
            else
                strcat(msgtext, civ_names[src]);
        }
        strcat(msgtext, "!\n ");
        if (civ != civnow) {
            for (i = 0; i < 24; i++) {
                if (improvements[i].prereq == adv) {
                    strcat(msgtext, improvements[i].name);
                    strcat(msgtext, "\n ");
                }
            }
            for (i = 0; i < 28; i++) {
                if (utypes[i].prereq == adv) {
                    strcat(msgtext, utypes[i].name);
                    strcat(msgtext, "\n ");
                }
            }
        }
        if (civ != civnow || src == -2) {
            if (mouse_mode == -1) {
                box_info_flags = 1;
                if (civ != civnow)
                    message_mode = 0;
                else
                    message_mode = 6;
                show_msg(msgtext, 100, 80);
            }
        }
    } else {
        if (!(advance_owners[adv] & 7)) {
            if (has_tech(civ, 8)) {
                if (!has_tech(civnow, adv)) {
                    if (rand_scaled(3) == 0) {
                        if (audience_open(civ, -1, 0) != -1) {
                            audience_active = mouse_mode = 0;
                            msgtext[0] = 0;
                            strcpy(name_buf, civ_names[civ]);
                            strcpy(message_slot, advances[adv].name);
                            named_msg("*GAPE");
                            style = 2;
                            setting_value = 12;
                            leader_menu(msgtext, 20, 139);
                            audience_close();
                        }
                    }
                }
            }
        }
    }
    if (civ == civnow) {
        if (adv != 71)
            pedia_show(adv, 0);
        if (adv == 20) {
            cabinet();
            sel_player();
        }
        if (mouse_mode == -1) {
            sel_player();
            if (adv == 5)
                notice_a("*MONARCHY");
        }
    } else if (cur_tier == 0 && adv == 22
               /* Preserve the original precedence bug: (!word) & 0x40. */
               && (!treaties[civnow][civ] & 0x40)
               && mouse_mode == -1) {
        treaties[civnow][civ] |= 0x40;
        strcpy(msgtext, civ_names[civ]);
        strcat(msgtext, " agree to\nan exchange of ambassadors.\n");
        message_mode = 1;
        show_msg(msgtext, 80, 100);
        spy_rpt();
    }
    if (adv == 34 || adv == 37) {
        for (k = 0; k < 128; k++) {
            if (townbuf[k].status != -1 && townbuf[k].owner == civ)
                townbuf[k].buildings &= ~2L;
        }
        if (civ == civnow && mouse_mode == -1) {
            strcpy(msgtext, "Development of ");
            strcat(msgtext, advances[adv].name);
            strcat(msgtext, "\nmakes existing Barracks\nobsolete.\n");
            message_mode = 3;
            show_msg(msgtext, 80, 100);
        }
    }
    if (!(advance_owners[adv] & 7)) {
        advance_owners[adv] |= civ;
        post_event(5, 2, civ, adv);
        for (k = 1; k <= 21; k++) {
            if (improvement_words[k] == adv && townbuf[marvel_site[k]].owner == civnow) {
                strcpy(msgtext, advances[adv].name);
                strcat(msgtext, " cancels\nthe effect of\n");
                strcat(msgtext, (improvements + 24)[k].name);
                strcat(msgtext, ".\n");
                news_box(-1);
            }
        }
    }
    m = marvel_site[5];
    if (m == -1)
        return;
    m = townbuf[m].owner;
    if (has_tech(m, adv))
        return;
    if (!civ_wonder(m, 5))
        return;
    j = 0;
    for (k = 1; k < 8; k++) {
        if (has_tech(k, adv))
            j++;
    }
    if (j < 2)
        return;
    give_advance(m, adv, -2);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int has_tech(int civ, int adv)
{
    int w;

    if (adv == -1)
        return 1;
    if (adv >= 71)
        return 0;
    if (civ == 0)
        return 0;
    w = adv / 16;
    if (advances_known[civ][w] & (1 << (adv % 16)))
        return 1;
    return 0;
}

int advance_value(int civ, int adv)
{
    return (signed char) advance_attrs[adv].a + civt[civ_tribe_of[civ]].f2 * advance_attrs[adv].b;
}
