/* Overlay 14: advisor and city-status reports. */
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "CONTROL.H"
#include "DIALOG.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "TURN.H"
#include "ADVISORS.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CORE.H"
#include "DRAW.H"
#include "UNITS.H"
#include "ECONOMY.H"
#include "MESSAGE.H"
#include "MOUSE.H"

extern unsigned char far advance_owners[][2];


void advisor_open(char *title, int mode)
{
    mouse_off();
    menu_begin();
    advisor_header(title, mode);
}

void advisor_header(char *title, int mode)
{
    rect_fill(scrbuf, 0, 0, 320, 200, mode);
    draw_text(title, 160, 2, 15);
    switch (civ_gov[civnow] / 2) {
    case 0:
        strcpy(msgtext, "Empire");
        break;
    case 1:
        strcpy(msgtext, "Kingdom");
        break;
    case 2:
        strcpy(msgtext, "Republic");
        break;
    }
    strcat(msgtext, " of the ");
    strcat(msgtext, civ_names[civnow]);
    draw_text(msgtext, 160, 10, 15);

    strcpy(msgtext, leader_titles[civ_gov[civnow]]);
    strcat(msgtext, " ");
    strcat(msgtext, leader_names[civnow]);
    strcat(msgtext, ": ");
    format_year();
    draw_text(msgtext, 160, 18, 15);
}

void advisor_close(void)
{
    ad_wait();
    menu_end();
    mouse_on();
}

void sci_rpt(int civ)
{                               /* science report */
    int x, x0, count, diff, cost, y, bw, i, step;

    advisor_open("SCIENCE REPORT", 1);
    x0 = 0;
    diff = cur_tier * 2 + cost_penalty + 6;
    if (diff < 11 - tech_count[civ]) {
        diff = 11 - tech_count[civ];
    }
    cost = tech_count[civ] * ((year < 0) ? 1 : 2) * diff;
    step = clamp(0x980 / (cost + 1), 1, 0x40);
    bw = clamp((step * cost) / 8 + 8, 8, 0x130);
    x = 160 - bw / 2;
    if (x > 40) {
        blit(tile_page, 0x118, 0x8c, 40, 60, scrbuf, 0x116, 2);
    }
    rect_fill(scrbuf, x, 25, bw, 16, 9);

    for (i = 0; i < clamp(civ_research_tally[civ], 0, cost); i++) {
        label_with_style(12, x + x0 / 8, 0x21);
        x0 += step;
    }

    y = 26;
    if (selected_index != -1) {
        strcpy(msgtext, "Researching ");
        strcat(msgtext, advances[selected_index].name);
        draw_text(msgtext, 161, 26, 0);
        draw_text(msgtext, 160, 26, 15);
    }

    y = 42;
    count = 0;
    for (i = 0; i < 72; i++) {
        if (!has_tech(civ, i))
            continue;
        strcpy(msgtext, advances[i].name);
        draw_string(msgtext, (count % 3) * 100 + 8, y, ((advance_owners[i][0] & 7) == civ) ? 15 : 11);
        count++;
        if (count % 3 == 0) {
            y += 7;
        }
        if (y > 192)
            break;
    }

    advisor_close();
}

extern struct unit_type utypes[];
extern int far unit_losses[8][28];

void mil_rpt(int civ)
{                               /* military status report */
    int type, y, n;

    advisor_open("MILITARY STATUS", 1);
    n = 0;
    y = 32;
    for (type = 0; type < 28; type++) {
        if (forces_present[civ][type] <= 0 && civ_units_ordered[civ][type] <= 0)
            continue;
        draw_line(36, y - 2, 319, y - 2, 9);
        label_with_style((civ << 5) + type + 64, ((n++ & 1) << 4) + 2, y - 4);
        draw_string(utypes[type].name, 36, y, 15);
        strcpy(msgtext, "(");
        strcat(msgtext, itoa(utypes[type].attack, nstr, 10));
        strcat(msgtext, "/");
        strcat(msgtext, itoa(utypes[type].defense, nstr, 10));
        strcat(msgtext, "/");
        strcat(msgtext, itoa(utypes[type].moves, nstr, 10));
        strcat(msgtext, ")");
        draw_string(msgtext, 112, y, 11);
        if (forces_present[civ][type] > 0) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(forces_present[civ][type], nstr, 10));
            strcat(msgtext, " active");
            draw_string(msgtext, 168, y, 15);
        }
        if (civ_units_ordered[civ][type] > 0) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(civ_units_ordered[civ][type], nstr, 10));
            strcat(msgtext, " in production");
            draw_string(msgtext, 232, y, 11);
        }
        y += 9;
    }
    blit(tile_page, 0xA0, 0x8C, 40, 60, scrbuf, 0x116, 2);
    advisor_close();
    military_losses(civ);
}

void military_losses(int civ)
{
    int x, any, y, i, j, n;

    advisor_open("MILITARY LOSSES", 4);
    n = 0;
    y = 28;
    for (i = 0; i < 27; i++) {
        any = 0;
        for (j = 0; j < 8; j++) {
            if (unit_losses[j][i] != 0) {
                any = 1;
            }
        }
        if (!any)
            continue;
        draw_line(36, y - 1, 319, y - 1, 8);
        label_with_style((civ << 5) + i + 64, ((n++ & 1) << 4) + 2, y - 4);
        draw_string(utypes[i].name, 36, y, 15);
        x = 104;
        if (unit_losses[civ][i] != 0) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(unit_losses[civ][i], nstr, 10));
            draw_string(msgtext, 104, y, 12);
        }
        for (j = 1; j < 8; j++) {
            if (j == civ)
                continue;
            x += 28;
            if (unit_losses[j][i] != 0) {
                msgtext[0] = 0;
                strcat(msgtext, itoa(unit_losses[j][i], nstr, 10));
                draw_string(msgtext, x, y, civ_colours[j]);
            }
        }
        y += 7;
    }
    draw_box(100, 25, 18, y - 25, civ_colours[civnow]);
    advisor_close();
}

extern struct city townbuf[];


void trade_rpt(int civ)
{                               /* trade report */
    int tot_inc, tc, dummy, rate, i, turn, city, y, building_costs[24];

    rate = 0;
    if (has_tech(civ, 34))
        rate++;
    if (has_tech(civ, 37))
        rate++;

    advisor_open("TRADE REPORT", 2);
    blit(tile_page, 200, 140, 40, 60, scrbuf, 0x116, 2);
    draw_string("City Trade", 8, 32, 15);

    i = tot_inc = tc = 0;
    for (i = 0; i < 24; i++) {
        building_costs[i] = 0;
    }
    y = 40;

    for (city = 0; city < 128; city++) {
        if (townbuf[city].status != -1 && townbuf[city].owner == civ) {
            if (y >= 180) {
                ad_wait();
                advisor_header("TRADE REPORT", 2);
                y = 40;
            }

            city_process(city, -1);
            msgtext[0] = 0;
            append_place_name(city);
            truncate_text(msgtext, 62);
            if (y < 180) {
                draw_shadowed(msgtext, 16, y, 15);
            }

            msgtext[0] = 0;
            if (townbuf[city].status & 1) {
                strcat(msgtext, " \x8c(Disorder)");
            } else {
                strcat(msgtext, " ");
                strcat(msgtext, itoa(city_revel_sums, nstr, 10));
                strcat(msgtext, "\\/");
                strcat(msgtext, itoa(taxes, nstr, 10));
                strcat(msgtext, "$/");
                strcat(msgtext, itoa(learning_yield, nstr, 10));
                strcat(msgtext, "~");
                tot_inc += taxes;
                tc += learning_yield;
            }
            if (y < 180) {
                draw_string(msgtext, 80, y, 10);
                y += 8;
            }

            for (i = 1; i < 24; i++) {
                if (townbuf[city].buildings & (1L << i)) {
                    building_costs[i] += building_names[i].maintenance;
                    if (i == 1) {
                        building_costs[i] += rate;
                    }
                }
            }
        }
    }

    strcpy(msgtext, "Total Income: ");
    strcat(msgtext, itoa(tot_inc, nstr, 10));
    strcat(msgtext, "$");
    draw_string(msgtext, 8, y + 4, 10);

    if (civ_wonder(civnow, 20)) {
        tc += tc / 2;
    }
    if (tc != 0 && y < 180) {
        turn = (((year < 0) ? 1 : (year < 1750) ? 2 : 3) * tech_count[civnow] * (cur_tier * 2 + 6)) / tc;
        strcpy(msgtext, "Discoveries: ");
        strcat(msgtext, itoa(turn, nstr, 10));
        strcat(msgtext, " turns");
        draw_string(msgtext, 8, y + 12, 10);
    }

    draw_string("Maintenance Costs", 160, 32, 15);
    y = 40;
    tot_inc = 0;
    for (i = 0; i < 24; i++) {
        if (building_costs[i] != 0) {
            msgtext[0] = 0;
            if (i == 1) {
                strcat(msgtext, itoa(building_costs[i] / rate, nstr, 10));
            } else {
                strcat(msgtext, itoa(building_costs[i] / building_names[i].maintenance, nstr, 10));
            }
            strcat(msgtext, " ");
            strcat(msgtext, building_names[i].name);
            strcat(msgtext, ", ");
            strcat(msgtext, itoa(building_costs[i], nstr, 10));
            strcat(msgtext, "$");
            if (y < 180) {
                draw_string(msgtext, 160, y, 14);
                y += 8;
            }
            tot_inc += building_costs[i];
        }
    }
    strcpy(msgtext, "Total Cost: ");
    strcat(msgtext, itoa(tot_inc, nstr, 10));
    strcat(msgtext, "$");
    draw_string(msgtext, 160, y + 4, 14);
    y += 12;

    advisor_close();
}

