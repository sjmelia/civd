/* Overlay 9: game replay and world-conquest ending. */
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "AUDIENCE.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "FADE.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "MAP.H"
#include "RESIDENT.H"
#include "TURN.H"
#include "UNITS.H"
#include "REPLAY.H"
#include "MESSAGE.H"

struct civ_type {
    char leader[16], plural[16], adjective[16];
    int f0, f1, f2, f3, f4;
};

extern struct civ_type civt[];
extern struct unit_type utypes[];

extern int far replay_cities[3][128];
extern unsigned char far replay_log[];
extern char far city_site_x[], far city_site_y[];
extern char far place_name[][13];
extern char far fame_ranks[];
extern int far portrait_size[][2], far portrait_offset[][2];

extern int open();
extern int write(), close();

extern int terrain_at(int, int), map_flags(int, int);

extern void flip(int);
extern int getch(void);
extern void sound();
extern void disk_insert();

int replay_length = 0;          /* bytes used in replay_log */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void replay(void)
{
    int p;
    int year;
    int type;
    int what;
    int x;
    int i;
    int y;
    int sav[8];
    int z;
    int ter;
    int turns;

    do {
        if (replay_auto == 0)
            replay_mode = show_msg("See Replay?\n No replay\n Quick Replay\n Complete Replay\n Write Replay to Disk\n", 100, 80);
        else
            replay_mode = 4;
        if (replay_mode <= 0)
            return;
        if (replay_mode == 3) {
            replay_file = open("replay.txt", 0x8301, 0x80);
            if (replay_file == -1) {
                strcpy(msgtext, "File Error\n");
                show_msg(msgtext, 100, 80);
                return;
            }
            replay_map();
        }
        mouse_off();
        menu_begin();
        for (i = 0; i < 128; i++)
            replay_cities[0][i] = -1;
        for (i = 1; i < 8; i++)
            sav[i] = civ_tribe_of[i];
        for (i = 1; i < 8; i++) {
            if (i == civnow)
                continue;
            civ_tribe_of[i] = (civ_flags_bits & (1 << i)) ? i + 8 : i;
            replay_names(i);
        }
        if (replay_mode != 4) {
            rect_fill(scrbuf, 0, 0, 320, 200, 0);
            for (y = 1; y < 49; y++)
                for (x = 0; x < 80; x++)
                    rect_fill(scrbuf, x * 4, y * 4 + 4, 4, 4, (ter = terrain_at(x, y)) == 10 ? 1 : 8);
        }
        for (p = 0; p < replay_length;) {
            turns = replay_log[p] & 0x0f;
            type = replay_log[p++] >> 4;
            turns = (turns << 8) + replay_log[p++];
            year = replay_year(turns);
            replay_civ = -1;
            msgtext[0] = 0;
            strcat(msgtext, itoa(abs(year), nstr, 10));
            strcat(msgtext, year < 0 ? " BC" : " AD");
            strcat(msgtext, ": ");
            switch (type) {
            case 1:
            case 9:
                replay_civ = replay_log[p++];
                what = replay_log[p++];
                x = replay_log[p++];
                y = replay_log[p++];
                if (replay_civ != 255) {
                    strcat(msgtext, civ_names[replay_civ]);
                    if (type == 1) {
                        strcat(msgtext, " found the city of ");
                        replay_found(replay_civ, x, y);
                    } else {
                        strcat(msgtext, " capture ");
                        replay_owner(replay_civ, x, y);
                    }
                    copy_from_far(place_name[what], msgtext + strlen(msgtext), 13);
                    replay_line();
                } else {
                    copy_from_far(place_name[what], msgtext + strlen(msgtext), 13);
                    strcat(msgtext, " destroyed");
                    replay_line();
                    replay_owner(-1, x, y);
                }
                replay_area(x, y);
                if (replay_mode == 1)
                    flip(30);
                break;
            case 2:
                x = replay_log[p] >> 4;
                y = replay_log[p++] & 0x0f;
                strcat(msgtext, civ_names[x]);
                strcat(msgtext, " declare war on ");
                strcat(msgtext, civ_names[y]);
                replay_line();
                break;
            case 3:
                x = replay_log[p] >> 4;
                y = replay_log[p++] & 0x0f;
                strcat(msgtext, civ_names[x]);
                strcat(msgtext, " make peace with ");
                strcat(msgtext, civ_names[y]);
                replay_line();
                break;
            case 5:
                replay_civ = replay_log[p++];
                y = replay_log[p++];
                strcat(msgtext, civ_names[replay_civ]);
                strcat(msgtext, " discover ");
                strcat(msgtext, advances[y].name);
                replay_line();
                break;
            case 6:
                replay_civ = replay_log[p++];
                y = replay_log[p++];
                strcat(msgtext, civ_names[replay_civ]);
                strcat(msgtext, " produce first ");
                strcat(msgtext, utypes[y].name);
                replay_line();
                break;
            case 8:
                replay_civ = replay_log[p++];
                y = replay_log[p++];
                strcat(msgtext, civ_names[replay_civ]);
                strcat(msgtext, " form ");
                strcat(msgtext, government_names[y]);
                replay_line();
                break;
            case 10:
                replay_civ = replay_log[p++];
                y = replay_log[p++];
                strcat(msgtext, civ_names[replay_civ]);
                strcat(msgtext, " build ");
                if (y <= 7)
                    strcat(msgtext, "the ");
                strcat(msgtext, improvements[y + 24].name);
                replay_line();
                break;
            case 11:
                replay_civ = civnow;
                x = replay_log[p++];
                y = replay_log[p++];
                z = replay_log[p++];
                strcat(msgtext, civ_names[civnow]);
                strcat(msgtext, ": ");
                strcat(msgtext, itoa(x, nstr, 10));
                strcat(msgtext, " cities; ");
                format_total((y << 8) + z);
                strcat(msgtext, " population");
                replay_line();
                break;
            case 12:
                strcpy(msgtext, "*** ");
                for (x = 0; x < 4; x++) {
                    if (replay_log[p] & 0xf0) {
                        strcat(msgtext, itoa(x * 2 + 1, nstr, 10));
                        strcat(msgtext, ":");
                        strcat(msgtext, civ_names[replay_log[p] >> 4]);
                        strcat(msgtext, " ");
                    }
                    if (replay_log[p] & 0x0f) {
                        strcat(msgtext, itoa(x * 2 + 2, nstr, 10));
                        strcat(msgtext, ":");
                        strcat(msgtext, civ_names[replay_log[p] & 0x0f]);
                        strcat(msgtext, " ");
                    }
                    p++;
                }
                if (replay_mode == 3)
                    replay_line();
                break;
            case 13:
                replay_civ = replay_log[p++];
                x = replay_log[p++];
                strcat(msgtext, civ_adjectives[replay_civ]);
                strcat(msgtext, " civilization destroyed by ");
                strcat(msgtext, civ_names[x]);
                if (replay_mode == 4 && x == civnow)
                    conquest_scene(replay_civ, year);
                civ_tribe_of[replay_civ] ^= 8;
                replay_names(replay_civ);
                replay_line();
                break;
            }
            if (replay_mode == 2 && getch() == 27)
                break;
        }
        for (i = 1; i < 8; i++)
            civ_tribe_of[i] = sav[i];
        for (i = 1; i < 8; i++)
            if (i != civnow)
                replay_names(i);
        menu_end();
        mouse_on();
        if (replay_mode == 3)
            close(replay_file);
    } while (replay_mode < 3);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void replay_line(void)
{
    int len;

    if (replay_mode == 4)
        return;
    if (replay_civ == civnow)
        strupr(msgtext);
    rect_fill(scrbuf, 0, 0, 320, 8, 15);
    draw_string(msgtext, 4, 1, 0);
    if (replay_mode == 3) {
        len = strlen(msgtext);
        msgtext[len] = 13;
        msgtext[len + 1] = 10;
        msgtext[len + 2] = 0;
        write(replay_file, msgtext, len + 2);
    }
    truncate_text(msgtext, 0x138);
    rect_fill(scrbuf, 0, 0, 320, 8, 15);
    draw_string(msgtext, 4, 1, 0);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int replay_year(int turn)
{
    if (turn <= 250)
        return turn * 20 - 4000;
    turn -= 250;
    if (turn <= 50)
        return turn * 10 + 1000;
    turn -= 50;
    if (turn <= 50)
        return turn * 5 + 1500;
    turn -= 50;
    if (turn <= 50)
        return turn * 2 + 1750;
    turn -= 50;
    return turn + 1850;
}

void replay_found(int civ, int x, int y)
{
    int i;

    for (i = 0; i < 128; i++) {
        if (replay_cities[0][i] == -1) {
            replay_cities[1][i] = x;
            replay_cities[2][i] = y;
            replay_cities[0][i] = civ;
            break;
        }
    }
}

void replay_owner(int civ, int x, int y)
{
    int i;

    for (i = 0; i < 128; i++) {
        if (replay_cities[1][i] == x && replay_cities[2][i] == y) {
            replay_cities[0][i] = civ;
            break;
        }
    }
}

int replay_nearest(int x, int y)
{
    int d;
    int i;
    int city;

    nearest_dist = 999;
    for (i = 0; i < 128; i++) {
        if (replay_cities[0][i] != -1) {
            d = map_dist(x, replay_cities[1][i], y, replay_cities[2][i]);
            if (d < nearest_dist) {
                nearest_dist = d;
                city = i;
            }
        }
    }
    return city;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void replay_area(int x, int y)
{
    int x0;
    int y0;
    int x1;
    int city;
    int y1;
    int ter;

    if (replay_mode == 4)
        return;
    x0 = clamp(x - 5, 0, 80);
    x1 = clamp(x + 5, 0, 80);
    y0 = clamp(y - 5, 2, 49);
    y1 = clamp(y + 5, 2, 49);
    for (y = y0; y < y1; y++) {
        for (x = x0; x < x1; x++) {
            if ((ter = terrain_at(x, y)) == 10)
                continue;
            city = replay_nearest(x, y);
            if (nearest_dist < 6 && region_key(x, y) == region_key(replay_cities[1][city], replay_cities[2][city])) {
                rect_fill(scrbuf, x * 4, y * 4 + 4, 4, 4, nearest_dist != 0 ? civ_colours[replay_cities[0][city]] : (replay_cities[0][city] == 1 ? 7 : 15));
            } else
                rect_fill(scrbuf, x * 4, y * 4 + 4, 4, 4, 8);
        }
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void replay_map(void)
{
    int c;
    int x;
    int y;
    int j;

    for (y = 0; y < 50; y++) {
        msgtext[80] = 13;
        msgtext[81] = 10;
        msgtext[82] = 0;
        for (x = 0; x < 80; x++)
            msgtext[x] = terrain_at(x, y) == 10 ? ' ' : '.';
        for (x = 0; x < 80; x++) {
            if (terrain_at(x, y) == 10)
                continue;
            for (j = 0; j < 256; j++) {
                if (city_site_x[j] == x && city_site_y[j] == y) {
                    msgtext[x] = (map_flags(x, y) & 1) ? '*' : '+';
                    c = 0;
                    do {
                        msgtext[x + c + 1] = place_name[j][c];
                        c++;
                    } while (x + c + 1 < 80 && place_name[j][c] != 0);
                    break;
                }
            }
        }
        write(replay_file, msgtext, 82);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void replay_names(int civ)
{
    char buf[16];

    strcpy(civ_adjectives[civ], civt[civ_tribe_of[civ]].adjective);
    strcpy(buf, civt[civ_tribe_of[civ]].plural);
    if (buf[0] == 0) {
        strcpy(buf, civt[civ_tribe_of[civ]].adjective);
        strcat(buf, "s");
    }
    strcpy(civ_names[civ], buf);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void conquest_replay(void)
{
    int i;

    replay_auto = 1;
    cycle_off();
    mouse_off();
    takeover_seen = 0;
    for (i = 0; i < 14; i++)
        fame_ranks[i + 0x20] = -1;
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    scrbuf->font = 6;
    replay();
    sound(civt[civ_tribe_of[civnow]].f4, 3);
    rect_fill(scrbuf, 18, 150, 284, 32, vga_modes ? 34 : 7);
    strcpy(msgtext, "The entire world hails");
    draw_text(msgtext, 160, 152, 20);
    draw_text(msgtext, 160, 151, vga_modes ? 23 : 15);
    strcpy(msgtext, leader_names[civnow]);
    strcat(msgtext, " the CONQUEROR!");
    draw_text(msgtext, 160, 168, 20);
    draw_text(msgtext, 160, 167, vga_modes ? 23 : 15);
    repaint();
    ad_wait();
    sound(1);
    scrbuf->font = 1;
    disk_insert(0, 1L);
    mouse_on();
    sel_player();
    replay_auto = 0;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void conquest_scene(int civ, int year)
{
    char name[16];
    int head;
    char sname[16];
    int ki;
    int place;
    int x;
    int i;
    int y;
    int cv;
    int h3;
    int dummy;
    int lp;
    int img_c;

    sprite_arena_init();
    lp = leader_portrait[civ_tribe_of[civ]];
    strcpy(name, "king00.pic");
    name[5] += lp % 10;
    name[4] += lp / 10;
    if (vga_modes == 0 && (lp == 0 || lp == 3))
        name[3] = 'k';
    disk_insert(lp / 7 + 4, 1L);
    load_screen(1, name);
    head = cut_tile(1, 181, 67, 139, 133);
    img_c = cut_tile(1, 1, 51, 59, 49);
    h3 = cut_tile(1, 1, 151, 59, 49);
    sprite_arena_release();
    load_screen(1, "slam1.pic");
    if (vga_modes)
        set_palette(5, 0, 0, 0);
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    strcpy(sname, "slam2.pic");
    if (!vga_modes)
        sname[3] = 'g';
    LOAD_PIC(1, 0, 133, sname, 0);
    for (i = 0; i < 14; i++) {
        if (fame_ranks[i + 0x20] != -1) {
            cv = fame_ranks[i + 0x20];
            blit(back_page, cv % 7 * 28 + 1, cv / 7 * 34 + 133, 27, 33, scrbuf, i % 7 * 46 + 8, -(i / 7) * 42 + 49);
        }
    }
    ki = lp;
    x = 90;
    y = 0;
    x += portrait_size[ki][0] - 180;
    x -= portrait_size[ki][1];
    y += portrait_offset[ki][0] - 64;
    y -= portrait_offset[ki][1];
    sprite_draw(scrbuf, 90, 0, head);
    sprite_draw(scrbuf, x, y - 2, img_c);
    sound(civt[civ_tribe_of[civ]].f4, 3);
    if (vga_modes) {
        strcpy(name + 7, "pal");
        load_palette(name, pic_colours_data);
        load_palette("slam1.pal", colours_area);
        for (i = 0xc0; i < 0x1b0; i++)
            colours_area[i + 6] = pic_colours_data[i + 6];
        install_palette(8, colours_area);
    }
    place = takeover_seen % 7;
    if (place & 1)
        place = 6 - place / 2;
    else
        place /= 2;
    if (takeover_seen++ >= 7)
        place += 7;
    fame_ranks[place + 0x20] = lp;
    timer_reset();
    load_screen(1, "slam1.pic");
    LOAD_PIC(1, 0, 133, sname, 0);
    for (i = 0; i < 14; i++) {
        if (fame_ranks[i + 0x20] != -1) {
            cv = fame_ranks[i + 0x20];
            blit(back_page, cv % 7 * 28 + 1, cv / 7 * 34 + 133, 27, 33, back_page, i % 7 * 46 + 8, -(i / 7) * 42 + 49);
        }
    }
    while (timer_read() < 240)
        msgtext[0] = 0;
    strcat(msgtext, itoa(abs(year), nstr, 10));
    strcat(msgtext, year < 0 ? " BC" : " AD");
    strcat(msgtext, ": ");
    strcat(msgtext, civ_names[civnow]);
    strcat(msgtext, " destroy");
    draw_text(msgtext, 160, 152, 20);
    draw_text(msgtext, 160, 151, vga_modes ? 23 : 15);
    strcpy(msgtext, "the ");
    strcpy(msgtext, civ_adjectives[civ]);
    strcat(msgtext, " civilization!");
    draw_text(msgtext, 160, 168, 20);
    draw_text(msgtext, 160, 167, vga_modes ? 23 : 15);
    blit(scrbuf, 0, 133, 320, 67, back_page, 0, 133);
    sprite_draw(scrbuf, x, y - 2, h3);
    free_block(head);
    sound(1);
    flip(60);
    sound(civt[civ_tribe_of[civnow]].f4, 3);
    page_dissolve(1, 30);
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    set_page(0);
    sound(1);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Replay city owners and coordinates; original far-data block. */
int far replay_cities[3][128] = { 0 };
