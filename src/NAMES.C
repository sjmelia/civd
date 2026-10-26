#include "CAPTURE.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "UNITS.H"
#include "NAMES.H"
#include "DIALOG.H"
#include "MAP.H"

struct city {
    unsigned long flags;
    unsigned char x, y;
    char status, f7, f8, f9, f10, owner;
    char pad12[10];
    unsigned char f22, slot[5];
};

extern struct city townbuf[];
extern char far place_name[][13];
extern char far map_visible_to[];

extern int kbhit(void), getch(void);
extern int strlen(), strnicmp();
extern char *strcpy(char *, char *);

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int rename_city(int city)
{
    int ret;
    int reserve;

    mouse_off();
    menu_begin();
    do {
        rect_fill(scrbuf, 80, 80, 160, 32, 15);
        draw_box(80, 80, 160, 32, 11);
        draw_string("City Name...", 88, 82, 0);
        msgtext[0] = 0;
        append_place_name(city);
        ret = edit_line(88, 96, 12);
    } while (strlen(msgtext) == 0);
    if (ret)
        copy_to_far(msgtext, place_name[townbuf[city].f22], 13);
    menu_end();
    mouse_on();
    return ret;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void rename_leader(void)
{
    mouse_off();
    rect_fill(scrbuf, 158, 88, 160, 32, 15);
    draw_box(158, 88, 160, 32, 11);
    draw_string("Your Name...", 166, 90, 0);
    strcpy(msgtext, leader_names[civnow]);
    edit_line(166, 104, 13);
    strcpy(leader_names[civnow], msgtext);
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void rename_tribe(void)
{
    int n;

    mouse_off();
    rect_fill(scrbuf, 158, 88, 160, 32, 15);
    draw_box(158, 88, 160, 32, 11);
    draw_string("Name of your Tribe...", 166, 90, 0);
    strcpy(msgtext, civ_names[civnow]);
    edit_line(166, 104, 11);
    strcpy(civ_names[civnow], msgtext);
    n = strlen(msgtext) - 1;
    if (msgtext[n] == 's')
        msgtext[n] = 0;
    strcpy(civ_adjectives[civnow], msgtext);
    civ_adjectives[civnow][10] = 0;
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void goto_city(void)
{                               /* find a city by name and centre on it */
    int city;

    mouse_off();
    menu_begin();
    city = find_city("Where in the heck is ... (city name)");
    if (city != -1 && (map_visible_to[50 * townbuf[city].x + townbuf[city].y] & (1 << civnow))) {
        reveal(civnow, townbuf[city].x - 7, townbuf[city].y - 6);
    } else {
        rect_fill(scrbuf, 64, 78, 224, 10, 15);
        draw_string("Unknown city.", 82, 80, 0);
        ad_wait();
        menu_end();
    }
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int find_city(char *prompt)
{
    int k;
    int index;
    char name[16];

    rect_fill(scrbuf, 64, 78, 224, 24, 15);
    draw_box(64, 78, 224, 24, 0);
    draw_string(prompt, 66, 80, 0);
    msgtext[0] = 0;
    edit_line(66, 89, 16);
    for (k = 0; k < 256; k++) {
        copy_from_far(place_name[k], name, 13);
        if (strnicmp(msgtext, name, strlen(msgtext)) == 0) {
            for (index = 0; index < 128; index++)
                if (townbuf[index].status != -1 && townbuf[index].f22 == k)
                    return index;
        }
    }
    return -1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int edit_line(int x, int y, int n)
{
    int code;
    int accepted;
    int index;
    int edit;
    char input[64];

    rect_fill(scrbuf, x, y, 7 * n, 11, 15);
    draw_box(x, y - 1, (n << 3) + 8, 13, 0);
    scrbuf->text_colour = 15;
    edit_cursor = 0;
    edit = 1;
    for (index = strlen(msgtext); index < n; index++)
        msgtext[index] = ' ';
    msgtext[n] = 0;
    edit_redraw(x, y, n);
    accepted = 1;
    for (;;) {
        while (!kbhit()) {
            if (mse_held)
                goto keys;
            recolour_rect(scrbuf, edit_cursor_x, y + 1, edit_cursor_w, 10, 15, 11);
            flip(10);
            recolour_rect(scrbuf, edit_cursor_x, y + 1, edit_cursor_w, 10, 11, 15);
            if (!kbhit())
                flip(10);
            poll_mouse();
        }
keys:
        if (mse_held)
            code = 13;
        else
            code = getch();
        if (code == 0)
            code = getch() + 128;
        if (code == 13) {
finish:
            index = n - 1;
            goto done;
        }
        if (code == 27) {
            accepted = 0;
            goto finish;
        }
        if (code == 0xcb && edit_cursor)
            edit_cursor--;
        if (code == 0xcd && n - 1 > edit_cursor)
            edit_cursor++;
        if (code == 0xd2) {
            strcpy(input, msgtext + edit_cursor);
            strcpy(msgtext + edit_cursor + 1, input);
            msgtext[edit_cursor] = ' ';
            msgtext[n] = 0;
        }
        if (code == 0xd3) {
            strcpy(msgtext + edit_cursor, msgtext + edit_cursor + 1);
            msgtext[edit_cursor] = ' ';
            msgtext[n - 1] = ' ';
            msgtext[n] = 0;
        }
        if (code == 8 && edit_cursor) {
            strcpy(msgtext + edit_cursor - 1, msgtext + edit_cursor);
            msgtext[n - 1] = ' ';
            msgtext[n] = 0;
            edit_cursor--;
        }
        if (code < 128 && code >= 32) {
            if (edit)
                for (index = 0; index < n; index++)
                    msgtext[index] = ' ';
            msgtext[edit_cursor] = code;
            if (n - 1 > edit_cursor)
                edit_cursor++;
        }
        edit = 0;
        edit_redraw(x, y, n);
    }
done:
    for (; index > 0; index--) {
        if (msgtext[index] == ' ')
            msgtext[index] = 0;
        else
            break;
    }
    repaint();
    return accepted;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int edit_redraw(int x, int y, int n)
{
    int index;
    int character;

    x += 2;
    y += 2;
    for (index = 0; index < n; index++) {
        rect_fill(scrbuf, x, y, 8, 8, 15);
        if (strlen(msgtext) > index) {
            character = msgtext[index + 1];
            msgtext[index + 1] = 0;
            draw_string_dim(msgtext + index, x, y, 0);
            msgtext[index + 1] = character;
            if (index == edit_cursor) {
                edit_cursor_x = x;
                edit_cursor_w = char_width(scrbuf->font, msgtext[index]);
            }
            x += char_width(scrbuf->font, msgtext[index]);
        }
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */
