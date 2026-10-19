/* Overlay 16: technology-picture quiz. */
#include <string.h>
#include <stdlib.h>
#include "CIV.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "SPACESHP.H"
#include "UNITS.H"
#include "CAPTURE.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "FILEIO.H"
#include "GAME.H"
#include "MESSAGE.H"
#include "PEDIA.H"
#include "QUIZ.H"
#include "TURN.H"

extern struct unit armydata[8][128];

extern char far advance_pages[];

extern void cycle_off(void);

extern void disk_insert(int, long);

extern void restore_palette(char *);

extern int rand_scaled(int);

extern int quiz();              /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int quiz_event(void)
{
    cycle_off();
    if (!quiz(1L))
        quiz(0L);
    sel_player();
    cycle_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int quiz(a)
int a;
{
    int colour;
    char least[32];
    int req1;
    char cur[32];
    int req2;
    int k, ypos, ytop, adv1, adv2;
    int right, tile, n, col;
    int next, result, topic;

    mouse_off();
    do {
        topic = rand_scaled(0x48);
        req1 = advances[topic].prereq1;
        req2 = advances[topic].prereq2;
    } while (req1 == -1 || req2 == -1 || req1 == 0x7f || topic == 0x28);
    tile = pedia_advance_index(topic);
    strcpy(msgtext, "iconpg1.pic");
    msgtext[6] += tile / 9;
    disk_insert(3, 1L);
    load_screen(1, msgtext);
    disk_insert(0, 1L);
    colour = vga_modes ? 0xfd : 0x0f;
    rect_fill(scrbuf, 0, 0, 320, 200, colour);
    restore_palette("iconpg1.pal");
    draw_text("CIVILIZATION QUIZ", 160, 8, 0);
    msgtext[0] = 0;
    named_msg(a ? "*QUIZ" : "*QUIZ2");
    ypos = news_effect(99, 48, 24, 12) + 8;
    draw_string("Which civilization advances are required to learn", 24, ypos, 0);
    ypos += 8;
    draw_string("the advance pictured below.", 24, ypos, 0);
    ypos += 8;
    blit_centred(tile % 3 * 111 + 1, tile % 9 / 3 * 69 + 1, 110, 68, 58, ypos + 48);
    adv1 = (advance_pages[topic] - 1) / 10;
    strcpy(msgtext, "Pages ");
    strcat(msgtext, itoa(10 * adv1 + 1, nstr, 10));
    strcat(msgtext, "-");
    strcat(msgtext, itoa(10 * adv1 + 10, nstr, 10));
    draw_text(msgtext, 62, 184, 0);
    ypos += 16;
    strcpy(cur, "a");
    ytop = ypos;
    n = 0;
    strcpy(msgtext, " ");
    do {
        strcpy(least, "zzz");
        next = -1;
        for (k = 0; k < 0x48; k++) {
            if (((char) (k + topic) & 15) && !(req1 == advances[k].prereq1 || req2 == advances[k].prereq1 || topic == advances[k].prereq1) && !(req1 == advances[k].prereq2 || req2 == advances[k].prereq2 || topic == advances[k].prereq2))
                continue;
            if (advances[k].prereq1 == 0x7f || advances[k].prereq2 == 0x7f)
                continue;
            if (stricmp(advances[k].name, cur) > 0 && stricmp(advances[k].name, least) < 0) {
                next = k;
                strcpy(least, advances[k].name);
            }
        }
        if (next != -1 && (next == topic || n < 11)) {
            if (next == topic)
                right = n;
            adv1 = advances[next].prereq1;
            if (adv1 != -1) {
                adv2 = advances[next].prereq2;
                if (adv2 != -1) {
                    strcat(msgtext, advances[adv1].name);
                    strcat(msgtext, " and ");
                    strcat(msgtext, advances[adv2].name);
                    strcat(msgtext, "\n ");
                    n++;
                }
            }
        }
        strcpy(cur, least);
    } while (next != -1 && n < 12);
    mouse_on();
    rect_fill(scrbuf, 100, ytop, 32, 64, colour);
    result = menu_dialog(msgtext, 100, ytop | 1);
    mouse_off();
    if (result == right) {
        msgtext[0] = 0;
        named_msg("*CORRECT");
        rect_fill(scrbuf, 24, 24, 280, 48, colour);
        news_effect(99, 48, 24, 9);
        ad_wait();
        game_flags |= 1;
    } else if (!a) {
        msgtext[0] = 0;
        named_msg("*INCORRECT");
        rect_fill(scrbuf, 24, 24, 280, 48, colour);
        news_effect(99, 48, 24, 12);
        ad_wait();
        quiz_busy = 1;
        for (k = 0; k < 128; k++)
            if (armydata[civnow][k].type != -1)
                drop_u_stack(civnow, k);
        quiz_busy = 0;
    }
    mouse_on();
    return result == right;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Civilopedia page of each advance; original far-data block. */
char far advance_pages[68] = {
    8, 20, 38, 0, 39, 79, 9, 65, 81, 72, 77, 87, 42, 123, 64, 41,
    67, 14, 56, 13, 55, 26, 124, 115, 122, 15, 80, 83, 86, 111, 62, 49,
    44, 120, 48, 54, 17, 21, 45, 7, 114, 70, 91, 22, 119, 36, 113, 118,
    78, 92, 85, 121, 116, 12, 40, 99, 43, 117, 11, 47, 88, 93, 19, 112,
    35, 59, 46, 10
};
