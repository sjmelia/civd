/* Overlay 17: Palace view and upgrades. */
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "CAPTURE.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "FADE.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "TURN.H"
#include "MESSAGE.H"
#include "PALACE.H"

struct civ_type {
    char strings[48];
    int f0, f1, f2, f3, f4;
};

extern struct civ_type civt[];

extern void disk_insert();

extern void sound();

extern int kbhit(void);
extern void LOAD_PIC();

void palpic();                  /* NOLINT(clang-diagnostic-deprecated-non-prototype) */
void calc_section_type();       /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

void load_castle_tiles();       /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

int centre_width = 48;
int wing_width = 24;
int spare_width = 24;           /* unused */
int tower_width = 25;
int end_width = 25;

char section_rows[5][15] = {
    { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 2, 2, 1, 0, 0, 1, 1, 1, 0, 0, 2, 2 },
    { 1, 0, 0, 2, 2, 1, 0, 0, 1, 1, 1, 0, 0, 2, 2 },
    { 1, 3, 3, 2, 2, 1, 0, 0, 1, 1, 1, 0, 0, 2, 2 },
    { 1, 3, 3, 2, 2, 1, 0, 0, 1, 1, 1, 0, 0, 2, 2 }
};

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void palace_view(int count)
{
    /* Letter suffixes retain MSC local allocation buckets. */
    char message_buf_p[256];
    int has_choice_l, draw_x_n, preview_height_j, section_index_i, shape_or_style_e, unused_word_h, chosen_section_d, new_level_h;

    mouse_off();
    msgtext[0] = 0;
    named_msg("*PALACE");
    strcpy(message_buf_p, msgtext);
    disk_insert(2, 1L);
    scrbuf->number = 1;
    palpic(0, 0);
    scrbuf->number = 0;
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    cycle_off();
    sound(civt[civ_tribe_of[civnow]].f4, 3);
    load_back("cback.pic");
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    menu_dialog(message_buf_p, 20, 16);
    blit(back_page, 20, 16, 280, 44, scrbuf, 20, 16);
    blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
    abort_flag = 1;
    menu_dialog("Which section shall we improve?\n", 40, 16);
    has_choice_l = 0;
    for (section_index_i = 1; section_index_i < 8; section_index_i++) {
        if (floors_palace[section_index_i] < 4 && (floors_palace[section_index_i - 1] > 0 || floors_palace[section_index_i + 1] > 0 || floors_palace[section_index_i] >= 0)) {
            draw_x_n = 48 * section_index_i - 36;
            msgtext[0] = 0;
            strcat(msgtext, itoa(section_index_i, nstr, 10));
            draw_shadowed(msgtext, draw_x_n, 144, 14);
            has_choice_l = 1;
        }
    }
    for (section_index_i = 0; section_index_i < 3; section_index_i++) {
        if (floors_palace[9 + section_index_i] < 3) {
            strcpy(msgtext, "A");
            msgtext[0] += section_index_i;
            draw_shadowed(msgtext, 120 * section_index_i + 40, 160, 14);
            has_choice_l = 1;
        }
    }
    if (has_choice_l == 0) {
        goto finish;
    }
    mouse_on();
    while (1) {
        do {
            poll_mouse();
        } while (mse_held == 0 && !kbhit());
        if (!kbhit()) {
            if (ptr_y < 154)
                chosen_section_d = (ptr_x + 12) / 48 + 1;
            else
                chosen_section_d = ptr_x / 107 + 9;
        } else {
            chosen_section_d = key_cmd();
            if (chosen_section_d < '8')
                chosen_section_d -= '0';
            else
                chosen_section_d -= 0x58;
            if (chosen_section_d < 1 || chosen_section_d > 11)
                continue;
        }
        if (floors_palace[chosen_section_d] >= 4) {
            continue;
        } else if (chosen_section_d >= 9 && floors_palace[chosen_section_d] >= 3) {
            continue;
        }
        break;
    }
    /* Extend an unbuilt wing from the nearest existing section. */
    if (chosen_section_d < 9) {
        while (chosen_section_d < 4 && floors_palace[chosen_section_d + 1] <= 0 && floors_palace[chosen_section_d] == -1)
            chosen_section_d++;
        while (chosen_section_d > 4 && floors_palace[chosen_section_d - 1] <= 0 && floors_palace[chosen_section_d] == -1)
            chosen_section_d--;
    }
    mouse_off();
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    new_level_h = clamp(floors_palace[chosen_section_d] + 1, 1, 4);
    if (chosen_section_d < 9) {
        for (section_index_i = 1; section_index_i < 8; section_index_i++) {
            royal_choices[section_index_i][0] = royal_choices[section_index_i][1] = -1;
        }
        calc_section_type(chosen_section_d, 0L);
        for (section_index_i = 1; section_index_i <= 3; section_index_i++) {
            royal_choices[section_index_i][0] = royal_choices[chosen_section_d][0];
            royal_choices[section_index_i][1] = royal_choices[chosen_section_d][1];
        }
        selected_tile = -1;
        sprite_arena_init();
        load_castle_tiles(1 << new_level_h, 0L);
        sprite_arena_release();
        preview_height_j = 92;
        rect_fill(scrbuf, 16, 2, 288, 102, 9);
        draw_text("Select new section.", 160, 3, 15);
        for (section_index_i = 1; section_index_i <= 3; section_index_i++) {
            draw_x_n = 100 * section_index_i - 80;
            rect_fill(scrbuf, draw_x_n, 10, 80, preview_height_j, !vga_modes ? 3 : 1);
            draw_x_n += 16;
            shape_or_style_e = 2;
            if (chosen_section_d < 4) {
                shape_or_style_e = 1;
                if (floors_palace[chosen_section_d - 1] == -1 || chosen_section_d == 1)
                    shape_or_style_e = 0;
            }
            if (chosen_section_d > 4) {
                shape_or_style_e = 1;
                if (floors_palace[chosen_section_d + 1] == -1 || chosen_section_d == 7)
                    shape_or_style_e = 3;
            }
            /* Shapes: 0 left tower, 1 wing, 2 centre, 3 right end. */
            switch (shape_or_style_e) {
            case 0:
                if (chosen_section_d != 1)
                    sprite_draw(scrbuf, draw_x_n + tower_width, section_rows[new_level_h][5 * section_index_i - 4] + 1, section_art[section_index_i][1]);
                sprite_draw(scrbuf, draw_x_n, section_rows[new_level_h][5 * section_index_i - 2] + 1, section_art[section_index_i][0]);
                draw_x_n += tower_width;
                break;
            case 1:
                sprite_draw(scrbuf, draw_x_n, section_rows[new_level_h][5 * section_index_i - 4] + 1, section_art[section_index_i][0]);
                draw_x_n += wing_width;
                sprite_draw(scrbuf, draw_x_n, section_rows[new_level_h][5 * section_index_i - 3] + 1, section_art[section_index_i][1]);
                break;
            case 2:
                sprite_draw(scrbuf, draw_x_n, section_rows[new_level_h][5 * section_index_i - 5] + 1, section_art[section_index_i][0]);
                break;
            case 3:
                if (chosen_section_d != 7) {
                    sprite_draw(scrbuf, draw_x_n, section_rows[new_level_h][5 * section_index_i - 4] + 1, section_art[section_index_i][0]);
                    draw_x_n += wing_width;
                }
                sprite_draw(scrbuf, draw_x_n - (section_index_i == 2 ? 3 : 2), section_rows[new_level_h][5 * section_index_i - 1] + 1, section_art[section_index_i][1]);
                break;
            }
            msgtext[0] = 0;
            strcat(msgtext, itoa(section_index_i, nstr, 10));
            draw_shadowed(msgtext, 100 * section_index_i - 44, 12, 15);
        }
        free_block(selected_tile, "pal1");
        repaint();
        mouse_on();
        do {
            do {
                poll_mouse();
            } while (mse_held == 0 && !kbhit());
            if (!kbhit()) {
                shape_or_style_e = ptr_x / 107;
                break;
            }
            shape_or_style_e = key_cmd() - '1';
        } while (shape_or_style_e < 0 || shape_or_style_e > 2);
        mouse_off();
        scrbuf->number = 1;
        palpic(0, 0);
        scrbuf->number = 0;
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    }
    /* The preview shape word is reused for the chosen architectural style. */
    floors_palace[chosen_section_d] = new_level_h;
    section_grades[chosen_section_d] = shape_or_style_e;
    scrbuf->number = 1;
    palpic(0, 0);
    scrbuf->number = 0;
    page_dissolve(1, 20);
    repaint();
    ad_wait();
finish:
    set_page(0);
    sound(1);
    disk_insert(0, 1L);
    if (!vga_modes)
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
    sel_player();
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */


/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void palpic(flag, b)
int flag, b;
{
    /* Cache both halves of each section at the levels present in the palace. */
    char filename_p[16];
    int left_tower_g, level_mask_p, unused_a_p, right_edge_c, unused_b, draw_x_b, unused_c_b, draw_y_c, i, j, section_shape_g, unused_d_f, left_brush_p, right_brush_n, level_h;

    disk_insert(2, 1L);
    scrbuf->number = 1;
    selected_tile = -1;
    level_mask_p = 0;
    for (i = 0; i < 9; i++) {
        if (floors_palace[i] != -1)
            level_mask_p |= (1 << floors_palace[i]);
    }
    for (i = 1; i < 8; i++) {
        royal_choices[i][0] = royal_choices[i][1] = -1;
        if (floors_palace[i] != -1)
            calc_section_type(i, 1L);
    }
    sprite_arena_init();
    load_castle_tiles(level_mask_p, 1L);
    sprite_arena_release();
    if (flag != 0)
        load_screen(1, "sad.pic");
    else
        load_screen(1, "cback.pic");
    if (floors_palace[10] > 0) {
        strcpy(filename_p, "cbacks0.pic");
        filename_p[6] = '0' + floors_palace[10];
        LOAD_PIC(1, 0, 135, filename_p, 0);
    }
    draw_x_b = 16;
    if (floors_palace[2] == -1)
        draw_x_b -= 24;
    if (section_grades[4] == 1)
        draw_x_b++;
    draw_y_c = 36;
    /* Preserve the original edge scan and its conditional initialization. */
    i = 8;
    while (floors_palace[i--] == -1)
        right_edge_c = i;
    for (i = 1; i < 8; i++) {
        if (floors_palace[i] != -1) {
            level_h = floors_palace[i];
            section_shape_g = 2;
            if (i < 4) {
                section_shape_g = 1;
                if (floors_palace[i - 1] == -1 || i == 1)
                    section_shape_g = 0;
            }
            if (i > 4) {
                section_shape_g = 1;
                if (floors_palace[i + 1] == -1 || i == 7)
                    section_shape_g = 3;
            }
            switch (section_shape_g) {
            case 0:
                if (i == 3)
                    draw_section(level_h, i, draw_x_b + tower_width, draw_y_c, 1);
                if (section_grades[i] != 1 || i == 3)
                    draw_section(level_h, i, draw_x_b, draw_y_c, 0);
                draw_x_b += tower_width;
                if (i == 3)
                    draw_x_b += wing_width;
                left_tower_g = i;
                break;
            case 1:
                if (left_tower_g + 1 == i && section_grades[i] == section_grades[left_tower_g])
                    draw_section(level_h, left_tower_g, draw_x_b + wing_width - 17, draw_y_c - 2, 0);
                if (right_edge_c - 1 == i && section_grades[i] == section_grades[right_edge_c])
                    draw_section(level_h, right_edge_c, draw_x_b + wing_width - 17, draw_y_c - 2, 1);
                draw_section(level_h, i, draw_x_b, draw_y_c, 0);
                draw_x_b += wing_width;
                draw_section(level_h, i, draw_x_b, draw_y_c, 1);
                draw_x_b += wing_width;
                break;
            case 2:
                if (section_grades[4] == 1)
                    draw_x_b--;
                draw_section(level_h, i, draw_x_b - 2, draw_y_c, 0);
                draw_x_b += centre_width;
                if (section_grades[4] == 1)
                    draw_x_b--;
                break;
            case 3:
                if (i == 5) {
                    draw_section(level_h, i, draw_x_b, draw_y_c, 0);
                    draw_x_b += wing_width;
                }
                if (section_grades[i] == 1 && floors_palace[i - 1] > floors_palace[i]) {
                    draw_section(level_h, i, draw_x_b - 3, draw_y_c + floors_palace[i - 1] - floors_palace[i] + 1, 1);
                } else {
                    draw_section(level_h, i, draw_x_b - (section_grades[i] == 1 ? 3 : 2), draw_y_c, 1);
                }
                draw_x_b += end_width;
                break;
            }
        } else {
            draw_x_b += 48;
        }
    }
    if (section_grades[left_tower_g] == 1 && left_tower_g != 3) {
        draw_section(floors_palace[left_tower_g], left_tower_g, 48 * left_tower_g - 36, draw_y_c + floors_palace[left_tower_g + 1] - floors_palace[left_tower_g], 0);
    }
    free_block(selected_tile, "pal2");
    if (floors_palace[9] != 0 || floors_palace[11] != 0) {
        sprite_arena_init();
        strcpy(filename_p, "cbrush0.pic");
        if (floors_palace[9] != 0) {
            filename_p[6] = '0' + floors_palace[9] * 2 - 2;
            left_brush_p = load_cursor(filename_p);
            sprite_draw(scrbuf, 0, floors_palace[9] > 1 ? 94 : 105, left_brush_p);
        }
        if (floors_palace[11] != 0) {
            filename_p[6] = '0' + floors_palace[11] * 2 - 1;
            right_brush_n = load_cursor(filename_p);
            sprite_draw(scrbuf, 184, floors_palace[11] > 1 ? 94 : 105, right_brush_n);
        }
        sprite_arena_release();
        free_block((floors_palace[9] != 0) ? left_brush_p : right_brush_n);
    }
    if (flag != 0) {
        scrbuf->number = 0;
        if (vga_modes && found_floppy > 7)
            set_palette(5, 0, 0, 0);
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
        load_back("sad.pic");
        disk_insert(0, 1L);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */


/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void calc_section_type(section, flag)
int section, flag;
{
    int height;
    int side;

    height = floors_palace[section];
    side = 2;
    if (section < 4) {
        side = 1;
        if (floors_palace[section - 1] == -1 || section == 1)
            side = 0;
    }
    if (section > 4) {
        side = 1;
        if (floors_palace[section + 1] == -1 || section == 7)
            side = 3;
    }
    /* Match exposed ends and adjoining wing heights to the appropriate artwork. */
    switch (side) {
    case 0:
        royal_choices[section][0] = 3;
        if (section == 3 || flag == 0) {
            royal_choices[section][1] = (height != 0) ? 1 : 2;
        }
        break;
    case 1:
        if (floors_palace[section - 1] > floors_palace[section] || section == 5 || floors_palace[section - 2] == -1)
            royal_choices[section][0] = 1;
        else
            royal_choices[section][0] = 2;
        royal_choices[section][1] = 2;
        break;
    case 2:
        royal_choices[section][0] = 0;
        break;
    case 3:
        if (section == 5 || flag == 0) {
            if (floors_palace[section - 1] > floors_palace[section] || section == 5)
                royal_choices[section][0] = 1;
            else
                royal_choices[section][0] = 2;
        }
        royal_choices[section][1] = 4;
        break;
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void draw_section(int a, int section, int x, int y, int d)
{
    int unused_height_e, style;

    unused_height_e = floors_palace[section];
    style = section_grades[section];
    sprite_draw(scrbuf, x, (int) section_rows[a][style * 5 + royal_choices[section][d]] + y + a, section_art[section][d]);
#undef unused_height_e
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void load_castle_tiles(mask, flag)
int mask, flag;
{
    int section, part, x, y, lvl;
    char filename[16];

    for (lvl = 0; lvl < 5; lvl++) {
        if (mask & (1 << lvl)) {
            strcpy(filename, "castle0.pic");
            filename[6] = '0' + lvl;
            load_screen(1, filename);
            for (section = 1; section < 8; section++) {
                if (floors_palace[section] == lvl || flag == 0) {
                    x = (section_grades[section] & 1) * 160;
                    y = (section_grades[section] & 2) * 50;
                    if (flag == 0) {
                        x = ((section - 1) & 1) * 160;
                        y = ((section - 1) & 2) * 50;
                    }
                    for (part = 0; part < 2; part++) {
                        switch (royal_choices[section][part]) {
                        case 0:
                            section_art[section][part] = cut_tile(1, x, y + 1, 52, 99);
                            break;
                        case 1:
                            section_art[section][part] = cut_tile(1, x + 53, y + 1, 24, 99);
                            break;
                        case 2:
                            section_art[section][part] = cut_tile(1, x + 78, y + 1, 24, 99);
                            break;
                        case 3:
                            if ((flag != 0 && section_grades[section] != 1) || (flag == 0 && section != 2))
                                section_art[section][part] = cut_tile(1, x + 103, y + 1, 28, 99);
                            else
                                section_art[section][part] = cut_tile(1, 160, 101, 35, 99);
                            break;
                        case 4:
                            if ((flag != 0 && section_grades[section] != 1) || (flag == 0 && section != 2))
                                section_art[section][part] = cut_tile(1, x + 132, y + 1, 27, 99);
                            else
                                section_art[section][part] = cut_tile(1, 196, 101, 35, 99);
                            break;
                        }
                        if (royal_choices[section][part] != -1 && selected_tile == -1) {
                            selected_tile = section_art[section][part];
                        }
                    }
                }
            }
        }
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */
