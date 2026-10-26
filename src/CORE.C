/* Video modes, random numbers, map distance, unit searches and memory helpers. */
#include <dos.h>
#include "GAME.H"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "CORE.H"
#include "DRAW.H"
#include "TURN.H"
#include "DRIVER.H"
#include "FILEIO.H"
#include "PALETTE.H"

struct unit_record {
    char x, y, type, size;
    char unknown[3];
    char owner;
    char rest[20];
};
struct group_record {
    char unknown[5];
    unsigned char x, y;
    char type;
    char rest[4];
};

extern char townbuf[];
extern char armydata[];
/* City records seen from their x field, and units from 4 bytes before. */
#define units ((struct unit_record *) (townbuf + 4))
#define group_records ((struct group_record (*)[128]) (armydata - 4))
extern union REGS bios_call_reg_block_io;



void bios_request_13(void)
{
    bios_call_reg_block_io.h.ah = 0;
    bios_call_reg_block_io.h.al = 13;
    int86(16, &bios_call_reg_block_io, &bios_call_reg_block_io);
}

void bios_request_3(void)
{
    bios_call_reg_block_io.h.ah = 0;
    bios_call_reg_block_io.h.al = 3;
    int86(16, &bios_call_reg_block_io, &bios_call_reg_block_io);
}

void initialize_random(void)
{
    map_rng_state = (int) time(0) & 0x7fff;
    srand(map_rng_state);
}

int rand_scaled(int value)
{
    return ((long) value * rand()) >> 15;
}

int clamp(int value, int lower, int upper)
{
    if (value < lower)
        value = lower;
    if (value > upper)
        value = upper;
    return value;
}

void label_with_style(int style, int x, int y)
{
    sprite_draw(scrbuf, x, y, statuspics[style]);
}

int city_at(int x, int y)
{
    int i;
    for (i = 0; i < 128; i++)
        if (units[i].type != -1 && (unsigned char) units[i].x == x && (unsigned char) units[i].y == y)
            return i;
    return -1;
}

int near_city(int x, int y)
{                               /* nearest city to a tile */
    int i, index, min_dist, dist;
    min_dist = 999;
    index = -1;
    i = 0;
    do {
        if (units[i].type == -1)
            goto next_unit;
        dist = map_dist(x, (unsigned char) units[i].x, y, (unsigned char) units[i].y);
        if (dist < min_dist) {
            min_dist = dist;
            index = i;
        }
next_unit:
        i++;
    } while (i < 128);
    town_gap = min_dist;
    return index;
}

int nearest_record(int group, int excluded, int x, int y)
{
    int i, index, min_dist, dist;
    min_dist = 999;
    index = -1;
    for (i = 0; i < 128; i++) {
        if (group_records[(int) (unsigned int) group][i].type == -1 || i == excluded)
            continue;
        dist = map_dist(x, group_records[(int) (unsigned int) group][i].x, y, group_records[(int) (unsigned int) group][i].y);
        if (dist < min_dist) {
            min_dist = dist;
            index = i;
        }
    }
    town_gap = min_dist;
    return index;
}

int half_dist(int x, int y)
{
    x = abs(x);
    y = abs(y);
    if (x > y)
        return x + y / 2;
    else
        return y + x / 2;
}

int distance_twice(int x, int y)
{
    x = abs(x);
    if (x > 40)
        x = 80 - x;
    y = abs(y);
    if (x > y)
        return x * 2 + y;
    else
        return y * 2 + x;
}

int map_dist(int x1, int x2, int y1, int y2)
{
    int x, y;
    x = abs(x1 - x2);
    if (x > 40)
        x = 80 - x;
    y = abs(y1 - y2);
    return half_dist(x, y);
}

int count_weighted_units(int owner)
{
    int i, j, points;
    points = 0;
    for (i = 0; i < 128; i++) {
        if (units[i].owner == owner && units[i].type != -1)
            for (j = 1; j <= units[i].size; j++)
                points += j;
    }
    format_total(points);
    return points;
}

void format_total(int total)
{
    if (total >= 100) {
        strcat(msgtext, itoa(total / 100, nstr, 10));
        strcat(msgtext, ",");
        if (total % 100 < 10)
            strcat(msgtext, "0");
    }
    strcat(msgtext, itoa(total % 100, nstr, 10));
    strcat(msgtext, "0,000");
}

void draw_backdrop(int x, int y, int w, int h)
{
    int tile_w, tile_h, px, py, total_w;
    total_w = w;
    if (vga_modes) {
        py = y;
        do {
            tile_h = clamp(h, 0, 16);
            px = x;
            w = total_w;
            do {
                tile_w = clamp(w, 0, 32);
                blit(tile_page, 0x120, 0x78, tile_w, tile_h, scrbuf, px, py);
                px += tile_w;
                w -= tile_w;
            } while (w > 0);
            py += tile_h;
            h -= tile_h;
        } while (h > 0);
    } else
        rect_fill(scrbuf, x, y, w, h, 7);
}

void restore_palette(int which)
{
    int i;
    if (vga_modes) {
        load_palette(which, colours_area);
        for (i = 0; i < 48; i++)
            colours_area[i + 6] = intro_kept_pal[i];
        apply_palette(colours_area);
        set_dac(0x2d, 0x33, 0x27, 0x19);
    }
}

void copy_to_far(char *p, unsigned int off, unsigned int seg, unsigned int n)
{
    char far *fp;
    fp = (char far *) p;
    movedata(FP_SEG(fp), FP_OFF(fp), seg, off, n);
}

void copy_from_far(unsigned int off, unsigned int seg, char *p, unsigned int n)
{
    char far *fp;
    fp = (char far *) p;
    movedata(seg, off, FP_SEG(fp), FP_OFF(fp), n);
}

void free_block(unsigned int seg, char *name)
{
    if (_dos_freemem(seg)) {
        strcpy(msgtext, "MEM.ERR:");
        strcat(msgtext, name);
        strcat(msgtext, "\n");
        show_msg(msgtext, 100, 80);
    }
}

void show_memory(void)
{
    strcpy(msgtext, "Mem:");
    strcat(msgtext, itoa(free_paragraphs(), nstr, 10));
    draw_text(msgtext, 160, 3, 0);
    draw_text(msgtext, 160, 2, 15);
}

void setup_cycles(void)
{
    set_cycle(1, 15, 0x60, 0x67);
    set_cycle(2, 15, 0x68, 0x6f);
    set_cycle(3, 15, 0x70, 0x7f);
}

int cycling = 0;

void cycle_on(void)
{
    if (!cycling && vga_modes) {
        start_cycle(1);
        start_cycle(2);
        start_cycle(3);
    }
    cycling = 1;
}

void cycle_off(void)
{
    if (cycling && vga_modes) {
        stop_cycle(1);
        stop_cycle(2);
        stop_cycle(3);
    }
    cycling = 0;
}

void set_dac(int reg, int red, int green, int blue)
{
    union REGS r;
    if (vga_modes) {
        r.h.ah = 0x10;
        r.h.al = 0x10;
        r.x.bx = reg;
        r.h.dh = red;
        r.h.ch = green;
        r.h.cl = blue;
        int86(0x10, &r, &r);
    }
}
