/* Overlay 13: civilization profiles, unit tables and relations chart. */
#include <string.h>
#include <stdlib.h>
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "CAPTURE.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "STATUS.H"
#include "UNITS.H"
#include "ECONOMY.H"

struct unit_type {
    char name[12];
    int fields[11];
};

extern struct unit_type utypes[];
extern unsigned char far advance_owners[][2];
extern int treaties[8][8];

extern void label_with_style(int, int, int);

void civ_info(int p)
{                               /* civilization profile (debug) */
    int i, y, j, z, k;
    char flags;

    mouse_off();
    blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
    rect_fill(scrbuf, 0, 0, 320, 200, 1);
    draw_string(civ_adjectives[p], 128, 4, 15);
    label_with_style(p * 32 + 64, 16, 4);
    strcpy(msgtext, "Government: ");
    strcat(msgtext, government_names[civ_gov[p]]);
    draw_string(msgtext, 16, 24, 15);
    strcpy(msgtext, "Population: ");
    count_weighted_units(p);
    draw_string(msgtext, 16, 32, 15);
    strcpy(msgtext, "Treasury: $");
    strcat(msgtext, itoa(money[p], nstr, 10));
    strcat(msgtext, ", Taxes/Science: ");
    strcat(msgtext, itoa(civ_toll[p], nstr, 10));
    strcat(msgtext, "/");
    strcat(msgtext, itoa(sci[p], nstr, 10));
    draw_string(msgtext, 16, 40, 15);
    draw_string("/Units/Cities/Settlers/Land", 16, 48, 15);
    msgtext[0] = 0;
    strcat(msgtext, itoa(troops_total[p], nstr, 10));
    strcat(msgtext, "/");
    strcat(msgtext, itoa(city_count[p], nstr, 10));
    strcat(msgtext, "/");
    strcat(msgtext, itoa(civ_settlers_n[p], nstr, 10));
    strcat(msgtext, "/");
    strcat(msgtext, itoa(civ_held_cells[p], nstr, 10));
    strcat(msgtext, "/");
    draw_string(msgtext, 192, 48, 15);
    y = 4;
    for (i = 1; i < 15; i++) {
        if (!defenders_island[p][i])
            continue;
        strcpy(msgtext, "Settle");
        if (civ_area_plan[p][i] == 2)
            strcpy(msgtext, "Defend");
        if (civ_area_plan[p][i] == 1)
            strcpy(msgtext, "Attack");
        if (civ_area_plan[p][i] == 5)
            strcpy(msgtext, "Transport");
        strcat(msgtext, "/");
        strcat(msgtext, itoa(i, nstr, 10));
        draw_string(msgtext, 256, y, 7);
        y += 8;
    }
    y = 24;
    for (i = 1; i < 8; i++) {
        if (i == p)
            continue;
        strcpy(msgtext, civ_adjectives[i]);
        if ((treaties[p][i] & 0xb) == 1)
            strcat(msgtext, ": Contact");
        flags = treaties[p][i];
        if (flags & 2)
            strcat(msgtext, (flags & 4) ? ": Allied" : ": Peace");
        if (treaties[p][i] & 8)
            strcat(msgtext, ": Vendetta");
        if (treaties[p][i] & 0xb) {
            draw_string(msgtext, 192, y, 7);
            y += 8;
        }
    }
    y = 56;
    k = 0;
    draw_string("Civilization:", 16, y, 15);
    y += 8;
    for (i = 0; i < 72; i++) {
        if (has_tech(p, i)) {
            strcpy(msgtext, advances[i].name);
            draw_string(msgtext, k % 3 * 100 + 8, y, (advance_owners[i][0] & 7) == p ? 15 : 7);
            k++;
            if (k % 3 == 0)
                y += 8;
            if (y > 192)
                break;
        }
    }
    ad_wait();
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    mouse_on();
}

void unit_table(void)
{
    int present, other, civ, pad, screen_y, column, shown;

    mouse_off();
    blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
    panel_frame(0, 0, 317, 197, 1);
    shown = 0;
    screen_y = 18;
    for (column = 0; column < 28; column++) {
        present = 0;
        for (civ = 1; civ < 8; civ++)
            if (forces_present[civ][column] || civ_units_ordered[civ][column])
                present = 1;
        if (present) {
            draw_line(36, screen_y - 1, 319, screen_y - 1, 9);
            label_with_style(column + 96, (shown++ & 1) * 16 + 2, screen_y - 4);
            draw_string(utypes[column].name, 36, screen_y, 15);
            screen_y += 8;
        }
    }
    for (column = 0; column < 5; column++);
    for (civ = 0; civ < 8; civ++) {
        msgtext[0] = 0;
        strcat(msgtext, itoa(nation_ratings[civ], nstr, 10));
        if (xray_on)
            draw_string(msgtext, civ * 30 + 84, 1, 15);
        label_with_style(civ * 32 + 64, civ * 30 + 90, 2);
        if (civ != civnow && !xray_on)
            continue;
        screen_y = 18;
        for (column = 0; column < 28; column++) {
            present = 0;
            for (other = 1; other < 8; other++)
                if (forces_present[other][column] || civ_units_ordered[other][column])
                    present = 1;
            if (!present)
                continue;
            if (forces_present[civ][column] || civ_units_ordered[civ][column]) {
                strcpy(msgtext, "");
                strcat(msgtext, itoa(forces_present[civ][column], nstr, 10));
                strcat(msgtext, "/");
                strcat(msgtext, itoa(civ_units_ordered[civ][column], nstr, 10));
                draw_string(msgtext, civ * 30 + 88, screen_y, 15);
            }
            screen_y += 8;
        }
    }
    ad_wait();
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    mouse_on();
    kill_table();
}

void kill_table(void)
{
    int count, i, j, dummy;

    mouse_off();
    menu_begin();
    rect_fill(scrbuf, 0, 0, 320, 200, 4);
    for (i = 1; i < 8; i++) {
        draw_shadowed(civ_adjectives[i], i * 40, 8, civ_colours[i]);
        label_with_style(i * 32 + 64, i * 40, 16);
        draw_shadowed(civ_adjectives[i], 4, i * 12 + 32, civ_colours[i]);
    }
    for (i = 1; i < 8; i++) {
        for (j = 1; j < 8; j++) {
            if (i == j)
                continue;
            if (!xray_on && i != civnow && j != civnow)
                continue;
            msgtext[0] = 0;
            strcat(msgtext, itoa(killed[i][j], nstr, 10));
            strcat(msgtext, "/");
            strcat(msgtext, itoa(killed[j][i], nstr, 10));
            draw_string(msgtext, i * 40, j * 12 + 32, civ_colours[i]);
        }
    }
    ad_wait();
    menu_end();
    mouse_on();
}
