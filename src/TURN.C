/* Turn processing, events, ratings, status displays and the player screen. */
#include <string.h>
#include <stdlib.h>
#include "CORE.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "SPACESHP.H"
#include "UNITS.H"
#include "CAPTURE.H"
#include "CITY.H"
#include "CONTROL.H"
#include "DIALOG.H"
#include "DIPLOMAC.H"
#include "DRAW.H"
#include "ECONOMY.H"
#include "FAME.H"
#include "FILEIO.H"
#include "INTRO.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "NEWS.H"
#include "PALACE.H"
#include "PEDIA.H"
#include "QUIZ.H"
#include "REPLAY.H"
#include "REPORTS.H"
#include "SAVE.H"
#include "TURN.H"
#include "UNITAI.H"
#include "WORLDGEN.H"
#include "DRIVER.H"
#include "FADE.H"

struct unit_record {
    char x, y, type, size;
    char unknown4, kind, unknown6, owner;
    char pad[2];
    int progress;
    char rest[16];
};
struct unit_flag {
    char flags;
    char rest[27];
};
struct group_record {
    char unknown[5];
    unsigned char x, y;
    char type;
    char rest[4];
};
struct record_state {
    char flags;
    char pad[8];
    char mark;
    char pad2[2];
};
struct unit_type {
    char pad0[24];
    int cost;
    char pad26[8];
};
struct relation {
    char a, b;
};

#define wonders (improvements + 24)

/* initialised data */
int palace_stage = 0;
int unused_word = 0;
int pop_milestone = 1;
int next_event_turn = 80;
int player_screen_flag = 0;

/* DGROUP variables */

extern int debug;

#define units ((struct unit_record *) ((char *) townbuf + 4))   /* city records from x */
extern struct unit_flag townbuf[];
#define group_records ((struct group_record (*)[128]) ((char *) armydata - 4))
extern struct record_state armydata[];
extern struct unit_type utypes[];
extern struct relation treaties[8][8];
extern char far advance_owners[], far power_history[];
extern char civ_vis_grid[];
extern char far war_record[];

/* functions in other segments */









/* functions of this unit */

int show_menu(char *a, int b, int c)
{
    return menu_dialog(a, b, c);
}

int show_msg(char *a, int b, int c)
{
    int r;

    mouse_off();
    blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
    mouse_on();
    r = show_menu(a, b, c);
    mouse_off();
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    mouse_on();
    return r;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void game_turn(void)
{
    int old, n, who, buf[4], m, l, found, i, r, p;

    mouse_off();
    cycle_on();
    old = money[civnow];
    i = live_civilizations = max_rating = human_counts = num_ai = 0;
    do {
        if (players & (1 << i)) {
            ++live_civilizations;
            if (max_rating < tech_count[i])
                max_rating = tech_count[i];
        }
        ++i;
    } while (i < 8);
    if (turn_tick == 0) {
        map_x0 = group_records[civnow][0].x - 7;
        map_y0 = group_records[civnow][0].y - 6;
        sel_player();
        if (cur_tier == 0) {
            uinit(civnow, 0);
            show_start();
            sel_player();
        }
        sound(1);
    }
    for (i = 0; i < 8; i++) {
        civ_ratings[i] = 0;
        if (!((players & (1 << i)) && (i >= civnow || !reload)))
            continue;
        rect_fill(scrbuf, 2, 0xc0, 6, 6, 8);
        who = 0;
        if (i == civnow) {
            mouse_on();
            who = 1;
        }
        if (!reload)
            civ_economy(i);
        if (money[i] > 30000)
            money[i] = 30000;
        city_phase(i);
        unit_phase(i);
        unit_cleanup(i);
        rect_fill(scrbuf, 2, 0xc0, 6, 6, 15);
        if (i == civnow) {
            status_draw();
            if (city_count[i] == 0 && civ_settlers_n[i] == 0) {
                mouse_off();
                civ_destroyed();
                leave = -1;
                mouse_on();
            }
            if (year > 0)
                ++player_peace_run;
        }
        if (!leave)
            ai_turn(i);
        reload = 0;
        if (who)
            mouse_off();
        if (leave && !complete_flag) {
            if (leave == 2) {
                show_hall_of_fame();
                show_ad_screen();
            }
            return;
        }
    }
    mouse_on();
    uprising();
    ai_event();
    if (turn_tick > 50) {
        for (i = 0; i < 2; i++) {
            i = rand_scaled(128);
            if (!(units[i].type != -1 && city_count[units[i].owner] > 1 && units[i].size >= 5))
                continue;
            city_disaster(i);
        }
    }
    rate_civs();
    if (!leave && city_count[civnow] > 1 && money[civnow] < 100 && (money[civnow] - old) * 10 + money[civnow] < 0 && (human_set & (1 << civnow))) {
        msgtext[0] = 0;
        named_msg("*TAXES");
        message_mode = 4;
        show_msg(msgtext, 80, 80);
    }
    ++turn_tick;
    if (year < 1000)
        year += 20;
    else if (year < 1500)
        year += 10;
    else if (year < 1750)
        year += 5;
    else if (year < 1850 && !(game_flags & 0xfe))
        year += 2;
    else
        ++year;
    if (year == 0) {
        for (i = 0; i < 8; i++)
            civ_research_tally[i] <<= 1;
        year = 1;
    }
    if (year == 21)
        year = 20;
    if (turn_tick % 50 == 0 && civnow != -1) {
        if (!(game_flags & 1) && year < 0)
            quiz_event();
        if (debug & 2)
            save_game((turn_tick / 50 - 1) % 6 + 4);
        max_rating = 0;
        i = 1;
        do {
            if (max_rating < tech_count[i])
                max_rating = tech_count[i];
            ++i;
        } while (i < 8);
        cost_penalty = 0;
        if (year >= 0)
            cost_penalty = clamp(max_rating - turn_tick / 9, 0, 6);
        n = count_weighted_units(civnow);
        post_event(11, 3, city_count[civnow], n / 256, (unsigned char) n);
        i = 0;
        do {
            buf[i] = 0;
            ++i;
        } while (i < 4);
        for (i = 1; i < 8; i++) {
            if (!(players & (1 << i)))
                continue;
            if (nation_ratings[i] & 1)
                buf[nation_ratings[i] / 2] |= i << 4;
            else
                buf[nation_ratings[i] / 2] |= i;
        }
        post_event(12, 4, buf[3], buf[2], buf[1], buf[0]);
    }
    if (turn_tick >= next_event_turn) {
        next_event_turn = rand_scaled(40) + turn_tick + 20;
        historian_report();
    }

    i = 1;
    do {
        if (marvel_site[i] != -1 && marvel_site[i] != 0x80)
            civ_ratings[units[marvel_site[i]].owner] += 25;
        ++i;
    } while (i <= 21);
    if (load_state == -3)
        load_state = -2;
    else {
        civ_ratings[civnow] -= 10 * polluted;
        if (load_state == -2)
            load_state = 0;
    }
    found = 0;
    for (i = 0; i < 128; i++)
        if (units[i].type != -1 && units[i].owner == civnow && (townbuf[i].flags & 1))
            found = 1;
    if (found && (debug & 0x80) && !(game_flags & 0x100)
        && civ_score(civnow, 0L) >= cost_sum(palace_stage + 1)) {
        ++palace_stage;
        if (palace_stage <= 37)
            palace_view(palace_stage);
    }
    status_draw();
    winner = -1;
    for (i = 1; i < 8; i++)
        if ((game_flags & (1 << i)) && spaceship_arrive_dates[i] == year)
            winner = i;
    if (-(20 * cur_tier - 0x820) == year) {
        strcpy(msgtext, leader_titles[civ_gov[civnow]]);
        strcat(msgtext, " ");
        strcat(msgtext, leader_names[civnow]);
        strcat(msgtext, "\nplans retirement\nin 20 years.\n");
        news_box(-1);
    }
    if (winner == -1 && !complete_flag && -(20 * cur_tier - 0x834) != year)
        return;
    mouse_off();
    strcpy(msgtext, "*** GAME OVER ***\n");
    if (complete_flag) {
        strcpy(msgtext, "Your civilization\nhas conquered\nthe entire planet!\n");
        winner = -1;
        news_box(-1);
        conquest_replay();
    }
    if (winner != -1) {
        spaceship_arrival(winner);
        sel_player();
        strcpy(msgtext, civ_adjectives[winner]);
        strcat(msgtext, " spaceship\narrives at Alpha Centauri.\n");
        news_box(-1);
    }
    if (-(20 * cur_tier - 0x834) == year) {
        strcpy(msgtext, leader_names[civnow]);
        strcat(msgtext, " dynasty\nends after glorious\n6000 year reign!\n");
        sound(civt[civ_tribe_of[civnow]].anthem, 3);
        news_box(-1);
        sound(1);
    }
    mouse_on();
    if (!(game_flags & 0x100))
        show_hall_of_fame(winner);
    sel_player();
    game_flags |= 0x100;
    if (!complete_flag) {
        msgtext[0] = 0;
        named_msg("*FAME");
        leave = -(show_msg(msgtext, 80, 80) - 1);
    } else
        leave = 1;
    if (leave)
        show_ad_screen();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int winner = -1;

/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void show_hall_of_fame(a)
int a;
{
    int h;

    power_graph();
    if (leave != 2)
        replay();
    mouse_off();
    if (!(game_flags & 0x100)) {
        h = civ_score(civnow, 1L);
        fame_header(palace_stage / 2 - 1, h);
        fame_load();
        fame_insert(a);
        fame_save();
    }
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void show_ad_screen(void)
{
    cycle_off();
    disk_insert(1, 1L);
    load_screen(1, "adscreen.pic");
    mouse_off();
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    load_back("adscreen.pic");
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    ad_wait();
}

void uprising(void)
{
    int p, t, e, f, w, g, h, x, count, i, y, k, dx, l;

    if ((5 - cur_tier) * 30 > turn_tick)
        return;
    if (!(((char) turn_tick + 1) & 7) && !(advance_owners[0x74] & 7)) {
        do {
            x = rand_scaled(80);
            y = rand_scaled(44) + 3;
        } while (terrain_at(x, y) != 10 || map_owner(x, y) != -1 || map_region(x, y) < 16);
        count = clamp(turn_tick / 150 + 1, 1, 3);
        create_unit(0, count < 3 ? 17 : 18, x, y);
        for (i = 0; i <= count; i++) {
            if (!(advance_owners[0x70] & 7))
                t = 3;
            else
                t = 7;
            if (i == count)
                t = 26;
            dx = create_unit(0, t, x, y);
            armydata[dx].flags |= 1;
        }
    }
    i = rand_scaled(128);
    if (units[i].type == -1 || units[i].owner != civnow)
        return;
    count = unrest_chance(i);
    if (rand_scaled(100) >= count)
        return;
    dx = 0;
    do {
        x = (unsigned char) units[i].x + rand_scaled(13) - 6;
        y = (unsigned char) units[i].y + rand_scaled(13) - 6;
        i = near_city(x, y);
        w = map_dist(x, (unsigned char) units[i].x, y, (unsigned char) units[i].y);
    } while (++dx < 99 && (w < 6 || unit_blocked(p, x, y) || terrain_at(x, y) == 10 || map_owner(x, y) != -1 || region_key(x, y) != region_key((unsigned char) units[i].x, (unsigned char) units[i].y)));
    if (dx >= 99)
        return;
    if (units[i].owner == civnow && (civ_vis_grid[50 * x + y] & (1 << civnow))) {
        reveal(civnow, (unsigned char) units[i].x - 8, (unsigned char) units[i].y - 6);
        strcpy(msgtext, "Travellers report:\n");
        strcat(msgtext, advance_owners[0x46] & 7 ? "Guerrilla uprising\nnear " : advance_owners[0x44] & 7 ? "Native unrest\nnear " : "Barbarian uprising\nnear ");
        append_place_name(i);
        strcat(msgtext, "!\n");
        message_mode = 2;
        show_msg(msgtext, 100, 32);
    }
    count = clamp(rand_scaled(units[i].size / 2), 2, 99);
    for (k = 0; k <= count; k++) {
        if (advance_owners[0x46] & 7)
            t = (k & 4) ? 9 : 4;
        else
            t = (k & 4) ? 10 : 6;
        if (k == count)
            t = 26;
        armydata[create_unit(0, t, x, y)].mark |= civ_vis_grid[50 * x + y];
        if (units[i].owner == civnow)
            map_note(x, y);
    }
}

void rate_civs(void)
{
    int champ, i, j, mask, rate[8];

    champ = 0;
    for (i = 1; i < 8; i++) {
        rate[i] = money[i] / 32 + (citizens[i] << 3) + tech_count[i];
        for (j = 0; j < 28; j++)
            rate[i] += utypes[j].cost * forces_present[i][j];
        if (!(players & (1 << i)))
            rate[i] = 0;
        if (i != civnow && champ < rate[i])
            champ = rate[i];
        if (turn_tick < 600)
            power_history[turn_tick / 4 % 150 * 8 + i] = clamp(rate[i] / 8, 0, 255);
        mask = 0;
        for (j = 0; j < 8; j++)
            if ((treaties[i][j].a & 3) == 1)
                mask |= 1 << j;
        if (turn_tick < 600)
            war_record[turn_tick / 4 % 150 * 8 + i] = (char) mask;
    }
    for (j = 1; j < 8; j++) {
        int rank;

        champ = -1;
        for (i = 1; i < 8; i++)
            if (champ < rate[i]) {
                rank = i;
                champ = rate[i];
            }
        nation_ratings[rank] = 8 - j;
        rate[rank] = -1;
    }
    gangmode = 0;
    if (nation_ratings[civnow] == 7 && city_count[civnow] > 4 && forces_present[civnow][25] == 0 && turn_tick > 200) {
        gangmode = 1;
        for (j = 1; j < 8; j++) {
            if (j == civnow)
                continue;
            if (city_count[j] <= 1)
                continue;
            {
                register struct relation *p = &treaties[j][civnow];
                if (p->a & 2) {
                    p->b |= 1;
                    continue;
                }
            }
            treaties[j][civnow].a |= 1;
        }
    }
}

void draw_banner(void)
{
    rect_fill(scrbuf, 0, 0, 320, 8, 0);
    draw_string("G\207AME", 8, 1, 15);
    draw_string("O\207RDERS", 64, 1, 15);
    draw_string("A\207DVISORS", 128, 1, 15);
    draw_string("W\207ORLD", 192, 1, 15);
    draw_string("C\207IVILOPEDIA", 240, 1, 15);
}

void status_draw(void)
{
    register int unit;
    int a, last, pop;
    if (still == 1)
        return;
    panel_prep(0, 58, 80, 39);
    rect_fill(scrbuf, 3, 60, 74, 11, 11);
    if (player_peace_run > 1)
        rect_fill(scrbuf, 3, 60, clamp(player_peace_run * 2, 0, 60), 2, 15);
    draw_palace(civnow, palace_stage, 8, 58);
    if (xray_on) {
        a = free_paragraphs();
        msgtext[0] = 0;
        strcat(msgtext, itoa(a, nstr, 10));
        draw_string(msgtext, 2, 59, 0);
    }
    msgtext[0] = 0;
    pop = count_weighted_units(civnow);
    strcat(msgtext, "#");
    if (pop)
        draw_string(msgtext, 2, 73, 0);
    msgtext[0] = 0;
    format_year();
    draw_string(msgtext, 2, 81, 0);
    unit = civnow * 2;
    a = *(int *) ((char *) civ_research_tally + unit) * 4 / ((cur_tier * 2 + cost_penalty + 6) * *(int *) ((char *) tech_count + unit) * (year < 0 ? 1 : 2) + 1);
    blit(tile_page, clamp(a, 0, 3) * 8 + 160, 120, 8, 8, scrbuf, 48, 80);
    if (warm_levels) {
        a = clamp(warm_levels / 4, 0, 3);
        blit(tile_page, a * 8 + 160, 128, 8, 8, scrbuf, 56, 80);
    }
    msgtext[0] = 0;
    strcat(msgtext, itoa(money[civnow], nstr, 10));
    strcat(msgtext, "$");
    strcat(msgtext, " ");
    unit = civnow * 2;
    strcat(msgtext, itoa(-(*(int *) ((char *) civ_toll + unit) + *(int *) ((char *) sci + unit) - 10), nstr, 10));
    strcat(msgtext, ".");
    strcat(msgtext, itoa(civ_toll[civnow], nstr, 10));
    strcat(msgtext, ".");
    strcat(msgtext, itoa(sci[civnow], nstr, 10));
    draw_string(msgtext, 2, 89, 0);
    if (player_screen_flag)
        return;
    last = pop_milestone;
    strcpy(msgtext, "The population of the\nfertile ");
    strcat(msgtext, civ_adjectives[civnow]);
    strcat(msgtext, " empire now\nexceeds ");
    if (pop < 100) {
        if (10 * pop_milestone < pop) {
            unit = pop / 10;
            strcat(msgtext, itoa(unit, nstr, 10));
            pop_milestone = unit;
        }
    } else {
        a = pop_milestone - 9;
        if (100 * a < pop) {
            unit = pop / 100;
            strcat(msgtext, itoa(unit, nstr, 10));
            strcat(msgtext, ",0");
            pop_milestone = unit + 9;
        }
    }
    if (last < pop_milestone && load_state >= -1) {
        strcat(msgtext, "00,000 citizens.\n");
        message_mode = 4;
        show_msg(msgtext, 100, 80);
    }
}

int unused_flag = -1;

void draw_palace(int civ, int count, int x, int y)
{
    int dummy;
    int heights[9], levels[9];
    int n, max, rnd, t, height, good, i, where;

    draw_line(x - 5, y + 13, x + 68, y + 13, 2);
    if (city_count[civ]) {
        n = clamp(count + 3, 3, 36);
        max = n / 5;
        for (i = 0; i < 9; i++)
            heights[i] = -1;
        for (i = 3; i <= 5; i++)
            heights[i] = 0;
        rnd = civ * map_rng_state;
        if (civ == civnow) {
            for (i = 0; i < 9; i++) {
                heights[i] = floors_palace[i];
                levels[i] = section_grades[i];
            }
        } else {
            for (i = 0; i < n; i++) {
                t = 0;
                do {
                    good = 1;
                    rnd = rnd * 5 + 1;
                    where = (unsigned int) (unsigned char) rnd % 9;
                    height = heights[where] + 1;
                    if (where < 4)
                        if (heights[where + 1] < height || heights[7 - where] == -1)
                            good = 0;
                    if (where > 4)
                        if (heights[where - 1] < height || heights[9 - where] == -1)
                            good = 0;
                    if (height > max || height > 3)
                        good = 0;
                } while (!good && ++t < 999);
                heights[where] = height;
                levels[where] = civ % 3;
            }
        }
        for (i = 0; i < 9; i++) {
            if (heights[i] > 0) {
                height = 2;
                if (i < 4) {
                    height = 1;
                    if (heights[i - 1] == -1 || i == 1)
                        height = 0;
                }
                if (i > 4) {
                    height = 3;
                    if (heights[i + 1] == -1 || i == 7)
                        height = 4;
                }
                sprite_draw(scrbuf, x + 7 * i, y - 1, bar_set[levels[i]][clamp(heights[i] - 1, 0, 3) * 5 + height]);
            }
        }
    }
}

int cost_sum(int n)
{
    int sum, i;

    sum = 0;
    for (i = 0; i <= n; i++)
        sum += i * (7 - cur_tier);
    return sum / 2 + 1;
}

void format_year(void)
{
    strcat(msgtext, itoa(abs(year), nstr, 10));
    strcat(msgtext, year < 0 ? " BC" : " AD");
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void ai_event(void)
{
    int unit, pad1, pad2, enemy, n, leader;

    warm_levels = clamp(map_random(polluted * 2 - warming * 4 - warm_levels) + warm_levels, 0, 99);
    if (warm_levels == 12 && polluted > 6) {
        strcpy(msgtext, "Scientists alarmed by\nrising temperatures.\nGlobal Warming feared!\n");
        news_box(-1);
    }
    if (warm_levels > 16) {
        global_warming(warming++);
        warm_levels = 0;
    }
    for (n = 0; n <= cur_tier; n++) {
        leader = rand_scaled(21) + 1;
        if (cur_tier == 0 && !has_tech(civnow, improvements[leader].prereq))
            continue;
        if (cur_tier < 2) {
            for (unit = 0; unit < 128; unit++)
                if (units[unit].owner == civnow && units[unit].type != -1 && -(leader + 24) == units[unit].kind)
                    leader = 17;
        }
        unit = rand_scaled(128);
        if (units[unit].type == -1)
            continue;
        enemy = units[unit].owner;
        if (treaties[enemy][0].a & 4)
            continue;
        if (leader == 17 && !has_tech(enemy, 46))
            continue;
        if (n == 0 && has_tech(enemy, 46) && !has_tech(civnow, 46) && marvel_site[17] == -1)
            leader = 17;
        if (leader == 17 && has_tech(civnow, 46) && military_table[enemy] > military_table[civnow])
            continue;
        if (enemy == -1)
            continue;
        if (enemy == civnow)
            continue;
        if (marvel_site[leader] == -1 && units[unit].size > wonders[leader].cost / 10 && has_tech(enemy, wonders[leader].prereq)) {
            marvel_site[leader] = unit;
            units[unit].progress = 0;
            post_event(10, 2, enemy, leader);
            msgtext[0] = 0;
            append_place_name(unit);
            strcat(msgtext, " (");
            strcat(msgtext, civ_adjectives[enemy]);
            strcat(msgtext, ")\nbuilds ");
            strcat(msgtext, wonders[leader].name);
            strcat(msgtext, ".\n");
            money[enemy] -= clamp(money[enemy] / 3, 0, 10 * wonders[leader].cost - units[unit].progress);
            if (still != 1) {
                sound(civt[civ_tribe_of[enemy]].motif);
                message_mode = (treaties[civnow][enemy].a & 64) ? 5 : 2;
                show_msg(msgtext, 100, 80);
                pedia_show(leader + 24, 1);
                sel_player();
                sound(1);
            }
            if (leader == 14) {
                reduce_attitude(enemy, 0);
                reduce_attitude(enemy, 0);
            }
            for (unit = 0; unit < 128; unit++)
                if (units[unit].type != -1 && units[unit].owner == civnow && -(leader + 24) == units[unit].kind)
                    kill(civnow, unit);

            break;
        }
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void sel_player(void)
{
    cycle_off(1);
    player_screen_flag = 1;
    if (vga_modes)
        set_palette(5, 0, 0, 0);
    mouse_off();
    reveal(civnow, map_x0, map_y0);
    panel_prep(0, 0x61, 0x50, 0x67);
    mouse_on();
    load_player_palette();
    player_screen_flag = 0;
    cycle_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void panel_prep(int a, int b, int c, int d)
{
    draw_backdrop(a, b, c, d);
    draw_panel(a, b, c - 1, d - 1, 15, 8);
}

void load_player_palette(void)
{
    char name[14];
    register int style;

    if (vga_modes) {
        cycle_off();
        strcpy(name, "back0a.pal");
        style = civ_gov[civnow];
        name[4] = style / 2 + '0';
        if (style == 3)
            name[4] = '3';
        else if (has_tech(civnow, 20))
            name[5] = 'm';
        load_palette(name, pic_colours_data);
        load_palette("sp256.pal", icon_colours_block);
        memcpy(palette_to_pal, pal_begin_colours, 0x180);
        install_palette(5, icon_colours_block);
        cycle_on();
    }
}

int steps_for(int n)
{
    int i = 0;
    do {
        i += 5;
        n -= i;
    } while (n > 0);
    return i / 5 - 1;
}
