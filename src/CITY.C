/* Overlay 20: city creation, city events and civilization scores. */
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "ADVISORS.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DIPLOMAT.H"
#include "DRAW.H"
#include "ECONOMY.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "MAP.H"
#include "RESIDENT.H"
#include "SPACESHP.H"
#include "TURN.H"
#include "UNITAI.H"
#include "UNITS.H"
#include "CITY.H"
#include "MESSAGE.H"
#include "NAMES.H"
#include "NOTES.H"
#include "PATH.H"

extern struct city townbuf[];
extern char far place_name[][13];
extern char civ_vis_grid[];
extern int work_rules[][6];
extern int debug;



/* Unit event declarations */


/* Score declarations */

extern int ship_success;
extern int icon_row_step;

/* --- Function 1: [0000, 0540) create_city --- */
#define scan_x ao
#define scan_y ap
#define is_first aa
#define city ab
#define flag ac
#define i ad
#define terrain ae
#define slot af
#define ptr ai

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int city_create(int player, int unused, int x, int y, int size)
{
    int scan_x;
    int scan_y;
    int is_first;
    int city;
    int flag;
    int i;
    int terrain;
    int slot;
    char far *ptr;

    clear_sites_near(player, 0, x, y, 4);

    for (city = 0; city < 128; city++) {
        if (townbuf[city].status != -1)
            continue;

        slot = civ_tribe_of[player] - 1;
        if (slot >= 7)
            slot--;
        slot <<= 4;
        scan_x = slot;
        do {
            flag = 1;
            if (city_site_x[slot] != -1) {
                flag = 0;
                if (scan_x + 16 == ++slot)
                    slot = 224;
                if (slot == 256) {
                    slot = 0;
                    if (scan_x == 999)
                        flag = 1;
                    scan_x = 999;
                }
            }
        } while (flag == 0);

        townbuf[city].name_id = slot;
        city_site_x[slot] = x;
        city_site_y[slot] = y;
        if (human_set & (1 << player)) {
            if (!rename_city(city))
                return -1;
            place_name[(unsigned char) townbuf[city].name_id][12] = 0;
        }

        i = 11;
        do {
            ptr = &place_name[(unsigned char) townbuf[city].name_id][i];
            if (*ptr != ' ')
                break;
            *ptr = 0;
        } while (--i > 0);

        post_event(1, 4, player, (unsigned char) townbuf[city].name_id, x, y);
        unflag(36, x, y);
        flag_tile(9, x, y);
        if (work_rules[terrain_at(x, y)][0] < -1) {
            flag_tile(2, x, y);
        }

        townbuf[city].status = 0;
        townbuf[city].x = x;
        townbuf[city].y = y;
        townbuf[city].size = size;
        townbuf[city].workers = townbuf[city].visible_size = 0;
        townbuf[city].buildings = townbuf[city].shield_box = townbuf[city].food_box = townbuf[city].trade = 0;
        townbuf[city].production = 1;
        if (has_tech(player, 34)) {
            townbuf[city].production = 4;
        }
        if (has_tech(player, 64)) {
            townbuf[city].production = 5;
        }
        townbuf[city].garrison[0] = townbuf[city].garrison[1] = -1;

        is_first = 1;
        for (i = 0; i < 128; i++) {
            if (townbuf[i].owner == player && i != city) {
                is_first = 0;
            }
        }
        if (is_first) {
            townbuf[city].buildings = 1;
        }

        townbuf[city].trade = 0;
        townbuf[city].owner = player;
        for (i = 0; i < 3; i++) {
            townbuf[city].routes[i] = -1;
        }
        townbuf[city].specialists = 0;

        for (i = 1; i <= 8; i++) {
            terrain = terrain_at(x + spiral_x[i], y + spiral_y[i]);
            if (terrain == 10 && (map_region(x + spiral_x[i], y + spiral_y[i]) > 256 || player == civnow)) {
                townbuf[city].status |= 2;
            }
            if (terrain == 11 || terrain == 5) {
                townbuf[city].status |= 8;
            }
        }

        if (is_first && player == civnow) {
            notice_a("*FIRSTPRODUCT");
        }

        for (i = 0; i < 45; i++) {
            scan_x = wrap_x(x + spiral_x[i]);
            scan_y = y + spiral_y[i];
            if (i <= 20) {
                map_claim(scan_x + 80, scan_y, player, 2);
            } else {
                slot = help_probe(2, scan_x + 80, scan_y);
                if (slot >= 8) {
                    map_claim(scan_x + 80, scan_y, 8, 2);
                }
            }
            if (player != civnow && i <= 24) {
                if (map_flags(scan_x, scan_y) & 1) {
                    if (map_city(scan_x, scan_y) == civnow) {
                        townbuf[city].visible_size = 1;
                        map_mark(x, y);
                    }
                }
            }
        }

        if (player == civnow) {
            if (debug & 8) {
                if (disk_insert(1, 0, 0)) {
                    city_founded(city);
                }
            }
            kill(player, city);
            sel_player();
        } else {
            townbuf[city].production = find_slot(player, city);
            if (civ_wonder(civnow, 19)) {
                civ_vis_grid[townbuf[city].x * 50 + townbuf[city].y] |= 1 << civnow;
                townbuf[city].visible_size = 1;
                map_mark(townbuf[city].x, townbuf[city].y);
            }
        }
        return city;
    }

    if (player == civnow) {
        advisor = 4;
        msgtext[0] = 0;
        named_msg("*PMAX");
        show_msg(msgtext, 64, 80);
    }
    return -1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

#undef scan_x
#undef scan_y
#undef is_first
#undef city
#undef flag
#undef i
#undef terrain
#undef slot
#undef ptr

/* --- Function 2: [0540, 0CA9) overlay20_unit_event --- */
#define city_food(c) (townbuf[c].food_box)
#define city_shields(c) (townbuf[c].shield_box)

#define other_city ao
#define dist ap
#define best_weight aa
#define weight ab
#define i ac
#define neighborhood ad
#define best_city ae
#define building af

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void city_disaster(int city)
{
    int other_city;
    int dist;
    int best_weight;
    int weight;
    int i;
    int neighborhood;
    int best_city;
    int building;

    neighborhood = 0;
    for (i = 0; i <= 8; i++) {
        neighborhood |= 1 << terrain_at(townbuf[city].x + spiral_x[i], townbuf[city].y + spiral_y[i]);
    }

    msgtext[0] = 0;
    append_place_name(city);
    strcpy(message_slot, msgtext);
    msgtext[0] = 0;

    switch (rand_scaled(10)) {
    case 0:
        if (neighborhood & 0x10) {
            if (townbuf[city].buildings & 0xfffe) {
                do {
                    building = rand_scaled(24);
                } while (building + 1 == 1 || !(townbuf[city].buildings & (long) (1 << building)));
                townbuf[city].buildings &= (long) ~(1 << building);
                strcpy(msgtext, "Earthquake in ");
                append_place_name(city);
                strcat(msgtext, "!\n");
                strcat(msgtext, building_names[building].name);
                strcat(msgtext, " destroyed.\n");
            }
        }
        break;

    case 1:
        if (!has_tech(townbuf[city].owner, 10)) {
            if (!(townbuf[city].buildings & 0x100)) {
                if (has_tech(townbuf[city].owner, improvements[9].prereq)) {
                    named_msg("*D1");
                    if (townbuf[city].size != 0) {
                        townbuf[city].size -= townbuf[city].size / 4;
                    }
                }
            }
        }
        break;

    case 2:
        if (neighborhood & 0x800) {
            if (!(townbuf[city].buildings & 0x80)) {
                if (has_tech(townbuf[city].owner, improvements[8].prereq)) {
                    named_msg("*D2");
                    if (townbuf[city].size > 1) {
                        townbuf[city].size -= townbuf[city].size / 4;
                    }
                }
            }
        }
        break;

    case 3:
        if (neighborhood & 0x20) {
            if (!(townbuf[city].buildings & 0x8)) {
                if (has_tech(townbuf[city].owner, improvements[4].prereq)) {
                    named_msg("*D3");
                    if (townbuf[city].size > 1) {
                        townbuf[city].size -= townbuf[city].size / 3;
                    }
                }
            }
        }
        break;

    case 4:
        if (townbuf[city].size != 0) {
            if (!(townbuf[city].buildings & 0x4)) {
                if (has_tech(townbuf[city].owner, improvements[3].prereq)) {
                    named_msg("*D4");
                    city_food(city) = 0;
                    townbuf[city].size -= townbuf[city].size / 3;
                    strcat(msgtext, "Citizens demand GRANARY.\n");
                }
            }
        }
        break;

    case 5:
        if (townbuf[city].buildings & 0xfffe) {
            if (!(townbuf[city].buildings & 0x100)) {
                if (has_tech(townbuf[city].owner, improvements[9].prereq)) {
                    do {
                        building = rand_scaled(24);
                    } while (building + 1 == 1 || !(townbuf[city].buildings & (long) (1 << building)));
                    townbuf[city].buildings &= (long) ~(1 << building);
                    strcpy(name_buf, building_names[building].name);
                    named_msg("*D5");
                }
            }
        }
        break;

    case 6:
        if (neighborhood & 0x400) {
            if (!(townbuf[city].buildings & 0x2)) {
                named_msg("*D6");
                city_food(city) = 0;
                city_shields(city) = 0;
            }
        }
        break;

    case 7:
    case 8:
    case 9:
        city_process(city, -1);
        msgtext[0] = 0;
        if (town_happy_ones_num < town_unhappy_counts) {
            switch (rand_scaled(3)) {
            case 0:
                strcpy(msgtext, "Rioting in ");
                break;
            case 1:
                strcpy(msgtext, "Corruption in ");
                break;
            case 2:
                strcpy(msgtext, "Scandal in ");
                break;
            }
            append_place_name(city);
            strcat(msgtext, "!\nCitizens demand ");
            if (!(townbuf[city].buildings & 0x8)) {
                strcat(msgtext, "TEMPLE\n");
            } else if (!(townbuf[city].buildings & 0x40)) {
                strcat(msgtext, "COURTHOUSE\n");
            } else if (!(townbuf[city].buildings & 0x10)) {
                strcat(msgtext, "MARKETPLACE\n");
            } else if (has_tech(townbuf[city].owner, 29) && !(townbuf[city].buildings & 0x400)) {
                strcat(msgtext, "CATHEDRAL\n");
            } else {
                strcat(msgtext, "lower taxes.\n");
            }
            if (human_set & (1 << townbuf[city].owner)) {
                show_msg(msgtext, 100, 80);
                note_map(townbuf[city].x, townbuf[city].y);
                msgtext[0] = 0;
            }
            city_food(city) = 0;
            city_shields(city) = 0;
            best_weight = 4;
            best_city = -1;
            if (town_happy_ones_num <= town_unhappy_counts) {
                if (!(townbuf[city].buildings & 1)) {
                    if (city_count[townbuf[city].owner] > 3) {
                        other_city = 0;
                        goto check_city;
                        do {
                            dist = map_dist(townbuf[city].x, townbuf[other_city].x, townbuf[city].y, townbuf[other_city].y);
                            city_process(other_city, -1);
                            weight = ((town_happy_ones_num - town_unhappy_counts) << 5) / dist;
                            if (weight > best_weight) {
                                best_weight = weight;
                                best_city = other_city;
                            }
                            other_city++;
check_city:                ;
                        } while (other_city < 128 && other_city != city && townbuf[other_city].status != -1);
                        if (best_city != -1) {
                            if (townbuf[best_city].owner != townbuf[city].owner) {
                                if (civ_vis_grid[townbuf[city].x * 50 + townbuf[city].y] & (1 << civnow)) {
                                    strcpy(msgtext, "Residents of ");
                                    append_place_name(city);
                                    strcat(msgtext, " admire\nthe prosperity of ");
                                    append_place_name(best_city);
                                    strcat(msgtext, ".\n");
                                    show_msg(msgtext, 80, 80);
                                }
                                city_revolt(city, townbuf[best_city].owner);
                                msgtext[0] = 0;
                            }
                        }
                    }
                }
            }
        }
        break;
    }

    if (msgtext[0] == 0)
        return;
    if (human_set & (1 << townbuf[city].owner)) {
        advisor = 4;
        show_msg(msgtext, 100, 80);
        note_map(townbuf[city].x, townbuf[city].y);
        return;
    }
    if (civ_vis_grid[townbuf[city].x * 50 + townbuf[city].y] & (1 << civnow)) {
        weight = 0;
        while (msgtext[weight++] != '\n');
        msgtext[weight] = 0;
        advisor = 2;
        show_msg(msgtext, 100, 64);
        reveal(civnow, townbuf[city].x - 10, townbuf[city].y - 6);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

#undef city_food
#undef city_shields
#undef other_city
#undef dist
#undef best_weight
#undef weight
#undef i
#undef neighborhood
#undef best_city
#undef building

/* --- Function 3: [0CA9, 13F8) overlay20_score --- */
#define restart ao
#define citizen_score ap
#define reserve1 aa
#define achievements_ok ab
#define saved_state ac
#define y ad
#define x ae
#define i af
#define line_y ag
#define j ah
#define peace_score ai
#define bonus_score aj

/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int civ_score(player, display)
int player;
int display;
{
    int restart;
    int citizen_score;
    int reserve1;
    int achievements_ok;
    int saved_state;
    int x;
    int i;
    int y;
    int j;
    int peace_score;
    int line_y;

    restart = 0;
    if (display)
        scrbuf->number = 1;
restart_score:
    if (display)
        advisor_open("CIVILIZATION SCORE", 3);
    civ_ratings[0] = 0;
    if (game_flags & 0x100) {
        draw_text("SCORING COMPLETED", 160, 100, 15);
        advisor_close();
        return 0;
    }

    achievements_ok = 1;
    line_y = 26;
    y = 26;
    x = 999;
    citizen_score = reserve1 = 0;
    icon_x_draw = 999;
    icon_y_draw = 26;
    saved_state = civ_toll[player];
    civ_toll[player] = 6 - sci[player];

    for (i = 0; i < 128; ++i) {
        if (townbuf[i].status != -1 && townbuf[i].owner == player) {
            city_process(i, -1);
            if (display) {
                for (j = 0; j < town_happy_ones_num; ++j)
                    place_icon(j & 1);
                for (j = 0; j < townbuf[i].size - town_happy_ones_num - town_unhappy_counts - city_experts_num; ++j)
                    place_icon((j & 1) + 2);
                for (j = 0; j < town_unhappy_counts; ++j)
                    place_icon((j & 1) + 4);
                for (j = 0; j < city_experts_num; ++j)
                    place_icon(get_specialist(j) + 5);
            }
            citizen_score += townbuf[i].size + town_happy_ones_num - town_unhappy_counts;
        }
    }
    civ_toll[player] = saved_state;
    merit_people = citizen_score;
    strcpy(msgtext, civ_adjectives[player]);
    strcat(msgtext, " Citizens (");
    strcat(msgtext, itoa(merit_people, nstr, 10));
    strcat(msgtext, ")");
    if (display)
        draw_shadowed(msgtext, 16, y, 15);

    line_y = icon_y_draw + 16;
    y = line_y;
    x = 999;
    marvel_points = 0;
    {
        register int city_id;
        for (i = 1; i <= 21; ++i) {
            city_id = marvel_site[i];
            if (city_id != -1 && city_id != 128 && townbuf[city_id].owner == player) {
                x += 152;
                if (x > 300) {
                    line_y += clamp(icon_row_step + 2, 8, 99);
                    x = 16;
                }
                j = i + 24;
                if (display) {
                    draw_icon(j, x, line_y - 2);
                    draw_string(achievement_names[i].name, x + 20, line_y, 11);
                }
                marvel_points += 20;
            }
            if (marvel_site[i] == -1)
                achievements_ok = 0;
        }
    }
    strcpy(msgtext, civ_adjectives[player]);
    strcat(msgtext, " Achievements (");
    strcat(msgtext, itoa(marvel_points, nstr, 10));
    strcat(msgtext, ")");
    if (display)
        draw_shadowed(msgtext, 16, y, 15);

    civ_ratings[0] = marvel_points + merit_people;
    line_y += 10;
    {
        register int ship;
        ship = space_ship_habitation[civnow];
        if (ship != 0 && ship_success != 0) {
            x = ship_success * ship / 2;
            if (x < 0)
                x = 0;
            strcpy(msgtext, "SpaceShip: (");
            strcat(msgtext, itoa(x, nstr, 10));
            if (spaceship_arrive_dates[civnow] == year) {
                civ_ratings[0] += x;
            } else {
                strcat(msgtext, " possible");
            }
            strcat(msgtext, ")");
            if (display)
                draw_shadowed(msgtext, 16, line_y, 15);
            line_y += 10;
        }
    }

    msgtext[0] = 0;
    if (polluted != 0) {
        strcpy(msgtext, "Pollution: (");
        strcat(msgtext, itoa(-10 * polluted, nstr, 10));
        strcat(msgtext, ")   ");
        civ_ratings[0] -= 10 * polluted;
    }
    if (year > 0 && player_peace_run != 0) {
        peace_score = clamp(3 * player_peace_run, 0, 100);
        strcat(msgtext, "Peace: (+");
        strcat(msgtext, itoa(peace_score, nstr, 10));
        strcat(msgtext, ") ");
        civ_ratings[0] += peace_score;
    }
    if (advances_future != 0) {
        strcat(msgtext, "Future Tech: (+");
        strcat(msgtext, itoa(5 * advances_future, nstr, 10));
        strcat(msgtext, ")");
        civ_ratings[0] += 5 * advances_future;
    }
    if (msgtext[0] && display && line_y < 184) {
        draw_shadowed(msgtext, 16, line_y, 15);
        line_y += 10;
    }

    if (complete_flag)
        civ_ratings[0] = civ_total * 100 + (550 - turn_tick) * 2 + 400;
    if (civ_ratings[0] < 0)
        civ_ratings[0] = 0;
    strcpy(msgtext, "Total Score: ");
    strcat(msgtext, itoa(civ_ratings[0], nstr, 10));
    if (display && line_y < 186)
        draw_shadowed(msgtext, 16, line_y, 15);
    if (line_y >= 186) {
        restart = 1;
        --icon_row_step;
        mouse_on();
        goto restart_score;
    }
    tally_final = civ_ratings[0];
    if (display) {
        rect_fill(scrbuf, 35, 192, 250, 7, 8);
        if (civ_ratings[0] > 0)
            rect_fill(scrbuf, 35, 192, clamp(civ_ratings[0] / 4, 0, 250), 7, 11);
        scrbuf->number = 0;
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
        ad_wait();
        mouse_on();
        sel_player();
    }
    return civ_ratings[0];
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

#undef restart
#undef citizen_score
#undef reserve1
#undef achievements_ok
#undef saved_state
#undef y
#undef x
#undef i
#undef line_y
#undef j
#undef peace_score
#undef bonus_score

/* --- Function 4: [13F8, 1444) overlay20_helper --- */
/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void place_icon(int index)
{
    icon_x_draw += 8;
    if (icon_x_draw > 0x124) {
        icon_y_draw += icon_row_step;
        icon_x_draw = ((icon_y_draw >> 1) & 4) + 0x10;
    }
    sprite_draw(scrbuf, icon_x_draw, icon_y_draw, city_glad_glyphs[index]);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int icon_row_step = 8;
