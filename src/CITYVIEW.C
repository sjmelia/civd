/* Overlay 19: city views, citizens and invasion, riot and celebration animations. */
#include "CIV.H"
#include "CAPTURE.H"
#include "CITYMGT.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "ECONOMY.H"
#include "FADE.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "PALETTE.H"
#include "RESIDENT.H"
#include "TURN.H"
#include "CITYVIEW.H"
#include "DIALOG.H"

struct rect {
    int x, y, w, h;
};

extern struct city townbuf[];
extern struct rect far wonder_rects[];
/* Only the segment of this second far array is observable. Its name and
   offset are unknown; the binding supplies F2767 with an arbitrary offset 0. */
extern struct rect far wonder_bounds[];

extern int strlen();
extern char *strcpy(char *, char *);
extern void srand();
extern int rand_scaled(int);
extern int disk_insert(), kbhit(void);

extern void sound();

extern void flip(int);

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void city_scene(int city, int mode)
{
    char title[128];
    int more;
    int wn;
    int edge;
    int cnt;
    int v;
    int rd;
    char g[19][12];
    int lev;
    int x;
    int row;
    int i;
    int good;
    int y;
    int owner;
    int ry;
    int size;
    int k;
    int code;

    owner = townbuf[city].owner;
    lev = tech_count[owner] / 2;
    size = townbuf[city].size;
    if (strlen(msgtext) < 120)
        strcpy(title, msgtext);
    msgtext[0] = 0;
    append_place_name(city);
    for (code = 0, i = 0; i < strlen(msgtext); i++)
        code += msgtext[i];
    srand(code);
    for (y = 0; y < 12; y++)
        for (x = 0; x < 19; x++)
            g[x][y] = -1;
    cnt = more = 0;
    x = 9;
    y = 11;
    for (i = 0; i < size * 8; i++) {
        if (g[x][y] == -1) {
            rd = half_dist(x - 9, y - 10);
            g[x][y] = clamp((lev - rd) / 3 - rand_scaled(2), lev / 6, townbuf[city].size / 4 + 6);
            if (g[x][y] > 9)
                g[x][y] = 9;
            if (rd * rd > size * 8) {
                x = 9;
                y = 11;
            }
        } else {
            cnt++;
            if (cnt < 999)
                i--;
        }
        ry = rand_scaled(8) + 1;
        x = clamp(spiral_x[ry] + x, 0, 18);
        y = clamp(spiral_y[ry] + y, 0, 11);
    }
    ry = rand_scaled(2) + 4;
    for (i = 2; i < 17; i++)
        g[i][ry] = -2;
    g[0][ry] = g[18][ry] = -1;
    ry = rand_scaled(2) + 8;
    for (i = 2; i < 17; i++)
        g[i][ry] = -2;
    g[0][ry] = g[18][ry] = -1;
    row = ry;
    ry = rand_scaled(4) + 9;
    for (i = 1; i < 12; i++)
        g[ry - (i + 1) / 2][i] = -2;
    ry = rand_scaled(4) + 14;
    for (i = 1; i < 12; i++)
        g[ry - (i + 1) / 2][i] = -2;
    if (city == marvel_site[7]) {
        g[0][0] = 15;
        g[1][0] = 15;
        g[2][0] = 15;
        g[0][1] = 15;
        g[1][1] = 15;
        g[0][2] = 15;
    }
    for (i = 1; i <= 21; i++) {
        if (marvel_site[i] == city && !((1 << i) & 0x808a)) {
            cnt = 0;
            do {
                x = rand_scaled(15) + 1;
                y = rand_scaled(10) + 1;
                good = 0;
                if (g[x][y] >= 0)
                    good |= 1;
                if (g[x + 1][y] >= 0)
                    good |= 1;
                if (g[x + 2][y] >= 0)
                    good |= 1;
                if (g[x - 1][y + 1] >= 0)
                    good |= 1;
                if (g[x][y + 1] >= 0)
                    good |= 1;
                if (g[x + 1][y + 1] >= 0)
                    good |= 1;
                if (cnt > 400)
                    good |= 1;
                if (g[x][y] >= 15)
                    good |= 2;
                if (g[x + 1][y] >= 15)
                    good |= 2;
                if (g[x + 2][y] >= 15)
                    good |= 2;
                if (g[x - 1][y + 1] >= 15)
                    good |= 2;
                if (g[x][y + 1] >= 15)
                    good |= 2;
                if (g[x + 1][y + 1] >= 15)
                    good |= 2;
                if (g[x + 2][y + 1] >= 15)
                    good |= 2;
                if (g[x][y] == -2)
                    good |= 2;
                if (g[x + 1][y] == -2)
                    good |= 2;
                if (g[x - 1][y + 1] == -2)
                    good |= 2;
                if (g[x][y + 1] == -2)
                    good |= 2;
                if (g[x + 1][y + 1] == -2)
                    good |= 2;
            } while (good != 1 && ++cnt < 500);
            if (cnt < 500) {
                g[x][y] = i + 64;
                g[x + 1][y] = 15;
                g[x + 2][y] = 15;
                g[x][y + 1] = 15;
                g[x + 1][y + 1] = 15;
                g[x + 2][y + 1] = 15;
                g[x - 1][y + 1] = 15;
                g[x + 1][y - 1] = 15;
                g[x + 2][y - 1] = 15;
                g[x + 3][y - 1] = 15;
                if (y < 7 && !(g[x + 1][y + 2] & 7))
                    g[x + 1][y + 2] = -1;
            }
        }
    }
    for (i = 23; i >= 1; i--) {
        if (!(townbuf[city].buildings & (1L << i)) || i == 7 || i == 8 || i == 12 || i == 18 || i == 19)
            continue;
        cnt = 0;
        do {
            x = rand_scaled(16) + 1;
            y = rand_scaled(10) + 1;
            good = 0;
            if (g[x][y] >= 0)
                good |= 1;
            if (g[x + 1][y] >= 0)
                good |= 1;
            if (g[x - 1][y + 1] >= 0)
                good |= 1;
            if (g[x][y + 1] >= 0)
                good |= 1;
            if (g[x + 1][y + 1] >= 0)
                good |= 1;
            if (cnt > 350)
                good |= 1;
            if (g[x][y] >= 15)
                good |= 2;
            if (g[x + 1][y] >= 15)
                good |= 2;
            if (g[x - 1][y + 1] >= 15)
                good |= 2;
            if (g[x][y + 1] >= 15)
                good |= 2;
            if (g[x + 1][y + 1] >= 15)
                good |= 2;
            if (g[x][y] == -2)
                good |= 2;
            if (g[x + 1][y] == -2)
                good |= 2;
            if (g[x - 1][y + 1] == -2)
                good |= 2;
            if (g[x][y + 1] == -2)
                good |= 2;
            if (g[x + 1][y + 1] == -2)
                good |= 2;
        } while (good != 1 && ++cnt < 500);
        if (cnt < 500) {
            g[x][y] = i + 15;
            g[x + 1][y] = 15;
            g[x][y + 1] = 15;
            g[x + 1][y + 1] = 15;
            g[x - 1][y + 1] = 15;
            if (y < 7 && !(g[x + 1][y + 2] & 7))
                g[x + 1][y + 2] = -1;
        }
    }
    if (townbuf[city].buildings & 0x80000L) {
        g[18][8] = 15;
        g[18][7] = 15;
        g[17][8] = 15;
        g[17][7] = 34;
    }
    if (townbuf[city].buildings & 0x100L) {
        g[0][row - 1] = 23;
        g[1][row - 1] = 15;
        g[2][row - 1] = 15;
        g[0][row] = 15;
        g[1][row] = 15;
        g[2][row] = 15;
    }
    cycle_off();
    if (mode != -2)
        disk_insert(1, 1L);
    city_load(mode, townbuf[city].owner, city);
again:
    load_screen(1, "hill.pic");
    for (i = 1; i <= 7; i++) {
        wn = i;
        if (wn == 2)
            wn = 15;
        if (marvel_site[wn] != city || wn + 24 == mode)
            continue;
        else if (!((1 << wn) & 0x808a))
            continue;
        sprite_draw(back_page, wonder_rects[wn].x + (wonder_rects[wn].x < 10 ? -1 : 2) + (wn == 3 ? 80 : 0), wn <= 7 ? 0 : wonder_rects[wn].y - 6, city_wonder_tiles[wn]);
    }
    if ((townbuf[city].buildings & 0x40000L) && mode != 19)
        for (i = city == marvel_site[7] ? 45 : 0; i < 310; i += 45)
            sprite_draw_clipped(back_page, i, 2, city_structure_glyphs[17]);
    for (y = 0; y < 12; y++)
        for (x = 0; x < 19; x++) {
            code = g[x][y];
            if (code == -2) {
                good = 0;
                for (k = 1; k <= 8; k++) {
                    if (y & 1)
                        v = g[(spiral_y[k] < 0 ? 1 : 0) + spiral_x[k] + x][spiral_y[k] + y];
                    else
                        v = g[(spiral_y[k] > 0 ? -1 : 0) + spiral_x[k] + x][spiral_y[k] + y];
                    if (v >= 0)
                        good = 1;
                }
                if (!good)
                    g[x][y] = -1;
            }
        }
    for (y = 0; y < 12; y++)
        for (x = 0; x < 19; x++) {
            code = g[x][y];
            if (code == -1 && x > 2 && g[x - 1][y] != -1 && ((x + y) & 1))
                city_pop(x, y, 0);
            if (code == -1 || code == 15)
                continue;
            else if (code == -2) {
                edge = 0;
                for (k = 1; k <= 8; k++) {
                    if (y & 1)
                        v = g[(spiral_y[k] < 0 ? 1 : 0) + spiral_x[k] + x][spiral_y[k] + y];
                    else
                        v = g[(spiral_y[k] > 0 ? -1 : 0) + spiral_x[k] + x][spiral_y[k] + y];
                    if (k & 1) {
                        edge >>= 1;
                        if (v == -2)
                            edge |= 8;
                    }
                }
                city_pop(x, y, edge);
                continue;
            }
            if (code < 16) {
                if (code < 2)
                    city_house(x, y, (code & 1) * 2 + (city & 1), code / 2);
                else
                    city_house(x, y, ((x + y) & 1) + (code & 1) * 2, code / 2);
            } else if (mode + 14 != code && mode + 40 != code)
                city_building(x, y, code - 16);
        }
    if ((townbuf[city].buildings & 0x80L) && mode != 8) {
        for (i = 0; i < 320; i += 43) {
            if (i == 172)
                i += 19;
            sprite_draw_clipped(back_page, i, 108, city_structure_glyphs[22]);
        }
        sprite_draw(back_page, 142, 108, city_structure_glyphs[6]);
    }
    if ((townbuf[city].buildings & 0x1000L) && mode != 13) {
        for (i = 0; i < 310; i += 46)
            sprite_draw_clipped(back_page, i, 115, city_structure_glyphs[21]);
        sprite_draw(back_page, 0, 115, city_structure_glyphs[11]);
    }
    scrbuf->number = 1;
    scrbuf->font = 6;
    if (title[0] == 0) {
        msgtext[0] = 0;
        append_place_name(city);
        draw_text(msgtext, 161, 3, 0);
        draw_text(msgtext, 160, 2, 15);
        msgtext[0] = 0;
        format_year();
        draw_text(msgtext, 161, 16, 0);
        draw_text(msgtext, 160, 15, 15);
    } else {
        abort_flag = 1;
        menu_dialog(title, 80, 8);
        scrbuf->font = 1;
    }
    scrbuf->font = 1;
    scrbuf->number = 0;
    if (more) {
        sound(44);
        page_dissolve(1, 10);
    } else {
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
        if (mode != -3)
            load_back("hill.pic");
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
        if (mode == -3 && vga_modes) {
            load_palette("hill.pal", colours_area);
            install_palette(15, colours_area);
        }
        {
            set_cycle(4, 15, 64, 79);
            start_cycle(4);
        }
    }
    if (mode == -1 && !more) {
        free_block(houses_sprite[0][0], "city");
        city_citizens(city, 24, 140, 256);
    }
    if (mode > -1 && !kbhit()) {
        mode = -1;
        more = 1;
        flip(60);
        if (!kbhit())
            goto again;
    }
    if (mode != -1 || more)
        free_block(houses_sprite[0][0], "cty");
    if (mode != -2) {
        if (!kbhit())
            ad_wait();
        else
            repaint();
        stop_cycle(4);
        disk_insert(0, 1L);
        if (vga_modes) {
            rect_fill(scrbuf, 0, 0, 320, 200, 0);
            load_player_palette();
        }
        cycle_on();
    } else
        stop_cycle(4);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void city_pop(int x, int y, int kind)
{
    sprite_draw(back_page, (y & 1) * 8 + x * 16 - 5, y * 8 + 50, pop_tiles[kind]);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void city_house(int x, int y, int style, int era)
{
    sprite_draw(back_page, (y & 1) * 8 + x * 16, y * 8 + 26, houses_sprite[era][style]);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void city_building(int x, int y, int n)
{
    /* Reconstructed dead reference: MSC 5.1 emits no instructions here, but
       retains a second F2767 segment word in CONST and flushes CODE at 106C.
       Both effects match the original object layout and relocation order.
       The original condition/reference spelling is unknown. Retain this
       plausible eliminated path to reproduce the complete original EXE. */
    if (0)
        y = wonder_bounds[n].h;
    if (n < 48)
        sprite_draw_clipped(back_page, clamp((y & 1) * 8 + x * 16 - 9, 0, 999), y * 8 + 16, city_structure_glyphs[n]);
    else {
        n -= 48;
        x = (y & 1) * 8 + x * 16 - 9;
        y = y * 8 - wonder_rects[n].h + 76;
        sprite_draw(back_page, clamp(x, 0, 999), y - 10, city_wonder_tiles[n]);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void city_citizens(int city, int x, int y, int width)
{
    int happy;
    int people[9];
    int unhappy;
    int i;
    int step;
    char sz;

    sprite_arena_init();
    load_screen(1, "pop.pic");
    for (i = 0; i < 9; i++)
        people[i] = cut_tile(1, i * 35 + 1, has_tech(civnow, 35) ? 1 : 52, 34, 50);
    sprite_arena_release();
    sz = townbuf[city].size;
    if (sz * 12 > width)
        step = width / sz;
    else
        step = 12;
    happy = clamp(town_happy_ones_num, 0, townbuf[city].size);
    unhappy = clamp(town_unhappy_counts, 0, townbuf[city].size);
    while (happy + unhappy > clamp(townbuf[city].size - city_experts_num, 0, 99)) {
        happy = clamp(--happy, 0, townbuf[city].size);
        unhappy = clamp(--unhappy, 0, townbuf[city].size);
    }
    for (i = 0; i < happy; i++) {
        sprite_draw(scrbuf, x, y, people[((i & 2) ? 0 : 0) + (i & 1)]);
        x += step;
    }
    if (happy)
        x += 8;
    for (i = 0; i < townbuf[city].size - happy - unhappy - city_experts_num; i++) {
        sprite_draw(scrbuf, x, y, people[((i & 2) ? 2 : 2) + (i & 1)]);
        x += step;
    }
    if (i)
        x += 8;
    for (i = 0; i < unhappy; i++) {
        sprite_draw(scrbuf, x, y, people[((i & 2) ? 4 : 4) + (i & 1)]);
        x += step;
    }
    x += 12;
    for (i = 0; i < city_experts_num; i++) {
        sprite_draw(scrbuf, x, y, people[5 + get_specialist(i)]);
        x += step;
    }
    free_block(people[0], "pop");
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void city_load(int mode, int owner, int city)
{
    int i;
    int loaded;
    int n;
    int col;

    sprite_arena_init();
    load_screen(1, "citypix1.pic");
    for (i = 0; i <= 4; i++) {
        col = i * 64;
        houses_sprite[i][0] = cut_tile(1, col + 1, 1, 31, 31);
        houses_sprite[i][2] = cut_tile(1, col + 1, 33, 31, 31);
        houses_sprite[i][1] = cut_tile(1, col + 33, 1, 31, 31);
        houses_sprite[i][3] = cut_tile(1, col + 33, 33, 31, 31);
    }
    for (i = 0; i < 16; i++)
        pop_tiles[i] = cut_tile(1, (i & 7) * 24, (has_tech(owner, 58) ? 81 : 65) + i / 8 * 8, 24, 8);
    load_screen(1, has_tech(owner, 37) ? "citypix3.pic" : "citypix2.pic");
    for (i = 0; i < 23; i++)
        if ((townbuf[city].buildings & (1L << (i + 1))) || mode - 1 == i || i > 20)
            city_structure_glyphs[i] = cut_tile(1, i / 4 * 50 + 1, (i & 3) * 50 + 1, 49, 49);
    loaded = 0;
    for (i = 1; i <= 21; i++)
        if (marvel_site[i] == city && !((1 << i) & 0x808a)) {
            if (!loaded)
                load_screen(1, "wonders.pic");
            loaded = 1;
            city_wonder_tiles[i] = wonder_tile(i);
        }
    loaded = 0;
    for (i = 1; i <= 21; i++)
        if (marvel_site[i] == city && ((1 << i) & 0x808a)) {
            if (!loaded)
                load_screen(1, "wonders2.pic");
            loaded = 1;
            city_wonder_tiles[i] = wonder_tile(i);
        }
    sprite_arena_release();
}

int wonder_tile(int n)
{
    int i;

    if (wonder_rects[1].w > 300)
        for (i = 1; i <= 21; i++) {
            wonder_rects[i].w = wonder_rects[i].w - wonder_rects[i].x + 1;
            wonder_rects[i].h = wonder_rects[i].h - wonder_rects[i].y + 1;
        }
    return cut_tile(1, wonder_rects[n].x, wonder_rects[n].y, wonder_rects[n].w, wonder_rects[n].h);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void invasion_scene(int civ)
{
    int tiles[16];
    int pos;
    int i;
    int frame;
    int sx;

    cycle_off();
    if (vga_modes)
        set_dac(253, 3, 3, 3);
    sprite_arena_init();
    mouse_off();
    if (has_tech(civ, 34)) {
        load_screen(1, has_tech(civ, 58) ? "invaders.pic" : "invader2.pic");
        for (i = 0; i < 10; i++)
            tiles[i] = cut_tile(1, i % 4 * 79, i / 4 * 61 + 2, 78, 60);
    } else {
        load_screen(1, "invader3.pic");
        for (i = 0; i < 10; i++)
            tiles[i] = cut_tile(1, i % 4 * 79 + 1, i / 4 * 66 + 1, 78, 65);
    }
    sprite_arena_release();
    blit(scrbuf, 0, 100, 320, 100, back_page, 0, 0);
    frame = 0;
    for (pos = 0; pos < 660; pos += 3) {
        timer_reset();
        blit(back_page, 0, 0, 320, 100, back_page, 0, 100);
        for (i = 7; i >= 0; i--) {
            sx = pos - i * 48;
            if (sx < 320 && sx > -48)
                sprite_draw_clipped(back_page, sx, 132, tiles[frame % 10]);
        }
        blit(back_page, 0, 100, 320, 100, scrbuf, 0, 100);
        frame++;
        while (timer_read() < 10);
        poll_mouse();
        if (kbhit() || mse_held)
            break;
    }
    mouse_on();
    repaint();
    free_block(tiles[0]);
    cycle_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int riot_x[4] = { -96, -36, 0, -56 };
int riot_y[4] = { -6, -4, -2, 0 };

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void riot_anim(void)
{
    int tiles[16];
    int pos;
    int i;
    int frame;
    int sx;

    cycle_off();
    mouse_off();
    sprite_arena_init();
    if (has_tech(civnow, 13)) {
        load_screen(1, "riot.pic");
        for (i = 0; i < 10; i++)
            tiles[i] = cut_tile(1, i % 4 * 79 + 1, i / 4 * 64 + 1, 78, 63);
    } else {
        load_screen(1, "riot2.pic");
        for (i = 0; i < 10; i++)
            tiles[i] = cut_tile(1, i % 4 * 75 + 1, i / 4 * 66 + 1, 74, 65);
    }
    sprite_arena_release();
    blit(scrbuf, 0, 100, 320, 100, back_page, 0, 0);
    frame = 0;
    for (pos = -48; pos < 420; pos += 3) {
        timer_reset();
        blit(back_page, 0, 0, 320, 100, back_page, 0, 100);
        for (i = 0; i < 4; i++) {
            sx = riot_x[i] + pos;
            if (sx >= 320)
                continue;
            if (sx > -48)
                sprite_draw_clipped(back_page, sx, riot_y[i] + 132, tiles[(i * 3 + frame) % 10]);
        }
        blit(back_page, 0, 100, 320, 100, scrbuf, 0, 100);
        frame++;
        while (timer_read() < 10);
        poll_mouse();
        if (kbhit() || mse_held)
            break;
    }
    mouse_on();
    repaint();
    free_block(tiles[0]);
    cycle_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void love_anim(void)
{
    int tiles[16];
    int pos;
    int i;
    int frame;
    int sx;

    cycle_off();
    sprite_arena_init();
    load_screen(1, !has_tech(civnow, 35) ? "love1.pic" : "love2.pic");
    for (i = 0; i < 10; i++)
        tiles[i] = cut_tile(1, i % 4 * 79 + 1, i / 4 * 66 + 1, 78, 65);
    sprite_arena_release();
    mouse_off();
    blit(scrbuf, 0, 100, 320, 100, back_page, 0, 0);
    frame = 0;
    for (pos = 420; pos > -48; pos -= 3) {
        timer_reset();
        blit(back_page, 0, 0, 320, 100, back_page, 0, 100);
        for (i = 0; i < 4; i++) {
            sx = riot_x[i] * 3 / 2 + pos;
            if (sx >= 320)
                continue;
            if (sx > -48)
                sprite_draw_clipped(back_page, sx, riot_y[i] + 132, tiles[(i * 3 + frame) % 10]);
        }
        blit(back_page, 0, 100, 320, 100, scrbuf, 0, 100);
        frame++;
        while (timer_read() < 10);
        poll_mouse();
        if (kbhit() || mse_held)
            break;
    }
    mouse_on();
    repaint();
    free_block(tiles[0]);
    cycle_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Wonder tile rectangles in the wonder sheet (x, y, right, bottom until
   wonder_tile converts them to widths and heights); original far-data block. */
struct rect far wonder_rects[22] = {
    { 0, 0, 0, 0 }, { 131, 54, 318, 82 }, { 159, 149, 227, 198 },
    { 88, 97, 211, 135 }, { 229, 116, 268, 198 }, { 61, 117, 101, 172 },
    { 164, 97, 227, 147 }, { 1, 38, 66, 118 }, { 268, 53, 318, 114 },
    { 9, 117, 59, 172 }, { 126, 1, 182, 53 }, { 276, 1, 318, 51 },
    { 103, 139, 157, 198 }, { 184, 1, 224, 69 }, { 40, 69, 101, 115 },
    { 1, 14, 147, 33 }, { 253, 1, 274, 47 }, { 226, 1, 251, 47 },
    { 103, 79, 162, 137 }, { 270, 116, 318, 198 }, { 63, 1, 124, 54 },
    { 240, 60, 266, 114 }
};
