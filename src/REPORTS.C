/* Overlay 12: world map, PowerGraph, wonders, rankings and demographics. */

#include "CIV.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DRAW.H"
#include "REPLAY.H"
#include "REPORTS.H"
#include "TURN.H"
#include "UNITS.H"
#include "DIALOG.H"
#include "ECONOMY.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "PATH.H"

extern struct unit armydata[][128];
extern struct city townbuf[];
extern char far map_visible_to[][50];

extern unsigned char far power_history[][8];

extern int treaties[8][8];

    /* historian ranking points; [0] counts reports */


extern int abs(int);
extern char *itoa(int, char *, int);
extern char *strcat(char *, char *);
extern char *strcpy(char *, char *);






/* Function 1: [0000, 03AC) - World Map View (F10) */
/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void world_map(mode)
int mode;
{
    int p, c, ox, h, x, oy, y, i, ter, sx, min_y, minx, sy, m, max_x, max_y, color;

    mouse_off();
    blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
    rect_fill(scrbuf, 0, 0, 320, 200, 0);

    m = 1 << civnow;
    p = civnow;
    minx = min_y = 999;
    max_x = max_y = 0;
    ox = 40 - x_cap[civnow];

    if (xray_on) {
        ox = oy = 0;
        goto draw_loop;
    }

    for (i = 0; i < 128; i++) {
        if (armydata[p][i].type != -1) {
            if (min_y > armydata[p][i].y)
                min_y = armydata[p][i].y;
            if (max_y < armydata[p][i].y)
                max_y = armydata[p][i].y;
        }
    }

    for (i = 0; i < 128; i++) {
        if (townbuf[i].owner == civnow) {
            if (min_y > townbuf[i].y)
                min_y = townbuf[i].y;
            if (max_y < townbuf[i].y)
                max_y = townbuf[i].y;
        }
    }

    for (i = 0; i < 80; i++) {
        if ((map_visible_to[i][0] & m) || (map_visible_to[i][49] & m)) {
            min_y = 0;
            max_y = 49;
        }
    }

    oy = 25 - (max_y + min_y) / 2;

draw_loop:
    for (y = 0; y < 50; y++) {
        for (x = 0; x < 80; x++) {
            if (!(map_visible_to[x][y] & m) || !map_in_bounds(0, y + oy)) {
                if (!xray_on)
                    continue;
            }

            sx = wrap_x(x + ox) << 2;
            sy = (y + oy) << 2;
            ter = terrain_at(x, y);

            if (mode) {
                blit(tile_page, 0xa0 + (ter << 2), 0x6f + (((x + y) & 1) << 2), 4, 4, scrbuf, sx, sy);
            } else {
                rect_fill(scrbuf, sx, sy, 4, 4, ter == 10 ? 1 : 2);
            }

            if (map_flags(x, y) & 1) {
                c = city_at(x, y);
                if (townbuf[c].visible_size || xray_on || townbuf[c].owner == civnow) {
                    color = civ_colours[map_city(x, y)];
                    rect_fill(scrbuf, sx, sy, 4, 4, color);
                }
            } else {
                color = map_owner(x, y);
                if (color != -1) {
                    if (color == civnow || xray_on || (armydata[color][map_stack(x, y)].visibility & (1 << civnow))) {
                        color = civ_colours[color];
                        rect_fill(scrbuf, sx + 1, sy + 1, 3, 3, 0);
                        rect_fill(scrbuf, sx, sy, 3, 3, color);
                    }
                }
            }
        }
    }

    if (still != 1) {
        ad_wait();
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    }
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Function 2: [03AC, 054B) - Relations and Trade Routes Table */
void cont_table(void)
{                               /* per-continent strength table (debug) */
    int civ, i, y, k;

    mouse_off();
    menu_begin();
    rect_fill(scrbuf, 0, 0, 320, 200, 4);

    for (civ = 1; civ < 8; civ++) {
        draw_shadowed(civ_adjectives[civ], civ * 40, 8, civ_colours[civ]);
        label_with_style(civ * 32 + 64, civ * 40, 16);
    }

    for (i = 1; i < 15; i++) {
        y = i * 8 + 24;
        msgtext[0] = 0;
        strcat(msgtext, itoa(continent_span[0][i], nstr, 10));
        draw_string(msgtext, 8, y, 0);

        for (civ = 1; civ < 8; civ++) {
            if (defenders_island[civ][i] == 0)
                continue;
            msgtext[0] = 0;
            strcat(msgtext, itoa(civ_cont_attackers[civ][i], nstr, 10));
            strcat(msgtext, "/");
            strcat(msgtext, itoa(defenders_island[civ][i], nstr, 10));
            strcat(msgtext, "/");
            strcat(msgtext, itoa(ai_continent_towns[civ][i], nstr, 10));
            draw_string(msgtext, civ * 40, y, civ_colours[civ]);
        }
    }

    ad_wait();
    menu_end();
    mouse_on();
}

/* Function 3: [054B, 0803) - Civilization PowerGraph Screen */
void power_graph(void)
{
    int p, max_val, x, i, y0, turn, y, yr;

    mouse_off();
    menu_begin();
    rect_fill(scrbuf, 0, 0, 320, 200, 8);
    max_val = 50;
    draw_box(4, 9, 311, 183, 0);
    scrbuf->font = 2;

    turn = 0;
    goto test_turn;
    do {
        draw_line(turn / 2 + 4, 9, turn / 2 + 4, 192, 0);
        if (turn % 100 == 0) {
            yr = replay_year(turn);
            msgtext[0] = 0;
            strcat(msgtext, itoa(abs(yr), nstr, 10));
            strcat(msgtext, yr < 0 ? "BC" : "AD");
            draw_string(msgtext, turn / 2, 194, 15);
        }
        turn += 50;
test_turn:
        ;
    } while (turn <= turn_tick && turn <= 600);

    scrbuf->font = 1;

    for (p = 1; p <= 8; p++) {
        if (!(players & (1 << p)) && p > civ_total + 1)
            continue;
        draw_string(civ_adjectives[p], 8, (p << 3) + 4, civ_colours[p]);
        for (i = 0; i < turn_tick / 4; i++) {
            if (max_val < power_history[i][p])
                max_val = power_history[i][p];
        }
    }

    draw_shadowed("CIVILIZATION POWERGraph", 100, 2, 15);

    for (p = 1; p < 8; p++) {
        if (!(players & (1 << p)) && p > civ_total + 1)
            continue;
        turn = 0;
        y0 = 0;
        i = 0;
        goto test_i;
        do {
            x = i * 2;
            y = (180U * power_history[i][p]) / max_val;
            draw_line(turn + 4, 192 - y0, x + 4, 192 - y, civ_colours[p]);
            turn = x;
            y0 = y;
            i++;
test_i:
            ;
        } while (i < turn_tick / 4 && i < 150);
    }

    ad_wait();
    empty_report();
    menu_end();
    mouse_on();
}

/* Function 4: [0803, 080D). The original is an empty function with a
   12-byte frame. These unused local names are reconstructed; the six words
   preserve that observed frame, rather than stand in for missing logic. */
void empty_report(void)
{
    int a, b, c, d, e, f;
}

/* Function 5: [080D, 09E2) - Wonders of the World Screen */
void wonders(void)
{
    int p, c, icon, i, y, owner;

    mouse_off();
    menu_begin();
    y = 999;
    for (i = 1; i <= 21; i++) {
        if (marvel_site[i] == -1)
            continue;
        if (y > 180) {
            if (y != 999) {
                ad_wait();
            }
            rect_fill(scrbuf, 0, 0, 320, 200, 3);
            draw_shadowed("The Wonders of the World", 100, 12, 15);
            y = 32;
        }
        c = marvel_site[i];
        owner = townbuf[c].owner;
        msgtext[0] = 0;
        if (i <= 7) {
            strcat(msgtext, "The ");
        }
        strcat(msgtext, achievement_names[i].name);
        if (marvel_site[i] != 128) {
            strcat(msgtext, i <= 7 ? " of " : " in ");
            append_place_name(c);
            strcat(msgtext, ". (");
            strcat(msgtext, civ_adjectives[owner]);
            strcat(msgtext, ")");
        } else {
            strcat(msgtext, " (Destroyed)");
            owner = 0;
        }
        draw_box(8, y, 303, 15, civ_colours[owner]);
        icon = i + 24;
        draw_icon(icon, 16, y + 3);
        truncate_text(msgtext, 270);
        draw_string(msgtext, 40, y + 5, 15);
        y += 24;
    }
    ad_wait();
    menu_end();
    mouse_on();
}

char *historians[] = {
    "Herodotus",
    "Pliny",
    "Toynbee",
    "Gibbon"
};

char *ratings[] = {
    "Glorious",
    "Great",
    "Fine",
    "Mediocre",
    "Puny",
    "Pathetic",
    "Hopeless"
};

char *categories[] = {
    "WEALTHIEST",
    "MOST POWERFUL",
    "MOST ADVANCED",
    "HAPPIEST",
    "LARGEST"
};

/* Function 6: [09B9, 0D6D) - Historian Epic History Ranking Report */
void historian_report(void)
{
    int p, h, max_val, cat, i, y, score[8], n, who;

    mouse_off();
    menu_begin();
    cat = rand_scaled(5);
    rect_fill(scrbuf, 0, 0, 320, 200, 3);
    strcpy(msgtext, historians[rand_scaled(4)]);
    strcat(msgtext, " completes his epic history:");
    draw_text(msgtext, 160, 4, 15);
    strcpy(msgtext, "'The ");
    strcat(msgtext, categories[cat]);
    strcat(msgtext, " Civilizations in the World'");
    draw_text(msgtext, 160, 12, 15);

    for (i = 1; i < 8; i++)
        score[i] = 0;

    for (i = 0; i < 128; i++) {
        if (townbuf[i].owner != -1 && townbuf[i].status != -1) {
            city_process(i, -1);
            if (cat == 3) {
                score[townbuf[i].owner] += townbuf[i].size + town_happy_ones_num - town_unhappy_counts;
            }
            if (cat == 4) {
                score[townbuf[i].owner] += townbuf[i].size;
            }
        }
    }

    for (p = 0; p < 8; p++) {
        switch (cat) {
        case 0:
            score[p] = money[p];
            break;
        case 1:
            score[p] = military_table[p];
            break;
        case 2:
            score[p] = 0;
            for (i = 0; i < 72; i++) {
                if (has_tech(p, i))
                    score[p]++;
            }
            break;
        }
    }

    y = 32;
    n = 0;
    i = 1;
    goto test_i;

update_p:
    max_val = score[p];
    who = p;
next_p:
    p++;
test_p:
    if (p < 8) {
        if (score[p] < max_val)
            goto next_p;
        if ((players & (1 << p)) == 0)
            goto next_p;
        goto update_p;
    }

    if (who == -1)
        goto next_i;

    msgtext[0] = 0;
    strcat(msgtext, itoa(n + 1, nstr, 10));
    strcat(msgtext, ". The ");
    strcat(msgtext, ratings[n]);
    strcat(msgtext, " Civilization of the ");
    strcat(msgtext, civ_names[who]);
    strcat(msgtext, ".");
    if ((treaties[civnow][who] & 0x40) || civnow == who || cur_tier == 0) {
        flip(60);
        draw_palace(who, steps_for(civ_ratings[who]), 128, y - 13);
        draw_box(8, y, 303, 15, civ_colours[who]);
        draw_string(msgtext, 40, y + 5, 15);
        y += 24;
    }
    history_rating[who] += 7 - n;
    n++;
    score[who] = -1;

next_i:
    i++;
test_i:
    if (i < 8) {
        if (!(players & (1 << i)))
            goto next_i;
        max_val = 0;
        who = -1;
        p = 1;
        goto test_p;
    }
    history_rating[0]++;
    repaint();
    ad_wait();
    menu_end();
    mouse_on();
}

/* Function 7: [0D6D, 1968) - Demographics Report */
void demographics(int player)
{
    int life[8];
    int app[8];
    int arms[8];
    int val[8];
    int mx;
    int lib[8];
    int i;
    int y;
    int mfg[8];
    int population[8];
    int pol[8];
    int income[8];
    int excess;
    int areas[8];
    int rank[8];
    int sz;

    mouse_off();
    menu_begin();
    rect_fill(scrbuf, 0, 0, 320, 200, 1);
    strcpy(msgtext, civ_adjectives[player]);
    strcat(msgtext, " Demographics");
    draw_text(msgtext, 160, 4, 15);

    for (i = 0; i < 8; i++) {
        population[i] = 1;
        mfg[i] = val[i] = app[i] = pol[i] = life[i] = lib[i] = income[i] = areas[i] = arms[i] = 0;
    }

    for (mx = 0; mx < 80; mx++) {
        for (y = 2; y < 48; y++) {
            if (terrain_at(mx, y) != 10) {
                areas[map_city(mx, y)]++;
            }
        }
    }

    i = 0;
    goto test_city;
city_body:
    city_process(i, -1);
    population[townbuf[i].owner] += (sz = townbuf[i].size);
    app[townbuf[i].owner] += sz + town_happy_ones_num - town_unhappy_counts;
    income[townbuf[i].owner] += (city_revel_sums << 1) + taxes;
    arms[townbuf[i].owner] += town_res_sums[0] - sz * 2;
    val[townbuf[i].owner] += town_res_sums[0] + town_res_sums[1] + town_res_sums[2];
    mfg[townbuf[i].owner] += town_res_sums[1];
    excess = (town_res_sums[1] / plant_waste_divisor) - 20;
    excess += (sz * pop_smog_advances) / 4;
    if (excess > 0) {
        pol[townbuf[i].owner] += excess;
    }
    if (townbuf[i].buildings & 0x4) {
        life[townbuf[i].owner] += townbuf[i].size;
    }
    if (townbuf[i].buildings & 0x100) {
        life[townbuf[i].owner] += townbuf[i].size;
    }
    if (townbuf[i].buildings & 0x20) {
        lib[townbuf[i].owner] += townbuf[i].size;
    }
    if (townbuf[i].buildings & 0x800) {
        lib[townbuf[i].owner] += townbuf[i].size;
    }
next_city:
    i++;
test_city:
    if (i < 128) {
        if (townbuf[i].owner != -1) {
            if (townbuf[i].status != -1) {
                goto city_body;
            }
        }
        goto next_city;
    }

    y = 24;
    draw_shadowed("Approval Rating:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = (50 * app[i]) / population[i];
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            strcat(msgtext, "%");
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, "%");
    y += 12;

    draw_shadowed("Population:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        msgtext[0] = 0;
        rank[i] = count_weighted_units(i);
        if (i == player) {
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, "0000");
    y += 12;

    draw_shadowed("GNP:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        msgtext[0] = 0;
        rank[i] = income[i];
        msgtext[0] = 0;
        strcat(msgtext, itoa(rank[i], nstr, 10));
        strcat(msgtext, " million $");
        if (i == player) {
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, "M$");
    y += 12;

    draw_shadowed("Mfg. Goods:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = mfg[i];
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            strcat(msgtext, " Mtons");
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, " Mtons");
    y += 12;

    draw_shadowed("Land Area:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        msgtext[0] = 0;
        rank[i] = areas[i];
        msgtext[0] = 0;
        strcat(msgtext, itoa(rank[i], nstr, 10));
        strcat(msgtext, ",000 sq.miles");
        if (i == player) {
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, ",000");
    y += 12;

    draw_shadowed("Literacy:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = ((population[i] * 2 + lib[i]) * 2) / population[i];
        if (has_tech(i, 0)) {
            rank[i] <<= 1;
        }
        if (has_tech(i, 22)) {
            rank[i] <<= 1;
        }
        if (has_tech(i, 30)) {
            rank[i] <<= 1;
        }
        if (has_tech(i, 13)) {
            rank[i] <<= 1;
        }
        rank[i] = clamp(rank[i], 0, 100);
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            strcat(msgtext, "%");
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, "%");
    y += 12;

    draw_shadowed("Disease:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = (50 * population[i]) / (population[i] + life[i]);
        if (has_tech(i, 10)) {
            rank[i] /= 2;
        }
        life[i] = 1500 / (rank[i] + 20);
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            strcat(msgtext, "%");
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, -y, "%");
    y += 12;

    draw_shadowed("Pollution:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = pol[i];
        life[i] -= (10 * pol[i]) / population[i];
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            strcat(msgtext, "0 tons/year");
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, -y, "0 tons");
    y += 12;

    draw_shadowed("Life Expectancy:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = clamp(life[i], 20, 99);
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            strcat(msgtext, " years");
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, " yrs");
    y += 12;

    draw_shadowed("Family Size:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = (40 * arms[i]) / population[i] + 20;
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i] / 10, nstr, 10));
            strcat(msgtext, ".");
            strcat(msgtext, itoa(rank[i] % 10, nstr, 10));
            strcat(msgtext, " children");
            draw_string(msgtext, 104, y, 15);
        }
        rank[i] /= 10;
    }
    demographic_rank(rank, y, "");
    y += 12;

    draw_shadowed("Military Service:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = (10 * troops_total[i]) / population[i];
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            strcat(msgtext, " years");
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, " yrs");
    y += 12;

    draw_shadowed("Annual Income:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        if (30000 / (tech_count[i] + 1) > income[i]) {
            rank[i] = (income[i] * tech_count[i]) / population[i];
        } else {
            rank[i] = (income[i] / population[i]) * tech_count[i];
        }
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            strcat(msgtext, "$ per capita");
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, "$");
    y += 12;

    draw_shadowed("Productivity:", 8, y, 15);
    for (i = 1; i < 8; i++) {
        rank[i] = (10 * val[i]) / population[i];
        if (i == player) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(rank[i], nstr, 10));
            draw_string(msgtext, 104, y, 15);
        }
    }
    demographic_rank(rank, y, "");
    y += 12;

    ad_wait();
    menu_end();
    mouse_on();
}

