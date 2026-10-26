/* Game, unit orders, advisor, world and Civilopedia menus. */
#include <string.h>
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "ECONOMY.H"
#include "MAP.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "SPACESHP.H"
#include "ADVISORS.H"
#include "FAME.H"
#include "GAME.H"
#include "MENUS.H"
#include "PEDIA.H"
#include "REPLAY.H"
#include "REPORTS.H"
#include "TURN.H"
#include "UNITS.H"
#include "CITY.H"
#include "STATUS.H"

struct unit12 {
    unsigned char x, y;
    char type;
    char pad[6];
    char cargo;
    char pad2[2];
};
struct terr_fx {
    int irrigate, a, mine, b, c, d;
};
struct unit_type {
    char name[12];
    int obsolete_by, domain, moves, fuel, attack, defense, cost, sight, capacity, role, prereq;
};
struct terrain_rec {
    char name[12];
    unsigned char f0, f1, f2, f3, f4, f5, f6;
};

extern int debug;
extern char armydata[];
#define unit_tab ((struct unit12 (*)[128]) (armydata + 1))      /* units from x */
extern struct terr_fx work_rules[];
extern struct unit_type utypes[];
extern struct terrain_rec terrains[];




void cmd_menu(int x, int y, int sel)
{
    cmd_code = -1;
    menu_result = 0;
    if (sel == -1)
        sel = clamp(ptr_x / 60, 0, 4);
    mouse_off();
    menu_begin();
    mouse_on();
    switch (sel) {
    case 0:
        game_menu_a();
        break;
    case 1:
        unit_orders_menu(x, y);
        break;
    case 2:
        advisor_menu();
        break;
    case 3:
        world_menu();
        break;
    case 4:
        civilopedia_menu();
        break;
    }
    mouse_off();
    if (menu_result == 1L)
        sel_player();
    if (menu_result == 0L)
        menu_end();
    mouse_on();
}

void game_menu_a(void)
{
    int choice_h, unused_a_o, unused_b_m;

    if (!turn_tick)
        dialog_dim = 0x10;
    strcpy(msgtext, " Tax Rate\n Luxuries Rate\n FindCity\n Options\n Save Game\n REVOLUTION!\n \n Retire\n QUIT to DOS\n");
    if (game_flags & 0x100)
        strcat(msgtext, " View Replay\n");
    choice_h = menu_choose(msgtext, 0x10, 8);
    repaint();
    switch (choice_h) {
    case 5:
        cmd_code = -2;
        break;
    case 0:
        cmd_code = 0x3d;
        break;
    case 1:
        cmd_code = 0x2d;
        break;
    case 2:
        cmd_code = 0x3f;
        break;
    case 3:
        strcpy(msgtext, "Options:\n Instant Advice\n AutoSave\n End of Turn\n Animations\n Sound\n Enemy Moves\n Civilopedia Text\n Palace\n");
        do {
            marked_mask = debug;
            choice_h = menu_choose(msgtext, 0x18, 0x10);
            if (choice_h != -1 && !edit_mode)
                debug ^= 1 << choice_h;
            if (choice_h != -1)
                editing = choice_h;
        } while (choice_h != -1 || edit_mode);
        break;
    case 4:
        cmd_code = 0x53;
        break;
    case 7:
        leave = 2;
q:     cmd_code = 0x1000;
        break;
    case 8:
        leave = 1;
        goto q;
    case 9:
        replay();
        break;
    }
}

void unit_orders_menu(int p, int u)
{
    int pos, ox, oy, key, ret, tileflags, ter;
    char ty;
    char keybuf[32];

    if (u == -1 || u == 0x80)
        return;
    pos = 0;
    msgtext[0] = 0;
    ox = unit_tab[p][u].x;
    oy = unit_tab[p][u].y;
    tileflags = map_flags(ox, oy);
    ter = terrain_at(ox, oy);
    strcat(msgtext, " No Orders \217space\n");
    keybuf[pos++] = ' ';
    dialog_dim = 0;
    if (unit_tab[p][u].type == 0) {
        strcat(msgtext, (tileflags & 1) ? " Add to City \217b\n" : " Found New City \217b\n");
        keybuf[pos++] = 'b';
        if (!(tileflags & 8)) {
            strcat(msgtext, " Build Road \217r\n");
            keybuf[pos++] = 'r';
        } else if (!(tileflags & 0x10)) {
            if (has_tech(p, 0x31)) {
                strcat(msgtext, " Build RailRoad \217r\n");
                keybuf[pos++] = 'r';
            }
        }
        if (!(tileflags & 2)) {
            if (work_rules[ter].irrigate == -2) {
                strcat(msgtext, " Build Irrigation");
                if (!build_ok(ox, oy))
                    dialog_dim |= 1 << pos;
            } else {
                if (work_rules[ter].irrigate >= 0) {
                    strcat(msgtext, " Change to ");
                    strcat(msgtext, terrains[terrain_map[work_rules[ter].irrigate]].name);
                }
            }
            if (work_rules[ter].irrigate != -1) {
                keybuf[pos++] = 'i';
                strcat(msgtext, " \217i\n");
            }
        }
        if (!(tileflags & 4)) {
            if (work_rules[ter].mine <= -2)
                strcat(msgtext, " Build Mines");
            else {
                if (work_rules[ter].mine >= 0) {
                    strcat(msgtext, " Change to ");
                    strcat(msgtext, terrains[terrain_map[work_rules[ter].mine]].name);
                }
            }
            if (work_rules[ter].mine != -1) {
                keybuf[pos++] = 'm';
                strcat(msgtext, " \217m\n");
            }
        }
        if (tileflags & 0x40) {
            strcat(msgtext, " Clean up Pollution \217p\n");
            keybuf[pos++] = 'p';
        }
    }
    if (unit_tab[p][u].type == 0) {
        strcat(msgtext, " Build Fortress \217f\n");
        if (!has_tech(p, 0x2d))
            dialog_dim |= 1 << pos;
    } else if (utypes[unit_tab[p][u].type].domain == 0) {
        strcat(msgtext, " Fortify \217f\n");
    }
    if (utypes[unit_tab[p][u].type].domain == 0)
        keybuf[pos++] = 'f';
    strcat(msgtext, " Wait \217w\n Sentry \217s\n GoTo\n");
    keybuf[pos++] = 'w';
    keybuf[pos++] = 's';
    keybuf[pos++] = 'g';
    if (tileflags & 0x1e) {
        ty = unit_tab[p][u].type;
        if (ty < 0x1a && ty != 0xe) {
            strcat(msgtext, " Pillage \217P\n");
            keybuf[pos++] = 'P';
        }
    }
    if (tileflags & 1) {
        strcat(msgtext, " Home City \217h\n");
        keybuf[pos++] = 'h';
    }
    ty = unit_tab[p][u].type;
    if (utypes[ty].role == 5 || ty == 0x17) {
        if (unit_tab[p][u].cargo != -1) {
            strcat(msgtext, " Unload \217u\n");
            keybuf[pos++] = 'u';
        }
    }
    strcat(msgtext, " \n Disband Unit \217D\n");
    keybuf[pos++] = 0;
    keybuf[pos++] = 'D';
    ret = menu_choose(msgtext, 0x48, 8);
    if (ret >= 0 && ret < pos)
        cmd_code = keybuf[ret];
    else
        cmd_code = -1;
}

void advisor_menu(void)
{
    int unused_word_c, choice_h;

    strcpy(msgtext, " City Status (F1)\n Military Advisor (F2)\n Intelligence Advisor (F3)\n Attitude Advisor (F4)\n");
    strcat(msgtext, " Trade Advisor (F5)\n Science Advisor (F6)\n");
    choice_h = menu_choose(msgtext, 0x70, 8);
    mouse_off();
    menu_end();
    mouse_on();
    menu_result = -1;
    switch (choice_h) {
    case 4:
        trade_rpt(civnow);
        break;
    case 1:
        if (xray_on)
            unit_table(civnow);
        else
            mil_rpt(civnow);
        break;
    case 2:
        spy_rpt(civnow);
        break;
    case 3:
        mood_rpt(civnow);
        break;
    case 0:
        city_rpt(civnow);
        break;
    case 5:
        sci_rpt(civnow);
        break;
    }
}

void world_menu(void)
{
    int unused_word_c, choice_h;

    if (!(game_flags & 0xfe00))
        dialog_dim = 0x20;
    strcpy(msgtext, " Wonders of the World (F7)\n Top 5 Cities (F8)\n Civilization Score (F9)\n");
    strcat(msgtext, " World Map (F10)\n Demographics\n SpaceShips\n");
    choice_h = menu_choose(msgtext, 0x90, 8);
    mouse_off();
    menu_end();
    mouse_on();
    menu_result = -1;
    switch (choice_h) {
    case 0:
        wonders();
        break;
    case 4:
        demographics(civnow);
        break;
    case 2:
        civ_score(civnow, 1L);
        break;
    case 1:
        top_five();
        break;
    case 3:
        world_map(1L);
        break;
    case 5:
        spaceship_inspect();
        break;
    }
}

void civilopedia_menu(void)
{
    int choice_h;

    strcpy(msgtext, " Complete\n Civilization Advances\n City Improvements\n Military Units\n Terrain Types\n");
    strcat(msgtext, " Miscellaneous\n");
    choice_h = menu_choose(msgtext, 0xb6, 8);
    if (choice_h != -1) {
        civilopedia(0, choice_h - 1);
        menu_result = -1;
    } else
        menu_result = 1;
}
