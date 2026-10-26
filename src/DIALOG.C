/* Menus, mouse and keyboard selection, boxes and panels. */
#include <stdlib.h>
#include "AUDIENCE.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "DRIVER.H"

extern int strlen(), abs(int);
extern char *strcpy(char *, char *), *strcat(char *, char *);

extern int debug;

extern int kbhit(void);



/* initialised data */
int busy_flag = 0;
int editing = -1;
int edit_mode = 0;
int advisor = -1;
int help_flag = 0;
int used_flag = 0;
int line_height = 8;
char *advisor_title[] = {
    "Spies", "Diplomats", "Travelers", "Defense Minister:",
    "Domestic Advisor:", "Foreign Minister:", "Science Advisor:"
};

int menu_choose(char *text_m, int x_k, int y_k)
{
    return menu_core(text_m, x_k, y_k, 0);
}

int menu_dialog(char *text_m, int x_k, int y_k)
{
    return menu_core(text_m, x_k, y_k, 1);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int menu_core(char *text, int x, int y, int mode)
{
    /* Letter suffixes retain MSC stack slots. */
    int mouse_active_k, highlight_height_o, background_o, foreground_d, n, accepted_e, key_e, selected_e, prev;

    if (mode)
        repaint();
    selected_e = prev = -1;
    edit_mode = used_flag = 0;
    if (!has_mouse && selected_e == -1 && !need_ok_key && !box_info_flags)
        selected_e = 0;
    selected_e = 0;
    if (editing != -1)
        selected_e = editing;
    entry_first = -1;
    refresh_pending = 1;
    line_height = font_height(scrbuf->font);
    if (line_height == 9)
        line_height--;
    if (style != -1)
        line_height = 8;
    foreground_d = 7;
    background_o = 22;
    if (y == 139)
        background_o = 8;
    if ((y & 1) && y != 139) {
        foreground_d = pixel_at(0, x, y);
        background_o = -1;
    }
    hilite_colour = foreground_d == 15 ? 3 : 15;
    if (!vga_modes)
        background_o = -1;
    highlight_height_o = line_height;
    accepted_e = mouse_active_k = 0;
    mouse_off();
    menu_layout(text, x, y, editing);
    mouse_on();
    if (abort_flag) {
        abort_flag = 0;
        return -1;
    }
    do {
        if (style != -1)
            leader_animate();
        refresh_pending = 0;
        mse_held = 0;
        poll_mouse();
        if (mse_held || mouse_active_k) {
            mouse_active_k = used_flag = 1;
            if (mse_held == 2)
                edit_mode = 1;
            selected_e = (ptr_y - y - 4) / line_height - entry_first;
            if (x > ptr_x || ptr_x > panel_frame_rim)
                selected_e = -1;
            if (dialog_dim & (1 << selected_e))
                selected_e = prev;
            else if (!mse_held)
                accepted_e = 1;
        } else if (kbhit()) {
            key_e = key_cmd();
            switch (key_e) {
            case 0x5000:
                if (selected_e < menu_item_count - 1)
                    selected_e++;
                break;
            case 0x4800:
                if (selected_e > 0)
                    selected_e--;
                break;
            case 0x20:
            case 0xd:
                if (!(dialog_dim & (1 << selected_e)))
                    accepted_e = 1;
                break;
            case 0x2300:
                accepted_e = edit_mode = 1;
                break;
            case 0x1b:
                selected_e = -1;
                accepted_e = 1;
                break;
            case 0x2f00:
                debug ^= 0x10;
                if (!(debug & 0x10))
                    sound(1);
                break;
            default:
                for (n = selected_e + 1; n < 32; n++) {
                    if (itemkeys[n] == 0xff)
                        continue;
                    if ((itemkeys[n] & 0x1f) == (key_e & 0x1f)) {
                        selected_e = n;
                        break;
                    }
                }
            }
        }
        if (selected_e < 0 || selected_e >= menu_item_count || need_ok_key || box_info_flags)
            selected_e = -1;
        if (selected_e != prev) {
            mouse_off();
            if (prev != -1) {
                recolour_rect(scrbuf, x + 3, (prev + entry_first) * line_height + y + 4, linewide + 5, highlight_height_o, 11, foreground_d);
                if (background_o != -1)
                    recolour_rect(scrbuf, x + 3, (prev + entry_first) * line_height + y + 4, linewide + 5, highlight_height_o, 3, background_o);
            }
            if (selected_e != -1) {
                recolour_rect(scrbuf, x + 3, (selected_e + entry_first) * line_height + y + 4, linewide + 5, highlight_height_o, foreground_d, 11);
                if (background_o != -1)
                    recolour_rect(scrbuf, x + 3, (selected_e + entry_first) * line_height + y + 4, linewide + 5, highlight_height_o, background_o, 3);
            }
            prev = selected_e;
            mouse_on();
        }
    } while (!accepted_e);
    if (selected_e != -1) {
        mouse_off();
        if (!vga_modes)
            recolour_rect(scrbuf, x + 3, (selected_e + entry_first) * line_height + y + 4, linewide + 5, highlight_height_o, 11, edit_mode ? 11 : hilite_colour);
        flip(20);
        mouse_on();
    } else
        edit_mode = 0;
    editing = advisor = -1;
    marked_mask = dialog_dim = box_info_flags = help_flag = 0;
    return selected_e;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int menu_layout(char *text, int x, int y, int preset)
{
    int panel_bottom_j;
    char buf[80];
    char line_marker_g;
    int item_index_j, title_width_or_gap_h, index_or_font_b, len;

    /* Leading space/underscore marks a selectable row; other rows are headings. */
    line_off[0] = 0;
    if (refresh_pending == 1) {
        line_height = font_height(scrbuf->font);
        if (line_height == 9)
            line_height--;
        if (style != -1)
            line_height = 8;
        for (index_or_font_b = 0; index_or_font_b < 32; index_or_font_b++)
            itemkeys[index_or_font_b] = -1;
        line_count = item_index_j = 0;
        linewide = len = 0;
        for (index_or_font_b = 0; strlen(text) > index_or_font_b; index_or_font_b++) {
            if (text[index_or_font_b] == 10) {
                if (len > linewide)
                    linewide = len;
                len = 0;
                line_count++;
                line_off[line_count] = index_or_font_b + 1;
            } else {
                if (len == 0) {
                    line_marker_g = text[index_or_font_b];
                    if (line_marker_g == 32 || line_marker_g == 95) {
                        if (item_index_j < 32)
                            itemkeys[item_index_j] = text[index_or_font_b + 1];
                        if (entry_first == -1)
                            entry_first = line_count;
                        item_index_j++;
                    }
                }
                len += char_width(scrbuf->font, text[index_or_font_b]);
            }
        }
        line_count = clamp(line_count, 0, (192 - y) / line_height);
        panel_frame_rim = x + linewide + 8;
        panel_bottom_j = line_count * line_height + y + 6;
        if (help_flag)
            panel_bottom_j += 2;
        if (text[strlen(text) - 1] != 10)
            item_index_j--;
        menu_item_count = item_index_j;
        if (!(y & 1)) {
            if (advisor == -1)
                panel_frame(x, y, panel_frame_rim - x, panel_bottom_j - y, 7);
            else {
                strcpy(buf, advisor_title[advisor]);
                if (advisor <= 2)
                    strcat(buf, " report:");
                title_width_or_gap_h = string_width(buf);
                if (panel_frame_rim < x + title_width_or_gap_h + 8)
                    panel_frame_rim = x + title_width_or_gap_h + 8;
                panel_frame(x - 42, y - 8, panel_frame_rim - x + 42, clamp(panel_bottom_j - y + 8, 61, 999), 7);
                title_width_or_gap_h = panel_bottom_j - y - 52;
                if (advisor <= 2)
                    sprite_draw(scrbuf, x - 40, y - 5, adviser_faces_glyphs[advisor]);
                else
                    blit(tile_page, 40 * advisor + 40, 140, 40, 60, (int) scrbuf, x - 40, (title_width_or_gap_h > 0 ? title_width_or_gap_h - 1 : 0) + y - 6);
                draw_string(buf, x + 5, y - 4, 15);
                draw_line(x + 5, y + 3, string_width(buf) + x + 5, y + 3, 11);
            }
        }
        if (help_flag) {
            index_or_font_b = scrbuf->font;
            scrbuf->font = 2;
            draw_string("(HELP AVAILABLE)", panel_frame_rim - 74, panel_bottom_j - 4, 10);
            scrbuf->font = index_or_font_b;
        }
        if (need_ok_key) {
            draw_string_dim("OK", panel_frame_rim - 17, panel_bottom_j - 8, 11);
            draw_box(panel_frame_rim - 20, panel_bottom_j - 10, 20, 10, 11);
        }
    }
    item_index_j = text[0] == 32 || text[0] == 95 ? 0 : -1;
    scrbuf->colour = hilite_colour;
    for (index_or_font_b = 0; index_or_font_b < line_count; index_or_font_b++) {
        if (refresh_pending || abs(preset - item_index_j) < 2) {
            /* Temporarily terminate each line, then restore its newline after drawing. */
            text[line_off[index_or_font_b + 1] - 1] = 0;
            if (item_index_j >= 0 && (marked_mask & (1 << item_index_j))) {
                text[line_off[index_or_font_b]] = 94;
                draw_string(text + line_off[index_or_font_b], x + 5, index_or_font_b * line_height + y + 5, item_index_j < 0 ? hilite_colour : (dialog_dim & (1 << item_index_j)) ? 3 : 0);
                text[line_off[index_or_font_b]] = 32;
            } else {
                if (item_index_j < 0 && line_height > 9)
                    draw_string(text + line_off[index_or_font_b], x + 5, index_or_font_b * line_height + y + 6, 0);
                draw_string(text + line_off[index_or_font_b], x + 5, index_or_font_b * line_height + y + 5, item_index_j < 0 ? hilite_colour : (dialog_dim & (1 << item_index_j)) ? 3 : 0);
            }
            text[line_off[index_or_font_b + 1] - 1] = 10;
        }
        line_marker_g = text[line_off[index_or_font_b + 1]];
        if (line_marker_g == 32 || line_marker_g == 95)
            item_index_j++;
    }
    return preset;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int panel_frame(int x, int y, int w, int h, int colour)
{
    if (colour == 7 && busy_flag)
        draw_backdrop(x + 1, y + 1, w, h);
    else
        fill_box(x + 1, y + 1, x + w, y + h, colour);
    scrbuf->text_colour = colour;
    colour = 15;
    draw_panel(x + 1, y + 1, w, h, colour, 8);
    draw_box(x, y, w + 2, h + 2, 0);
}

int draw_box(int x, int y, int w, int h, int colour)
{
    draw_line(x, y, x + w, y, colour);
    draw_line(x, y + h, x + w, y + h, colour);
    draw_line(x + w, y, x + w, y + h, colour);
    draw_line(x, y, x, y + h, colour);
}

int draw_panel(int x, int y, int w, int h, int light, int dark)
{
    draw_line(x, y, x + w, y, dark);
    draw_line(x, y + h, x + w, y + h, light);
    draw_line(x + w, y, x + w, y + h, dark);
    draw_line(x, y + 1, x, y + h, light);
}

int key_cmd(void)
{
    int key;

    key = read_key();
    switch (key) {
    case 0x4800:
    case 0x487e:
        key = 0x4800;
        break;
    case 0x4900:
        key = 0x4900;
        break;
    case 0xf400:
    case 0x4d00:
        key = 0x4d00;
        break;
    case 0x5100:
        key = 0x5100;
        break;
    case 0x5000:
    case 0x5060:
        key = 0x5000;
        break;
    case 0x4f00:
        key = 0x4f00;
        break;
    case 0x4b00:
    case 0x4b7c:
        key = 0x4b00;
        break;
    case 0x4700:
    case 0x475c:
        key = 0x4700;
        break;
    case 0x4838:
        key = 0x4838;
        break;
    case 0x4939:
        key = 0x4939;
        break;
    case 0x4d36:
    case 0x4d46:
        key = 0x4d36;
        break;
    case 0x5133:
        key = 0x5133;
        break;
    case 0x5032:
        key = 0x5032;
        break;
    case 0x4f31:
        key = 0x4f31;
        break;
    case 0x4b34:
    case 0x4b43:
        key = 0x4b34;
        break;
    case 0x4737:
        key = 0x4737;
        break;
    default:
        if (key & 0xff)
            key &= 0xff;
    }
    return key;
}