/* Function 8: [1968, 1B58) - Demographics Metric Ranking and Leader Display Helper */
void demographic_rank(int *values, int y, char *suffix)
{
    int r, max_val, i, best_civ;

    r = 1;
    max_val = -9999;
    best_civ = civnow;

    for (i = 1; i < 8; i++) {
        if (players & (1 << i)) {
            if (y < 0) {
                values[i] = -values[i];
            }
            if (values[i] > values[civnow]) {
                r++;
            }
            if (values[i] > max_val) {
                max_val = values[i];
                best_civ = i;
            }
        }
    }

    msgtext[0] = 0;
    strcat(msgtext, itoa(r, nstr, 10));
    switch (r) {
    case 1:
        strcat(msgtext, "st");
        break;
    case 2:
        strcat(msgtext, "nd");
        break;
    case 3:
        strcat(msgtext, "rd");
        break;
    default:
        strcat(msgtext, "th");
        break;
    }

    draw_line(4, abs(y) - 3, 316, abs(y) - 3, 9);
    draw_shadowed(msgtext, 192, abs(y), 15);

    if (best_civ != civnow && (treaties[civnow][best_civ] & 0x40)) {
        strcpy(msgtext, "(");
        strcat(msgtext, civ_names[best_civ]);
        strcat(msgtext, ": ");
        strcat(msgtext, itoa(abs(max_val), nstr, 10));
        strcat(msgtext, suffix);
        strcat(msgtext, ")");
        draw_string(msgtext, 318 - string_width(msgtext), abs(y), civ_colours[best_civ]);
    }
}
