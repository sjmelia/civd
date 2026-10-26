/* Overlay 8: Civilopedia. */
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "AUDIENCE.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "TURN.H"
#include "UNITS.H"
#include "MESSAGE.H"
#include "PEDIA.H"

struct terrain {
    char name[12];
    char move_cost, defense, food, shields, trade, f17, f18;
};

struct terrain_rule {
    int irrigate, irrigate_time, mine, mine_time, f8, f10;
};

extern int debug;
extern struct unit_type utypes[];
extern struct terrain terrains[];
extern struct terrain_rule work_rules[];
extern int far pedia_topics[];
extern int far topic_class[];
extern signed char far adv_source[][72];
extern signed char far pedia_icons[];


extern int kbhit(void);
extern char *strupr(char *);

int no_pictures = 0;            /* pedia picture disk absent: text only */
char *concepts[] = {
    "Veteran Units", "Fortify", "Irrigation", "Mining", "Roads",
    "RailRoads", "Anarchy", "Despotism", "Monarchy", "Communism",
    "Republic", "Democracy", "Fortress", "Pillage", "Disband",
    "Pollution", "Sentry", "Luxuries", "Taxes", "Science", "Trade Routes"
};

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void civilopedia(int unused, int category)
{
    int start, choice;

    cycle_off();
    mouse_off();
    start = 0;
    do {
        choice = pedia_list(start, category);
        if (choice == -1) {
            if (pedia_more)
                start += 78;
            else
                start = 0;
        }
    } while (choice != -2);
    disk_insert(0, 1L);
    sel_player();
    cycle_on();
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int pedia_list(int start, int category)
{
    int width;
    char label[24], low[24];
    int best_kind;
    char last[24];
    int x, count, selected, key, i, y, best_n;

    rect_fill(scrbuf, 0, 0, 320, 200, 14);
    strcpy(last, "a");
    rect_fill(scrbuf, 60, 2, 200, 9, 15);
    draw_text("ENCYCLOPAEDIA of CIVILIZATION", 161, 4, 0);
    draw_text("ENCYCLOPAEDIA of CIVILIZATION", 160, 3, 10);
    draw_string("EXIT", 286, 4, 12);
    if (category == -1)
        draw_string("MORE", 8, 4, 12);
    rect_fill(scrbuf, 2, 14, 316, 184, 15);
    width = category <= 0 ? 100 : 150;
    x = 10;
    y = 16;
    count = 0;
    do {
        strcpy(low, "zzz");
        best_n = -1;
        best_kind = -1;
        if (category == 0 || category == -1) {
            for (i = 0; i < 67; i++) {
                if (stricmp(advances[i].name, last) > 0 && stricmp(advances[i].name, low) < 0) {
                    best_n = i;
                    best_kind = 0;
                    strcpy(low, advances[i].name);
                }
            }
        }
        if (category == 1 || category == -1) {
            for (i = 1; i <= 45; i++) {
                if (stricmp(improvements[i].name, last) > 0 && stricmp(improvements[i].name, low) < 0) {
                    best_n = i;
                    best_kind = 1;
                    strcpy(low, improvements[i].name);
                }
            }
        }
        if (category == 2 || category == -1) {
            for (i = 0; i < 28; i++) {
                if (stricmp(utypes[i].name, last) > 0 && stricmp(utypes[i].name, low) < 0) {
                    best_n = i;
                    best_kind = 2;
                    strcpy(low, utypes[i].name);
                }
            }
        }
        if (category == 3 || category == -1) {
            for (i = 0; i < 12; i++) {
                if (stricmp(terrains[i].name, last) > 0 && stricmp(terrains[i].name, low) < 0) {
                    best_n = i;
                    best_kind = 3;
                    strcpy(low, terrains[i].name);
                }
            }
        }
        if (category == 4 || category == -1) {
            for (i = 0; i < 21; i++) {
                if (stricmp(concepts[i], last) > 0 && stricmp(concepts[i], low) < 0) {
                    best_n = i;
                    best_kind = 4;
                    strcpy(low, concepts[i]);
                }
            }
        }
        if (best_kind == -1)
            break;
        if (count >= start) {
            strcpy(label, low);
            truncate_text(label, width);
            draw_string(label, x, y, 0);
            pedia_topics[count - start] = best_n;
            topic_class[count - start] = best_kind;
            y += 7;
            if (y > 192) {
                x += width;
                y = 16;
            }
        }
        count++;
        strcpy(last, low);
    } while (x < 300);
    pedia_more = x >= 300 ? 1 : 0;
    if (has_mouse) {
        mouse_on();
        do
            poll_mouse();
        while (!mse_held && !kbhit());
        x = (ptr_x - 10) / width;
        if (ptr_y >= 16)
            y = (ptr_y - 16) / 7;
        else
            y = -1;
        selected = x * 26 + y;
        mouse_off();
        if (mse_held == 2) {
            y = -1;
            x = 2;
        }
        if (kbhit()) {
            key_cmd();
            y = -1;
            x = 2;
        }
    } else {
        x = y = 0;
        do {
            recolour_rect(scrbuf, x * width + 8, y * 7 + 15, width - 2, 8, 15, 10);
            key = key_cmd();
            recolour_rect(scrbuf, x * width + 8, y * 7 + 15, width - 2, 8, 10, 15);
            switch (key) {
            case 0x4800:
                if (y)
                    y--;
                else if (x) {
                    x--;
                    y = 25;
                }
                break;
            case 0x4D00:
                if (x * 26 + y + 26 < count - start)
                    x++;
                break;
            case 0x5000:
                if (x * 26 + y + 1 < count - start) {
                    if (y < 25 && x * 26 + y + 1 < count - start)
                        y++;
                    else {
                        y = 0;
                        x++;
                    }
                }
                break;
            case 0x4B00:
                if (x)
                    x--;
                break;
            }
            if (key == 'm')
                return -1;
            if (key == 27 || key == 'x')
                return -2;
            selected = x * 26 + y;
        } while (key != 13 && key != ' ');
    }
    if (y >= 0 && count - start <= selected)
        return 0;
    if (y >= 0) {
        if (which_data != 3)
            disk_insert(3, 1L);
        pedia_show(pedia_topics[selected], topic_class[selected]);
    } else
        selected = x ? -2 : -1;
    repaint();
    return selected;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

char blurb_file[] = "blurb0.txt";

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void pedia_show(int index, int kind)
{                               /* Civilopedia article */
    int back, unused, disk_ok, t, sprite, title_x, y0, i;
    char key[32];
    int y, pic;

    blurb_file[5] = kind + '0';
    cycle_off();
    mouse_off();
    disk_ok = -1;
    if (which_data != 3) {
        disk_ok = disk_insert(3, 0L);
        if (!disk_ok)
            no_pictures = 1;
    }
    back = 15;
    if (vga_modes) {
        if (no_pictures)
            back = 255;
        else {
            back = 253;
            set_dac(253, 61, 61, 61);
        }
    }
    rect_fill(scrbuf, 0, 0, 320, 200, back);
    menu_clear();
    scrbuf->font = 7;
    msgtext[0] = 0;
    title_x = 204;
    if (kind == 2)
        title_x = 224;
    if (kind == 4 || kind == 1 && index > 24 || no_pictures)
        title_x = 160;
    switch (kind) {
    case 0:
        if (!no_pictures) {
            pic = pedia_advance_index(index);
            strcpy(msgtext, "iconpg1.pic");
            msgtext[6] += pic / 9;
            load_screen(1, msgtext);
            restore_palette("iconpg1.pal");
            blit_centred(pic % 3 * 111 + 1, pic % 9 / 3 * 69 + 1, 110, 68, 62, 42);
        }
        strcpy(msgtext, advances[index].name);
        draw_text("Civilization Advance", title_x, 36, 7);
        break;
    case 1:
        if (!disk_insert(1, 0L))
            no_pictures = 1;
        if (index > 1 && index <= 21 && !no_pictures) {
            load_screen(1, "citypix2.pic");
            restore_palette("hill.pal");
            sprite_arena_init();
            sprite = cut_tile(1, (index - 2) / 4 * 50 + 1, ((index - 2) & 3) * 50 + 1, 49, 49);
            sprite_arena_release();
            sprite_draw(scrbuf, 37, 17, sprite);
            free_block(sprite);
        }
        strcpy(msgtext, improvements[index].name);
        draw_text(index <= 24 ? "City Improvement" : "Wonder of the World", title_x, 36, 7);
        break;
    case 2:
        if (!no_pictures) {
            pic = pedia_unit_index(index);
            strcpy(msgtext, "iconpga.pic");
            msgtext[6] += pic / 6;
            load_screen(1, msgtext);
            restore_palette("iconpga.pal");
            blit_centred((pic & 1) * 160 + 1, pic % 6 / 2 * 61, 158, 61, 88, 42);
        }
        strcpy(msgtext, utypes[index].name);
        draw_text("Military Unit", title_x, 36, 7);
        sprite_draw(scrbuf, 216, 48, colour_unit_glyph[32 + index]);    /* civ colour 1 */
        break;
    case 3:
        if (!no_pictures) {
            pic = pedia_terrain_index(index);
            strcpy(msgtext, "iconpgt1.pic");
            msgtext[7] += pic / 6;
            load_screen(1, msgtext);
            if (vga_modes) {
                load_palette("sp256.pal", icon_colours_block);
                apply_palette(icon_colours_block);
                set_dac(253, 61, 61, 61);
            }
            blit_centred(pic % 3 * 107 + 1, pic % 6 / 3 * 87 + 5, 106, 77, 76, 46);
        }
        strcat(msgtext, terrains[index].name);
        draw_text("Terrain Type", title_x, 36, 7);
        break;
    case 4:
        strcat(msgtext, concepts[index]);
        draw_text("Game Concept", title_x, 36, 7);
        break;
    }
    scrbuf->font = 6;
    draw_text(strupr(msgtext), title_x, 20, 0);
    scrbuf->font = 7;
    strcpy(key, "*");
    strcat(key, msgtext);
    msgtext[0] = 0;
    y = 76;
    if (disk_ok) {
        if ((debug & 0x40) && news_name(blurb_file, key) != -1) {
            news_effect(37, 12, 76, 1);
            ad_wait();
            rect_fill(scrbuf, 8, 76, 304, 116, back);
        }
        strcat(key, "2");
        msgtext[0] = 0;
        if (kind) {
            news_name(blurb_file, key);
            y = news_effect(36, 12, y, 1) + 8;
        }
    }
    switch (kind) {
    case 0:
        y += 8;
        msgtext[0] = 0;
        if (advances[index].prereq1 != -1) {
            strcpy(msgtext, "Requires ");
            strcat(msgtext, advances[advances[index].prereq1].name);
        }
        if (advances[index].prereq2 != -1) {
            strcat(msgtext, " and ");
            strcat(msgtext, advances[advances[index].prereq2].name);
        }
        draw_string(msgtext, 32, y, 1);
        y += 16;
        draw_string("Allows: ", 32, y, 1);
        y += 8;
        y0 = y;
        for (i = 0; i < 72; i++) {
            if (advances[i].prereq1 == index) {
                strcpy(msgtext, advances[i].name);
                if (advances[i].prereq2 != -1) {
                    strcat(msgtext, " (with ");
                    strcat(msgtext, advances[advances[i].prereq2].name);
                    strcat(msgtext, ")");
                }
                draw_string(msgtext, 40, y, 9);
                y += 8;
            }
            if (advances[i].prereq2 == index) {
                strcpy(msgtext, advances[i].name);
                if (advances[i].prereq1 != -1) {
                    strcat(msgtext, " (with ");
                    strcat(msgtext, advances[advances[i].prereq1].name);
                    strcat(msgtext, ")");
                }
                draw_string(msgtext, 40, y, 9);
                y += 8;
            }
        }
        y += 4;
        for (i = 0; i < 28; i++) {
            if (utypes[i].prereq == index) {
                label_with_style(i + (civnow << 5) + 0x40, 40, y - 4);
                strcpy(msgtext, utypes[i].name);
                strcat(msgtext, " unit");
                draw_string(msgtext, 60, y, 12);
                y += 12;
            }
        }
        for (i = 1; i <= 45; i++) {
            if (improvements[i].prereq == index) {
                draw_icon(i, 40, y - 2);
                strcpy(msgtext, improvements[i].name);
                strcat(msgtext, i <= 24 ? " improvement" : " Wonder");
                draw_string(msgtext, 60, y, 2);
                y += 12;
            }
        }
        if (y0 + 4 == y)
            draw_string("Allows: ", 32, y - 12, back);
        switch (adv_source[civnow][index]) {
        case -2:
            strcpy(msgtext, "(from Great Library)");
            break;
        case -1:
            msgtext[0] = 0;
            break;
        case 0:
            strcpy(msgtext, "(Discovered)");
            break;
        default:
            strcpy(msgtext, "(Taken from ");
            strcat(msgtext, civ_names[adv_source[civnow][index]]);
            strcat(msgtext, ")");
            break;
        }
        draw_text(msgtext, title_x, 48, 7);
        break;
    case 1:
        strcpy(msgtext, "Requires ");
        strcat(msgtext, advances[improvements[index].prereq].name);
        draw_string(msgtext, 12, y, 9);
        y += 8;
        strcpy(msgtext, "Cost: ");
        strcat(msgtext, itoa(improvements[index].cost * 10, nstr, 10));
        strcat(msgtext, " shields.");
        draw_string(msgtext, 12, y, 9);
        y += 8;
        strcpy(msgtext, "Maintenance: $");
        strcat(msgtext, itoa(improvements[index].maintenance, nstr, 10));
        draw_string(msgtext, 12, y, 12);
        y += 8;
        break;
    case 2:
        strcpy(msgtext, "Requires ");
        strcat(msgtext, advances[utypes[index].prereq].name);
        if (index == 25)
            strcat(msgtext, " and Nuclear Fission");
        draw_string(msgtext, 100, y, 9);
        y += 8;
        strcpy(msgtext, "Cost: ");
        strcat(msgtext, itoa(utypes[index].cost * 10, nstr, 10));
        strcat(msgtext, " resources.");
        draw_string(msgtext, 100, y, 9);
        y += 8;
        strcpy(msgtext, "Attack Strength: ");
        strcat(msgtext, itoa(utypes[index].attack, nstr, 10));
        draw_string(msgtext, 100, y, 12);
        y += 8;
        strcpy(msgtext, "Defense Strength: ");
        strcat(msgtext, itoa(utypes[index].defense, nstr, 10));
        draw_string(msgtext, 100, y, 12);
        y += 8;
        strcpy(msgtext, "Movement Rate: ");
        strcat(msgtext, itoa(utypes[index].moves, nstr, 10));
        draw_string(msgtext, 100, y, 0);
        y += 8;
        break;
    case 3:
        for (t = index; t <= index + 12; t += 12) {
            if (t != index && terrains[index].food == 2)
                continue;
            draw_string(terrains[t].name, 12, y, 1);
            y += 8;
            if (terrains[t].food || work_rules[index].irrigate < -1) {
                strcpy(msgtext, "Food: ");
                strcat(msgtext, itoa(terrains[t].food, nstr, 10));
                if (terrains[t].food >= 3)
                    strcat(msgtext, "*");
                if (work_rules[index].irrigate < -1) {
                    strcat(msgtext, " (");
                    strcat(msgtext, itoa(terrains[t].food - work_rules[index].irrigate - 1, nstr, 10));
                    if (terrains[t].food >= 2)
                        strcat(msgtext, "*");
                    strcat(msgtext, " with Irrigation)");
                }
                strcat(msgtext, " units.");
                draw_string(msgtext, 16, y, 9);
                y += 8;
            }
            if (terrains[t].shields || work_rules[index].mine < -1) {
                strcpy(msgtext, "Production: ");
                if (index == 2 || index == 11)
                    strcat(msgtext, "0/");
                strcat(msgtext, itoa(terrains[t].shields, nstr, 10));
                if (work_rules[index].mine < -1) {
                    strcat(msgtext, " (");
                    strcat(msgtext, itoa(terrains[t].shields - work_rules[index].mine - 1, nstr, 10));
                    if (terrains[t].shields - work_rules[index].mine - 1 >= 3)
                        strcat(msgtext, "*");
                    strcat(msgtext, " with Mining)");
                }
                strcat(msgtext, " units.");
                draw_string(msgtext, 16, y, 9);
                y += 8;
            }
            if (terrains[t].trade || index <= 2) {
                strcpy(msgtext, "Trade: ");
                strcat(msgtext, itoa(terrains[t].trade, nstr, 10));
                if (terrains[t].trade)
                    strcat(msgtext, "%");
                if (terrains[t].trade >= 3)
                    strcat(msgtext, "*");
                if (index <= 2) {
                    strcat(msgtext, " (");
                    strcat(msgtext, itoa(terrains[t].trade + 1, nstr, 10));
                    strcat(msgtext, "% with Roads)");
                }
                draw_string(msgtext, 16, y, 9);
                y += 8;
            }
            if (t == 7) {
                draw_string("nothing", 16, y, 9);
                y += 8;
            }
            y += 4;
        }
        draw_string("*  -1 if government is Despotism/Anarchy.", 16, y, 9);
        y += 8;
        draw_string("%  +1 if government is Republic/Democracy.", 16, y, 9);
        y += 12;
        strcpy(msgtext, "Movement cost: ");
        strcat(msgtext, itoa(terrains[index].move_cost, nstr, 10));
        strcat(msgtext, " MP");
        draw_string(msgtext, 12, y, 12);
        y += 8;
        strcpy(msgtext, "Defense bonus: +");
        strcat(msgtext, itoa(terrains[index].defense * 50 - 100, nstr, 10));
        strcat(msgtext, "%");
        draw_string(msgtext, 12, y, 12);
        y += 8;
        break;
    }
    scrbuf->font = 1;
    mouse_on();
    ad_wait();
    mouse_off();
    if (disk_ok == 1L)
        disk_insert(0, 1L);
    no_pictures = 0;
    if (mouse_mode == -1) {
        rect_fill(scrbuf, 8, 8, 304, 184, back);
        cycle_on();
    }
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int pedia_advance_index(int n)
{
    int i;

    for (i = 0; i < 72; i++)
        if (pedia_icons[i] == n)
            return i;
    return -1;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void blit_centred(int x, int y, int w, int h, int cx, int cy)
{
    int x2, y2;

    x2 = x + w;
    if (x2 > 319) {
        x2 = 319;
        w = 319 - x;
    }
    y2 = y + h;
    if (y2 > 199) {
        y2 = 199;
        h = 199 - y;
    }
    if (cx < w / 2) {
        x += w / 2 - cx;
        w = cx * 2;
    }
    blit(back_page, x, y, w, h, scrbuf, cx - w / 2, cy - h / 2);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int pedia_unit_index(int n)
{
    int i;

    for (i = 0; i < 28; i++)
        if (pedia_icons[i + 68] == n)
            return i;
    return -1;
}

int pedia_terrain_index(int n)
{
    int i;

    for (i = 0; i < 12; i++)
        if (pedia_icons[i + 97] == n)
            return i;
    return -1;
}

/* Civilopedia topic orders: advances, units and improvements, each ended
   by -1; original far-data block. */
signed char far pedia_icons[109] = {
    24, 32, 11, 65, 26, 36, 13, 59, 53, 44, 39, 5, 1, 51, 4, 35,
    16, 50, 52, 38, 29, 49, 22, 0, 21, 10, 33, 60, 19, 37, 45, 12,
    15, 43, 30, 40, 2, 41, 17, 27, 18, 3, 61, 56, 57, 6, 34, 55,
    14, 62, 20, 58, 23, 8, 64, 66, 63, 31, 9, 48, 42, 7, 47, 54,
    28, 25, 46, -1, 4, 21, 19, 14, 24, 15, 13, 18, 16, 17, 8, 9,
    12, 26, 20, 6, 1, 22, 25, 23, 11, 0, 10, 5, 2, 3, 27, 7,
    -1, 6, 7, 8, 9, 10, 11, 0, 2, 3, 1, 4, 5
};
