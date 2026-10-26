/* Relations between civilizations, government changes and technology exchanges. */
#include <string.h>
#include <stdlib.h>
#include "AUDIENCE.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "UNITS.H"
#include "CITYMGT.H"
#include "DIPLOMAC.H"
#include "ECONOMY.H"
#include "MESSAGE.H"
#include "NEWGAME.H"
#include "TURN.H"
#include "CORE.H"

struct unit_record {
    char flags, pad1[3];
    unsigned char x, y;
    char type, pad2[4];
    char owner;
    char rest[16];
};

extern int treaties[8][8];
extern struct unit_record townbuf[];


void city_phase(int civ)
{
    int cost, i;
    char flags;

    if (civ_gov[civ] == 0) {
        if (civ == civnow && !(turn_tick & 3)) {
            if (treaties[civ][0] & 4)
                treaties[civ][0] &= ~4;
            else
                set_government(civ, 1);
        } else if (!(turn_tick & 3) || civ_wonder(civ, 1)) {
            set_government(civ, 1);
            if (civ == civnow)
                treaties[civ][0] &= ~4;
        }
    }
    if (civ != civnow && (treaties[civ][0] & 4))
        set_government(civ, civ_gov[civ]);
    for (i = 1; i < 8; i++) {
        if (treaties[civ][i] & 0x100) {
            if (!rand_scaled(3)) {
                treaties[civ][i] &= ~0x100;
                treaties[civ][i] &= ~2;
                treaties[civ][i] |= 0x200;
            }
        }
        if (turn_tick & 0xf)
            continue;
        flags = treaties[civ][i];
        if (!(flags & 0x10)) {
            if ((flags & 1) && !(flags & 2)
                && ((treaties[civnow][civ] & 0x40)
                    || (treaties[civnow][i] & 0x40))
                && civ != civnow && i != civnow) {
                strcpy(msgtext, "The war between\nthe ");
                strcat(msgtext, civ_names[civ]);
                strcat(msgtext, " and the\n");
                strcat(msgtext, civ_names[i]);
                strcat(msgtext, " has ended.\n");
                message_mode = 1;
                show_msg(msgtext, 100, 80);
            }
            treaties[civ][i] &= ~1;
            treaties[i][civ] &= ~1;
        }
        treaties[civ][i] &= ~0x10;
        treaties[i][civ] &= ~0x10;
    }
    if (civ != 0 && ((unsigned char) turn_tick & 0x1f) == civ * 4 && civ != civnow) {
        for (i = 1; i < 8; i++) {
            if ((treaties[civ][i] & 0x20) && (treaties[civnow][i] & 2)
                && !(treaties[civ][civnow] & 0x20)) {
                if (military_table[civ] * 2 < military_table[i]) {
                    if (!audience_open(civ, -1, 0)) {
                        cost = clamp(money[civ] / 100, 1, 10) * 50;
                        strcpy(message_slot_a, civ_names[i]);
                        msgtext[0] = 0;
                        strcat(msgtext, itoa(cost, nstr, 10));
                        strcpy(message_slot_b, msgtext);
                        msgtext[0] = 0;
                        named_msg("*HELPME");
                        style = 1;
                        setting_value = 12;
                        if (leader_menu(msgtext, 20, 139) == 1) {
                            money[civnow] += cost;
                            money[civ] += cost;
                            relation_set(civ, civnow, 2);
                            accord(i, civnow, 2);
                            treaties[i][civnow] |= 8;
                        }
                        audience_close();
                    }
                }
                if (military_table[i] > military_table[civ]
                    && military_table[civ] * 2 >= military_table[i]) {
                    {
                        if (!audience_open(civ, -1, 0)) {
                            msgtext[0] = 0;
                            strcpy(message_slot_a, civ_names[i]);
                            strcpy(message_slot, (treaties[civ][civnow] & 2)
                                   ? "extend our" : "agree to a");
                            named_msg("*CRUSADE");
                            style = 1;
                            setting_value = 12;
                            if (leader_menu(msgtext, 20, 139) == 1) {
                                relation_set(civ, civnow, 2);
                                accord(i, civnow, 2);
                                treaties[i][civnow] |= 8;
                                last_met_turn[civ] = turn_tick + 16;
                            }
                            audience_close();
                        }
                    }
                }
            }
            treaties[civ][i] &= ~0x20;
        }
    }
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void set_government(int civ, int gov)
{
    int og, gl[6], nopt, t, pick;

    og = civ_gov[civ];
    if (human_set & (1 << civ)) {
again:
        strcpy(msgtext, "Select type of\nGovernment...\n ");
        nopt = pick = 0;
        for (t = 1; t < 6; t++) {
            if (!civ_wonder(civ, 1)) {
                if (t == 2 && !has_tech(civ, 5))
                    continue;
                if (t == 3 && !has_tech(civ, 0x2b))
                    continue;
                if (t == 4 && !has_tech(civ, 0x2c))
                    continue;
                if (t == 5 && !has_tech(civ, 4))
                    continue;
            }
            gl[nopt++] = t;
            strcat(msgtext, government_names[t]);
            strcat(msgtext, "\n ");
        }
        if (gov) {
            pick = show_msg(msgtext, 100, 64);
            if (pick < 0)
                goto again;
            civ_gov[civ] = gl[pick];
        } else
            civ_gov[civ] = gov;
        cabinet();
        sel_player();
        new_government();
    } else {
        if ((civ_gov[civ] != gov || (treaties[civ][0] & 4))
            && (treaties[civnow][civ] & 0x40)) {
            msgtext[0] = 0;
            if (treaties[civ][0] & 4) {
                strcat(msgtext, civ_adjectives[civ]);
                strcat(msgtext, " government\nchanged to ");
                strcat(msgtext, government_names[gov]);
                strcat(msgtext, "!\n");
                treaties[civ][0] &= ~4;
            } else {
                strcat(msgtext, "The ");
                strcat(msgtext, civ_adjectives[civ]);
                strcat(msgtext, " government\nhas been overthrown!\n");
                treaties[civ][0] |= 4;
            }
            message_mode = 1;
            show_msg(msgtext, 80, 64);
        }
        civ_gov[civ] = gov;
        civ_toll[civ] = 4 - gov / 2;
    }
    if (og != civ_gov[civ]) {
        post_event(8, 2, civ, civ_gov[civ]);
        for (t = 0; t < 8; t++)
            treaties[civ][t] &= ~8;
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void diplomacy_event(int a, int b, int c, int d)
{
    int unused_c, unused_d, unused_e, old;

    if (!a || !b)
        return;
    if (a == civnow) {
        if (!(treaties[a][b] & 1) || turn_tick - last_met_turn[b] >= 16)
            leader_audience(b, c, d, 0, 0);
    }
    if (b == civnow) {
        if (!(treaties[a][b] & 1) || turn_tick - last_met_turn[a] >= 16)
            leader_audience(a, c, d, 0, 0);
    }
    old = treaties[a][b];
    treaties[a][b] |= 0x11;
    treaties[b][a] |= 0x11;
    if (a == civnow || b == civnow)
        return;
    if (((a + b + turn_tick) & 3) && (old & 1))
        return;
    if (treaties[a][b] & 8)
        return;
    if (treaties[b][a] & 8)
        return;
    if (!((alliance_ok(a, b) && !civ_wonder(b, 7) && civ_gov[a] < 4)
          || (alliance_ok(b, a) && !civ_wonder(a, 7) && civ_gov[b] < 4))) {
        trade_advances(a, b);
        if (!(treaties[a][b] & 2)) {
            if ((treaties[civnow][a] & 0x40)
                || (treaties[civnow][b] & 0x40)) {
                strcpy(msgtext, civ_names[a]);
                strcat(msgtext, " and\n");
                strcat(msgtext, civ_names[b]);
                strcat(msgtext, " sign\na peace treaty.\n");
                message_mode = 1;
                show_msg(msgtext, 100, 80);
            }
            relation_set(a, b, 2);
        }
    } else {
        if (!(treaties[a][b] & 2) && (old & 1))
            return;
        if ((treaties[civnow][a] & 0x40)
            || (treaties[civnow][b] & 0x40)) {
            strcpy(msgtext, civ_names[a]);
            strcat(msgtext, " and\n");
            strcat(msgtext, civ_names[b]);
            strcat(msgtext, (treaties[a][b] & 2) ? " cancel\npeace treaty.\n" : " declare\nwar on each other.\n");
            message_mode = 1;
            show_msg(msgtext, 100, 80);
        }
        accord(a, b, 2);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void relation_set(int a, int b, int flag)
{
    if (flag == 2 && !(treaties[a][b] & flag))
        post_event(3, 1, a * 16 + b);
    treaties[a][b] |= flag;
    treaties[b][a] |= flag;
    if (flag == 2) {
        treaties[a][b] &= ~0x20;
        treaties[b][a] &= ~0x20;
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void accord(int a, int b, int flag)
{
    if (flag == 2 && (treaties[a][b] & flag))
        post_event(2, 1, a * 16 + b);
    treaties[a][b] &= ~flag;
    treaties[b][a] &= ~flag;
    if (flag & 2) {
        treaties[a][b] &= ~4;
        treaties[b][a] &= ~4;
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int alliance_ok(int a, int b)
{
    int wc, j, t, den, x, wv;

    if (!(rule_options & 1))
        return 1;
    if ((treaties[a][civnow] & 3) == 1 && military_table[b] < military_table[civnow]
        && !(treaties[b][civnow] & 4))
        return 0;
    if ((treaties[b][civnow] & 3) == 1 && military_table[a] < military_table[civnow]
        && !(treaties[a][civnow] & 4))
        return 0;
    if (forces_present[a][25] < power_of(b))
        return 0;
    wc = 0;
    for (j = 1; j < 8; j++)
        if (j != a && j != b && (treaties[a][j] & 3) == 1)
            wc++;
    if (treaties[a][b] & 2)
        wc++;
    wc -= civt[civ_tribe_of[a]].aggressive;
    x = t = 0;
    den = 1;
    for (j = 1; j < 15; j++) {
        if (ai_continent_towns[a][j] != 0 && civ_cont_attackers[a][j] / 2 + defenders_island[a][j] < score_of(b, j, a))
            return 0;
        if (civ_cont_attackers[a][j] != 0) {
            wv = defenders_island[b][j];
            if (wv != 0) {
                x += civ_cont_attackers[a][j];
                den += score_of(b, j, a) / 2 + wv;
                t += ai_continent_towns[a][j];
            }
        }
    }
    if (x * 4 / den < wc + 4)
        if (t)
            return 0;
    return 1;
}

int trade_advances(int a, int b)
{
    int p, q, k, flags;

    flags = 0;
    for (k = 0; k < 0x48; k++) {
        if (!has_tech(a, k) && has_tech(b, k)) {
            flags |= 1;
            p = k;
        }
        if (!has_tech(b, k) && has_tech(a, k)) {
            flags |= 2;
            q = k;
        }
    }
    if (flags == 3) {
        give_advance(a, p, b);
        give_advance(b, q, a);
        return 1;
    }
    return 0;
}

int nearest_unit_distance(int civ, int x, int y)
{
    int mind, i, d;

    mind = 16;
    for (i = 0; i < 128; i++) {
        if (townbuf[i].type != -1 && townbuf[i].owner == civ && (townbuf[i].flags & 1)) {
            d = map_dist(townbuf[i].x, x, townbuf[i].y, y);
            if (d < mind)
                mind = d;
        }
    }
    return mind;
}

int score_of(int x, int y, int z)
{
    return civ_cont_attackers[x][y];
}

int value_of(int x, int y)
{
    return defenders_island[x][y];
}

int power_of(int a)
{
    return forces_present[a][25];
}
