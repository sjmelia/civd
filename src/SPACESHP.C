/* Overlay 18: spaceship screens. */
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
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "PALETTE.H"
#include "RESIDENT.H"
#include "TURN.H"
#include "SPACESHP.H"

struct ship_part {
    char name[16];
    int mass;
    int x, y, w, h;
};

extern int debug;
     /* habitation modules, launch year per civ */

extern char far ship_layout[][3];
extern char far spaceships[][15][12];

extern void disk_insert();
extern void LOAD_SPRITES();
extern int rand_scaled(int);

extern void flip(int), world_draw(int *mode, int x, int y, int val);
extern int kbhit(void);

int spaceship_view();           /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

struct ship_part ship_parts[10] = {
    { "Structure1", 1, 0, 16, 16, 16 },
    { "Structure2", 1, 16, 16, 16, 16 },
    { "Structure3", 1, 32, 16, 16, 16 },
    { "Propulsion", 4, 256, 48, 32, 16 },
    { "Fuel", 4, 0, 80, 16, 16 },
    { "NONE", 4, 192, 96, 32, 32 },
    { "Habitation", 16, 224, 96, 32, 32 },
    { "Life Support", 16, 288, 96, 32, 32 },
    { "Solar Panel", 4, 0, 96, 32, 32 },
    { "Support", 16, 256, 96, 32, 32 }
};

int ship_success = 0;           /* human spaceship success chance, percent */
int ai_ship_success = 0;        /* AI spaceship success chance, percent */
int game_flags = 0;

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void spaceship_open(void)
{
    int i;

    mouse_off();
    cycle_off();
    disk_insert(1, 1L);
    sprite_arena_init();
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    LOAD_SPRITES("docker.pic", 1);
    for (i = 0; i < 10; i++)
        ship_tiles[i] = cut_tile(1, ship_parts[i].x, ship_parts[i].y, ship_parts[i].w, ship_parts[i].h);
    sprite_arena_release();
    for (i = 0; i < 20; i++) {
        star_x[i] = rand_scaled(240);
        star_y[i] = rand_scaled(192) + 8;
        star_on[i] = 0;
    }
    mouse_on();
    set_cycle(1, 30, 208, 223);
    set_cycle(2, 11, 192, 207);
    set_cycle(3, 20, 224, 239);
    set_cycle(4, 8, 144, 159);
    set_cycle(5, 150, 128, 129);
    for (i = 1; i <= 5; i++)
        start_cycle(i);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void spaceship_close(void)
{
    int i;
    char name[16];

    free_block(ship_tiles[0], "dock");
    for (i = 1; i <= 5; i++)
        stop_cycle(i);
    disk_insert(1, 1L);
    strcpy(name, "back0a.pal");
    name[4] = civ_gov[civnow] / 2 + '0';
    if (has_tech(civnow, 20))
        name[5] = 'm';
    load_palette(name, pic_colours_data);
    load_palette("sp256.pal", icon_colours_block);
    memcpy(palette_to_pal, pal_begin_colours, 0x180);
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    apply_palette(icon_colours_block);
    setup_cycles();
    sel_player();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int spaceship_view(civ, wait, ask)
int civ, wait, ask;
{
    int key;
    int count;
    int joined;
    int y;
    int i;
    int success;
    int part;
    int mass;
    int unused;
    int show;
    int x;
    int dx;
    int dy;
    int xcnt;
    int dummy2;
    int ycnt;

    mouse_off();
    if (show = civ == civnow || (debug & 0x20)) {
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
        rect_fill(scrbuf, 0, 0, 320, 8, civ_colours[civ]);
        strcpy(msgtext, civ_adjectives[civ]);
        strcat(msgtext, " SpaceShip: X.S.S. ");
        count = strlen(msgtext);
        strcat(msgtext, leader_names[civ]);
        msgtext[count - 7] = *civ_adjectives[civ];
        draw_text(msgtext, 160, 1, 0);
    }
    for (i = 0; i < 10; i++)
        rocket_module[i] = 0;
    i = 0;
    do {
        x = ship_layout[i][0];
        y = ship_layout[i][1];
        part = spaceships[civ][x][y];
        if (part != -1) {
            if (show)
                sprite_draw(scrbuf, x * 16 - 16, y * 16 + 8, ship_tiles[part]);
            xcnt = (part < 6 && part != 3) ? 1 : 2;
            ycnt = part < 6 ? 1 : 2;
            joined = part < 3 ? 1 : 0;
            for (dx = 0; dx < xcnt; dx++)
                for (dy = 0; dy < ycnt; dy++) {
                    joined += ship_connected(civ, x + dx + 1, y + dy);
                    joined += ship_connected(civ, x + dx - 1, y + dy);
                    joined += ship_connected(civ, x + dx, y + dy + 1);
                    joined += ship_connected(civ, x + dx, y + dy - 1);
                }
            if (joined)
                rocket_module[part]++;
            else if (show)
                draw_box(x * 16 - 16, y * 16 + 8, xcnt * 16 - 1, ycnt * 16 - 1, 12);
        } else if (ship_layout[i][2] < 3)
            joined = 0;
    } while (ship_layout[++i][0] != -1);
    mass = 0;
    for (i = 0; i < 10; i++)
        mass += ship_parts[i].mass * rocket_module[i];
    if (mass >= 80 && show)
        sprite_draw(scrbuf, 152, 88, ship_tiles[5]);
    if (show) {
        rect_fill(scrbuf, 240, 8, 80, 192, civ_colours[civ]);
        draw_line(0, 8, 0, 199, civ_colours[civ]);
        draw_line(0, 199, 240, 199, civ_colours[civ]);
    }
    success = 100;
    y = 16;
    draw_string("Population:", 242, y, 0);
    y += 8;
    msgtext[0] = 0;
    strcat(msgtext, itoa(rocket_module[6], nstr, 10));
    strcat(msgtext, "0,000");
    if (show)
        draw_string(msgtext, 242, y, 0);
    y += 8;
    count = 100 * rocket_module[7] / clamp(rocket_module[6], 1, 99);
    strcpy(msgtext, "Support: ");
    strcat(msgtext, itoa(count, nstr, 10));
    strcat(msgtext, "%");
    draw_string(msgtext, 242, y, count < 100 ? 4 : 0);
    y += 8;
    if (show)
        success = clamp(count, 0, 100) * success / 100;
    count = 200 * rocket_module[8] / clamp(rocket_module[6] + rocket_module[7], 1, 99);
    strcpy(msgtext, "Energy: ");
    strcat(msgtext, itoa(count, nstr, 10));
    strcat(msgtext, "%");
    success = clamp(count, 0, 100) * success / 100;
    if (show)
        draw_string(msgtext, 242, y, count < 100 ? 4 : 0);
    y += 16;
    if (show)
        draw_string("Mass:", 242, y, 0);
    y += 8;
    msgtext[0] = 0;
    strcat(msgtext, itoa(mass / 10, nstr, 10));
    strcat(msgtext, ",");
    strcat(msgtext, itoa(mass % 10, nstr, 10));
    strcat(msgtext, "00 tons");
    if (show)
        draw_string(msgtext, 242, y, 0);
    y += 8;
    count = 100 * rocket_module[4] / clamp(rocket_module[3], 1, 99);
    strcpy(msgtext, "Fuel: ");
    strcat(msgtext, itoa(count, nstr, 10));
    strcat(msgtext, "%");
    if (show)
        draw_string(msgtext, 242, y, count < 100 ? 4 : 0);
    y += 8;
    if (show)
        draw_string("Flight Time:", 242, y, 0);
    y += 8;
    count = 50 * mass / (clamp(10 * rocket_module[3], 0, 10 * rocket_module[4]) + 1);
    if (count > 150)
        success -= (count - 150) / 10;
    msgtext[0] = 0;
    strcat(msgtext, itoa(count / 10, nstr, 10));
    strcat(msgtext, ".");
    strcat(msgtext, itoa(count % 10, nstr, 10));
    strcat(msgtext, " Years");
    if (show)
        draw_string(msgtext, 242, y, 0);
    y += 16;
    success = clamp(success, 0, 100);
    if (show)
        draw_string("Prob. of Success", 242, y, 0);
    y += 8;
    strcpy(msgtext, "--- ");
    strcat(msgtext, itoa(success, nstr, 10));
    strcat(msgtext, "% ---");
    if (show)
        draw_string(msgtext, 242, y, 0);
    y += 16;
    if (rocket_module[6] == 0)
        ask = 0;
    if (game_flags & (1 << civ)) {
        if (show) {
            draw_string(spaceship_arrive_dates[civ] < year ? "Landed!" : "Launched!", 242, y, 0);
            y += 16;
        }
    } else {
        spaceship_arrive_dates[civ] = count / 10 + year;
        if (civ == civnow && wait < 0 && ask) {
            draw_string("LAUNCH", 256, 190, 0);
            draw_box(242, 188, 76, 9, 0);
        }
    }
    if (show)
        repaint();
    mouse_on();
    if (show) {
        do {
            mouse_off();
            spaceship_stars();
            mouse_on();
            flip((game_flags & (1 << civ)) ? 2 : 4);
            poll_mouse();
        } while (!kbhit() && --wait && !mse_held);
        if (wait && !mse_held)
            key = key_cmd();
        if (civ == civnow && ask && wait < 0 && ((mse_held && ptr_x > 242 && ptr_y >= 190) || key == 'l')
            && !(game_flags & (1 << civnow))) {
            strcpy(msgtext, "Confirm spaceship launch.\n(");
            strcat(msgtext, itoa(success, nstr, 10));
            strcat(msgtext, "% chance of success)\n No Launch\n Launch CONFIRMED!\n");
            if (show_msg(msgtext, 80, 100) == 1)
                spaceship_launch(civnow);
        }
    }
    space_ship_habitation[civ] = rocket_module[6];
    return success;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int ship_connected(int civ, int x, int y)
{
    char c;

    c = spaceships[civ][x][y];
    return c != -1 && c < 3;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void spaceship_stars(void)
{
    int colour;
    int i;
    int step;

    for (i = 0; i < 20; i++) {
        if (star_on[i])
            world_draw(scrbuf, star_x[i], star_y[i], 0);
        switch (i & 3) {
        case 0:
            star_x[i] += 2;
            step = 7;
            break;
        case 1:
            star_x[i]++;
            step = 3;
            break;
        case 2:
            if (ship_state[0] & 1)
                star_x[i]++;
            step = 8;
            break;
        case 3:
            if (ship_state[0] & 1)
                star_x[i]++;
            step = 8;
            break;
        }
        if (star_x[i] >= 240) {
            star_x[i] = 1;
            star_y[i] = rand_scaled(192) + 8;
            star_on[i] = 0;
        }
        if ((colour = pixel_at(0, star_x[i], star_y[i])) == 0) {
            star_on[i] = 1;
            world_draw(scrbuf, star_x[i], star_y[i], step);
        } else
            star_on[i] = 0;
    }
    ship_state[0]++;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int spaceship_ai_build(int civ, int type)
{
    int first;
    int found;
    int n;

    n = ai_ship_success = 0;
    found = 1;
    first = 999;
    if (type == 2 || (spaceship_arrive_dates[civ] != -1 && spaceship_arrive_dates[civ] - year < 15 && (spaceship_arrive_dates[civ] < spaceship_arrive_dates[civnow] || spaceship_arrive_dates[civnow] == -1)))
        n = 33;
    do {
        if (spaceships[civ][ship_layout[n][0]][ship_layout[n][1]] == -1) {
            if (first == 999 && ship_layout[n][2] >= type * 3 + 3)
                first = n;
            if (ship_layout[n][2] / 3 == type) {
                found = 0;
                break;
            }
        }
    } while (ship_layout[++n][0] != -1 && (n - first < 4 || type == 2));
    if (ship_layout[n][0] == -1 && first == 999)
        return 2;
    if (found)
        return found;
    if (ship_layout[n][0] == -1 && first != 999)
        return 1;
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    strcpy(msgtext, civ_names[civ]);
    strcat(msgtext, " build space ship.\n");
    draw_text(msgtext, 160, 96, 15);
    ad_wait();
    spaceship_open();
    repaint();
    spaceships[civ][ship_layout[n][0]][ship_layout[n][1]] = ship_layout[n][2];
    ai_ship_success = spaceship_view(civ, -1);
    game_flags |= 1 << (civ + 8);
    repaint();
    spaceship_close();
    return found;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void spaceship_build(int civ, int type)
{
    int p;
    int start2;
    int end1;
    int end2;
    int j;
    int k;
    int dummy;
    int m;
    int n;
    int item;
    int start1;

    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    spaceship_open();
    spaceship_view(civ, 15, 0L);
    repaint();
retry:
    item = 2;
    mouse_on();
    if (type == 1) {
        if ((dialog_dim = ship_state[1]) == 3)
            goto done;
        if ((item = show_msg("Type?\n Propulsion\n Fuel\n", 100, 80) + 3) == 2)
            goto cancel;
    }
    if (type == 2) {
        if ((dialog_dim = ship_state[2]) == 7)
            goto done;
        if ((item = show_msg("Type?\n Habitation\n Life Support\n Solar Panel\n", 100, 80) + 6) == 5)
            goto cancel;
    }
    mouse_off();
    if (type == 0) {
        start1 = 99;
        end1 = -1;
        for (k = 0; k < 33; k++) {
            if (spaceships[civ][ship_layout[k][0]][ship_layout[k][1]] != -1) {
                if (ship_layout[k][2] >= 3)
                    end1 = k;
            } else if (ship_layout[k][2] < 3 && start1 == 99)
                start1 = k;
        }
        start2 = 99;
        end2 = -1;
        k = 33;
        do {
            if (spaceships[civ][ship_layout[k][0]][ship_layout[k][1]] != -1) {
                if (ship_layout[k][2] >= 3)
                    end2 = k;
            } else if (ship_layout[k][2] < 3 && start2 == 99)
                start2 = k;
        } while (ship_layout[++k][0] != -1);
        if (end2 - start2 < end1 - start1)
            k = start1;
        else
            k = start2;
    } else {
        k = 0;
        do {
            if (spaceships[civ][ship_layout[k][0]][ship_layout[k][1]] != -1)
                continue;
            if (ship_layout[k][2] == item)
                break;
        } while (ship_layout[++k][0] != -1);
    }
    if (k != -1) {
        spaceships[civ][ship_layout[k][0]][ship_layout[k][1]] = ship_layout[k][2];
    } else if (type) {
        ship_state[type] |= 1 << item;
        strcpy(msgtext, "No further ");
        strcat(msgtext, ship_parts[type * 3 + item].name);
        strcat(msgtext, " required.\n");
        show_msg(msgtext, 120, 80);
        goto retry;
    }
    if (!(game_flags & (1 << (civ + 8)))) {
        for (p = 1; p < 8; p++)
            if (game_flags & (1 << (p + 8)))
                last_met_turn[p] = 0;
        game_flags |= 1 << (civ + 8);
    }
done:
    mouse_on();
    ship_success = spaceship_view(civ, -1, 1L);
cancel:
    mouse_off();
    spaceship_close();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void spaceship_tile(int civ, int x, int y)
{
    int d;
    int v;
    int mask;
    int tile;
    char old;

    old = spaceships[civ][x][y];
    if (old > 2 || old == -1)
        return;
    mask = 0;
    for (d = 1; d <= 8; d += 2) {
        mask <<= 1;
        if ((v = spaceships[civ][spiral_x[d] + x][spiral_y[d] + y]) >= 0 && v <= 2)
            mask |= 1;
    }
    mask = (mask >> 2 | mask) & 3;
    if (mask == 0 || mask == 3)
        tile = 2;
    else
        tile = mask == 1 ? 0 : 1;
    spaceships[civ][x][y] = tile;
}

int spaceship_fits(int civ, int type, int x, int y)
{
    int dx;
    int dy;

    for (dx = 0; dx < ship_parts[type].w / 16; dx++)
        for (dy = 0; dy < ship_parts[type].h / 16; dy++)
            if (!spaceship_free(civ, x + dx, y + dy) || x + dx >= 15 || y + dy >= 12)
                return 0;
    return 1;
}

int spaceship_free(int civ, int x, int y)
{
    int n;

    if (spaceships[civ][x][y] != -1)
        return 0;
    n = spaceships[civ][x - 1][y];
    if (n != -1 && ship_parts[n].w > 16)
        return 0;
    n = spaceships[civ][x][y - 1];
    if (n != -1 && ship_parts[n].h > 16)
        return 0;
    n = spaceships[civ][x - 1][y - 1];
    if (n != -1 && ship_parts[n].h > 16 && ship_parts[n].w > 16)
        return 0;
    return 1;
}

void spaceship_inspect(void)
{
    int i;
    int civ;

    if (game_flags & 0xfe00) {
        strcpy(msgtext, "Which nation?\n ");
        for (i = 1; i < 8; i++) {
            strcat(msgtext, civ_adjectives[i]);
            strcat(msgtext, "\n ");
        }
        dialog_dim = (game_flags >> 9) ^ 0xff;
        if ((civ = show_msg(msgtext, 100, 80) + 1) > 0) {
            spaceship_open();
            spaceship_view(civ, -1, 1L);
            spaceship_close();
        }
    }
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void spaceship_launch(int civ)
{
    game_flags |= 1 << civ;
    mouse_off();
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    strcpy(msgtext, civ_names[civ]);
    strcat(msgtext, " LAUNCH space ship!");
    draw_text(msgtext, 160, 92, 15);
    strcpy(msgtext, "Estimate arrival in ");
    strcat(msgtext, itoa(spaceship_arrive_dates[civ], nstr, 10));
    strcat(msgtext, ".\n");
    draw_text(msgtext, 160, 100, 15);
    liftoff_when[civ] = year;
    if (civ != civnow)
        spaceship_open();
    repaint();
    ad_wait();
    spaceship_view(civ, -1, 0L);
    mouse_on();
    if (civ != civnow)
        spaceship_close();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Spaceship part positions and kinds, ended by -1; original far-data block. */
char far ship_layout[68][3] = {
    { 11, 5, 1 }, { 11, 4, 2 }, { 11, 6, 1 }, { 11, 7, 2 }, { 12, 4, 0 }, { 13, 4, 0 },
    { 12, 5, 4 }, { 13, 5, 3 }, { 12, 7, 0 }, { 13, 7, 0 }, { 12, 6, 4 }, { 13, 6, 3 },
    { 12, 8, 4 }, { 13, 8, 3 }, { 12, 3, 4 }, { 13, 3, 3 }, { 11, 9, 1 }, { 11, 10, 2 },
    { 12, 10, 0 }, { 13, 10, 0 }, { 12, 9, 4 }, { 13, 9, 3 }, { 11, 3, 2 }, { 11, 2, 1 },
    { 11, 1, 2 }, { 12, 1, 0 }, { 13, 1, 0 }, { 12, 2, 4 }, { 13, 2, 3 }, { 12, 11, 4 },
    { 13, 11, 3 }, { 12, 0, 4 }, { 13, 0, 3 }, { 11, 5, 1 }, { 11, 4, 2 }, { 11, 6, 1 },
    { 11, 7, 2 }, { 9, 6, 6 }, { 9, 4, 7 }, { 11, 3, 2 }, { 10, 3, 0 }, { 9, 3, 0 },
    { 8, 1, 8 }, { 11, 8, 2 }, { 10, 8, 0 }, { 9, 8, 0 }, { 8, 8, 0 }, { 7, 8, 0 },
    { 7, 6, 7 }, { 8, 3, 0 }, { 7, 3, 0 }, { 8, 9, 8 }, { 7, 4, 6 }, { 6, 3, 0 },
    { 5, 3, 0 }, { 5, 4, 7 }, { 6, 8, 0 }, { 5, 8, 0 }, { 4, 9, 8 }, { 5, 6, 6 },
    { 4, 3, 0 }, { 3, 3, 0 }, { 3, 6, 7 }, { 4, 8, 0 }, { 3, 8, 0 }, { 4, 1, 8 },
    { 3, 4, 6 }, { -1, -1, -1 }
};
