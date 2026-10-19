#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MAP.H"
#include "TURN.H"
#include "BATTLE.H"
#include "CAPTURE.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "ECONOMY.H"
#include "MESSAGE.H"
#include "UNITS.H"

#include "CIV.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"

extern struct city townbuf[];
extern struct unit armydata[4][128];
extern struct unit_type utypes[];
extern char far map_visible_to[];

extern int wrap_x(int x);

extern int rand_scaled(int max_val);

extern int recolour_rect(int handle, int x, int y, int w, int h, int a, int b);

extern int label_with_style(int style, int x, int y);
extern int city_at(int x, int y);

extern int world_draw(int *mode, int x, int y, int val);

        /* city labels queued on the map view */

 /* grassland shield overlay */

int map_note(int x, int y);

int map_flags(int x, int y);

int terrain_at(int x, int y);
int map_stack(int x, int y);

int seen_map(int x, int y);
int map_owner(int x, int y);

int map_mark(int x, int y);

void reveal(int player, int x, int y)
{
    int r1;
    int s;
    int w;
    int h;
    int i;
    int sx;
    int sy;
    int l;
    int dummy;
    int ty;
    int by_c;
    int cx_c;
    int rx_d;
    int dy;

    mouse_off();
    disk_swap = 0;
    draw_banner();
    status_draw();
    s = stk_plr;
    map_x0 = wrap_x(x);
    map_y0 = clamp(y, 0, 38);
    player = 1 << player;
    city_label_n = 0;
    r1 = rand_scaled(256);
    for (i = 0; i < 256; i++) {
        cx_c = r1 & 0xf;
        ty = r1 >> 4;
        if (ty < 12 && cx_c < 15) {
            if (xray_on != 0 || (map_visible_to[wrap_x(cx_c + map_x0) * 50 + ty + map_y0] & player)) {
                map_note(wrap_x(cx_c + map_x0), ty + map_y0);
            } else {
                rect_fill(scrbuf, (cx_c << 4) + 80, (ty << 4) + 8, 16, 16, 0);
            }
        }
        r1 = (unsigned char) (r1 * 5 + 1);
    }
    for (i = 0; i < city_label_n; i++) {
        if (map_label_yorg[i] < 184) {
            msgtext[0] = 0;
            append_place_name(label_no[i]);
            truncate_text(msgtext, 327 - map_label_xorg[i]);
            draw_shadowed(msgtext, clamp(map_label_xorg[i] - 8, 80, 999), map_label_yorg[i] + 16, 11);
        }
    }
    sx = x - 32;
    sy = y - 19;
    if (has_tech(player, 8)) {
        sy = 0;
    }
    rx_d = (sx >= 0) ? sx : sx + 80;
    w = 80;
    if (rx_d > 0) {
        w -= rx_d;
    }
    by_c = (sy >= 0) ? sy : 0;
    dy = by_c - sy;
    h = 50 - dy;
    if (sy > 0) {
        h -= sy;
    }
    view_xorg = sx;
    view_yorg = sy;
    rect_fill(scrbuf, 0, 8, 80, 50, 0);
    if (xray_on != 0) {
        sx = clamp(sx, 0, 16);
        view_xorg = sx;
        sy = clamp(sy, 0, -6);
        view_yorg = sy;
        blit(tile_page, sx, sy, 80, 50, scrbuf, 0, 8);
    } else {
        blit(tile_page, rx_d + 240, by_c, w, h, scrbuf, 0, dy + 8);
        if (w < 80) {
            blit(tile_page, 240, by_c, 80 - w, h, scrbuf, w, dy + 8);
        }
    }
    for (i = 0; i < 128; i++) {
        if (townbuf[i].status != -1 && (townbuf[i].visible_size != 0 || townbuf[i].owner == civnow)) {
            cx_c = wrap_x((unsigned char) townbuf[i].x - rx_d);
            ty = (int) (unsigned char) townbuf[i].y - by_c + dy;
            if (ty >= 0 && ty < 50) {
                plot_pixel(cx_c, ty + 8, civ_colours[townbuf[i].owner]);
            }
        }
    }
    draw_panel(0, 8, 79, 49, 15, 8);
    draw_box(x - sx - 1, y - sy + 7, 17, 10, 15);
    stk_plr = s;
    mouse_on();
}

int seen_map(int x, int y)
{
    int penx;
    int owner;
    int nflags;
    int xtop;
    int ter;
    int unused_h;
    int seen;
    int neigh;
    int wd;
    int size;
    int k;
    int y2;
    int ypos;
    int sx;
    int nterr;
    int spare;

    if (still == 1) {
        if (map_flags(x, y) & 1)
            return rect_fill(scrbuf, x * 4, y * 4, 4, 4, civ_colours[map_city(x, y)]);
        return rect_fill(scrbuf, x * 4, y * 4, 4, 4, terrain_at(x, y) == 10 ? 1 : 2);
    }
    sx = (wrap_x(x - map_x0) << 4) + 80;
    y2 = ((y - map_y0) << 4) + 8;
    if (sx < 80 || sx >= 320 || y2 < 8 || y2 > 192)
        return 0;
    ter = terrain_at(x, y);
    seen = known_flags(x, y);
    if (xray_on)
        seen = map_flags(x, y);
    mouse_off();
    if (ter == 10) {
        neigh = 0;
        if (vga_modes == 0) {
            for (k = 1; k < 9; k += 2) {
                neigh >>= 1;
                if (terrain_at(wrap_x(spiral_x[k] + x), spiral_y[k] + y) != 10 && map_in_bounds(0, spiral_y[k] + y))
                    neigh |= 8;
            }
            blit(spare_page, neigh << 4, 64, 16, 16, scrbuf, sx, y2);
        } else {
            for (k = 1; k < 9; k++) {
                neigh >>= 1;
                if (terrain_at(wrap_x(spiral_x[k] + x), spiral_y[k] + y) != 10 && map_in_bounds(0, spiral_y[k] + y))
                    neigh |= 128;
            }
            size = neigh;
            neigh = ((neigh >> 6) & 3) + (neigh << 2);
            for (k = 0; k < 4; k++) {
                if (k < 2)
                    sprite_draw(scrbuf, sx + ((k & 1) << 3), y2, map_tile_edge[(neigh >> (k << 1)) & 7][k]);
                else
                    sprite_draw(scrbuf, sx - ((k & 1) << 3) + 8, y2 + 8, map_tile_edge[(neigh >> (k << 1)) & 7][k]);
            }
            if (size == 28)
                blit(tile_page, 224, 100, 16, 16, scrbuf, sx, y2);
            if (size == 193)
                blit(tile_page, 240, 100, 16, 16, scrbuf, sx, y2);
            if (size == 7)
                blit(tile_page, 256, 100, 16, 16, scrbuf, sx, y2);
            if (size == 112)
                blit(tile_page, 272, 100, 16, 16, scrbuf, sx, y2);
            if (size == 143)
                blit(tile_page, 288, 100, 16, 16, scrbuf, sx, y2);
            if (size == 248)
                blit(tile_page, 304, 100, 16, 16, scrbuf, sx, y2);
            for (k = 1; k < 9; k += 2) {
                if (terrain_at(wrap_x(spiral_x[k] + x), spiral_y[k] + y) == 11)
                    sprite_draw(scrbuf, sx, y2, mouth_deltas_sprites[k / 2]);
            }
        }
    }
    if (ter != 10) {
        if (vga_modes == 0)
            blit(spare_page, 0, 80, 16, 16, scrbuf, sx, y2);
        else
            blit(tile_page, 256, 120, 16, 16, scrbuf, sx, y2);
    }
    if ((seen & 2) && ter != 10 && bare_flag == 0 && !(seen & 1))
        label_with_style(4, sx, y2);
    if (ter == 11) {
        neigh = 0;
        for (k = 1; k < 9; k += 2) {
            neigh >>= 1;
            nterr = terrain_at(spiral_x[k] + x, spiral_y[k] + y);
            if (nterr == 11 || nterr == 10)
                neigh |= 8;
        }
        if (vga_modes == 0 || (help_probe(2, x, y + 150) & 8))
            neigh += 16;
        sprite_draw(scrbuf, sx, y2, terrain_coast_sprites[neigh]);
    }
    if (ter != 10 && ter != 11) {
        if (vga_modes) {
            neigh = 0;
            for (k = 1; k < 9; k += 2) {
                neigh >>= 1;
                if (terrain_at(wrap_x(spiral_x[k] + x), spiral_y[k] + y) == ter && map_in_bounds(0, spiral_y[k] + y))
                    neigh |= 8;
            }
            sprite_draw(scrbuf, sx, y2, world_landscape_sprite[ter][neigh]);
            if (ter == 2 && !((7 * x + 11 * y) & 2))
                sprite_draw(scrbuf, sx + 4, y2 + 4, colour_shield_tiles);
        } else {
            if (ter == 2)
                sprite_draw(scrbuf, sx, y2, world_landscape_sprite[ter][((unsigned char) (7 * x + 11 * y) & 2) >> 1]);
            else
                sprite_draw(scrbuf, sx, y2, world_landscape_sprite[ter][(x + y) & 1]);
        }
    }
    if (seen & 64) {
        if (vga_modes)
            sprite_draw(scrbuf, sx, y2, map_fumes_tile);
        else {
            recolour_rect(scrbuf, sx, y2, 16, 16, 2, 0);
            recolour_rect(scrbuf, sx, y2, 16, 16, 10, 15);
            recolour_rect(scrbuf, sx, y2, 16, 16, 9, 8);
            recolour_rect(scrbuf, sx, y2, 16, 16, 11, 0);
        }
    }
    if (seen & 8) {
        penx = (seen & 16) ? 0 : 6;
        for (k = 1; k <= 8; k++) {
            nflags = known_flags(spiral_x[k] + x, spiral_y[k] + y);
            if (nflags & 8) {
                penx = -1;
                if ((seen & 17) && (nflags & 17))
                    sprite_draw(scrbuf, sx, y2, map_rail_shapes[k - 1]);
                else
                    sprite_draw(scrbuf, sx, y2, map_paths_icon[k - 1]);
            }
        }
        if (penx != -1)
            rect_fill(scrbuf, sx + 7, y2 + 7, 2, 2, penx);
    }
    if ((seen & 4) && bare_flag == 0)
        label_with_style(5, sx, y2);
    if (patterned_cell(ter, x, y))
        label_with_style(ter + 16, sx, y2);
    if (has_hut(ter, x, y))
        label_with_style(31, sx, y2);
    if (seen & 32)
        label_with_style(30, sx, y2);
    if (xray_on == 0) {
        for (k = 1; k <= 8; k += 2) {
            if (!((1 << civnow) & map_visible_to[wrap_x(spiral_x[k] + x) * 50 + spiral_y[k] + y]))
                sprite_draw(scrbuf, sx, y2, marks_pics[k / 2]);
        }
    }
    owner = map_city(x, y);
    if ((seen & 1) && bare_flag == 0) {
        k = city_at(x, y);
        if (townbuf[k].owner == civnow || townbuf[k].visible_size || xray_on) {
            draw_box((sx + 1), (y2 + 1), 13, 13, 15);
            draw_box(sx + 2, (y2 + 1), 12, 12, civ_shadows[townbuf[k].owner]);
            rect_fill(scrbuf, sx + 2, y2 + 2, 12, 12, civ_colours[townbuf[k].owner]);
            label_with_style(28, (sx + 1), (y2 + 1));
            recolour_rect(scrbuf, sx + 2, y2 + 2, 12, 12, 5, civ_shadows[townbuf[k].owner]);
            size = (townbuf[k].owner == civnow || xray_on) ? townbuf[k].size : townbuf[k].visible_size;
            msgtext[0] = 0;
            strcat(msgtext, itoa(size, nstr, 10));
            if (townbuf[k].status & 1)
                sprite_draw(scrbuf, sx + 5, y2 + 2, city_angry_images[0]);
            else
                draw_string(msgtext, (size < 10 ? 6 : 3) + sx, y2 + 5, 0);
            if (map_stack(x, y) != -1 || townbuf[k].garrison[0] != -1)
                draw_box(sx, y2, 15, 15, 0);
            if (townbuf[k].buildings & 0x80L)
                label_with_style(29, sx + 1, y2 + 1);
            if (city_label_n < 32) {
                map_label_xorg[city_label_n] = sx;
                map_label_yorg[city_label_n] = y2;
                label_no[city_label_n++] = k;
            }
        }
    }
    mouse_on();
    return 1;
}

int unit_redraw(int player, int unit)
{
    int dom;
    int sx;
    int dx;
    int sy;
    int terr;
    int dy;
    int wh;

    if (still == 1) {
        rect_fill(scrbuf, armydata[player][unit].x * 4 + 1, armydata[player][unit].y * 4 + 1, 3, 3, armydata[player][unit].type == 0 ? 6 : 0);
        rect_fill(scrbuf, armydata[player][unit].x * 4, armydata[player][unit].y * 4, 3, 3, civ_colours[player]);
    } else {
        sx = wrap_x(armydata[player][unit].x - map_x0) << 4;
        sy = ((armydata[player][unit].y - map_y0) << 4) + 8;
        sx += 0x50;
        if (sx < 0x50 || sx >= 0x140 || sy < 8 || sy > 0xc0)
            return 0;
        dom = armydata[player][unit].type;
        terr = terrain_at(armydata[player][unit].x, armydata[player][unit].y);
        if (terr == 10 && (armydata[player][unit].status & 1) && utypes[armydata[player][unit].type].domain != 2)
            return 0;
        mouse_off();
        if (armydata[player][unit].next != -1)
            label_with_style(player * 32 + dom + 0x40, sx + 1, sy + 1);
        draw_unit(player, unit, sx, sy);
        mouse_on();
        return 1;
    }
}

int draw_unit(int player, int unit, int sx, int sy)
{
    int t;
    char st;

    label_with_style(armydata[player][unit].type + player * 32 + 0x40, sx, sy);
    if (armydata[player][unit].status & 8)
        label_with_style(0x1d, sx, sy);
    else if (armydata[player][unit].status & 4)
        draw_shadowed("F", sx + 4, sy + 7, player == 1 ? 9 : 15);
    if (player == civnow && armydata[player][unit].goto_x != -1)
        draw_shadowed("G", sx + 4, sy + 7, player == 1 ? 9 : 15);
    if ((armydata[player][unit].status & 0xc2) && utypes[armydata[player][unit].type].domain != 1) {
        t = 0x52;
        if (armydata[player][unit].status & 0x40)
            t = armydata[player][unit].type ? 0x3f : 0x49;
        st = armydata[player][unit].status;
        if (st & 0x80) {
            t = 0x4d;
            if (st & 0x40)
                t = 0x46;
            if (armydata[player][unit].status & 2)
                t = 0x50;
        }
        strcpy(msgtext, " ");
        msgtext[0] = t;
        draw_shadowed(msgtext, sx + 4, sy + 7, player == 1 ? 9 : 15);
        draw_box(sx - 1, sy - 1, 15, 15, 7);
    }
    if (armydata[player][unit].status & 1) {
        recolour_rect(scrbuf, sx, sy, 16, 16, 5, 7);
        recolour_rect(scrbuf, sx, sy, 16, 16, 8, 7);
    }
}

int terrain_map[16] = { -1, 10, 3, 8, -1, -1, 1, 6, -1, 11, 2, 9, 4, 5, 0, 7 };

/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int map_note(x, y)
int x;
int y;
{
    int s;

    seen_map(x, y);
    if (bare_flag == 0) {
        s = map_stack(x, y);
        if (s != -1) {
            if (xray_on != 0 || stk_plr == civnow || ((1 << civnow) & armydata[stk_plr][s].visibility)) {
                if (!(map_flags(x, y) & 1))
                    redraw_stack(stk_plr, s);
            }
        }
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int redraw_stack(int player, int unit)
{
    int a;
    int b;
    int terr;

    terr = terrain_at(armydata[player][unit].x, armydata[player][unit].y);
    if (terr == 10 && armydata[player][unit].next != -1 && utypes[armydata[player][unit].type].domain != 2) {
        a = unit;
        b = unit;
        do {
            a = armydata[player][a].next;
            if (utypes[armydata[player][a].type].domain == 2) {
                unit_redraw(player, a);
                a = -1;
            }
        } while (a != -1 && a != b);
        if (a == b)
            unit_redraw(player, unit);
    } else
        unit_redraw(player, stack_other(player, unit));
}

int map_in_bounds(int x, int y)
{
    if (x < 0 || x >= 80 || y < 0 || y >= 50)
        return 0;
    else
        return 1;
}

int terrain_at(int x, int y)
{
    return terrain_map[help_probe(2, x, y)];
}

int map_city(int x, int y)
{
    int v;
    v = help_probe(2, x + 160, y);
    return v & 7;
}

int map_refresh(int a, int x, int y)
{
    int v;
    v = help_probe(2, x + 160, y);
    map_claim(x + 160, y, (v & 8) + a, 2);
}

int put_unit(int a, int b, int x, int y)
{                               /* place a unit on a tile and its stack */
    int i;
    i = map_stack(x, y);
    if (i != -1)
        stack_add(a, b, i);
    map_claim(x + 160, y, a + 8, 2);
}

int lift_unit(int a, int b, int c, int d)
{
    if (armydata[a][b].next != -1)
        stack_remove(a, b);
    else
        map_claim(c + 160, d, a, 2);
}

int map_stack(int x, int y)
{
    int i;
    int civ;
    if ((civ = map_owner(x, y)) == -1)
        return -1;
    for (i = 0; i < 128; i++) {
        if (armydata[civ][i].type != -1 && armydata[civ][i].x == x && armydata[civ][i].y == y) {
            the_enemy = civ;
            stk_plr = civ;
            return i;
        }
    }
    map_claim(x + 160, y, 0, 2);
    return -1;
}

int map_owner(int x, int y)
{
    int v;
    v = help_probe(2, x + 160, y);
    if (v & 8)
        return v & 7;
    else
        return -1;
}

int occupy(int x, int y)
{                               /* set the tile's unit-present bit */
    map_claim(x + 160, y, (help_probe(2, x + 160, y) & 7) + 8, 2);
}

int clear_occupied(int x, int y)
{
    map_claim(x + 160, y, help_probe(2, x + 160, y) & 7, 2);
}

int has_road(int x, int y)
{
    return map_flags(x, y) & 8;
}

int map_flags(int x, int y)
{
    return help_probe(2, x, y + 100) + (help_probe(2, x, y + 150) << 4);
}

int known_flags(int x, int y)
{
    return help_probe(2, x + 80, y + 100) + (help_probe(2, x + 80, y + 150) << 4);
}

int map_mark(int x, int y)
{
    map_claim(x + 80, y + 100, help_probe(2, x, y + 100), 2);
    map_claim(x + 80, y + 150, help_probe(2, x, y + 150), 2);
}

int flag_tile(int a, int x, int y)
{                               /* set tile improvement flags */
    if (a) {
        if (a < 16)
            map_claim(x, y + 100, help_probe(2, x, y + 100) | a, 2);
        else
            map_claim(x, y + 150, help_probe(2, x, y + 150) | (a >> 4), 2);
    } else {
        map_claim(x, y + 100, 0, 2);
        map_claim(x, y + 150, 0, 2);
    }
    if (civ_id == civnow)
        map_mark(x, y);
}

int unflag(int a, int x, int y)
{                               /* clear tile improvement flags */
    if (a & 0xf)
        map_claim(x, y + 100, ~a & help_probe(2, x, y + 100), 2);
    if (a & 0xf0)
        map_claim(x, y + 150, ~(a >> 4) & help_probe(2, x, y + 150), 2);
}

int get_city(int x, int y)
{                               /* city at a tile; reports an error if none */
    int v;
    int i;
    v = map_city(x, y);
    for (i = 0; i < 128; i++) {
        if (townbuf[i].status != -1 && townbuf[i].x == x && townbuf[i].y == y) {
            the_enemy = v;
            stk_plr = v;
            return i;
        }
    }
    show_msg("CityAt Error\n", 100, 80);
    return -1;
}

int map_claim(int x, int y, int val, int mode)
{
    int old;
    old = *((int *) scrbuf);
    *((int *) scrbuf) = mode;
    world_draw(((int *) scrbuf), x, y, val);
    *((int *) scrbuf) = old;
}

int plot_pixel(int x, int y, int val)
{
    world_draw(((int *) scrbuf), x, y, val);
}

int map_random(int v)
{
    if (v > 0)
        return 1;
    if (v < 0)
        return -1;
    return 0;
}

int patterned_cell(int unused, int x, int y)
{
    int seed;
    if (y <= 1 || y >= 48)
        return 0;
    seed = (x >> 2) * 13 + (y >> 2) * 11 + map_rng_state;
    if ((x & 3) * 4 + (y & 3) == ((unsigned char) seed & 15))
        return 1;
    else
        return 0;
}

int has_hut(int a, int x, int y)
{
    int seed;
    if (a == 10 || (map_visible_to[x * 50 + y] & 1) || y <= 1 || y >= 48)
        return 0;
    seed = (x >> 2) * 13 + (y >> 2) * 11 + map_rng_state + 8;
    if ((x & 3) * 4 + (y & 3) == ((unsigned char) seed & 31) && !(map_flags(x, y) & 1))
        return 1;
    else
        return 0;
}

int site_value(int x, int y)
{
    return help_probe(2, x + 80, y + 50);
}

int region_key(int x, int y)
{
    return help_probe(2, x, y + 50);
}

int map_region(int x, int y)
{
    if (terrain_at(x, y) != 10)
        return continent_span[0][region_key(x, y)];
    else
        return continent_span[1][region_key(x, y)];
}
