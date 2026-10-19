/* Overlay 2: startup, credits, title sequence and game-ending screens. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "CAPTURE.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "ECONOMY.H"
#include "FADE.H"
#include "FCHECK.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "LOADER.H"
#include "LOADTAIL.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "PALACE.H"
#include "PALETTE.H"
#include "RESIDENT.H"
#include "SETUP.H"
#include "STRBIOS.H"
#include "TIMER.H"
#include "TURN.H"
#include "INTRO.H"

struct civ_type {
    char strings[48];
    int f0, f1, f2, f3, f4;
};

extern char graphics_driver[], sound_driver[];
extern int debug;
extern char *news_values[];

extern struct terrain_type terrains[];
extern struct civ_type civt[];

extern FILE *credits_stream;

extern int disk_insert();
extern int kbhit(void);
extern int getch(void);

extern void sound(), world_draw(int *mode, int x, int y, int val);
extern void flip(int);

extern int wrap_x(int), terrain_at(int, int);

static void show_label(int, int, int, int);
static void next_credit(char *);

struct page page4 = { 4, 0, 0, 319, 199, 0, 15, 4, 0 };

struct page *page4_handle = &page4;
int page_memory = 0;

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void game_startup(void)
{
    int pages;
    char filename[64];
    int saved;
    int temp;
    int i;
    int j;
    int found;
    int tile;

    found_floppy = 1;
    for (i = 1; i < 8; i++) {
        if (disk_insert(i, 0L))
            found_floppy = i + 1;
        else
            break;
    }
    found = 0;
    do {
        found = file_exists("fonts.cv", 0) != -1;
        set_cursor(12, 0);
        for (i = 1; i <= 12; i++)
            dos_print(clear_line);
        print_at(13, 0, "\377\016");
        if (found)
            print_at(16, 29, "One Moment Please...");
        else {
            strcpy(msgtext, "Please insert Disk 1.");
            msgtext[19] += 6 / found_floppy;
            print_at(16, 29, msgtext);
            getch();
        }
    } while (!found);
    vga_modes = graphics_driver[0] == 'm' || graphics_driver[0] == 'M';
    egamode = graphics_driver[0] == 'e' || graphics_driver[0] == 'E';
    run_program(load_program(graphics_driver, "fonts.cv"));
    run_program(load_program(sound_driver, 0));
    *(char far *) 0x00000417L &= 0xdf;
    scrbuf->font = 1;
    video_init(0);
    small_space_check = 0;
    pages = vga_modes ? 3 : 4;
    for (i = 0; i < pages; i++) {
        if (page_memory = alloc_page_memory(i))
            set_page_memory(i, page_memory);
        else
            small_space_check = 1;
    }
    sound_init();
    resident_init();
    initialize_random();
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    if (!vga_modes)
        rect_fill(spare_page, 0, 0, 320, 200, 0);
    load_ega_palette(ega_palette);
    sprite_arena_init();
    load_picture(1, "logo.pic");
    tile = cut_tile(1, 0, 64, 320, 80);
    sprite_arena_release();
    load_picture(1, "birth0.pic");
    blit(back_page, 0, 76, 320, 24, back_page, 0, 176);
    load_picture(2, "birth1.pic");
    blit(tile_page, 0, 76, 320, 24, tile_page, 0, 176);
    load_back("logo.pic");
    rect_fill(scrbuf, 0, 12, 320, 176, 0);
    set_cycle(1, 14, 146, 152);
    if (vga_modes)
        for (i = 1; i <= 3; i++)
            start_cycle(i);
    debug |= 0x10;
    credits_stream = fopen("credits.txt", "rt");
    timer_reset();
    sound(3);
    msgtext[0] = 0;
    for (i = 0; i < 320; i++) {
        rect_fill(back_page, 0, 176, 320 - i, 24, 0);
        blit(back_page, 0, 64, i, 24, back_page, 320 - i, 176);
        if (i == 10) {
            scrbuf->font = 5;
            next_credit(msgtext);
        }
        if (i == 50)
            msgtext[0] = 0;
        if (i == 70)
            next_credit(msgtext);
        if (i == 120)
            next_credit(msgtext);
        if (i == 160)
            msgtext[0] = 0;
        if (i == 170)
            next_credit(msgtext);
        if (i == 190)
            next_credit(msgtext);
        if (i == 210)
            next_credit(msgtext);
        if (i == 230)
            next_credit(msgtext);
        if (i == 250)
            next_credit(msgtext);
        if (i == 270)
            next_credit(msgtext);
        if (i == 290)
            next_credit(msgtext);
        if (i == 310)
            next_credit(msgtext);
        if (strlen(msgtext)) {
            saved = scrbuf->number;
            scrbuf->number = 1;
            draw_text(msgtext, 160, 184, vga_modes ? 0xFC : 7);
            draw_text(msgtext, 160, 182, vga_modes ? 0xF8 : 15);
            draw_text(msgtext, 160, 183, vga_modes ? 0xFA : 15);
            scrbuf->number = saved;
        }
        if (!vga_modes)
            scrbuf->number = scrbuf->number ? 0 : 3;
        blit(back_page, 0, 0, i, 64, scrbuf, 320 - i, 12);
        blit(back_page, 0, 176, 320, 24, scrbuf, 0, 76);
        blit(back_page, 0, 88, i, 88, scrbuf, 320 - i, 100);
        if (!vga_modes) {
            saved = scrbuf->number;
            scrbuf->number = 0;
            world_draw(scrbuf, 0, 0, 0);
            scrbuf->number = saved;
            set_page(saved);
        }
        if (sound_quiet_ticks() > i * 21 / 4 && !(i & 1))
            i++;
        while (sound_quiet_ticks() < i * 21 / 4);
        if (kbhit())
            i = 999;
    }
    for (i = 0; i < 320; i++) {
        if (i < 320)
            blit(back_page, i, 64, 320 - i, 24, back_page, 0, 176);
        if (i)
            blit(tile_page, 0, 64, i, 24, back_page, 320 - i, 176);
        if (i == 10)
            next_credit(msgtext);
        if (i == 30)
            next_credit(msgtext);
        if (i == 50)
            msgtext[0] = 0;
        if (i == 64)
            next_credit(msgtext);
        if (i == 84)
            next_credit(msgtext);
        if (i == 116)
            msgtext[0] = 0;
        if (i == 130)
            next_credit(msgtext);
        if (i == 150)
            next_credit(msgtext);
        if (i == 170)
            next_credit(msgtext);
        if (i == 190)
            next_credit(msgtext);
        if (i == 210)
            next_credit(msgtext);
        if (i == 230)
            next_credit(msgtext);
        if (i == 250)
            next_credit(msgtext);
        if (i == 270)
            next_credit(msgtext);
        if (i == 290)
            next_credit(msgtext);
        if (i == 310)
            msgtext[0] = 0;
        if (strlen(msgtext)) {
            saved = scrbuf->number;
            scrbuf->number = 1;
            draw_text(msgtext, 160, 184, vga_modes ? 0xFC : 7);
            draw_text(msgtext, 160, 182, vga_modes ? 0xF8 : 15);
            draw_text(msgtext, 160, 183, vga_modes ? 0xFA : 15);
            scrbuf->number = saved;
        }
        if (!vga_modes)
            scrbuf->number = scrbuf->number ? 0 : 3;
        if (i < 320)
            blit(back_page, i, 0, 320 - i, 64, scrbuf, 0, 12);
        if (i)
            blit(tile_page, 0, 0, i, 64, scrbuf, 320 - i, 12);
        blit(back_page, 0, 176, 320, 24, scrbuf, 0, 76);
        if (i < 320)
            blit(back_page, i, 88, 320 - i, 88, scrbuf, 0, 100);
        if (i)
            blit(tile_page, 0, 88, i, 88, scrbuf, 320 - i, 100);
        if (!vga_modes) {
            saved = scrbuf->number;
            scrbuf->number = 0;
            world_draw(scrbuf, 0, 0, 0);
            scrbuf->number = saved;
            set_page(saved);
        }
        if (sound_quiet_ticks() > (i + 320) * 21 / 4 && !(i & 1))
            i++;
        while (!kbhit() && sound_quiet_ticks() < (i + 320) * 21 / 4);
        if (kbhit())
            i = 999;
    }
    main_option_result = -2;
    if (kbhit()) {
        main_option_result = -1;
        switch (key_cmd()) {
        case 'N':
        case 'n':
            main_option_result = 0;
            break;
        case 'L':
        case 'l':
            main_option_result = 1;
            break;
        case 'E':
        case 'e':
            main_option_result = 2;
            break;
        case 'C':
        case 'c':
            main_option_result = 3;
            break;
        }
    }
    if (main_option_result == -2) {
        while (sound_quiet_ticks() < 3463);
        blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
        sprite_draw(back_page, 0, 64, tile);
        if (vga_modes) {
            set_cycle(4, 14, 224, 239);
            start_cycle(4);
        }
        for (i = 0; i < 320; i += 4) {
            blit(back_page, i, 64, 4, 80, scrbuf, i, 64);
            flip(1);
        }
        for (i = 0; i < 100 && !kbhit(); flip(5), i++);
        repaint();
        main_option_result = -1;
    }
    if (vga_modes) {
        for (i = 1; i <= 4; i++)
            stop_cycle(i);
    } else
        scrbuf->number = 0;
    rect_fill(back_page, 0, 0, 320, 200, 0);
    sprite_draw(back_page, 0, 64, tile);
    page_dissolve(1, 20);
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    set_page(0);
    scrbuf->font = 1;
    free_block(tile);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void spaceship_arrival(int civ)
{
    int tmp;
    char story[512];
    int pos;
    int earth[24];
    int t;
    int last;
    int keep;
    int first;
    int count;
    int i;
    int xpos;
    int ypos;
    int color;

    disk_insert(6, 1L);
    load_screen(1, "planet2.pic");
    sprite_arena_init();
    for (i = 0; i < 24; i++)
        earth[i] = cut_tile(1, i % 6 * 50 + 1, i / 6 * 50 + 1, 49, 49);
    sprite_arena_release();
    cycle_off();
    color = vga_modes ? 0xEC : 0;
    rect_fill(scrbuf, 0, 0, 320, 200, color);
    load_picture(1, "planet1.pic");
    blit(back_page, 0, 24, 320, 176, back_page, 0, 0);
    blit(back_page, 0, 32, 320, 24, back_page, 0, 176);
    disk_insert(0, 1L);
    if (!vga_modes)
        rect_fill(spare_page, 0, 0, 320, 200, 0);
    rect_fill(scrbuf, 0, 0, 320, 200, color);
    rect_fill(scrbuf, 0, 12, 320, 176, color);
    strcpy(msgtext, leader_titles[civ_gov[civnow]]);
    strcat(msgtext, " ");
    strcat(msgtext, leader_names[civnow]);
    strcpy(news_values[0], msgtext);
    strcpy(news_values[1], civ_adjectives[civ]);
    msgtext[0] = 0;
    strcat(msgtext, itoa(space_ship_habitation[civ], nstr, 10));
    strcpy(news_values[2], msgtext);
    msgtext[0] = 0;
    strcat(msgtext, itoa(liftoff_when[civ], nstr, 10));
    strcpy(news_values[3], msgtext);
    msgtext[0] = 0;
    strcat(msgtext, itoa(year, nstr, 10));
    strcpy(news_values[4], msgtext);
    news_name("king.txt", civ == civnow ? "*SPACE" : "*SPACE2");
    help_open();
    strcpy(story, msgtext);
    scrbuf->font = 6;
    xpos = -10;
    ypos = 92;
    first = pos = -1;
    count = strlen(msgtext);
    sound(3);
    msgtext[0] = 0;
    for (i = 0; i < 320; i++) {
        timer_reset();
        if (i < 320)
            rect_fill(back_page, 0, 176, 320 - i, 24, color);
        if (i)
            blit(back_page, 0, 20, i, 24, back_page, 320 - i, 176);
        if (i % 22 == 0) {
            first = pos + 1;
            while (msgtext[++pos] != '^' && pos < count);
            if (pos < count)
                msgtext[pos] = 0;
        }
        if (pos < count) {
            keep = scrbuf->number;
            scrbuf->number = 1;
            draw_text(msgtext + first, 160, 184, vga_modes ? 0x38 : 7);
            draw_text(msgtext + first, 160, 183, vga_modes ? 0x3C : 15);
            scrbuf->number = keep;
        }
        if (++t >= 23)
            t = 0;
        sprite_draw(back_page, 135, 51, earth[t]);
        if (!vga_modes)
            scrbuf->number = scrbuf->number ? 0 : 3;
        if (i < 320)
            rect_fill(scrbuf, 0, 12, 319, 20, color);
        if (i)
            blit(back_page, 0, 0, i, 20, scrbuf, 320 - i, 12);
        blit(back_page, 0, 176, 320, 24, scrbuf, 0, 32);
        if (i < 320)
            rect_fill(scrbuf, 0, 56, 320 - i, 132, color);
        if (i)
            blit(back_page, 0, 44, i, 132, scrbuf, 320 - i, 56);
        if (i & 1) {
            xpos++;
            world_draw(scrbuf, xpos - i, ypos, 15);
        }
        xpos++;
        if (!vga_modes) {
            keep = scrbuf->number;
            scrbuf->number = 0;
            world_draw(scrbuf, 0, 0, 0);
            scrbuf->number = keep;
            set_page(keep);
        }
        if (timer_read() > 12 && !(i & 1))
            i++;
        while (timer_read() < 8);
    }
    free_block(earth[0]);
    scrbuf->number = 0;
    set_page(0);
    strcpy(story, msgtext + pos + 1);
    count -= pos + 1;
    pos = -1;
    if (civ == civnow) {
        disk_insert(6, 1L);
        load_screen(1, "spacest.pic");
        sound(1);
        if (vga_modes)
            set_palette(5, 0, 0, 0);
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
        load_back("spacest.pic");
        disk_insert(0, 1L);
        sound(0x22);
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
        if (vga_modes) {
            set_cycle(1, 15, 16, 31);
            set_cycle(2, 100, 32, 34);
            set_cycle(3, 42, 35, 40);
            set_cycle(4, 300, 41, 47);
            set_cycle(5, 60, 48, 53);
            set_cycle(6, 37, 54, 63);
            for (i = 1; i <= 6; i++)
                start_cycle(i);
        }
    } else {
        palpic(1L);
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    }
    do {
        first = pos + 1;
        while (story[++pos] != '^' && pos < count);
        if (pos < count)
            story[pos] = 0;
        if (pos < count) {
            blit(back_page, 0, 0, 320, 32, scrbuf, 0, 0);
            draw_text(story + first, 160, 5, 11);
            draw_text(story + first, 160, 4, 15);
            world_draw(scrbuf, 280, 32, 15);
            flip(80);
            world_draw(scrbuf, 280, 32, 0);
            flip(80);
        }
    } while (pos < count);
    ad_wait();
    if (civ == civnow && vga_modes)
        for (i = 1; i <= 6; i++)
            stop_cycle(i);
    scrbuf->number = 0;
    sound(1);
    fclose(credits_stream);
    scrbuf->font = 1;
    if (!vga_modes) {
        disk_insert(7, 1L);
        load_screen(3, "sprites.pic");
        disk_insert(0, 1L);
    }
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    load_player_palette();
    setup_cycles();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void civ_destroyed(void)
{
    int pos;
    int line;
    int i;
    int len;

    cycle_off();
    disk_insert(6, 1L);
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    sound(0x23);
    load_picture(1, "arch.pic");
    disk_insert(0, 1L);
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    strcpy(msgtext, leader_names[civnow]);
    strupr(msgtext);
    strcpy(news_values[0], msgtext);
    strcpy(msgtext, "Irrigation");
    for (i = 0; i < 72; i++)
        if (has_tech(civnow, i))
            strcpy(msgtext, advances[i].name);
    strcpy(news_values[3], msgtext);
    msgtext[0] = 0;
    news_name("king.txt", "*ARCH");
    help_open();
    scrbuf->font = 6;
    line = pos = -1;
    len = strlen(msgtext);
    do {
        line = pos + 1;
        while (msgtext[++pos] != '^' && pos < len);
        if (pos < len)
            msgtext[pos] = 0;
        if (pos < len) {
            blit(back_page, 0, 6, 320, 20, scrbuf, 0, 6);
            if (vga_modes) {
                draw_text(msgtext + line, 160, 7, 15);
                draw_text(msgtext + line, 160, 9, 13);
                draw_text(msgtext + line, 160, 8, 14);
            } else {
                draw_text(msgtext + line, 160, 9, 1);
                draw_text(msgtext + line, 160, 8, 9);
            }
            if (msgtext[pos - 1] != '!' && !kbhit())
                flip(200);
        }
    } while (pos < len);
    sound(1);
    sound(civt[civ_tribe_of[civnow]].f3);
    flip(300);
    sound(1);
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    load_player_palette();
    scrbuf->font = 1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void show_start(void)
{
    int x;
    int i;
    int y;
    int xc;
    int code;
    int yc;

    mouse_off();
    xc = map_x0 + 7;
    yc = map_y0 + 6;
    for (i = 1; i <= 8; i++) {
        x = wrap_x(spiral_x[i] + xc);
        y = spiral_y[i] + yc;
        code = terrain_at(x, y);
        strcpy(msgtext, terrains[code].name);
        if (patterned_cell(code, x, y))
            strcpy(msgtext, terrains[code + 12].name);
        if (has_hut(code, x, y))
            strcpy(msgtext, "Village");
        show_label(spiral_x[i] * 16 + 200, spiral_y[i] * 16 + 112, spiral_x[i] * 64 + 170, spiral_y[i] * 48 + 100);
    }
    strcpy(msgtext, "Menu Bar");
    show_label(160, 6, 200, 16);
    strcpy(msgtext, "Map Window");
    show_label(48, 32, 88, 24);
    strcpy(msgtext, "Active Unit");
    show_label(40, 128, 88, 170);
    mouse_on();
    ad_wait();
}

static void show_label(int x1, int y1, int x2, int y2)
{
    draw_line(x1, y1, x2, y2 + 6, 15);
    strcat(msgtext, "\n");
    abort_flag = 1;
    menu_dialog(msgtext, x2, y2);
}

static void next_credit(char *buffer)
{
    fscanf(credits_stream, "%[^\n]\n", buffer);
}

char *rank_title[5] = { "Chief", "Lord", "Prince", "King", "Emperor" };