extern int treaties[8][8];
extern int kbhit(void);

/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void spy_rpt(civ_arg)           /* intelligence report */
int civ_arg;
{
    int num, civ, cap, civs[8], done, x, y, i, j, ch, flag2;
    char relation;

    advisor_open("INTELLIGENCE REPORT", 1);
    y = 32;
    num = flag2 = 0;

    for (civ = 1; civ < 8; civ++) {
        if (players & (1 << civ)) {
            y = num * 24 + 32;
            draw_line(4, y - 2, 316, y - 2, 9);
            strcpy(msgtext, civ_names[civ]);
            if (civ != civnow && !(treaties[civnow][civ] & 0x40)) {
                draw_string("No embassy established.", 96, y, civ_colours[civ]);
                num++;
            } else {
                cap = -1;
                for (i = 0; i < 128; i++) {
                    if (townbuf[i].owner == civ && townbuf[i].status != -1 && (townbuf[i].buildings & 1)) {
                        cap = i;
                    }
                }
                strcat(msgtext, ": ");
                strcat(msgtext, leader_names[civ]);
                draw_shadowed(msgtext, 8, y, 15);
                strcpy(msgtext, government_names[civ_gov[civ]]);
                strcat(msgtext, ", ");
                strcat(msgtext, itoa(money[civ], nstr, 10));
                strcat(msgtext, "$");
                if (cur_tier <= 1) {
                    strcat(msgtext, ", ");
                    strcat(msgtext, itoa(troops_total[civ], nstr, 10));
                    strcat(msgtext, " Units.");
                }
                draw_string(msgtext, 160, y, civ_colours[civ]);
                y += 8;
                x = 16;
                for (i = 1; i < 8; i++) {
                    msgtext[0] = 0;
                    strcpy(msgtext, civ_names[i]);
                    if (x > 240) {
                        x = 16;
                        y += 8;
                    }
                    relation = treaties[civ][i];
                    if (relation & 2) {
                        if (relation & 4) {
                            strcat(msgtext, ":\x8f ALLIED");
                        } else {
                            strcat(msgtext, ":\x8f PEACE");
                        }
                        draw_string(msgtext, x, y, 7);
                        x += 96;
                    }
                    if ((treaties[civ][i] & 3) == 1) {
                        strcat(msgtext, ":\x8c WAR");
                        draw_string(msgtext, x, y, 7);
                        x += 96;
                    }
                }
                civs[num++] = civ;
                if (civ != civnow) {
                    strcpy(msgtext, "INFO");
                    strcat(msgtext, itoa(num, nstr, 10));
                    draw_button(281, num * 24 + 20, 318, num * 24 + 29, msgtext, civ_colours[civ]);
                }
            }
        }
    }

    mouse_on();
    do {
        do {
            poll_mouse();
        } while (mse_held == 0 && !kbhit());

        done = 1;
        if (kbhit()) {
            ch = key_cmd();
            if (ch >= '1' && ch <= '7') {
                civ = ch - '1';
                goto select_civ;
            }
        }
        if (ptr_x > 281) {
            civ = (ptr_y - 28) / 24;
select_civ:
            if (civ >= 0 && civ < num) {
                civ = civs[civ];
                if (civ != civnow && (treaties[civnow][civ] & 0x40)) {
                    civ_dossier(civ);
                    flag2 = 1;
                    done = 0;
                }
            }
        }
        civ = ptr_x;
        repaint();
        mouse_off();
        set_text_pos(270, ptr_y);
        mouse_on();
    } while (!done);

    mouse_off();
    if (flag2) {
        sel_player();
    } else {
        menu_end();
    }
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void civ_dossier(int civ)
{
    int cap, i, y;

    advisor_open("INTELLIGENCE REPORT", 1);
    y = 32;

    strcpy(msgtext, "Subject: the ");
    strcat(msgtext, civ_names[civ]);
    draw_shadowed(msgtext, 16, 32, 15);
    y += 12;

    strcpy(msgtext, "Leader:\x8f ");
    strcat(msgtext, leader_titles[civ_gov[civ]]);
    strcat(msgtext, " ");
    strcat(msgtext, leader_names[civ]);
    draw_string(msgtext, 16, y, 9);
    y += 8;

    if (civt[civ_tribe_of[civ]].aggressive == 1) {
        draw_string("Aggressive", 24, y, 7);
        y += 8;
    }
    if (civt[civ_tribe_of[civ]].aggressive == -1) {
        draw_string("Friendly", 24, y, 7);
        y += 8;
    }
    if (civt[civ_tribe_of[civ]].expansionist == 1) {
        draw_string("Expansionistic", 24, y, 7);
        y += 8;
    }
    if (civt[civ_tribe_of[civ]].expansionist == -1) {
        draw_string("Perfectionist", 24, y, 7);
        y += 8;
    }
    if (civt[civ_tribe_of[civ]].civilized == 1) {
        draw_string("Civilized", 24, y, 7);
        y += 8;
    }
    if (civt[civ_tribe_of[civ]].civilized == -1) {
        draw_string("Militaristic", 24, y, 7);
        y += 8;
    }
    y += 4;

    cap = -1;
    for (i = 0; i < 128; i++) {
        if (townbuf[i].owner == civ && townbuf[i].status != -1 && (townbuf[i].buildings & 1)) {
            cap = i;
        }
    }

    strcpy(msgtext, "Capital:\x8f ");
    append_place_name(cap);
    draw_string(msgtext, 16, y, 9);
    y += 8;

    strcpy(msgtext, "Government:\x8f ");
    strcat(msgtext, government_names[civ_gov[civ]]);
    draw_string(msgtext, 16, y, 9);
    y += 8;

    strcpy(msgtext, "Treasury:\x8f ");
    strcat(msgtext, itoa(money[civ], nstr, 10));
    strcat(msgtext, "$");
    draw_string(msgtext, 16, y, 9);
    y += 8;

    if (cur_tier <= 1) {
        strcpy(msgtext, "Military:\x8f ");
        strcat(msgtext, itoa(troops_total[civ], nstr, 10));
        strcat(msgtext, " Units.");
        draw_string(msgtext, 16, y, 9);
        y += 12;
    }

    draw_string("Foreign Affairs:", 16, y, 9);
    y += 8;

    for (i = 1; i < 8; i++) {
        if (treaties[civ][i] & 2) {
            strcpy(msgtext, "At\x8f PEACE");
        }
        if ((treaties[civ][i] & 3) == 1) {
            strcpy(msgtext, "At\x8c WAR");
        }
        if (treaties[civ][i] & 1) {
            strcat(msgtext, "\x87 with the ");
            strcat(msgtext, civ_names[i]);
            strcat(msgtext, ".");
            draw_string(msgtext, 24, y, 7);
            y += 8;
        }
    }
    y += 4;

    draw_string("Technologies:", 16, y, 9);
    y += 8;

    for (i = 71; i >= 0; i--) {
        if (has_tech(civ, i)) {
            draw_string(advances[i].name, 24, y, 7);
            y += 8;
            if (y >= 192) {
                break;
            }
        }
    }

    advisor_close();
}

void mood_rpt(int civ)
{                               /* attitude survey */
    int happy_total, unhappy, tot, x, i, y, city, j, dummy, step;

    advisor_open("ATTITUDE SURVEY", 9);
    y = 32;
    city = step = tot = happy_total = unhappy = 0;

    for (city = 0; city < 128; city++) {
        if (townbuf[city].status != -1 && townbuf[city].owner == civ) {
            if (y > 190) {
                ad_wait();
                advisor_header("ATTITUDE SURVEY", 9);
                y = 32;
            }

            city_process(city, -1);
            msgtext[0] = 0;
            append_place_name(city);
            strcat(msgtext, ":");
            draw_string(msgtext, 16, y, 15);

            x = (step & 7) + 80;
            draw_city_view(city, x, y - 3, city_experts_num, 124);

            x = 212;
            rect_fill(scrbuf, 212, y - 1, 90, 10, 11);

            for (i = 0; i < 24; i++) {
                if ((townbuf[city].buildings & (1 << i)) & 0x2618) {
                    draw_icon(i + 1, x, y - 1);
                    x += 18;
                }
            }

            tot += townbuf[city].size;
            happy_total += town_happy_ones_num;
            unhappy += town_unhappy_counts;
            step += 4;
            y += 10;
        }
    }

    if (tot == 0) {
        tot++;
    }

    strcpy(msgtext, "Population: ");
    count_weighted_units(civ);
    strcat(msgtext, " Happy:");
    append_percent(happy_total, tot);
    strcat(msgtext, "% Content:");
    append_percent(tot - happy_total - unhappy, tot);
    strcat(msgtext, "% Unhappy:");
    append_percent(unhappy, tot);
    strcat(msgtext, "%");
    draw_string(msgtext, 16, y + 8, 15);

    advisor_close();
}

void append_percent(int part, int total)
{
    if (part < 300) {
        strcat(msgtext, itoa((100 * part) / total, nstr, 10));
    } else {
        strcat(msgtext, itoa((10 * part) / (total / 10), nstr, 10));
    }
}

void city_rpt(int civ)
{                               /* city status report */
    int y, city, dummy, i;

    advisor_open("CITY STATUS", 8);
    y = 32;

    for (city = 0; city < 128; city++) {
        if (townbuf[city].status != -1 && townbuf[city].owner == civ) {
            if (y >= 192) {
                ad_wait();
                advisor_header("CITY STATUS", 8);
                y = 32;
            }

            city_process(city, -1);
            msgtext[0] = 0;
            append_place_name(city);
            draw_string(msgtext, 8, y, 15);

            msgtext[0] = 0;
            strcat(msgtext, itoa(townbuf[city].size, nstr, 10));
            strcat(msgtext, "-");
            strcat(msgtext, itoa(town_res_sums[0], nstr, 10));
            strcat(msgtext, "{ ");
            strcat(msgtext, itoa(town_res_sums[1], nstr, 10));
            strcat(msgtext, "| ");
            strcat(msgtext, itoa(town_res_sums[2], nstr, 10));
            strcat(msgtext, "}");
            draw_string(msgtext, 80, y, 15);

            if (townbuf[city].production >= 0) {
                if (townbuf[city].production < 64) {
                    strcpy(msgtext, utypes[townbuf[city].production].name);
                    strcat(msgtext, "\x87 (");
                    strcat(msgtext, itoa(townbuf[city].shield_box, nstr, 10));
                    strcat(msgtext, "/");
                    strcat(msgtext, itoa(utypes[townbuf[city].production].cost * 10, nstr, 10));
                    strcat(msgtext, ")");
                }
            } else {
                strcpy(msgtext, improvements[-townbuf[city].production].name);
                truncate_text(msgtext, 96);
                strcat(msgtext, "\x87 (");
                strcat(msgtext, itoa(townbuf[city].shield_box, nstr, 10));
                strcat(msgtext, "/");
                strcat(msgtext, itoa(improvements[-townbuf[city].production].cost * 10, nstr, 10));
                strcat(msgtext, ")");
            }
            draw_string(msgtext, 172, y, 15);
            y += 8;
        }
    }

    advisor_close();
}
