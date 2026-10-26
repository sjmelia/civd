/* Overlay 15: civil war when a capital is captured. */
#include <string.h>
#include "CIV.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "CIVWAR.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "NEWGAME.H"
#include "UNITS.H"
#include "CORE.H"
#include "TURN.H"

extern struct city townbuf[];
extern struct unit armydata[8][128];
extern struct {
    char a, b;
} treaties[8][8];


int civil_war(int old)
{
    int rsz[16];
    int caprg, m, nxt2, l, owner, nxt1, k, new_civ, i, y, city, x, fresh, g, cpt, rc, dist, type;

    new_civ = -1;
    for (i = 1; i < 8; i++)
        if (troops_total[i] == 0 && city_count[i] == 0) {
            new_civ = i;
            break;
        }
    if (new_civ == -1)
        return 0;
    for (i = 0; i < 16; i++)
        rsz[i] = 0;
    caprg = -1;
    for (i = 0; i < 128; i++) {
        if (townbuf[i].status != -1 && townbuf[i].owner == old) {
            rsz[region_key(townbuf[i].x, townbuf[i].y)] += townbuf[i].size;
            if (townbuf[i].buildings & 1)
                if (caprg == -1 || x_cap[old] == townbuf[i].x) {
                    caprg = region_key(townbuf[i].x, townbuf[i].y);
                    x = townbuf[i].x;
                    y = townbuf[i].y;
                    cpt = i;
                }
        }
    }
    if (caprg == -1)
        return 0;
    fresh = new_civ > civ_total + 1;
    players &= ~(1 << new_civ);
    if (init_civ(new_civ) == 0)
        return 0;
    players |= 1 << new_civ;
    if (fresh || (civ_tribe_of[new_civ] & (new_civ != 7)) || civ_tribe_of[new_civ] > 15) {
        civ_tribe_of[new_civ] = rand_scaled(2) ? new_civ : new_civ + 8;
        if (civ_tribe_of[new_civ] >= 8)
            civ_flags_bits |= 1 << new_civ;
    } else {
        civ_tribe_of[new_civ] ^= 8;
    }
    strcpy(leader_names[new_civ], civt[civ_tribe_of[new_civ]].leader);
    strcpy(civ_adjectives[new_civ], civt[civ_tribe_of[new_civ]].adjective);
    strcpy(msgtext, civt[civ_tribe_of[new_civ]].plural);
    if (msgtext[0] == 0) {
        strcpy(msgtext, civ_adjectives[new_civ]);
        strcat(msgtext, "s");
    }
    strcpy(civ_names[new_civ], msgtext);
    msgtext[0] = 0;
    strcpy(name_buf, civ_adjectives[old]);
    strcpy(message_slot_a, civ_adjectives[new_civ]);
    named_msg(old == civnow ? "*SCHISM" : "*ESCHISM");
    message_mode = 5;
    show_msg(msgtext, 80, 80);
    money[new_civ] = money[old] / 2;
    money[old] -= money[new_civ];
    military_table[new_civ] = military_table[old] / 2;
    civ_research_tally[new_civ] = civ_research_tally[old];
    tech_count[new_civ] = tech_count[old];
    civ_gov[new_civ] = civ_gov[old];
    for (i = 0; i < 5; i++)
        advances_known[new_civ][i] = advances_known[old][i];
    last_met_turn[new_civ] = turn_tick - 8;
    treaties[old][new_civ].a |= 1;
    treaties[new_civ][old].a |= 9;
    l = rsz[caprg];
    m = 0;
    for (i = 0; i < 16; i++) {
        if (rsz[i]) {
            if (m > l || i == caprg) {
                if (i != caprg)
                    l += rsz[i];
                rsz[i] = 1;
            } else {
                m += rsz[i];
                rsz[i] = 2;
            }
        }
    }
    if (m * 2 > l && m <= l) {
        for (i = 0; i < 128; i++) {
            if (townbuf[i].status != -1 && townbuf[i].owner == old) {
                k = region_key(townbuf[i].x, townbuf[i].y);
                if (rsz[k] == 2)
                    transfer_city(i, old, new_civ);
            }
        }
    } else {
        m = 0;
        while (9 * m / (ratio_score + 3) < citizens[old]) {
            g = 1;
            for (i = 0; i < 128; i++) {
                if (townbuf[i].status != -1 && townbuf[i].owner == old) {
                    dist = map_dist(townbuf[i].x, x, townbuf[i].y, y);
                    if (dist > g) {
                        g = dist;
                        city = i;
                    }
                }
            }
            m += townbuf[city].size;
            transfer_city(city, old, new_civ);
        }
    }
    citizens[old] -= citizens[new_civ];
    for (i = 0; i < 128; i++) {
        if (armydata[old][i].type == -1)
            continue;
        type = armydata[old][i].type;
        if (map_flags(armydata[old][i].x, armydata[old][i].y) & 1) {
            city = city_at(armydata[old][i].x, armydata[old][i].y);
            type = armydata[old][i].type;
            if (townbuf[armydata[old][i].home_city].owner != townbuf[city].owner) {
                drop_u_stack(old, i);
                occupy(armydata[old][i].x, armydata[old][i].y);
                rc = create_unit(townbuf[city].owner, type, armydata[old][i].x, armydata[old][i].y);
            } else {
                if (townbuf[city].owner == new_civ) {
                    drop_u_stack(old, i);
                    occupy(armydata[old][i].x, armydata[old][i].y);
                    rc = create_unit(new_civ, type, armydata[old][i].x, armydata[old][i].y);
                    armydata[new_civ][rc].status = armydata[old][i].status & 0xfe;
                }
            }
        } else {
            owner = townbuf[armydata[old][i].home_city].owner;
            nxt1 = armydata[old][i].next;
            if (owner == new_civ) {
                drop_u_stack(old, i);
                map_refresh(new_civ, armydata[old][i].x, armydata[old][i].y);
                occupy(armydata[old][i].x, armydata[old][i].y);
                rc = create_unit(new_civ, type, armydata[old][i].x, armydata[old][i].y);
                armydata[new_civ][rc].status = armydata[old][i].status & 0xfe;
                armydata[new_civ][rc].home_city = armydata[old][i].home_city;
                for (;;) {
                    if (nxt1 == -1)
                        break;
                    if (nxt1 == i)
                        break;
                    nxt2 = armydata[old][nxt1].next;
                    type = armydata[old][nxt1].type;
                    drop_u_stack(old, nxt1);
                    map_refresh(new_civ, armydata[old][i].x, armydata[old][i].y);
                    if (type != -1) {
                        occupy(armydata[old][i].x, armydata[old][i].y);
                        rc = create_unit(new_civ, type, armydata[old][i].x, armydata[old][i].y);
                        armydata[new_civ][rc].status = armydata[old][i].status & 0xfe;
                        if (townbuf[armydata[old][nxt1].home_city].owner != owner)
                            armydata[new_civ][rc].home_city = armydata[old][i].home_city;
                    }
                    nxt1 = nxt2;
                }
            } else {
                while (nxt1 != -1 && nxt1 != i) {
                    nxt2 = armydata[old][nxt1].next;
                    if (townbuf[armydata[old][nxt1].home_city].owner != owner)
                        armydata[old][nxt1].home_city = armydata[old][i].home_city;
                    nxt1 = nxt2;
                }
            }
        }
    }
    city = central_city(new_civ);
    townbuf[city].buildings |= 1;
    x_cap[new_civ] = townbuf[city].x;
    if (old == civnow) {
        reveal(civnow, map_x0, map_y0);
    } else {
        townbuf[cpt].owner = -1;
        city = central_city(old);
        townbuf[city].buildings |= 1;
        x_cap[old] = townbuf[city].x;
        townbuf[cpt].owner = old;
    }
    return 1;
}

void transfer_city(int city, int old, int n)
{
    townbuf[city].owner = n;
    map_refresh(n, townbuf[city].x, townbuf[city].y);
    citizens[n] += townbuf[city].size;
}

int central_city(int p)
{
    int min, tot, i, j, best;

    best = -1;
    min = 0x7fff;
    for (i = 0; i < 128; i++) {
        if (townbuf[i].owner != p || townbuf[i].status == -1)
            continue;
        else {
            tot = 0;
            for (j = 0; j < 128; j++) {
                if (townbuf[j].owner != p || townbuf[j].status == -1)
                    continue;
                else
                    tot += map_dist(townbuf[i].x, townbuf[j].x, townbuf[i].y, townbuf[j].y);
            }
            if (tot < min) {
                min = tot;
                best = i;
            }
        }
    }
    return best;
}
