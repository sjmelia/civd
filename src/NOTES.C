#include <string.h>
#include "CIV.H"
#include "CAPTURE.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "RESIDENT.H"
#include "UNITS.H"
#include "NOTES.H"

struct terr_rec {
    char name[12];
    unsigned char move_cost;
    signed char defense, food, shields, trade, f5, f6;
};
struct city_rec {
    char pad[11];
    signed char owner;
    char rest[16];
};
struct player_rec {
    char pad[0x38];
};

extern struct terr_rec terrains[];
extern struct city_rec townbuf[];
extern struct unit armydata[8][128];

extern int civ_state[][28];
extern char debug;

extern char *strupr(char *);
extern int map_near();


/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void show_help(int civ)
{
    char buf[24];

    strcpy(buf, "*");
    strcat(buf, strupr(civ));
    msgtext[0] = 0;
    if (news_name("help.txt", buf)) {
        mouse_off();
        menu_begin();
        rect_fill(scrbuf, 0, 0, 0x140, 0xc8, 2);
        draw_text(buf, 0xa0, 0xc0, 0);
        news_effect(0x20, 0x20, 0x20, 0xf);
        ad_wait();
        menu_end();
        mouse_on();
    }
    edit_mode = 0;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void advice(int civ, int idx)
{
    int good_, city, j, k, flags_, terr;

    if (idx >= 128)
        return;
    terr = terrain_at(armydata[civ][idx].x, armydata[civ][idx].y);
    good_ = terrains[terr].food + terrains[terr].shields >= 2;
    j = help_probe(2, armydata[civ][idx].x + 0x50, armydata[civ][idx].y);
    if (j < 10)
        good_ = 0;
    for (k = 1; k <= 8; k++) {
        if (help_probe(2, armydata[civ][idx].x + spiral_x[k] + 0x50, armydata[civ][idx].y + spiral_y[k]) > j)
            good_ = 0;
    }
    if (good_) {
        reveal(civnow, armydata[civ][idx].x - 7, armydata[civ][idx].y - 6);
        notice_a("*BUILDCITY");
        return;
    }
    city = near_city(armydata[civ][idx].x, armydata[civ][idx].y);
    if (city != -1 && townbuf[city].owner == civ && town_gap == 1) {
        flags_ = map_flags(armydata[civ][idx].x, armydata[civ][idx].y);
        msgtext[0] = 0;
        append_place_name(city);
        strcpy(name_buf, msgtext);
        if (terr == 4 && !(flags_ & 6))
            notice_a("*MINING");
        if (terr == 1 && build_ok(armydata[civ][idx].x, armydata[civ][idx].y) && !(flags_ & 6))
            notice_a("*IRRIGATE");
        else if (terrains[terr].move_cost == 1 && !(flags_ & 8) && terr != 11 && forces_present[civ][0] < 2)
            notice_a("*ROAD");
    }
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void notice_a(char *topic)
{
    int n;

    if ((debug & 1) && year < 0) {
        msgtext[0] = 0;
        n = news_name("help.txt", topic);
        help_open();
        mouse_off();
        menu_begin();
        rect_fill(scrbuf, 0x38, 0x10, 0xd0, n * 6 + 10, 2);
        draw_box(0x38, 0x10, 0xd0, n * 6 + 10, 10);
        scrbuf->font = 2;
        draw_text("--- Civilization Note ---", 0xa0, 0x13, 0);
        news_effect(0x20, 0x40, 0x19, 0xf);
        scrbuf->font = 1;
        ad_wait();
        menu_end();
        mouse_on();
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void notice_b(char *topic)
{
    int n;

    if (civ_id == civnow) {
        msgtext[0] = 0;
        n = news_name("error.txt", topic);
        help_open();
        mouse_off();
        menu_begin();
        rect_fill(scrbuf, 0x38, 0x10, 0xd0, n * 6 + 10, 4);
        draw_box(0x38, 0x10, 0xd0, n * 6 + 10, 12);
        scrbuf->font = 2;
        draw_text("--- Civilization Note ---", 0xa0, 0x13, 0);
        news_effect(0x20, 0x40, 0x19, 0xf);
        scrbuf->font = 1;
        ad_wait();
        menu_end();
        mouse_on();
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */
