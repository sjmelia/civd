/* Overlay 10: world map and unit-roster report. */
#include "CIV.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "MAPVIEW.H"
#include "UNITS.H"
#include "CAPTURE.H"
#include "DRAW.H"
#include "MAP.H"
#include "TURN.H"

extern char far map_visible_to[];
extern struct unit_type utypes[];
extern struct unit armydata[8][128];
extern int row_flag[8][16];

extern int terrain_at(int, int), map_owner(int, int), map_flags(int, int);

extern int getch(void);
extern char *itoa(int, char *, int), *strcpy(char *, char *), *strcat(char *, char *);
extern int strlen();

void view_army(void)
{
    int i;
    int sel;

    strcpy(msgtext, "Which army?\n ");
    for (i = 1; i < 8; i++) {
        strcat(msgtext, civ_adjectives[i]);
        strcat(msgtext, "\n ");
    }
    sel = show_msg(msgtext, 100, 80) + 1;
    menu_begin();
    map_report(sel);
    menu_end();
}

int map_report(int civ)
{
    int g;
    int x;
    int k;
    int tx;
    int cy;
    int known;
    int ty;
    int terr;
    int col;
    unsigned char tcode;

    mouse_off();
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    known = 1 << civ;
    for (cy = 0; cy < 50; cy++) {
        for (x = 0; x < 80; x++) {
            if (!(map_visible_to[50 * x + cy] & known) && !xray_on)
                continue;
            tx = x * 4;
            ty = cy * 4;
            terr = terrain_at(x, cy);
            rect_fill(scrbuf, tx, ty, 4, 4, terr == 10 ? 1 : 2);
            if (map_flags(x, cy) & 1) {
                col = civ_colours[map_city(x, cy)];
                rect_fill(scrbuf, tx, ty, 4, 4, col);
            } else if (map_owner(x, cy) != -1) {
                col = civ_colours[map_owner(x, cy)];
                rect_fill(scrbuf, tx + 1, ty + 1, 3, 3, 0);
                rect_fill(scrbuf, tx, ty, 3, 3, col);
            }
        }
    }
    getch();
    scrbuf->font = 2;
    for (k = 0; k < 32; k++) {
        if ((tcode = objectives_type[civ][k]) == 0xff)
            continue;
        switch ((char) tcode) {
        case 0:
            strcpy(msgtext, "S");
            break;
        case 1:
            strcpy(msgtext, "A");
            break;
        case 2:
            strcpy(msgtext, "D");
            break;
        case 3:
            strcpy(msgtext, "N");
            break;
        }
        draw_shadowed(msgtext, civ_targets_x[civ][k] * 4, civ_targets_y[civ][k] * 4, 13);
    }
    for (k = 0; k < 16; k++) {
        if ((tcode = spot_classes[civ][k]) == 0xff)
            continue;
        switch ((char) tcode) {
        case 0:
            strcpy(msgtext, "S");
            break;
        case 1:
            strcpy(msgtext, "A");
            break;
        case 2:
            strcpy(msgtext, "D");
            break;
        }
        draw_shadowed(msgtext, camp_xpos[civ][k] * 4, civ_lot_y[civ][k] * 4, 14);
    }
    getch();
    cy = 1;
    for (k = 1; k < 15; k++) {
        if (defenders_island[civ][k] == 0)
            continue;
        msgtext[0] = 0;
        strcat(msgtext, itoa(k, nstr, 10));
        strcat(msgtext, "/");
        if (civ_area_plan[civ][k] == 0)
            strcat(msgtext, "S");
        if (civ_area_plan[civ][k] == 2)
            strcat(msgtext, "D");
        if (civ_area_plan[civ][k] == 1)
            strcat(msgtext, "A");
        if (civ_area_plan[civ][k] == 5)
            strcat(msgtext, "T");
        strcat(msgtext, " ");
        strcat(msgtext, itoa(civ_cont_attackers[civ][k], nstr, 10));
        strcat(msgtext, "/");
        strcat(msgtext, itoa(defenders_island[civ][k], nstr, 10));
        draw_string(msgtext, 2, cy, 7);
        cy += 6;
    }
    getch();
    scrbuf->font = 1;
    mouse_on();
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int report_unit(int civ, int idx)
{
    int n;

    strcpy(msgtext, civ_adjectives[civ]);
    strcat(msgtext, "\n");
    strcat(msgtext, itoa(idx, nstr, 10));
    strcat(msgtext, utypes[armydata[civ][idx].type].name);
    strcat(msgtext, "\n");
    append_place_name(armydata[civ][idx].home_city);
    strcat(msgtext, "\n");
    strcat(msgtext, "Loc:");
    strcat(msgtext, itoa(armydata[civ][idx].x, nstr, 10));
    strcat(msgtext, ",");
    strcat(msgtext, itoa(armydata[civ][idx].y, nstr, 10));
    strcat(msgtext, "\n");
    if ((unsigned char) armydata[civ][idx].goto_x != 0xff) {
        strcat(msgtext, "To:");
        strcat(msgtext, itoa((unsigned char) armydata[civ][idx].goto_x, nstr, 10));
        strcat(msgtext, ",");
        strcat(msgtext, itoa((unsigned char) armydata[civ][idx].goto_y, nstr, 10));
        strcat(msgtext, "\n");
    }
    strcat(msgtext, "*");
    n = armydata[civ][idx].next;
    while (n != -1) {
        if (n == idx)
            break;
        if (strlen(msgtext) >= 0x200)
            break;
        strcat(msgtext, utypes[armydata[civ][n].type].name);
        strcat(msgtext, "\n*");
        n = armydata[civ][n].next;
    }
    show_msg(msgtext, 10, 10);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */
