/* Overlay 7: world generation and global warming. */
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "CITYMGT.H"
#include "CORE.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "FADE.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "MAP.H"
#include "NEWS.H"
#include "PALETTE.H"
#include "PATH.H"
#include "RASTER.H"
#include "RESIDENT.H"
#include "UNITS.H"
#include "WORLDGEN.H"
#include "DIALOG.H"

extern int terrain_at(int, int);
extern void flip(int), sound();

extern void map_note();

#pragma pack(1)
struct story_data_type {
    unsigned char cycles[131];
    unsigned int delays[45];
};
#pragma pack()
extern unsigned char far story_cycles[131];
extern unsigned int far story_delays[45];

extern char far map_visible_to[][50];

    /* land mass, temperature, climate, age */
extern void disk_insert();
extern void world_draw(int *mode, int x, int y, int val);
extern int rand_scaled(int), abs(int);
     /* good city sites per continent; tiles per land [0] and sea [1] region */
extern unsigned char block_ground_way[20][13];
struct terrain_signed {
    char name[12];
    signed char moves, defense, food, shields, trade, f5, f6;
};
struct work_rules {
    int food, shields, trade, a, b, c;
};
extern struct terrain_signed terrains[];
extern struct work_rules work_rules[];

void link_land(), link_seas();  /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

void page_zero(void)
{
    /* The original retains a 34-byte frame although no local is read.
       Its historical declaration names and grouping are unknown. */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q;
    scrbuf->number = 0;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void make_world(void)
{
    int i_h, y_h, x_i, rain_g, t_m, latitude_e, tile_c;
    disk_insert(7, 1L);
    scrbuf->number = 1;
    story_skip = 0;
    if (beginner_help)
        load_picture(1, "map.pic");
    else {
        story_step();
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
        do {
            grow_land();
            story_step();
        }
        while (land_count < (custom_params[0] << 3) * 40 + 640);
        draw_box(0, 0, 79, 49, 0);
        draw_box(1, 1, 77, 47, 0);
        for (x_i = 1; x_i < 79; x_i++) {
            for (y_h = 1; y_h < 49; y_h++) {
                tile_c = 0;
                if (help_probe(1, x_i, y_h))
                    tile_c |= 1;
                if (help_probe(1, x_i + 1, y_h))
                    tile_c |= 2;
                if (help_probe(1, x_i, y_h + 1))
                    tile_c |= 4;
                if (help_probe(1, x_i + 1, y_h + 1))
                    tile_c |= 8;
                if (tile_c == 6 || tile_c == 9) {
                    map_claim(x_i + 1, y_h, 1, 1);
                    map_claim(x_i, y_h + 1, 1, 1);
                    map_claim(x_i + 1, y_h + 1, 1, 1);
                    if (x_i != 0)
                        x_i--;
                    if (y_h != 0)
                        y_h--;
                }
            }
            story_step();
        }
        for (x_i = 0; x_i < 80; x_i++) {
            for (y_h = 0; y_h < 50; y_h++) {
                t_m = help_probe(1, x_i, y_h);
                switch (t_m) {
                case 0:
                    map_claim(x_i, y_h, 1, 1);
                    break;
                case 1:{
                        latitude_e = abs(29 - (rand_scaled(8) + y_h));
                        latitude_e += 1 - custom_params[1];
                        switch (latitude_e / 6) {
                        case -1:
                        case 0:
                            map_claim(x_i, y_h, 14, 1);
                            break;
                        case 1:
                        case 2:
                            map_claim(x_i, y_h, 6, 1);
                            break;
                        case 3:
                        case 4:
                            map_claim(x_i, y_h, 7, 1);
                            break;
                        case 5:
                        case 6:
                            map_claim(x_i, y_h, 15, 1);
                            break;
                        }
                    }
                    break;
                case 2:
                    map_claim(x_i, y_h, 12, 1);
                    break;
                default:
                    map_claim(x_i, y_h, 13, 1);
                    break;
                }
            }
            story_step();
        }
        for (y_h = 0; y_h < 50; y_h++) {
            latitude_e = abs(25 - y_h);
            rain_g = 0;
            for (x_i = 0; x_i < 80; x_i++) {
                tile_c = help_probe(1, x_i, y_h);
                if (tile_c == 1) {
                    if (rain_g < abs(12 - latitude_e) + custom_params[2] * 4)
                        rain_g++;
                } else if (rain_g > 0) {
                    rain_g -= rand_scaled(7 - custom_params[2] * 2);
                    switch (tile_c) {
                    case 14:
                        map_claim(x_i, y_h, 6, 1);
                        break;
                    case 6:
                        map_claim(x_i, y_h, 10, 1);
                        break;
                    case 12:
                        map_claim(x_i, y_h, 2, 1);
                        break;
                    case 7:
                        map_claim(x_i, y_h, 15, 1);
                        break;
                    case 13:
                        rain_g -= 3;
                        break;
                    }
                }
            }
            rain_g = 0;
            for (x_i = 79; x_i >= 0; x_i--) {
                tile_c = help_probe(1, x_i, y_h);
                if (tile_c == 1) {
                    if (rain_g < latitude_e / 2 + custom_params[2])
                        rain_g++;
                } else if (rain_g > 0) {
                    rain_g -= rand_scaled(7 - custom_params[2] * 2);
                    switch (tile_c) {
                    case 14:
                        map_claim(x_i, y_h, 6, 1);
                        break;
                    case 6:
                        map_claim(x_i, y_h, 10, 1);
                        break;
                    case 10:
                        map_claim(x_i, y_h, latitude_e < 10 ? 11 : 3, 1);
                        rain_g = -2;
                        break;
                    case 13:
                        rain_g -= 3;
                    case 3:
                    case 12:
                        map_claim(x_i, y_h, 2, 1);
                        break;
                    }
                }
            }
            story_step();
        }
        for (i_h = 0; i_h < custom_params[3] * 800 + 800; i_h++) {
            if (i_h & 1) {
                tile_c = rand_scaled(8) + 1;
                x_i += spiral_x[tile_c];
                y_h += spiral_y[tile_c];
            } else {
                x_i = rand_scaled(80);
                y_h = rand_scaled(50);
            }
            tile_c = help_probe(1, x_i, y_h);
            switch (tile_c) {
            case 14:
                map_claim(x_i, y_h, 6, 1);
                break;
            case 13:
                land_corner(x_i, y_h);
                break;
            case 10:
                map_claim(x_i, y_h, 2, 1);
                break;
            case 2:
                map_claim(x_i, y_h, 11, 1);
                break;
            case 6:
            case 7:
                map_claim(x_i, y_h, 12, 1);
                break;
            case 12:
            case 15:
                map_claim(x_i, y_h, 13, 1);
                break;
            case 3:
                map_claim(x_i, y_h, 10, 1);
                break;
            case 11:
                map_claim(x_i, y_h, 3, 1);
                break;
            }
        }
        make_rivers();
    }
    rect_fill(scrbuf, 160, 0, 160, 200, 0);
    number_regions();
    story_skip = 1;
    while (story_step());
    scrbuf->font = 1;
    blit(scrbuf, 0, 0, 320, 200, tile_page, 0, 0);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int land_corner(int x, int y)
{
    if (help_probe(1, x - 1, y - 1) == 1)
        return 0;
    if (help_probe(1, x - 1, y + 1) == 1)
        return 0;
    if (help_probe(1, x + 1, y - 1) == 1)
        return 0;
    if (help_probe(1, x + 1, y + 1) == 1)
        return 0;
    map_claim(x, y, 1, 1);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void make_rivers(void)
{
    int dir_b, old_dir_d, tries_j, rivers_f, previous_d, start_y_k, neighbor_c, y_h, start_x_l, x_i, near_water_i, tile_c;
    tries_j = rivers_f = 0;
    do {
        blit(scrbuf, 0, 0, 160, 100, scrbuf, 160, 0);
        old_dir_d = 0;
        do {
            x_i = rand_scaled(80);
            y_h = rand_scaled(50);
        } while (help_probe(1, x_i, y_h) != 12);
        start_x_l = x_i;
        start_y_k = y_h;
        dir_b = rand_scaled(4) * 2;
        previous_d = dir_b;
        do {
            map_claim(x_i, y_h, 9, 1);
            near_water_i = 0;
            for (neighbor_c = 1; neighbor_c <= 8; neighbor_c += 2)
                if (help_probe(1, x_i + spiral_x[neighbor_c], y_h + spiral_y[neighbor_c]) == 1)
                    near_water_i = 1;
            previous_d = dir_b;
            dir_b = (dir_b + (rand_scaled(2) - (old_dir_d & 1)) * 2) & 7;
            if ((previous_d ^ 4) > dir_b)
                map_claim(x_i, y_h + 50, 8, 1);
            x_i += spiral_x[dir_b + 1];
            y_h += spiral_y[dir_b + 1];
            tile_c = help_probe(1, x_i, y_h);
            old_dir_d++;
        } while (!near_water_i && tile_c != 13 && tile_c != 1 && tile_c != 9);
        if ((!near_water_i && tile_c != 9) || old_dir_d < 5)
            blit(scrbuf, 160, 0, 160, 100, scrbuf, 0, 0);
        else {
            rivers_f++;
            for (neighbor_c = 1; neighbor_c <= 21; neighbor_c++) {
                x_i = start_x_l + spiral_x[neighbor_c];
                y_h = start_y_k + spiral_y[neighbor_c];
                if (help_probe(1, x_i, y_h) == 2)
                    map_claim(x_i, y_h, 11, 1);
            }
        }
        story_step();
        tries_j++;
    } while (tries_j < 256 && rivers_f < (custom_params[0] + custom_params[2]) * 2 + 6);
    blit(scrbuf, 0, 50, 80, 50, scrbuf, 0, 150);
    rect_fill(scrbuf, 0, 50, 80, 50, 0);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void grow_land(void)
{
    int i_h, y_h, x_i;
    rect_fill(scrbuf, 160, 0, 80, 50, 0);
    x_i = rand_scaled(72) + 4;
    y_h = rand_scaled(34) + 8;
    for (i_h = 0; i_h < 1; i_h++)
        walk_land(x_i, y_h);
    for (x_i = 0; x_i < 80; x_i++)
        for (y_h = 0; y_h < 50; y_h++) {
            if (!help_probe(1, x_i + 160, y_h))
                continue;
            map_claim(x_i, y_h, help_probe(1, x_i, y_h) + 1, 1);
            land_count++;
        }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void walk_land(int x, int y)
{
    int dir_b, left_f;
    left_f = rand_scaled(64) + 1;
    do {
        map_claim(x + 160, y, 15, 1);
        map_claim(x + 161, y, 15, 1);
        map_claim(x + 160, y + 1, 15, 1);
        dir_b = rand_scaled(4) * 2 + 1;
        x += spiral_x[dir_b];
        y += spiral_y[dir_b];
    } while (--left_f && x > 2 && x < 77 && y > 2 && y < 46);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void number_regions(void)
{
    char far *cell;
    int neighbor_c, ranked_l[16], id_d, position_l, terrain_l, y_h, rank_e, order_e[64];
    int cx_f, label, counts_e[12], pass_j, running_p, cy_e, x_i;
    for (cx_f = 0; cx_f < 12; cx_f++)
        counts_e[cx_f] = 0;
    for (pass_j = 0; pass_j < 2; pass_j++) {
        for (cx_f = 0; cx_f < 80; cx_f++)
            for (y_h = 0; y_h < 50; y_h++)
                map_visible_to[cx_f][y_h] = 0;
        id_d = 0;
        for (y_h = 0; y_h < 50; y_h++) {
            label = -1;
            running_p = 0;
            for (cx_f = 0; cx_f < 80; cx_f++) {
                terrain_l = gen_terrain_at(cx_f, y_h);
                if (pass_j == 0)
                    counts_e[terrain_l]++;
                if ((terrain_l != 10) == (!pass_j)) {
                    if (y_h > 0)
                        for (position_l = cx_f ? -1 : 0; position_l <= (cx_f < 79 ? 1 : 0); position_l++) {
                            neighbor_c = map_visible_to[cx_f + position_l][y_h - 1];
                            if (neighbor_c) {
                                if (label != -1 && label != neighbor_c) {
                                    for (cy_e = 0; cy_e <= y_h; cy_e++)
                                        for (x_i = 0; x_i < 80; x_i++) {
                                            cell = &map_visible_to[x_i][cy_e];
                                            if (*cell == label)
                                                *cell = neighbor_c;
                                            continent_span[pass_j][neighbor_c] += continent_span[pass_j][label];
                                            continent_span[pass_j][label] = 0;
                                        }
                                }
                                label = neighbor_c;
                                story_step();
                            }
                        }
                    if (label == -1) {
                        if (!running_p) {
                            id_d = 0;
                            while (continent_span[pass_j][++id_d]);
                        }
                        label = id_d;
                    }
                    map_visible_to[cx_f][y_h] = label;
                    continent_span[pass_j][label]++;
                    running_p = 1;
                } else {
                    running_p = 0;
                    label = -1;
                }
            }
            story_step();
        }
        for (cx_f = 0; cx_f < 16; cx_f++)
            ranked_l[cx_f] = 0;
        order_e[0] = 0;
        for (cx_f = 1; cx_f < 64; cx_f++) {
            order_e[cx_f] = 15;
            for (y_h = 1; y_h < 15; y_h++) {
                if (continent_span[pass_j][cx_f] > continent_span[pass_j][ranked_l[y_h]]) {
                    for (position_l = 15; position_l > y_h; position_l--) {
                        order_e[ranked_l[position_l - 1]] = position_l;
                        ranked_l[position_l] = ranked_l[position_l - 1];
                    }
                    order_e[cx_f] = y_h;
                    ranked_l[y_h] = cx_f;
                    break;
                }
            }
        }
        order_e[0] = 0;
        for (cx_f = 0; cx_f < 80; cx_f++)
            for (y_h = 0; y_h < 50; y_h++) {
                rank_e = order_e[map_visible_to[cx_f][y_h]];
                if (rank_e)
                    map_claim(cx_f, y_h + 50, rank_e, scrbuf->number);
            }
        for (cx_f = 1; cx_f < 15; cx_f++)
            ranked_l[cx_f] = continent_span[pass_j][ranked_l[cx_f]];
        for (cx_f = 1; cx_f < 15; cx_f++)
            continent_span[pass_j][cx_f] = ranked_l[cx_f];
        continent_span[pass_j][0] = 0;
        continent_span[pass_j][15] = 1;
    }
    story_step();
    blit(scrbuf, 0, 0, 320, 200, tile_page, 0, 0);
    link_land(1L);
    link_seas(1L);
    story_step();
    score_sites();
    page_line(scrbuf, 0, 0, 79, 0, 15);
    page_line(scrbuf, 0, 49, 79, 49, 15);
    for (cx_f = 0; cx_f < 20; cx_f++) {
        world_draw(scrbuf, rand_scaled(80), 0, 7);
        world_draw(scrbuf, rand_scaled(80), 1, 7);
        world_draw(scrbuf, rand_scaled(80), 48, 7);
        world_draw(scrbuf, rand_scaled(80), 49, 7);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void score_sites(void)
{
    int tile_c, i_h, y_h, x_i, ny_j, weights_f[24], nx_k, total_m, value_d, type_o;
    for (i_h = 0; i_h < 16; i_h++)
        ai_zone_places[i_h] = 0;
    for (i_h = 0; i_h < 24; i_h++) {
        weights_f[i_h] = terrains[i_h].food * 3 + terrains[i_h].trade;
        type_o = i_h % 12;
        if (type_o != 2 && type_o != 11)
            weights_f[i_h] += terrains[i_h].shields * 2;
        if (work_rules[type_o].trade < 0)
            weights_f[i_h] += (-1 - work_rules[type_o].trade);
        else if (work_rules[type_o].food < 0)
            weights_f[i_h] += (-1 - work_rules[type_o].food) * 2;
    }

    for (x_i = 2; x_i < 78; x_i++)
        for (y_h = 2; y_h < 48; y_h++) {
            tile_c = gen_terrain_at(x_i, y_h);
            if (tile_c == 11 || tile_c == 2 || tile_c == 1) {
                total_m = 0;
                for (i_h = 0; i_h <= 20; i_h++) {
                    value_d = 0;
                    nx_k = x_i + /* Economy's city radius, not the eight-direction spiral */ radius_dx[i_h];
                    ny_j = y_h + radius_dy[i_h];
                    tile_c = gen_terrain_at(nx_k, ny_j);
                    if ((tile_c == 2 || tile_c == 11) && !((nx_k * 7 + ny_j * 11) & 2))
                        value_d += 2;
                    if (patterned_cell(tile_c, nx_k, ny_j))
                        tile_c += 12;
                    value_d += weights_f[tile_c];
                    if (i_h < 9)
                        value_d += value_d;
                    if (i_h == 0)
                        value_d += value_d;
                    total_m += value_d;
                }
                tile_c = gen_terrain_at(x_i, y_h);
                if (tile_c != 1 && ((x_i * 7 + y_h * 11) & 2))
                    total_m -= 16;
                value_d = clamp((total_m - 120) / 8, 1, 15);
                value_d = value_d / 2 + 8;
                map_claim(x_i + 80, y_h, value_d, scrbuf->number);
                map_claim(x_i + 80, y_h + 50, value_d, scrbuf->number);
                ai_zone_places[gen_region_at(x_i, y_h)]++;
            } else {
                map_claim(x_i + 80, y_h, 0, scrbuf->number);
                map_claim(x_i + 80, y_h + 50, 0, scrbuf->number);
            }
        }
}

void link_land(int show)
{
    int spare26_n, ty_d, spare22_b, tx_e, spare1e_p, target_y_b, cy_e, cid, target_x_c, cx_f, spare12_c, unused_m, nid_f, oy_i, dir_b, ox_j, spare6_p, y_h, x_i;
    memset(block_ground_way, 0, 260);
    for (cx_f = 0; cx_f < 20; cx_f++)
        for (cy_e = 0; cy_e < 13; cy_e++) {
            ox_j = cx_f * 4 + 1;
            oy_i = cy_e * 4 + 1;
            cid = -1;
            /* Share the selected Y coordinate between neighbouring-cell tests. */
            if (gen_terrain_at(ox_j, oy_i) != 10) {
                cid = region_key(ox_j, oy_i);
                tx_e = ox_j;
set_y1:        ty_d = oy_i;
            } else if (gen_terrain_at(ox_j + 1, oy_i) != 10) {
                cid = region_key(ox_j + 1, oy_i);
                tx_e = ox_j + 1;
                goto set_y1;
            } else if (gen_terrain_at(ox_j, oy_i + 1) != 10) {
                cid = region_key(ox_j, oy_i + 1);
                tx_e = ox_j;
                ty_d = oy_i + 1;
            } else
                goto extra0;
join0:     ;
            if (cid == -1)
                continue;
            goto skip_src;
extra0:    if (gen_terrain_at(ox_j + 1, oy_i + 1) != 10) {
                cid = region_key(ox_j + 1, oy_i + 1);
                tx_e = ox_j + 1;
                ty_d = oy_i + 1;
            }
            goto join0;
skip_src:  ;
            for (dir_b = 1; dir_b <= 4; dir_b++) {
                x_i = ox_j + spiral_x[dir_b] * 4;
                y_h = oy_i + spiral_y[dir_b] * 4;
                nid_f = -1;
                if (gen_terrain_at(x_i, y_h) != 10) {
                    nid_f = region_key(x_i, y_h);
                    target_x_c = x_i;
set_y2:            target_y_b = y_h;
                } else if (gen_terrain_at(x_i + 1, y_h) != 10) {
                    nid_f = region_key(x_i + 1, y_h);
                    target_x_c = x_i + 1;
                    goto set_y2;
                } else if (gen_terrain_at(x_i, y_h + 1) != 10) {
                    nid_f = region_key(x_i, y_h + 1);
                    target_x_c = x_i;
                    target_y_b = y_h + 1;
                } else
                    goto extra1;
join1:         ;
                goto skip_extra;
extra1:        if (gen_terrain_at(x_i + 1, y_h + 1) != 10) {
                    nid_f = region_key(x_i + 1, y_h + 1);
                    target_x_c = x_i + 1;
                    target_y_b = y_h + 1;
                }
                goto join1;
skip_extra:    ;
                if (cid != nid_f)
                    continue;
                unused_m = path_probe(tx_e, ty_d, target_x_c, target_y_b, 0, 20);
                if (unused_m == -1 || unused_m >= 20)
                    continue;
                block_ground_way[cx_f][cy_e] |= 1 << (dir_b - 1);
                x_i = cx_f + spiral_x[dir_b];
                y_h = cy_e + spiral_y[dir_b];
                if (x_i >= 0 && x_i < 20 && y_h >= 0 && y_h < 13)
                    block_ground_way[x_i][y_h] |= 1 << ((dir_b + 3) & 7);
            }
        }
}

extern unsigned char coarse_ocean_map[20][13];

void link_seas(int show)
{
    int cy_e, cid, cx_f, step_e, valid, nid_f, oy_i, dir_b, ox_j, count_h, y_h, x_i;
    memset(coarse_ocean_map, 0, 260);
    for (cx_f = 0; cx_f < 20; cx_f++) {
        for (cy_e = 0; cy_e < 12; cy_e++) {
            ox_j = cx_f * 4 + 1;
            oy_i = cy_e * 4 + 1;
            cid = -1;
            /* Original O7 [1440,176D) uses 2D parameter tail-merging (right-to-left push):
               Tiles 0/1 share y=oy_i; Tiles 2/3 share y=oy_i+1; Tiles 0/2 share x=ox_j; Tiles 1/3 share x=ox_j+1.
               Tiles 2 and 3 jump forward into Tiles 0 and 1; Tile 1 jumps forward to Tile 0 at +118 bytes (EB 76). */
            if (gen_terrain_at(ox_j, oy_i) == 10) {
                cid = gen_region_at(ox_j, oy_i);
                goto got_cid;
            }
            if (gen_terrain_at(ox_j + 1, oy_i) == 10) {
                cid = gen_region_at(ox_j + 1, oy_i);
                goto got_cid;
            }
            if (gen_terrain_at(ox_j, oy_i + 1) == 10) {
                cid = gen_region_at(ox_j, oy_i + 1);
                goto got_cid;
            }
            if (gen_terrain_at(ox_j + 1, oy_i + 1) == 10) {
                cid = gen_region_at(ox_j + 1, oy_i + 1);
                goto got_cid;
            }
got_cid:   ;
            if (cid == -1)
                continue;
            for (dir_b = 1; dir_b <= 8; dir_b++) {
                x_i = ox_j + spiral_x[dir_b] * 4;
                y_h = oy_i + spiral_y[dir_b] * 4;
                if (gen_terrain_at(x_i, y_h) == 10) {
                    nid_f = gen_region_at(x_i, y_h);
                    goto got_nid;
                }
                if (gen_terrain_at(x_i + 1, y_h) == 10) {
                    nid_f = gen_region_at(x_i + 1, y_h);
                    goto got_nid;
                }
                if (gen_terrain_at(x_i, y_h + 1) == 10) {
                    nid_f = gen_region_at(x_i, y_h + 1);
                    goto got_nid;
                }
                if (gen_terrain_at(x_i + 1, y_h + 1) == 10) {
                    nid_f = gen_region_at(x_i + 1, y_h + 1);
                    goto got_nid;
                }
got_nid:       ;
                if (cid != nid_f)
                    continue;
                x_i = ox_j;
                y_h = oy_i;
                valid = 1;
                for (step_e = 0; step_e <= 4; step_e++) {
                    x_i += spiral_x[dir_b];
                    y_h += spiral_y[dir_b];
                    count_h = 4;
                    if (gen_terrain_at(x_i, y_h) != 10 || !map_in_bounds(x_i, y_h))
                        count_h--;
                    if (gen_terrain_at(x_i + 1, y_h) != 10 || !map_in_bounds(x_i + 1, y_h))
                        count_h--;
                    if (gen_terrain_at(x_i, y_h + 1) != 10 || !map_in_bounds(x_i, y_h + 1))
                        count_h--;
                    if (gen_terrain_at(x_i + 1, y_h + 1) != 10 || !map_in_bounds(x_i + 1, y_h + 1))
                        count_h--;
                    if (count_h < 2) {
                        valid = 0;
                        break;
                    }
                }
                if (!valid && !(cy_e == 11 && dir_b == 3))
                    continue;
                if (spiral_y[dir_b] + cy_e < 12) {
                    coarse_ocean_map[cx_f][cy_e] |= 1 << (dir_b - 1);
                    x_i = spiral_x[dir_b] + cx_f;
                    y_h = spiral_y[dir_b] + cy_e;
                    if (x_i >= 0 && x_i < 20 && y_h >= 0 && y_h <= 12) {
                        coarse_ocean_map[x_i][y_h] |= 1 << ((dir_b + 3) & 7);
                    }
                }
            }
        }
        if (show)
            story_step();
    }
    for (cy_e = 0; cy_e < 12; cy_e++) {
        coarse_ocean_map[0][cy_e] |= 0xe0;
        coarse_ocean_map[19][cy_e] |= 0x0e;
    }
    coarse_ocean_map[0][0] &= 0x7f;
    coarse_ocean_map[0][11] &= 0xdf;
    coarse_ocean_map[19][0] &= 0xfd;
    coarse_ocean_map[19][11] &= 0xf7;
}

int gen_terrain_at(int x, int y)
{
    return terrain_map[help_probe(scrbuf->number, x, y)];
}

int gen_region_at(int x, int y)
{
    return help_probe(scrbuf->number, x, y + 50);
}

int coarse_valid(int x, int y)
{
    if (x < 0 || y < 0 || x >= 20 || y >= 13)
        return 0;
    return 1;
}

/* Initialized story state follows literals emitted by overlay7_story. */
int story_mode = 0, story_picture = 1, story_cycle_offset = 0, story_cycle_count = 0;
/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int story_step(void)
{
    int speed_j_g, read_result_g, cycle, last_m, first_i;
    char picture_file_f[16];
    scrbuf->number = 0;
    if (story_mode >= 2 && !kbhit()) {
        if (story_mode < 5) {
            story_mode++;
            if (story_mode == 5)
                rect_fill(scrbuf, 40, 160, 240, 8, 0);
        } else {
            story_clock = timer_read() / 60;
            draw_text("BUILDING NEW WORLD...", 160, 160, (story_clock & 1) ? 15 : 3);
        }
        scrbuf->number = 1;
        return 0;
    }
    if (story_mode == 0) {
        extern FILE *story_file;
        story_file = fopen("story.txt", "rt");
        story_mode = 1;
        story_clock = story_wait = 0;
        strcpy(msgtext, "");
        scrbuf->font = 7;
        story_line = 0;
    }
    if (story_line != 0)
        story_clock = sound_quiet_ticks();
    if (story_clock < story_wait) {
        scrbuf->number = 1;
        return 1;
    }
    if (story_mode != 2 && story_line < 40) {
        draw_text(msgtext, 160, 160, 3);
        flip(5);
    }
    {
        extern FILE *story_file;
        read_result_g = fscanf(story_file, "%[^\n]\n", msgtext);
    }
    if (read_result_g != -1 && !(kbhit() && story_skip)) {
        if (story_picture > 1)
            blit(tile_page, 0, 160, 320, 8, scrbuf, 0, 160);
        flip(10);
        draw_text(msgtext, 160, 160, 3);
        flip(5);
        draw_text(msgtext, 160, 160, 11);
        story_wait = 0;
        if ((int) strlen(msgtext) > 3) {
            if (story_line == 0) {
                timer_reset();
                sound(1);
                sound(4);
            }
            story_wait += 2;
            story_line++;
        } else {
            strcpy(picture_file_f, "birth0.pic");
            picture_file_f[5] += story_picture++;
            if (vga_modes)
                for (cycle = 1; cycle <= story_cycle_count; cycle++)
                    stop_cycle(cycle);
            load_screen(2, picture_file_f);
            if (vga_modes)
                set_palette(8, 0, 0, 0);
            blit(tile_page, 0, 0, 320, 200, scrbuf, 0, 0);
            if (vga_modes) {
                strcpy(picture_file_f + 7, "pal");
                load_palette(picture_file_f, icon_colours_block);
                install_palette(8, icon_colours_block);
                story_cycle_count = 0;
                while (story_cycles[story_cycle_offset]) {
                    first_i = story_cycles[story_cycle_offset++];
                    last_m = story_cycles[story_cycle_offset++];
                    speed_j_g = 300 / story_cycles[story_cycle_offset++];
                    set_cycle(++story_cycle_count, speed_j_g, first_i, last_m);
                }
                story_cycle_offset++;
                for (cycle = 1; cycle <= story_cycle_count; cycle++)
                    start_cycle(cycle);
            }
        }
        if ((int) strlen(msgtext) > 19)
            story_wait++;
        story_wait = clamp(story_delays[story_line], story_clock + 45 * story_wait, 32767);
    } else {
        extern FILE *story_file;
        if (read_result_g == -1)
            flip(180);
        fclose(story_file);
        sound(1);
        if (vga_modes)
            for (cycle = 1; cycle <= story_cycle_count; cycle++)
                stop_cycle(cycle);
        story_mode = 2;
    }
    set_page(0);
    scrbuf->number = 1;
    return story_mode != 2;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void global_warming(int severity)
{
    char far *seen_f;
    int sea_h, terrain_l, y_h, dir_b, x_i;
    strcpy(msgtext, "Global temperature\nrises! Icecaps melt.\nSevere Drought.\n");
    news_box(-1);
    for (x_i = 0; x_i < 80; x_i++)
        for (y_h = 0; y_h < 50; y_h++) {
            terrain_l = terrain_at(x_i, y_h);
            if (terrain_l > 3)
                continue;
            sea_h = 0;
            for (dir_b = 1; dir_b <= 8; dir_b++)
                if (terrain_at(x_i + spiral_x[dir_b], y_h + spiral_y[dir_b]) == 10)
                    sea_h++;
            if (sea_h >= 7 - severity) {
                seen_f = &map_visible_to[x_i][y_h];
                *seen_f |= 1;
                map_claim(x_i, y_h, terrain_l == 3 ? 11 : 3, 2);
                unflag(6, x_i, y_h);
            } else if (((unsigned char) (x_i * 11 + y_h * 13) & 7) == severity) {
                seen_f = &map_visible_to[x_i][y_h];
                *seen_f |= 1;
                map_claim(x_i, y_h, terrain_l <= 1 ? 14 : 6, 2);
            } else
                continue;
            if (*seen_f & (1 << civnow))
                map_note(x_i, y_h);
        }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Same-unit far arrays share ES, but MSC retains both CONST selectors. */
unsigned char far story_cycles[131] = {
    146, 152, 21, 183, 187, 5, 188, 191, 17, 0, 48, 56, 11, 32, 47, 10, 141, 148, 13, 176, 183, 23, 211, 215, 3, 152, 155, 1, 57, 79, 15, 0, 80, 95, 17, 96, 111, 7, 112, 127, 30, 136, 140, 10, 184, 188, 11, 0, 134, 138, 5, 139, 143, 10, 240, 241, 23, 96, 101, 25, 242, 244, 30, 245, 250, 12, 251, 252, 11, 102, 105, 21, 0, 184, 187, 21, 180, 183, 16, 135, 140, 12, 214, 215, 21, 96, 102, 17, 0, 136, 138, 21, 129, 131, 8, 246, 249, 21, 250, 254, 12, 0, 246, 249, 21, 250, 251, 10, 208, 210, 11, 135, 138, 16, 0, 208, 210, 16, 216, 218, 11, 131, 134, 16, 48, 53, 25, 246, 249, 64, 0
};

unsigned int far story_delays[45] = {
    0, 192, 336, 624, 768, 912, 1104, 1200, 1440, 1632, 1872, 2016, 2256, 2352, 2496, 2688, 2880, 3168, 3456, 3648, 3744, 3840, 3936, 4032, 4224, 4416, 4608, 4800, 4992, 5184, 5376, 5568, 5760, 6000, 6192, 6336, 6528, 6720, 7008, 7296, 7584, 7872, 8064, 8512, 65535
};
