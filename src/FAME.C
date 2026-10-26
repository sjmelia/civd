/* Overlay 3: Hall of Fame. */
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include "CIV.H"
#include "CORE.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "INTRO.H"
#include "RESIDENT.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "DRAW.H"
#include "FAME.H"
#include "UNITS.H"
#include "DIALOG.H"
#include "DRIVER.H"

struct fame {
    int score, level, f4, year, f8, f10, f12;
    char leader[16];
    char name[16];
};

extern struct city townbuf[];
extern struct fame far fame_ranks[];
extern char far hall_text[][25];

extern int open();
extern int read(), write(), close();




void clear_fame_records(void)
{
    int i, n;

    for (i = 0; i < 6; i++)
        fame_ranks[i + 1].score = -1;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void fame_load(void)
{
    int file;

    clear_fame_records();
    file = open("fame.dta", 0x8102, 0x80);
    read(file, msgtext, 0x114);
    copy_to_far(msgtext, (char far *) &fame_ranks[1], 0x114);
    close(file);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void fame_save(void)
{
    int file;

    file = open("fame.dta", 0x8302, 0x80);
    copy_from_far((char far *) &fame_ranks[1], msgtext, 0x114);
    write(file, msgtext, 0x114);
    close(file);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int show_fame(int who)
{
    int unused, x;
    int i, y;
    int tail, code;
    int scores[8];

    rect_fill(scrbuf, 0, 0, 320, 200, 15);
    draw_text("CIVILIZATION", 160, 16, 0);
    draw_text("HALL OF FAME", 160, 24, 0);
    draw_line(80, 32, 240, 32, 14);
    y = 40;
    for (i = 0; i < 6; i++) {
        if (i != who && i == 5)
            break;
        if (who == 5 && i == 4)
            continue;
        x = 8;
        scores[i] = fame_ranks[i + 1].score;
        if (fame_ranks[i + 1].score <= 0) {
            strcpy(msgtext, " ---");
            draw_string(msgtext, 8, y, 0);
            y += 26;
            continue;
        }
        msgtext[0] = 0;
        strcat(msgtext, itoa(i + 1, nstr, 10));
        strcat(msgtext, ". ");
        copy_from_far(fame_ranks[i + 1].leader, msgtext + strlen(msgtext), 16);
        strcat(msgtext, ", ");
        strcat(msgtext, rank_title[fame_ranks[i + 1].level]);
        strcat(msgtext, " of the ");
        copy_from_far(fame_ranks[i + 1].name, msgtext + strlen(msgtext), 16);
        tail = strlen(msgtext);
        strcat(msgtext, " to ");
        strcat(msgtext, itoa(abs(fame_ranks[i + 1].year), nstr, 10));
        if (fame_ranks[i + 1].year < 0)
            strcat(msgtext, " BC.");
        else
            strcat(msgtext, " AD.");
        if (string_width(msgtext) + x >= 320)
            msgtext[tail] = 0;
        draw_string(msgtext, x, y, 0);
        y += 8;
        strcpy(msgtext, "Population: ");
        format_total(fame_ranks[i + 1].f12);
        strcat(msgtext, ", Score: ");
        strcat(msgtext, itoa(fame_ranks[i + 1].score, nstr, 10));
        switch (fame_ranks[i + 1].level) {
        case 0:
            strcat(msgtext, " (Chieftan)");
            break;
        case 1:
            strcat(msgtext, " (Warlord)");
            break;
        case 2:
            strcat(msgtext, " (Prince)");
            break;
        case 3:
            strcat(msgtext, " (King)");
            break;
        case 4:
            strcat(msgtext, " (Emperor)");
            break;
        }
        draw_string(msgtext, x + 12, y, 7);
        y += 8;
        strcpy(msgtext, "    --- CIVILIZATION RATING: ");
        strcat(msgtext, itoa((fame_ranks[i + 1].level + 1) * fame_ranks[i + 1].score / 50, nstr, 10));
        strcat(msgtext, "% ---");
        draw_string(msgtext, x + 12, y, 12);
        y += 8;
        y += 2;
        if (i == who)
            recolour_rect(scrbuf, x - 4, y - 28, 296, 26, 15, 14);
    }
    if (who != -1) {
        draw_string("C\203lear", 252, 189, 0);
        draw_box(236, 187, 60, 10, 11);
    }
    code = ad_wait();
    return who != -1 && (code == 0x63 || code == 0x43);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int fame_insert(void)
{
    int unused;
    int i, j;
    struct fame far *dst, far * old;

    unused = 0;
again:
    for (i = 0; i < 6; i++) {
        if ((fame_ranks[i + 1].level + 2) * fame_ranks[i + 1].score < (cur_tier + 2) * tally_final || i == 5) {
            for (j = 5; j > i; j--) {
                dst = &fame_ranks[j + 1];
                old = &fame_ranks[j];
                movedata(FP_SEG(old), FP_OFF(old), FP_SEG(dst), FP_OFF(dst), 46);
            }
            fame_ranks[i + 1].score = tally_final;
            copy_to_far(leader_names[civnow], (char far *) fame_ranks[i + 1].leader, 14);
            copy_to_far(civ_names[civnow], (char far *) fame_ranks[i + 1].name, 16);
            fame_ranks[i + 1].level = cur_tier;
            fame_ranks[i + 1].f4 = ratio_score;
            fame_ranks[i + 1].f8 = marvel_points;
            fame_ranks[i + 1].f10 = merit_people;
            fame_ranks[i + 1].year = year;
            fame_ranks[i + 1].f12 = count_weighted_units(civnow);
            if (show_fame(i)) {
                clear_fame_records();
                goto again;
            }
            return i;
        }
    }
    return -1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int fame_slot = -1;

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void fame_header(int rank, int h)
{
    int t, i;
    int j, m, n;

    if (h) {
        t = h * (cur_tier + 1) / 50;
        rank = -1;
        for (i = 1; i <= 20; i++) {
            if (i * i / 3 < t)
                rank = i - 1;
        }
    }
    if (rank < 0)
        return;
    if (rank > 19)
        rank = 19;
    mouse_off();
    menu_begin();
    sound(34);
    rect_fill(scrbuf, 0, 0, 320, 200, 15);
    menu_clear();
    blit(tile_page, 200, 140, 40, 60, scrbuf, 10, 10);
    blit(tile_page, 240, 140, 40, 60, scrbuf, 270, 10);
    scrbuf->font = 7;
    draw_backdrop(80, 4, 160, 27);
    draw_panel(80, 4, 159, 26, 11, 8);
    if (h == 0) {
        copy_from_far(hall_text[20], msgtext, 32);
        draw_text(msgtext, 160, 6, 15);
        copy_from_far(hall_text[21], msgtext, 32);
        draw_text(msgtext, 160, 14, 15);
        copy_from_far(hall_text[19 - rank], msgtext, 32);
        strcat(msgtext, "!");
    } else {
        copy_from_far(hall_text[22], msgtext, 32);
        draw_text(msgtext, 160, 6, 15);
        copy_from_far(hall_text[23], msgtext, 32);
        strcat(msgtext, itoa(t, nstr, 10));
        strcat(msgtext, "% exceeds even");
        draw_text(msgtext, 160, 14, 15);
        copy_from_far(hall_text[19 - rank], msgtext, 32);
        strcat(msgtext, "!");
    }
    draw_text(msgtext, 160, 22, 15);
    for (i = 0; i <= rank; i++) {
        copy_from_far(hall_text[19 - i], msgtext, 32);
        draw_text(msgtext, 160, 184 - 8 * i, i == rank ? 0 : 7);
    }
    recolour_rect(scrbuf, 80, 183 - 8 * rank, 160, 8, 15, 14);
    ad_wait();
    sound(1);
    scrbuf->font = 1;
    menu_end();
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void top_five(void)
{                               /* top five cities */
    int top_score[5];
    int j, i, city;
    int city_rank[5];
    int total;
    int top, xx;

    for (i = 0; i < 5; i++)
        top_score[i] = city_rank[i] = -1;
    for (city = 0; city < 128; city++) {
        if (townbuf[city].status == -1)
            continue;
        city_process(city, -1);
        total = townbuf[city].size + town_happy_ones_num - town_unhappy_counts;
        for (i = 1; i <= 21; i++)
            if (marvel_site[i] == city)
                total += 10;
        for (i = 0; i < 5; i++) {
            if (top_score[i] < total) {
                for (j = 3; j >= i; j--) {
                    top_score[j + 1] = top_score[j];
                    city_rank[j + 1] = city_rank[j];
                }
                top_score[i] = total;
                city_rank[i] = city;
                break;
            }
        }
    }
    mouse_off();
    menu_begin();
    rect_fill(scrbuf, 0, 0, 320, 200, 3);
    draw_shadowed("The Top Five Cities in the World", 80, 12, 15);
    top = 32;
    for (i = 0; i < 5; i++) {
        if (top_score[i] == -1)
            continue;
        city = city_rank[i];
        msgtext[0] = 0;
        strcat(msgtext, itoa(i + 1, nstr, 10));
        strcat(msgtext, ". ");
        append_place_name(city);
        strcat(msgtext, " (");
        strcat(msgtext, civ_adjectives[townbuf[city].owner]);
        strcat(msgtext, ")");
        draw_text(msgtext, 160, top + 3, civ_colours[townbuf[city].owner]);
        city_process(city, -1);
        xx = 48;
        draw_city_view(city, 48, top + 10, city_experts_num, 160);
        xx += clamp(townbuf[city].size << 3, 0, 160) + 8;
        for (j = 1; j <= 21; j++) {
            if (marvel_site[j] == city) {
                if (xx < 293)
                    draw_icon(j + 24, xx, top + 12);
                xx += 19;
            }
        }
        draw_box(8, top, 303, 25, civ_colours[townbuf[city].owner]);
        top += 32;
    }
    ad_wait();
    menu_end();
    mouse_on();
}

/* Hall of Fame leaders and messages; original far-data block. */
char far hall_text[24][25] = {
    "Solomon the Wise",
    "Emperor Augustus",
    "King Charlemagne",
    "Thomas Jefferson",
    "Shogun Tokugawa",
    "Franklin Roosevelt",
    "Sulayman the Magnificent",
    "Winston Churchill",
    "Cleopatra",
    "Charles De Gaulle",
    "Vladimir Lenin",
    "Simon Bolivar",
    "Otto von Bismarck",
    "Eric the Red",
    "Kaiser Wilhelm",
    "Louis the XVI",
    "Neville Chamberlain",
    "Ferdinand Marcos",
    "Emperor Nero",
    "Dan Quayle",
    "Sire, your fame now",
    "exceeds even that of",
    "Sire, your Civilization",
    "Rating of "
};
