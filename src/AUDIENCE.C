
#include "CIV.H"
#include "CAPTURE.H"
#include "CORE.H"
#include "DIPLOMAC.H"
#include "ECONOMY.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "INTRO.H"
#include "MESSAGE.H"
#include "PEDIA.H"
#include "RESIDENT.H"
#include "ADVISORS.H"
#include "AUDIENCE.H"
#include "CITYMGT.H"
#include "FILEIO.H"
#include "NOTES.H"
#include "TURN.H"
#include "UNITS.H"
#include "DIALOG.H"
#include "DRAW.H"
#include "MAP.H"
#include "DRIVER.H"

extern char far leader_fonts[16];
extern int far portrait_size[14][2];
extern int far portrait_offset[14][2];
extern int far portrait_coords[7][4][2];
extern int far portrait_origins[7][4][2];

extern struct unit armydata[][128];
extern struct city townbuf[];

extern int treaties[8][8];

extern int debug;


extern char *strcpy(char *, char *);
extern char *strcat(char *, char *);
extern char *itoa(int, char *, int);








/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void leader_audience(civ, x0, y0, mode)
int civ;
int x0;
int y0;
int mode;
{
    int num;
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int w[7];
    int g;
    int h;
    int x;
    int i;
    int y;
    int j;
    int z;
    int k;
    int k00;
    int l;
    int m;
    int n;

    map_mark(x0, y0);
    map_note(x0, y0);
    n = last_met_turn[civ];
    if (civ_tribe_of[civ] == 14) {
        strcpy(leader_titles[1], "Empress");
        strcpy(leader_titles[2], "Queen");
    }
    if (mode == 0) {
        last_met_turn[civ] = turn_tick;
    }
    y = audience_active = 0;
    mouse_mode = mode;
    b = k = 0;
    c = 1;
    ai_ruler_refused = 0;

    for (i = 1; i < 15; i++) {
        if (ai_continent_towns[civ][i] > (city_count[civ] > 1) && score_of(civnow, i, civ) > value_of(civ, i)) {
            k += (cur_tier ? 2 : 4) * score_of(civnow, i, civ) / (value_of(civ, i) + 1);
        } else {
            if (defenders_island[civnow][i] != 0 && defenders_island[civ][i] != 0) {
                y = 1;
            }
            if (ai_continent_towns[civnow][i] != 0) {
                if (ai_continent_towns[civ][i] > 1) {
                    b += civ_cont_attackers[civ][i] - score_of(civnow, i, civ);
                } else {
                    b += civ_cont_attackers[civ][i];
                }
            } else if (ai_continent_towns[civ][i] != 0) {
                b += (defenders_island[civ][i] - civ_cont_attackers[civnow][i]) / 2;
            } else {
                b += (civ_cont_attackers[civ][i] - civ_cont_attackers[civnow][i]) / 4;
            }
        }
    }

    a = (nation_ratings[civnow] == 7 && city_count[civnow] > 4 && city_count[civ] > 1 && forces_present[civnow][25] == 0 && turn_tick > 200);

    if (a || ((treaties[civ][civnow] & 8) && 3 * military_table[civ] > military_table[civnow])) {
        if (power_of(civnow) == 0) {
            y = 1;
            b = clamp(b, 10 * nation_ratings[civ], 9999);
            k = 0;
        }
    }

    if (forces_present[civ][25] != 0) {
        treaties[civ][civnow] |= 0x80;
    }
    if (forces_present[civ][25] != 0 && power_of(civnow) == 0) {
        y = 1;
        b = clamp(b, 100, 9999);
    }

    if (forces_present[civnow][25] != 0) {
        if (power_of(civ) == 0) {
            k = 4;
        } else {
            b = ((b / 2) * forces_present[civ][25]) / forces_present[civnow][25];
        }
    }

    if (k != 0) {
        y = 0;
        c = 0;
    }

    if (civ_wonder(civnow, 7) || civ_wonder(civnow, 18)) {
        y = a = 0;
        if (n == -2) {
            n = -1;
        }
    }

    if (treaties[civ][civnow] & 8) {
        b *= 2;
    } else if (n == -2) {
        b /= 4;
    }

    b = clamp(((cur_tier + 1) * b) / 32, 0, 20) * 50;

    if (b > money[civnow] && b <= 2 * money[civnow] && money[civnow] >= 50) {
        b = (money[civnow] / 50) * 50;
    }

    if (forces_present[civ][25] != 0) {
        if (!(treaties[civ][civnow] & 0x80) && money[civnow] < b) {
            b = (money[civnow] / 50) * 50;
        }
        treaties[civ][civnow] |= 0x80;
    }

    if (b == 0 || 3 * military_table[civ] < military_table[civnow]) {
        y = 0;
    }
    if (y != 0) {
        c = 2;
    }
    if (y == 0 && (treaties[civ][civnow] & 4)) {
        c = 0;
    }

    g = y;
    j = 0;
    if (treaties[civ][civnow] & 8) {
        j = -2;
    }
    l = -1;
    for (x = 1; x < 8; x++) {
        if (x != civnow && (treaties[civ][x] & 3) == 1) {
            if (military_table[civ] < 4 * military_table[x]) {
                j++;
            }
            if (military_table[x] > military_table[civ]) {
                j++;
            }
            if (treaties[civnow][x] & 2) {
                l = x;
            }
        }
    }
    j -= civt[civ_tribe_of[civ]].aggressive;
    if (military_table[civnow] < military_table[civ]) {
        j--;
    }
    if (j > ((treaties[civ][civnow] & 2) == 0)) {
        if (a == 0) {
            y = 0;
            if (rand_scaled(2) != 0) {
                b = 0;
            }
        }
    }

    if (civ_gov[civ] >= 4 && y != 0) {
        if (!(treaties[civ][civnow] & 8)) {
            if (3 * military_table[civnow] > 2 * military_table[civ] && a == 0) {
                y = 0;
            } else {
                set_government(civ, 2);
            }
        }
    }

    z = 0;
    if ((treaties[civ][civnow] & 2) && y != 0) {
        if (military_table[civ] < military_table[civnow] || (treaties[civ][civnow] & 8)) {
            y = 0;
            z = 1;
            c = 0;
        }
    }

    for (;;) {
        d = e = -1;
        f = m = k00 = 0;
        for (x = 0; x < 72; x++) {
            if (has_tech(civnow, x)) {
                f++;
            }
            if (has_tech(civ, x)) {
                m++;
            }
            if (!has_tech(civnow, x) && has_tech(civ, x)) {
                d = x;
            }
            if (has_tech(civnow, x) && !has_tech(civ, x)) {
                if (advance_value(civ, x) >= k00) {
                    e = x;
                    k00 = advance_value(civ, x);
                }
            }
        }

        if ((!(treaties[civ][civnow] & 0x20) && y == 0) || m <= 6) {
            if (f >= m && d != -1 && audience_open(civ, x0, y0) != -1) {
                style = 1;
                msgtext[0] = 0;
                strcpy(message_slot, advances[d].name);
                named_msg("*EXCHANGE");
                setting_value = 12;
                if (leader_menu(msgtext, 20, 139) == 1) {
                    mouse_off();
                    menu_begin();
                    mouse_on();
                    style = -1;
                    no_pictures = 1;
                    steal_advance(civ, civnow);
                    scrbuf->font = leader_font;
                    no_pictures = 1;
                    steal_advance(civnow, civ);
                    cycle_off();
                    scrbuf->font = leader_font;
                    y = 0;
                    style = 1;
                    mouse_off();
                    menu_end();
                    mouse_on();
                    continue;
                }
            }
        }
        break;
    }

    if (y == 0 && mode == 0 && (treaties[civ][civnow] & 2) == 2 && k > 2) {
        goto cleanup;
    }
    if (!(rule_options & 1)) {
        y = 1;
        c = 0;
        k = 0;
        z = 0;
    }

    if (l != -1 && audience_open(civ, x0, y0) != -1 && c != 0) {
        msgtext[0] = 0;
        strcpy(message_slot_a, civ_names[l]);
        strcpy(message_slot, y != 0 ? "demand" : "request");
        named_msg("*CANCEL");
        style = (y != 0 ? 2 : 1);
        setting_value = 12;
        if (y != 0) {
            sound(33);
        }
        if (leader_menu(msgtext, 20, 139) == 1) {
            y = 0;
            if (rand_scaled(2) != 0) {
                b = 0;
            }
            accord(civnow, l, 2);
            treaties[l][civnow] |= 8;
        }
    }

    if (b != 0 && c == 2 && money[civnow] > b && audience_open(civ, x0, y0) != -1) {
        msgtext[0] = 0;
        strcat(msgtext, itoa(b, nstr, 10));
        strcpy(message_slot_b, msgtext);
        msgtext[0] = 0;
        named_msg("*TRIBUTE");
        ai_ruler_refused = 0;
        style = 2;
        setting_value = 12;
        sound(33);
        if (leader_menu(msgtext, 20, 139) == 1) {
            money[civnow] -= b;
            money[civ] += b;
            y = 0;
        }
        b = 999;
    }

    if (c != 0 && e != -1 && b != 999 && audience_open(civ, x0, y0) != -1) {
        msgtext[0] = 0;
        strcpy(message_slot, advances[e].name);
        named_msg("*TAKECIV");
        ai_ruler_refused = 0;
        style = 2;
        setting_value = 12;
        sound(33);
        if (leader_menu(msgtext, 20, 139) == 1) {
            give_advance(civ, e, civnow);
            y = 0;
        }
        b = 999;
    }

    if (y != 0 && (treaties[civ][civnow] & 2) && b > 50) {
        if (audience_open(civ, x0, y0) != -1) {
            sound(33);
            msgtext[0] = 0;
            named_msg("*PROVOKE");
            news_wrap(40);
            ai_ruler_refused = 1;
            style = 2;
            setting_value = 12;
            leader_menu(msgtext, 20, 139);
        } else {
            msgtext[0] = 0;
            named_msg("*PROVOKE");
            news_wrap(40);
            show_msg(msgtext, 40, 100);
        }
        accord(civnow, civ, 2);
    } else if (y != 0 && b == 999) {
        if (audience_open(civ, x0, y0) != -1) {
            sound(33);
            msgtext[0] = 0;
            named_msg("*REJECT");
            news_wrap(40);
            ai_ruler_refused = 1;
            style = 2;
            setting_value = 12;
            leader_menu(msgtext, 20, 139);
            accord(civnow, civ, 2);
        }
    } else if (y != 0 && (!(treaties[civ][civnow] & 1) || mode != 0)) {
        if (audience_open(civ, x0, y0) != -1) {
            sound(33);
            msgtext[0] = 0;
            named_msg("*RID");
            news_wrap(40);
            ai_ruler_refused = 1;
            style = 2;
            setting_value = 12;
            leader_menu(msgtext, 20, 139);
        } else {
            msgtext[0] = 0;
            named_msg("*RID");
            news_wrap(40);
            show_msg(msgtext, 40, 100);
        }
        accord(civnow, civ, 2);
    }

    if (y != 0) {
        goto closing;
    }
    if ((treaties[civ][civnow] & 2) && mode == 0) {
        goto closing;
    }
    if (audience_open(civ, x0, y0) == -1) {
        goto closing;
    }
    msgtext[0] = 0;
    named_msg("*PEACE");
    ai_ruler_refused = 0;
    style = 0;
    setting_value = 12;
    if (leader_menu(msgtext, 20, 139) == 1) {
        treaty_prompt(civ);
        make_peace(civ);
        last_met_turn[civ] = turn_tick + 16;
        goto closing;
    } else if (civ_gov[civnow] >= 4) {
        strcpy(msgtext, "You are overruled by the Senate.\nTreaty signed.\n");
        leader_menu(msgtext, 20, 139);
        make_peace(civ);
        goto closing;
    } else if (treaties[civ][civnow] & 2) {
        accord(civnow, civ, 2);
    }
    if (k >= 4 && d != -1 && !(treaties[civ][civnow] & 10)) {
        strcpy(message_slot, advances[d].name);
        msgtext[0] = 0;
        named_msg("*GIVECIV");
        style = 0;
        setting_value = 12;
        if (leader_menu(msgtext, 20, 139) == 1) {
            mouse_off();
            menu_begin();
            mouse_on();
            cycle_off();
            style = -1;
            no_pictures = 1;
            give_advance(civnow, d, civ);
            scrbuf->font = leader_font;
            cycle_off();
            style = 0;
            mouse_off();
            menu_end();
            mouse_on();
            treaty_prompt(civ);
            make_peace(civ);
            n = turn_tick;
        }
    }

    if (!(treaties[civ][civnow] & 10)) {
        h = clamp(2 * k - 4, 0, money[civ] / 50) * 50;
        if (h != 0) {
            msgtext[0] = 0;
            strcat(msgtext, itoa(h, nstr, 10));
            strcpy(message_slot_b, msgtext);
            msgtext[0] = 0;
            named_msg("*GIVECASH");
            style = 0;
            setting_value = 12;
            if (leader_menu(msgtext, 20, 139) == 1) {
                treaty_prompt(civ);
                make_peace(civ);
                money[civnow] += h;
                money[civ] -= h;
                n = turn_tick;
            }
        }
    }

    if (k != 0 && city_count[civ] <= 1 && !(treaties[civ][civnow] & 10)) {
        msgtext[0] = 0;
        strcat(msgtext, itoa(money[civ], nstr, 10));
        strcpy(message_slot_b, msgtext);
        msgtext[0] = 0;
        named_msg("*GROVEL");
        style = 0;
        setting_value = 12;
        if (leader_menu(msgtext, 20, 139) == 1) {
            treaty_prompt(civ);
            make_peace(civ);
            money[civnow] += money[civ];
            money[civ] = 0;
            mouse_off();
            menu_begin();
            mouse_on();
            cycle_off();
            style = -1;
            for (x = 0; x < 72; x++) {
                if (!has_tech(civnow, x) && has_tech(civ, x)) {
                    no_pictures = 1;
                    give_advance(civnow, x, civ);
                }
            }
            scrbuf->font = leader_font;
            cycle_off();
            style = 0;
            mouse_off();
            menu_end();
            mouse_on();
            n = turn_tick;
        }
    }

    if (!(treaties[civ][civnow] & 2)) {
        sound(33);
        msgtext[0] = 0;
        named_msg("*MOBILIZE");
        ai_ruler_refused = 1;
        style = 2;
        setting_value = 4;
        leader_menu(msgtext, 20, 139);
    }
closing:
    if (civ_tribe_of[civ] == 14) {
        strcpy(leader_titles[1], rank_title[4]);
        strcpy(leader_titles[2], rank_title[3]);
    }
    ai_ruler_refused = 0;
    if (y == 0 && (treaties[civ][civnow] & 2) == 2 && audience_open(civ, x0, y0) != -1) {
        if (c != 0 && b != 999) {
            style = 1;
            setting_value = 4;
            msgtext[0] = 0;
            named_msg("*ATTITUDE");
        } else {
            style = 0;
            setting_value = 12;
            strcpy(msgtext, "We welcome the friendship of the\n");
            strcat(msgtext, civ_adjectives[civnow]);
            strcat(msgtext, " people and their most\nwise and munificent leader:\n");
            strcat(msgtext, leader_titles[civ_gov[civnow]]);
            strcat(msgtext, " ");
            strcat(msgtext, leader_names[civnow]);
            strcat(msgtext, ".\n");
        }
        leader_menu(msgtext, 20, 139);
    }

    ai_ruler_refused = 1;
    if ((treaties[civ][civnow] & 2) && audience_open(civ, x0, y0) != -1) {
        for (;;) {
            strcpy(message_slot, civ_names[civ]);
            msgtext[0] = 0;
            named_msg("*THREAT");
            if (turn_tick - n < 16 || civ_gov[civnow] >= 4) {
                dialog_dim = 4;
            }
            if (((1 << civ) | (1 << civnow)) == players) {
                dialog_dim |= 2;
            }
            i = leader_menu(msgtext, 20, 139);
            if (i == 2) {
                h = clamp(k, 0, money[civ] / 50) * 50;
                if (h != 0) {
                    msgtext[0] = 0;
                    strcat(msgtext, itoa(h, nstr, 10));
                    strcpy(message_slot_b, msgtext);
                    msgtext[0] = 0;
                    named_msg("*GIVEMORE");
                    style = 0;
                    setting_value = 12;
                    leader_menu(msgtext, 20, 139);
                    money[civnow] += h;
                    money[civ] -= h;
                    last_met_turn[civ] = turn_tick;
                } else if (g != 0) {
                    sound(33);
                    msgtext[0] = 0;
                    named_msg("*PROVOKE");
                    accord(civnow, civ, 2);
                    style = 2;
                    setting_value = 12;
                    leader_menu(msgtext, 20, 139);
                } else {
                    strcpy(msgtext, "We ignore your feeble threats.\n");
                    style = 1;
                    setting_value = 4;
                    leader_menu(msgtext, 20, 139);
                }
            }
            if (i == 1) {
                strcpy(msgtext, "You must attack the evil:\n ");
                num = 0;
                for (x = 1; x < 8; x++) {
                    if (x == civnow || x == civ || !(players & (1 << x)))
                        continue;
                    w[num++] = x;
                    strcat(msgtext, civ_names[x]);
                    strcat(msgtext, "\n ");
                }
                h = leader_menu(msgtext, 20, 139);
                style = 1;
                if (h != -1) {
                    x = w[h];
                    strcpy(message_slot, civ_names[x]);
                    setting_value = 4;
                    if (!(treaties[civ][x] & 1)) {
                        msgtext[0] = 0;
                        named_msg("*NOCONTACT");
                        leader_menu(msgtext, 20, 139);
                    } else if (!(treaties[civ][x] & 2)) {
                        msgtext[0] = 0;
                        named_msg("*ATWAR");
                        leader_menu(msgtext, 20, 139);
                        continue;
                    } else {
                        h = clamp(((military_table[x] / 2) * (money[civ] / 100)) / 50, 2, 9999) * 50;
                        msgtext[0] = 0;
                        strcat(msgtext, itoa(h, nstr, 10));
                        strcpy(message_slot_b, msgtext);
                        msgtext[0] = 0;
                        named_msg("*MERCENARY");
                        if (leader_menu(msgtext, 20, 139) == 1) {
                            if (money[civnow] >= h) {
                                money[civ] += h;
                                money[civnow] -= h;
                                strcpy(msgtext, civ_names[civ]);
                                strcat(msgtext, " declare war\non the ");
                                strcat(msgtext, civ_names[x]);
                                strcat(msgtext, ".\n");
                            } else {
                                msgtext[0] = 0;
                                named_msg("*UNFORTUNATE");
                            }
                            leader_menu(msgtext, 20, 139);
                        }
                    }
                }
            }
            break;
        }
    }

    if (z != 0) {
        treaties[civ][civnow] |= 0x100;
    }
    treaties[civ][civnow] &= ~0x20;
    treaties[civnow][civ] &= ~0x20;
    if (mouse_mode != -1 && city_count[civ] > 4 && nation_ratings[civnow] + 1 < nation_ratings[civ] && !(treaties[civnow][civ] & 2)) {
        strcpy(message_slot_a, civ_names[civ]);
        notice_a("*SCHISM");
    }
cleanup:
    if (mouse_mode != -1) {
        audience_close();
    }
    if (civ_tribe_of[civ] == 14) {
        strcpy(leader_titles[1], rank_title[4]);
        strcpy(leader_titles[2], rank_title[3]);
    }
    audience_active = 0;
    mouse_mode = -1;
    style = -1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int mouse_mode = -1;
int style = -1;
char leader_portrait[] = { 3, 10, 7, 12, 1, 4, 13, 2, 3, 8, 3, 11, 9, 6, 0, 5 };

/* Function 1: [16AC, 16CD) - Emissary Audience Reset Helper (33 bytes) */
void audience_close(void)
{
    free_leader();
    scrbuf->font = 1;
    sel_player();
    style = -1;
    audience_active = mouse_mode = 0;
}

/* Function 2: [16CD, 1874) - Emissary Audience Reception & Inquiry Dialog (423 bytes) */
/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int audience_open(int civ, int x, int y)
{
    if (audience_active)
        return mouse_mode;
    audience_active = 1;
    if (mouse_mode == 0) {
        if (x != -1) {
            reveal(civnow, x - 8, y - 2);
        }
reload_text:
        strcpy(msgtext, "An emissary from the\n");
        strcat(msgtext, civ_names[civ]);
        strcat(msgtext, " wishes to\nspeak with you.\n");
        strcat(msgtext, "Will you receive him?\n Yes.\n No.\n");
        if (treaties[civnow][civ] & 0x40) {
            strcat(msgtext, " Check Intelligence\n");
        }
        sound(civt[civ_tribe_of[civ]].motif, 3);
        mouse_force_on();
        mouse_mode = -show_msg(msgtext, 100, 80);
        mouse_restore();
        if (mouse_mode == -2) {
            civ_dossier(civ);
            goto reload_text;
        }
    }
    if (mouse_mode == -1)
        return mouse_mode;

    mouse_off();
    load_leader(civ, 5, 1L);
    sound(civt[civ_tribe_of[civ]].anthem, 3);
    mouse_on();
    leader_font = (int) leader_fonts[civ_tribe_of[civ]] + 8;
    if (leader_font == 10)
        leader_font = 3;
    scrbuf->font = leader_font;
    style = parley_tone = 1;
    shown_face = -1;
    council_faces = 0;
    setting_value = 12;
    leader_greeting(civ);
    leader_menu(msgtext, 20, 139);
    return mouse_mode;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Function 3: [1874, 1A3F) - Leader Greeting and Nuclear Threat Generator (459 bytes) */
void leader_greeting(int civ)
{
    int dummy;

    if (treaties[civ][civnow] & 8) {
        strcpy(msgtext, "O most untrustworthy leader of the\ninfidels, hear now ");
    } else {
        strcpy(msgtext, "Greetings from ");
    }

    switch (rand_scaled(4)) {
    case 0:
        strcat(msgtext, "the most exalted\n");
        strcat(msgtext, leader_titles[civ_gov[civ]]);
        strcat(msgtext, ": ");
        strcat(msgtext, leader_names[civ]);
        strcat(msgtext, " ");
        break;
    case 1:
        strcat(msgtext, leader_names[civ]);
        strcat(msgtext, ", ruler\nand ");
        strcat(msgtext, leader_titles[civ_gov[civ]]);
        strcat(msgtext, " ");
        break;
    case 2:
        strcat(msgtext, "our most wise\n");
        strcat(msgtext, leader_titles[civ_gov[civ]]);
        strcat(msgtext, ": ");
        strcat(msgtext, leader_names[civ]);
        strcat(msgtext, " ");
        break;
    case 3:
        strcat(msgtext, "he who makes mortals\ntremble: ");
        strcat(msgtext, leader_names[civ]);
        strcat(msgtext, " ");
        break;
    }

    strcat(msgtext, "of the ");
    strcat(msgtext, civ_names[civ]);
    strcat(msgtext, "...\n");
    if (forces_present[civ][25] != 0) {
        strcat(msgtext, "Our words are backed\nwith NUCLEAR WEAPONS!\n");
    }
}

/* Function 4: [1A3F, 1A99) - Treaty Proposal Dialog Prompt (90 bytes) */
int treaty_prompt(int civ)
{
    strcpy(message_slot, civ_adjectives[civ]);
    strcpy(name_buf, civ_adjectives[civnow]);
    setting_value = 12;
    msgtext[0] = 0;
    named_msg("*TREATY");
    return leader_menu(msgtext, 20, 139);
}

/* Function 5: [1A99, 1B33) - Treaty Effect and Border Unit Cleanup (154 bytes) */
void make_peace(int civ)
{
    int u, i;

    relation_set(civnow, civ, 2);
    last_met_turn[civ] = turn_tick + 16;
    if (cur_tier == 0) {
        for (i = 0; i < 128; i++) {
            if (armydata[civ][i].type != -1) {
                u = near_city(armydata[civ][i].x, armydata[civ][i].y);
                if (townbuf[u].owner == civnow && town_gap <= 2) {
                    drop_u_stack(civ, i);
                }
            }
        }
    }
}

char face_eyes[18] = { 8, 0, 1, 2, 2, 4, 4, 4, 2, 2, 12, 13, 9, 10, 10, 10, 9, 11 };    /* eye frame for each animation step */
char face_mouths[18] = { 0, 0, 0, 2, 3, 4, 5, 4, 3, 2, 5, 5, 7, 10, 11, 10, 7, 11 };    /* mouth frame for each animation step */
char face_next[18] = { 0, 1, -1, 1, 1, 1, 0, 1, 1, -1, 1, -2, 1, 1, 0, 1, -1, -3 };     /* step advance: >0 next, 0 hold, <0 jump */

/* Function 6: [1B33, 1D2B) - Talking Head Animation Polling Loop (504 bytes) */
void leader_animate(void)
{
    int mouth_frame;
    int eye_frame;
    int next;

    council_faces = -1;
    if (setting_value == 0 || anim_step != 0) {
        if (face_next[anim_step] != 0) {
            if (face_next[anim_step] > 0) {
                anim_step++;
            } else {
                switch (face_next[anim_step]) {
                case -1:
                    anim_step = 0;
                    break;
                case -2:
                    anim_step = 6;
                    break;
                case -3:
                    anim_step = 14;
                    break;
                }
            }
        } else {
            next = -1;
            switch (style) {
            case 0:
                face_eyes[6] = (ai_ruler_refused != 0) ? 4 : 2;
                if (anim_step != 6)
                    next = 3;
                break;
            case 1:
                if (anim_step != 0)
                    next = 0;
                break;
            case 2:
                face_mouths[14] = (ai_ruler_refused != 0) ? 11 : 7;
                face_mouths[17] = (ai_ruler_refused != 0) ? 11 : 7;
                if (anim_step != 14)
                    next = 12;
                break;
            }

            if (setting_value != 0 && anim_step != 0)
                next = 0;

            if (next != -1) {
                if (anim_step == 0)
                    anim_step = next;
                if (anim_step == 6 || anim_step == 14)
                    anim_step++;
            } else {
                council_faces = 0;
                if (anim_step == 14)
                    council_faces = 1;
                if (anim_step == 6)
                    council_faces = 2;
            }
        }

        if (talk_hold == 0) {
            if (rand_scaled(12) == 0 && face_next[anim_step] == 0) {
                if (anim_step == 0)
                    anim_step = 1;
                if (anim_step == 6)
                    anim_step = 10;
                if (anim_step == 14)
                    anim_step = 17;
            }
        }

        eye_frame = face_eyes[anim_step];
        mouth_frame = face_mouths[anim_step];
    } else {
        eye_frame = (anim_step != 0) ? 5 : -1;
        mouth_frame = rand_scaled(2) ? 9 : 0;
        if ((setting_value & 3) == 0)
            mouth_frame = 8;
        setting_value--;
        talk_hold = 4;
    }

    if (talk_hold != 0)
        talk_hold--;

    draw_leader_face(eye_frame, mouth_frame, council_faces);
    flip(10);
}

int advisor_x[4] = { 41, 199, 1, 239 }; /* x of each advisor figure beside the throne */

/* Function 7: [1D2B, 21AC) - Leader Portrait & Backdrop Loader (1,153 bytes) */
/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void load_leader(int civ, int mode, long flag)
{
    char filename[16];
    int i;
    int j;

    audience_civ = civ;
    no_portraits = 0;
    if ((debug & 8) == 0) {
        if (disk_insert(2, 0L) == 0) {
            no_portraits = 1;
            mouse_off();
            draw_backdrop(0, 128, 320, 72);
            draw_panel(0, 128, 319, 71, 15, 8);
            strcpy(msgtext, leader_names[civ]);
            strcat(msgtext, ": ");
            strcat(msgtext, leader_titles[civ_gov[civ]]);
            strcat(msgtext, " of the ");
            strcat(msgtext, civ_names[civ]);
            strcat(msgtext, ".");
            draw_string(msgtext, 8, 130, 15);
            mouse_on();
            return;
        }
    } else {
        disk_insert(2, 1L);
    }

    sprite_arena_init();
    strcpy(filename, "govt0a.pic");
    filename[4] = civ_gov[civ] / 2 + '0';
    modern_court = has_tech(civ, 20);
    if (civ_gov[civ] == 3)
        filename[4] = '3';
    else if (modern_court)
        filename[5] = 'm';

    load_screen(1, filename);

    for (i = 0; i < 4; i++) {
        icons_council[i][0] = cut_tile(1, i * 80 + 1, 101, 78, 99);
    }

    portrait = leader_portrait[civ_tribe_of[civ]];
    disk_insert(portrait / 7 + 4, 1L);

    strcpy(filename, "king00.pic");
    filename[5] += portrait % 10;
    filename[4] += portrait / 10;

    if (vga_modes == 0) {
        if (portrait == 0 || portrait == 3) {
            filename[3] = 'k';
        }
    }

    load_screen(1, filename);
    throne_room = cut_tile(1, 181, 67, 139, 133);
    sprite_arena_release();
    leader_backdrop(civ);
    free_block(icons_council[0][0], "king");
    sprite_arena_init();
    load_screen(1, filename);

    j = 0;
    for (i = 0; i < 14; i++) {
        if (i == 8 || i == 11)
            j += 2;
        portrait_sprites[i][0] = cut_tile(1, (j % 5) * 60 + 1, (j / 5) * 50 + 1, 59, 20);
        portrait_sprites[i][1] = cut_tile(1, (j % 5) * 60 + 1, (j / 5) * 50 + 21, 59, 29);
        j++;
    }

    blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);

    if (disk_insert(2, 0L) != 0) {
        strcpy(filename, "govt0a.pic");
        filename[4] = civ_gov[civ] / 2 + '0';
        if (civ_gov[civ] == 3)
            filename[4] = '3';
        else if (modern_court)
            filename[5] = 'm';

        load_screen(1, filename);

        for (i = 0; i < 4; i++) {
            for (j = 0; j < 3; j++) {
                icons_council[i][j] = cut_tile(1, j * 29 + 161, i * 25 + 1, 28, 24);
            }
        }

        sprite_arena_release();
        advisors_loaded = 1;
        blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
        disk_insert(0, 1L);
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    } else {
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
        advisors_loaded = 0;
    }

    faces_loaded = 1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Function 8: [21AC, 233C) - Backdrop & Palette Setup (400 bytes) */
void leader_backdrop(int civ)
{
    char pic[14];
    char palfile[14];
    int n;

    if (no_portraits)
        return;

    mouse_off();
    strcpy(pic, "back0a.pic");
    pic[4] = civ_gov[civ] / 2 + '0';
    if (civ_gov[civ] == 3)
        pic[4] = '3';
    else if (modern_court)
        pic[5] = 'm';

    load_screen(1, pic);

    for (n = 0; n < (nation_ratings[civ] + 1) / 2; n++) {
        sprite_draw(back_page, advisor_x[n], 34, icons_council[(n + civ) & 3][0]);
    }

    sprite_draw(back_page, 90, 0, throne_room);
    anim_step = 0;

    strcpy(&pic[7], "pal");
    strcpy(palfile, "king00.pal");
    palfile[5] += portrait % 10;
    palfile[4] += portrait / 10;

    load_palette(palfile, pic_colours_data);
    load_palette(pic, colours_area);

    for (n = 0xc0; n < 0x1b0; n++) {
        colours_area[n + 6] = pic_colours_data[n + 6];
    }

    cycle_off();
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    apply_palette(colours_area);
    blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
    mouse_on();
}

/* Function 9: [233C, 2362) - Portrait memory release helper (38 bytes) */
/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void free_leader(void)
{
    sound(1);
    if (faces_loaded) {
        free_block(portrait_sprites[0][0]);
    }
    faces_loaded = 0;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Function 10: [2362, 251D) - Leader face/eyes/mouth blitter (443 bytes) */
void draw_leader_face(int eyes_frame, int mouth_frame, int portrait_frame)
{
    int p;
    int x;
    int i, y;
    int j;
    int k;
    int face;

    if (no_portraits)
        return;

    mouse_off();
    face = portrait_frame;
    j = (civ_gov[audience_civ] / 2) % 3;
    if (civ_gov[audience_civ] == 3)
        j = 6;
    else if (modern_court)
        j += 3;

    if (face != -1 && face != shown_face && advisors_loaded != 0) {
        for (i = 0; i < (nation_ratings[audience_civ] + 1) / 2; i++) {
            k = (i + audience_civ) & 3;
            x = advisor_x[i];
            y = 0;
            x += portrait_coords[j][k][0] - k * 80;
            x -= portrait_coords[j][k][1] - 160;
            y += portrait_origins[j][k][0] - 100;
            y -= portrait_origins[j][k][1] - k * 25;
            sprite_draw(scrbuf, x, y + 34, icons_council[k][face]);
            shown_face = face;
        }
    }

    p = portrait;
    x = 90;
    y = 0;
    x += portrait_size[p][0] - 180;
    x -= portrait_size[p][1];
    y += portrait_offset[p][0] - 64;
    y -= portrait_offset[p][1];

    if (eyes_frame != -1) {
        sprite_draw(scrbuf, x, y - 2, portrait_sprites[eyes_frame][0]);
    }

    if (mouth_frame != -1) {
        sprite_draw(scrbuf, x, y + 18, portrait_sprites[mouth_frame][1]);
    }

    mouse_on();
}

/* Function 11: [251D, 25AA) - Dialog Choice Prompt (141 bytes) */
int leader_menu(char *text_ptr, int x, int y)
{
    int result;

    mouse_off();
    blit(scrbuf, 0, 128, 320, 72, back_page, 0, 128);
    if (x == 20)
        x = 36;
    mouse_force_on();
    result = show_menu(text_ptr, x, y);
    mouse_restore();
    blit(back_page, 0, 128, 320, 72, scrbuf, 0, 128);
    mouse_on();
    return result;
}

/* Portrait fonts, dimensions and animation origins; original far-data block. */
char far leader_fonts[16] = {
    0, 0, 1, 2, 1, 2, 0, 1, 0, 2, 0, 2, 1, 0, 2, 1
};

int far portrait_size[14][2] = {
    250, 27, 242, 13, 253, 30, 244, 23, 241, 23, 245, 25, 232, 15, 231, 13, 234, 14, 230, 16, 240, 20, 241, 27, 226, 8, 242, 21
};

int far portrait_offset[14][2] = {
    101, 6, 96, 1, 114, 15, 109, 14, 102, 16, 123, 11, 107, 9, 114, 11, 102, 10, 115, 14, 102, 14, 120, 12, 99, 11, 101, 16
};

int far portrait_coords[7][4][2] = {
    41, 169, 104, 167, 189, 174, 277, 167, 44, 175, 121, 178, 203, 177, 283, 176, 36, 170, 119, 170, 195, 171, 275, 170, 37, 172, 114, 172, 196, 166, 275, 168, 29, 166, 119, 167, 195, 174, 275, 173, 36, 170, 113, 174, 202, 169, 272, 170, 35, 176, 111, 168, 193, 168, 282, 174
};

int far portrait_origins[7][4][2] = {
    125, 8, 121, 34, 124, 61, 117, 85, 115, 9, 121, 36, 116, 63, 118, 84, 126, 11, 116, 33, 115, 61, 117, 86, 122, 8, 119, 36, 119, 63, 112, 85, 118, 10, 115, 35, 123, 60, 120, 87, 115, 9, 117, 37, 118, 62, 115, 84, 117, 10, 118, 33, 118, 61, 121, 84
};
