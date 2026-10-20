/* Unit movement and command handling for human and computer players. */
#include "CIV.H"
#include "RESIDENT.H"
#include "GAME.H"
#include <string.h>
#include <stdlib.h>
#include "ADVISORS.H"
#include "BATTLE.H"
#include "CAPTURE.H"
#include "CITY.H"
#include "CITYMGT.H"
#include "CONTROL.H"
#include "CORE.H"
#include "DIALOG.H"
#include "DIPLOMAC.H"
#include "DIPLOMAT.H"
#include "ECONOMY.H"
#include "FAME.H"
#include "FILEIO.H"
#include "MAP.H"
#include "MAPVIEW.H"
#include "MENUS.H"
#include "MESSAGE.H"
#include "NAMES.H"
#include "NEWGAME.H"
#include "NEWS.H"
#include "NOTES.H"
#include "PEDIA.H"
#include "REPLAY.H"
#include "REPORTS.H"
#include "SAVE.H"
#include "STATUS.H"
#include "TURN.H"
#include "UNITAI.H"
#include "UNITS.H"
#include "DRAW.H"
#include "PATH.H"
#include "GLOBALS.H"

extern int debug;

extern struct unit armydata[4][128];
extern struct unit_type utypes[];
extern struct city townbuf[];
extern char civ_vis_grid[];

extern int map_flags(int, int), city_at(int, int), adj_grp();
extern int terrain_at(int, int), wrap_x(int);
extern int abs(int);

struct terrain_work_rule {
    int irrigation_change, irrigation_turns, mining_change, mining_turns, road_change, road_turns;
};
struct terrain_control {
    char name[12];
    signed char move_cost;
    char rest[6];
};
extern struct terrain_work_rule work_rules[];
extern struct terrain_control terrains[];
extern int unit_redraw(int, int);
extern void capunit();
struct civ_type {
    char leader[16], plural[16], adjective[16];
    int f0, f1, f2, f3, f4;
};
extern struct civ_type civt[];

extern void spy_rpt();

extern void civ_score(), world_map();

extern void init_mouse(void), set_page(int);
extern int getch(void);
extern int treaties[8][8];
extern int rand_scaled(int), pixel_at();
extern void seen_map(int, int), lift_unit(int, int, int, int);

extern int battle(), map_owner(int, int);

extern void sound();

extern int each_stack();

extern void map_mark(int, int);

extern int map_stack(int, int);
extern void map_note();         /* NOLINT(clang-diagnostic-deprecated-non-prototype) */
extern void palpic();
extern void unit_sight(), flip(int), reveal(int, int, int);
extern void set_mousecur(int, int, int);
extern void unit_remove(), unit_icon();
extern void disk_insert(), select_page(), cycle_off();

extern int in_view(int, int);
#define U armydata[civ][jf]
/* ai_turn is too large for the post-optimizer (C4063); its exact code depends
   on the compiler memory taken by identifiers it references. Keep its labels,
   locals and the globals it names short, and verify after any rename. */
/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void ai_turn(int civ)
{
    int ao, bn, cm, dl, ek, fj, fz, gi, gy, hh, hx, ig, iw, jf, jv, ke, ku, ld, lt, mc, ms, nb, nr, oa, oq, pp, qo, rn, sm, tl, uk, vj, vz, wi, wy, xh, xx, yg;
    civ_id = civ;
    nr = human_set;
    vj = -1;
    xx = 0;
    if (civ == civnow) {
        human_set = 1 << civnow;
        if (turn_tick == 20 || turn_tick == 60)
            notice_a("*HELP1");
        if (turn_tick == 40 || turn_tick == 80)
            notice_a("*HELP2");
    }
    for (jf = 0; jf < 128; jf++) {
        if (U.type != -1 && reload != 1) {
            U.moves = 3 * utypes[U.type].moves;
            if (utypes[U.type].domain == 2) {
                if (civ_wonder(civ, 4) != 0 || civ_wonder(civ, 8) != 0)
                    U.moves += 3;
            }
        }
    }
    ke = lt = wy = xh = 0;
    dl = 127;
    do {
        oa = 1;
        for (jf = 0; jf < 128; jf++) {
            if (disk_swap)
                sel_player();
            if (leave) {
                jf = 128;
                continue;
            }
            if (jf == dl)
                ke = 1;
            if (U.type == -1)
                continue;
            if (U.type >= 28)
                U.type = 1;
            if (wy == 0)
                unit_sight(U.x, U.y, civ, jf, 0L);
            if (U.status & 9)
                continue;
            if (U.moves == 0)
                continue;
            if (U.status & 4) {
                U.status &= ~4;
                U.status |= 8;
                U.moves = 0;
                unit_note(civ, jf);
                if (civ != civnow) {
                    if (map_flags(U.x, U.y) & 1) {
                        hh = city_at(U.x, U.y);
                        if (townbuf[hh].size >= 3)
                            U.home_city = hh;
                    }
                }
                if (map_flags(U.x, U.y) & 1) {
                    hh = city_at(U.x, U.y);
                    if (U.home_city == hh && adj_grp(U.x, U.y) == 0)
                        garrison_in(hh);
                }
                continue;
            }
            if (civ == civnow && !(U.status & 0xc2)) {
                if (!(map_x0 < U.x && map_y0 < U.y && map_x0 + 14 >= U.x && map_y0 + 10 > U.y)) {
                    if (ke == 0) {
                        oa = 0;
                        continue;
                    } else {
                        if (still != 1)
                            flip(30);
                        reveal(civ, U.x - 7, U.y - 6);
                        dl = (jf - 1) & 127;
                        ke = 0;
                        if (turn_tick == 0)
                            uinit(civ, jf);
                    }
                }
            }
rd:                            /* redraw map and units */
            if (human_set & (1 << civ))
                repaint();
            wi = 0;
            if (ig != 0)
                set_mousecur(0, 0, splashhand);
            lt = ig = vz = 0;
            do {
                if (!(human_set & (1 << civ))) {
                    ek = ai_order(civ, jf);
                    if (ek)
                        U.goto_x = -1;
                    if (++wi > 4) {
                        ek = 32;
                        U.goto_x = U.f8 = -1;
                    }
                    goto uc;
                }
                {
                    if (jf < 128 && ((U.status & 0xc2) || U.goto_x != -1)) {
                        ek = 'r';
                        if (U.status & 0x40)
                            ek = U.type != 0 ? 'm' : 'i';
                        if (U.status & 0x80) {
                            ek = 'm';
                            if (U.status & 0x40)
                                ek = 'f';
                            if (U.status & 2)
                                ek = 'p';
                        }
                        if (utypes[U.type].domain == 1)
                            ek = 'h';
                        goto cmd;
                    }
                    uinit(civ, jf);
                    if (turn_tick == 0 && wy == 0) {
                        strcpy(name_buf, civ_names[civ]);
                        notice_a("*FIRSTMOVE");
                        wy = 1;
                    }
                    if (U.type == 0 && wy == 0) {
                        /* Overlay 8 entry at 058E. */
                        advice(civ, jf);
                        wy = 1;
                    }
                    blink_unit(civ, jf, vj, tl);
                    if (jf != 128)
                        xh = 1;
                    if (mse_held == 0) {
                        ek = key_cmd();
                        if (ek == 13 && xx != 0) {
                            oq = vj;
                            mc = tl;
                            ptr_x = 80;
                            ptr_y = 8;
                            mse_held = 1;
                            goto ck;
                        }
                        goto cmd;
                    }
                    oq = wrap_x(map_x0 + (ptr_x - 80) / 16);
                    mc = map_y0 + (ptr_y - 8) / 16;
                    if (ptr_y < 8) {
                        cmd_menu(civ, jf, -1);
                        repaint();
                        if (cmd_code != -1) {
                            ek = cmd_code;
                            if (jf < 128)
                                goto uc;
                            else
                                goto gc;
                        }
                        goto dn;
                    }
                    if (ptr_x < 80) {
                        if (ptr_y < 58) {
                            reveal(civ, wrap_x(ptr_x + view_xorg - 7), clamp(ptr_y + view_yorg - 14, 0, 49));
                        } else if (ptr_y < 72) {
                            mouse_off();
                            palpic(0L);
                            scrbuf->number = 0;
                            cycle_off();
                            rect_fill(scrbuf, 0, 0, 320, 200, 0);
                            load_back("cback.pic");
                            blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
                            mouse_on();
                            ad_wait();
                            disk_insert(0, 1L);
                            sel_player();
                            cycle_on();
                        } else {
                            if (jf == 128)
                                goto fn;
                        }
                        mse_held = 0;
                    }
                    if (mse_held == 2 && jf < 128) {
                        do {
                            poll_mouse();
                        } while (mse_held != 0);
                        oq = wrap_x(map_x0 + (ptr_x - 80) / 16);
                        mc = map_y0 + (ptr_y - 8) / 16;
                        qo = half_dist(oq - U.x, mc - U.y);
                        if (abs(oq - U.x) == 79 && abs(mc - U.y) <= 1)
                            qo = 1;
                        if (qo == 1) {
                            U.goto_x = oq;
                            U.goto_y = mc;
                            U.f8 = -1;
                            mse_held = 0;
                            ig = 0;
                            set_mousecur(0, 0, splashhand);
                        } else
                            mse_held = 2;
                    }
                    if (mse_held == 2) {
                        if ((civ_vis_grid[oq * 50 + mc] & (1 << civnow)) || xray_on) {
                            mouse_off();
                            pedia_show(terrain_at(oq, mc), 3);
                            sel_player();
                            mouse_on();
                        }
                    }
ck:                            /* mouse click */
                    if (mse_held == 1) {
                        if (ig != 0 && jf < 128) {
                            gi = terrain_at(oq, mc);
                            if (ig == 2 || ((civ_vis_grid[oq * 50 + mc] & (1 << civnow)) && ((utypes[U.type].domain == 2 && gi == 10) || (utypes[U.type].domain == 0 && gi != 10) || (map_flags(oq, mc) & 1) || utypes[U.type].domain == 1))) {
                                U.goto_x = oq;
                                U.goto_y = mc;
                                U.f8 = -1;
                            }
                            ig = 0;
                            set_mousecur(0, 0, splashhand);
                            if (xx != 0) {
                                xx = 0;
                                if (civ_vis_grid[vj * 50 + tl] & (1 << civ))
                                    map_note(vj, tl);
                                vj = -1;
                            }
                        } else {
                            if ((map_flags(oq, mc) & 1) && (map_city(oq, mc) == civnow || xray_on)) {
                                cm = map_city(oq, mc);
                                gy = city_at(oq, mc);
                                kill(cm, gy);
                                status_draw();
                                oa = 0;
                                if (xx != 0) {
                                    xx = 0;
                                    if (civ_vis_grid[vj * 50 + tl] & (1 << civ))
                                        map_note(vj, tl);
                                    vj = -1;
                                }
                            } else {
                                ek = map_stack(oq, mc);
                                if (xray_on && ek != -1)
                                    report_unit(fight_enemy, ek);
                                if (ek != -1 && civ == fight_enemy) {
                                    hh = pick_unit(fight_enemy, ek);
                                    if (!(armydata[civ][hh].status & 9)) {
                                        jf = hh;
                                        dl = (hh - 1) & 127;
                                        ke = 0;
                                        oa = 0;
                                        if (xx != 0) {
                                            xx = 0;
                                            if (civ_vis_grid[vj * 50 + tl] & (1 << civ))
                                                map_note(vj, tl);
                                            vj = -1;
                                        }
                                        goto dn;
                                    }
                                } else
                                    reveal(civ, oq - 7, mc - 6);
                            }
                        }
                    }
                    ek = -1;
                    if (vz != 0)
                        vz = 0;
                    else
                        repaint();

                }
cmd:
                if (jf < 128) {
uc:                            /* unit command */
                    if (U.goto_x != -1)
                        ek = unit_goal(civ, jf);
                    else
                        U.f8 = -1;
                    ku = U.x;
                    jv = U.y;
                    switch (ek) {
                    case 'D':
                        U.moves = 0;
                        drop_u_stack(civ, jf);
                        break;
                    case 'P':
                        if ((map_flags(ku, jv) & 1) || U.type >= 26 || U.type == 14) {
                            human_msg("*PILLAGE");
                            break;
                        } else {
                            uk = near_city(ku, jv);
                            if (U.moves != 0 && attack_ok(townbuf[uk].owner) != -1) {
                                hh = map_flags(ku, jv);
                                if (hh & 6)
                                    unflag(6, ku, jv);
                                else if (hh & 16)
                                    unflag(16, ku, jv);
                                else
                                    unflag(8, ku, jv);
                                if (townbuf[uk].owner == civnow)
                                    map_mark(ku, jv);
                                note_map(ku, jv);
                                accord(civnow, townbuf[uk].owner, 2);
                                if (civ != townbuf[uk].owner)
                                    add_target(townbuf[uk].owner, ku, jv, 1, 4);
                                U.moves = 0;
                            }
                        }
                        break;
                    case ' ':
                        U.moves = 0;
                        break;
                    case 'w':
                        lt = 1;
                        ke = 1;
                        break;
                    case 's':
                        U.status |= 1;
                        U.moves = 0;
                        unit_note(civ, jf);
                        break;
                    case 'u':
                        if ((utypes[U.type].capacity != 0 || U.type == 23) && U.next != -1) {
                            each_stack(civ, jf, unit_reset);
                            oa = 0;
                            lt = 1;
                            jf = land_unit(civ, jf) - 1;
                            dl = jf & 127;
                            ke = 0;
                        }
                        break;
                    case 'g':
                        if (has_mouse == 0) {
                            if (ig == 0) {
                                if (xx == 0) {
                                    xx = 1;
                                    vj = U.x;
                                    tl = U.y;
                                }
                            } else {
                                if (xx != 0) {
                                    xx = 0;
                                    if (civ_vis_grid[vj * 50 + tl] & (1 << civ))
                                        map_note(vj, tl);
                                    vj = -1;
                                }
                            }
                        } else
                            set_mousecur(0, 0, statuspics[ig != 0 ? 7 : 2]);
                        ig ^= 1;
                        break;
                    case 'h':
                        uk = near_city(ku, jv);
                        if (town_gap == 0)
                            U.home_city = uk;
                        else {
                            if (utypes[U.type].domain == 1) {
                                ms = near_base(civ, jf);
                                if (ms != 0) {
                                    U.status |= 2;
                                    goto mv;
                                } else {
                                    if (civ == civnow)
                                        U.status &= ~2;
                                    else {
                                        U.moves = 0;
                                        drop_u_stack(civ, jf);
                                    }
                                }
                            }
                        }
                        break;
                    case 'p':
                        if (U.type != 0) {
                            human_msg("*SETTLERS");
                            break;
                        }
                        fz = map_flags(ku, jv);
                        if (!(fz & 64)) {
                            U.status &= 0x7d;
                            U.fuel = 0;
                            map_center(civ, ku, jv);
                            break;
                        }
                        U.status |= 0x82;
                        unit_note(civ, jf);
                        U.moves = 0;
                        U.goto_x = U.f8 = -1;
                        if (++U.fuel < 4)
                            break;
                        unflag(64, ku, jv);
                        map_mark(ku, jv);
                        --polluted;
                        unit_note(civ, jf);
                        U.status &= 0x7d;
                        U.fuel = 0;
                        break;
                    case 'f':
                        if (utypes[U.type].domain == 0) {
                            if (U.type != 0) {
                                U.status |= 4;
                                U.moves = 0;
                                unit_note(civ, jf);
                                if (civ != civnow && (map_flags(ku, jv) & 1))
                                    U.home_city = city_at(ku, jv);
                            } else {
                                if ((map_flags(ku, jv) & 0x21) || has_tech(civ, 45) == 0) {
                                    U.status &= 0x3f;
                                    U.fuel = 0;
                                    map_center(civ, ku, jv);
                                    break;
                                }
                                U.status |= 0xc0;
                                U.moves = 0;
                                unit_note(civ, jf);
                                if (terrains[gi].move_cost + 4 > ++U.fuel)
                                    break;
                                flag_tile(32, ku, jv);
                                unit_sight(ku, jv, civ, jf, 1L);
                                U.status &= 0x3f;
                                U.fuel = 0;
                            }
                        }
                        break;
                    case 'r':
                        if (U.type != 0) {
                            human_msg("*SETTLERS");
                            break;
                        }
                        fz = map_flags(ku, jv);
                        if (((fz & 8) && (has_tech(civ, 49) == 0 || (fz & 1))) || (fz & 16)) {
                            U.status &= ~2;
                            U.fuel = 0;
                            map_center(civ, ku, jv);
                            break;
                        }
                        gi = terrain_at(ku, jv);
                        if (gi == 11 && has_tech(civ, 19) == 0)
                            break;
                        U.status |= 2;
                        U.moves = 0;
                        unit_note(civ, jf);
                        if ((fz & 8 ? 4 : 2) * terrains[gi].move_cost > ++U.fuel)
                            break;
                        flag_tile(fz & 8 ? 16 : 8, ku, jv);
                        unit_sight(ku, jv, civ, jf, 1L);
                        U.status &= ~2;
                        U.fuel = 0;
                        break;
                    case 'i':
                        if (U.type != 0 || !(rule_options & 2) || (map_flags(ku, jv) & 2)) {
                            map_center(civ, ku, jv);
                            if (U.type != 0)
                                human_msg("*SETTLERS");
                            else
                                U.fuel = 0;
                            U.status &= 0xbf;
                            break;
                        }
                        gi = terrain_at(ku, jv);
                        hh = work_rules[gi].irrigation_change;
                        if (hh == -1) {
                            map_center(civ, ku, jv);
                            human_msg("*NOIRR");
                            U.status &= 0xbf;
                            break;
                        }
                        if (hh == -2 && build_ok(ku, jv) == 0) {
                            map_center(civ, ku, jv);
                            human_msg("*NOWATER");
                            U.status &= 0xbf;
                            break;
                        }
                        U.status |= 0x40;
                        unit_note(civ, jf);
                        U.moves = 0;
                        U.goto_x = U.f8 = -1;
                        if (work_rules[gi].irrigation_turns > ++U.fuel)
                            break;
                        if (hh >= 0) {
                            map_claim(ku, jv, hh, 2);
                            civ_vis_grid[ku * 50 + jv] |= 1;
                            unflag(6, ku, jv);
                        } else {
                            unflag(4, ku, jv);
                            flag_tile(2, ku, jv);
                        }
                        unit_note(civ, jf);
                        U.status &= 0xbf;
                        U.fuel = 0;
                        break;
                    case 'm':
                        if (U.type != 0 || (map_flags(ku, jv) & 4)) {
                            map_center(civ, ku, jv);
                            if (U.type != 0)
                                human_msg("*SETTLERS");
                            else
                                U.fuel = 0;
                            U.status &= 0x7f;
                            break;
                        }
                        gi = terrain_at(ku, jv);
                        hh = work_rules[gi].mining_change;
                        if (hh == -1) {
                            map_center(civ, ku, jv);
                            human_msg("*NOMINE");
                            U.status &= 0x7f;
                            break;
                        }
                        U.status |= 0x80;
                        unit_note(civ, jf);
                        U.moves = 0;
                        U.goto_x = U.f8 = -1;
                        if (work_rules[gi].mining_turns > ++U.fuel)
                            break;
                        if (hh >= 0) {
                            if (++U.fuel <= 5)
                                break;
                            map_claim(ku, jv, hh, 2);
                            civ_vis_grid[ku * 50 + jv] |= 1;
                            unflag(6, ku, jv);
                        } else {
                            unflag(2, ku, jv);
                            flag_tile(4, ku, jv);
                        }
                        unit_note(civ, jf);
                        U.status &= 0x7f;
                        U.fuel = 0;
                        break;
                    case 'b':
                        if (U.type != 0) {
                            human_msg("*SETTLERS");
                            break;
                        }
                        if (terrain_at(ku, jv) == 10)
                            break;
                        if (map_flags(ku, jv) & 1) {
                            uk = get_city(ku, jv);
                            if (townbuf[uk].size < 10) {
                                ++townbuf[uk].size;
                                U.moves = 0;
                                drop_u_stack(civ, jf);
                                unit_note(civ, jf);
                            } else
                                human_msg("*ADDCITY");
                            break;
                        }
                        U.moves = 0;
                        hh = city_create(civ, ms, ku, jv, 1);
                        if (hh != -1) {
                            drop_u_stack(civ, jf);
                            unit_note(civ, jf);
                        }
                        break;

                    }
                }
                if (jf < 128 || xx != 0) {
                    switch (ek) {
                    case 'c':
                        if (xx != 0)
                            reveal(civ, vj - 7, tl - 6);
                        else
                            reveal(civ, ku - 7, jv - 6);
                        break;
                    case 8:
                    case 0x4700:
                        ms = 8;
                        goto mv;
                        break;
                    case 1:
                    case 0x4800:
                        ms = 1;
                        goto mv;
                        break;
                    case 2:
                    case 0x4900:
                        ms = 2;
                        goto mv;
                        break;
                    case 7:
                    case 0x4b00:
                        ms = 7;
                        goto mv;
                        break;
                    case 3:
                    case 0x4d00:
                        ms = 3;
                        goto mv;
                        break;
                    case 6:
                    case 0x4f00:
                        ms = 6;
                        goto mv;
                        break;
                    case 5:
                    case 0x5000:
                        ms = 5;
                        goto mv;
                        break;
                    case 4:
                    case 0x5100:
                        ms = 4;
                        goto mv;
                        break;
mv:                            /* move unit */
                        if (xx != 0) {
                            if (civ_vis_grid[vj * 50 + tl] & (1 << civ))
                                map_note(vj, tl);
                            vj = wrap_x(spiral_x[ms] + vj);
                            tl = clamp(spiral_y[ms] + tl, 0, 49);
                            if (in_view(vj, tl) == 0)
                                reveal(civ, vj - 8, tl - 7);
                            break;
                        }
                        if (civ == 0 && U.type != 26) {
                            fz = map_flags(ku, jv);
                            if (!(fz & 1) && (fz & 6) && U.type != 3 && U.type != 7 && townbuf[near_city(ku, jv)].owner == civnow) {
                                unflag(6, ku, jv);
                                if (civ_vis_grid[ku * 50 + jv] & (1 << civnow)) {
                                    map_mark(ku, jv);
                                    map_note(ku, jv);
                                }
                                U.moves = 0;
                                break;
                            }
                        }
                        oq = wrap_x(spiral_x[ms] + ku);
                        mc = clamp(spiral_y[ms] + jv, 0, 49);
                        iw = terrain_at(ku, jv);
                        gi = terrain_at(oq, mc);
                        fz = map_flags(oq, mc);
                        qo = map_stack(oq, mc);
                        if (utypes[U.type].domain == 0) {
                            if (qo == -1 && if_open(civ, ku, jv) != 0 && if_open(civ, oq, mc) != 0 && U.type < 26 && iw != 10) {
                                if (civ == civnow)
                                    sound(37);
                                if (debug & 1)
                                    human_msg("*ZOC");
                                U.goto_x = -1;
                                break;
                            }
                            if (U.type >= 26) {
                                uk = city_at(oq, mc);
                                if (uk != -1) {
                                    if (townbuf[uk].owner != civ && iw == 10)
                                        break;
                                    if (U.type == 26 && civ != townbuf[uk].owner) {
                                        if (townbuf[uk].owner == civnow) {
                                            U.visibility |= 1 << civnow;
                                            map_center(civnow, U.x, U.y);
                                            flip(30);
                                            anim_unit(civ, jf, ms);
                                        }
                                        dip_menu(uk, civ, jf);
                                        break;
                                    }
                                    if (U.type == 27) {
                                        hh = map_dist(oq, townbuf[U.home_city].x, mc, townbuf[U.home_city].y);
                                        if (!(townbuf[uk].owner == civ && townbuf[uk].production >= -24 && hh < 10 && region_key(oq, mc) == region_key(townbuf[U.home_city].x, townbuf[U.home_city].y))) {
                                            strcpy(msgtext, "Will you?\n Keep moving\n Establish trade route\n");
                                            if (townbuf[uk].owner == civ && townbuf[uk].production < -24) {
                                                strcat(msgtext, " Help build WONDER.\n");
                                                if (hh < 10)
                                                    dialog_dim = 2;
                                            }
                                            if (townbuf[uk].owner != civ)
                                                hh = 1;
                                            else
                                                hh = show_msg(msgtext, 100, 80);
                                            if (hh == 1 && rpath(civ, jf, uk) != 0)
                                                break;
                                            if (hh == 2) {
                                                townbuf[uk].shield_box += 10 * utypes[U.type].cost;
                                                drop_u_stack(civ, jf);
                                                break;
                                            }
                                        }
                                    }
                                } else {
                                    if (U.type == 26 && qo != -1 && civ != stk_plr) {
                                        buy_u(stk_plr, qo, civ);
                                        U.goto_x = -1;
                                    }
                                }
                            }
                        }
                        if (gi == 10 && utypes[U.type].domain == 0) {
                            if (civ == stk_plr && qo != -1 && stack_cap(civ, qo) > 0) {
                                U.status |= 1;
                                U.status &= 0xf3;
                                U.moves = 3;
                            } else
                                break;
                        } else {
                            if (qo != -1 && civ != stk_plr) {
                                if (iw == 10 && utypes[U.type].domain == 0) {
                                    human_msg("*AMPHIB");
                                    U.goto_x = -1;
                                    break;
                                }
                                if (gi != 10 && U.type == 22)
                                    break;
                                if (civ != civnow && utypes[U.type].attack == 0)
                                    break;
                                if (U.type == 26)
                                    break;
                                if (utypes[armydata[stk_plr][qo].type].domain == 1 && U.type != 14 && !(fz & 1)) {
                                    human_msg("*FIGHTER");
                                    U.goto_x = -1;
                                    break;
                                }
                                if (U.moves < 3) {
                                    if (civ == civnow) {
                                        strcpy(msgtext, "Attack at\n");
                                        strcat(msgtext, itoa(U.moves, nstr, 10));
                                        strcat(msgtext, "/3 strength?\n Cancel\n Attack\n");
                                        if (show_msg(msgtext, 100, 16) != 1) {
                                            U.goto_x = -1;
                                            break;
                                        }
                                    } else {
                                        if (U.moves < 2)
                                            break;
                                    }
                                }
                                if (stk_plr == civnow)
                                    map_center(civnow, oq, mc);
                                qo = stack_other(stk_plr, qo);
                                if (civ == civnow || stk_plr == civnow)
                                    see_fight_all = 1;
                                hh = battle(civ, jf, stk_plr, qo, 1L);
                                U.goto_x = -1;
                                see_fight_all = 0;
                                if (hh == -1)
                                    break;
                                if (U.type == -1) {
                                    U.moves = 0;
                                    goto dn;
                                } else {
                                    U.moves -= 3;
                                    if (U.moves < 0)
                                        U.moves = 0;
                                    if (U.type == 15)
                                        U.moves = 0;
                                }
                                if (fz & 1) {
                                    uk = city_at(oq, mc);
                                    if (townbuf[uk].owner != civnow)
                                        townbuf[uk].status |= 0x10;
                                    if (U.type != -1 && !(townbuf[uk].buildings & 0x80L) && iw != 10 && (cur_tier != 0 || stk_plr != civnow))
                                        --townbuf[uk].size;
                                    if (townbuf[uk].size == 0) {
                                        vanish(uk, oq, mc);
                                        kill_civ(stk_plr, civ);
                                        if (civ == civnow) {
                                            qo = map_stack(oq, mc);
                                            if (qo != -1)
                                                armydata[stk_plr][qo].visibility |= 1 << civnow;
                                        }
                                    }
                                    if (civ == civnow || stk_plr == civnow || xray_on) {
                                        townbuf[uk].visible_size = townbuf[uk].size;
                                        map_note(oq, mc);
                                    }
                                    if (map_owner(oq, mc) == -1 && stk_plr != civnow)
                                        retarget(stk_plr, region_key(oq, mc));
                                    break;
                                }
                                if (fz & 32)
                                    break;
                                if (U.type == -1)
                                    break;
                                break;
                            } else {
                            }
                        }
                        if (gi != 10 && !(fz & 1) && utypes[U.type].domain == 2) {
                            U.goto_x = -1;
                            break;
                        }
                        if (utypes[U.type].domain != 0 && (fz & 1) && map_city(oq, mc) != civ) {
                            if (U.type != 25) {
                                human_msg("*OCCUPY");
                                U.goto_x = -1;
                                break;
                            } else {
                                drop_u_stack(civ, jf);
                                atom(civ, oq, mc);
                                break;
                            }
                        }
                        if (U.moves == 0)
                            break;
                        if (utypes[U.type].domain == 0) {
                            if ((map_flags(ku, jv) & fz & 8) && U.moves != 0) {
                                U.moves -= ((map_flags(ku, jv) & 16) && civ == civnow && U.goto_x == -1) ? 0 : 1;
                            } else {
                                if (U.moves != 3 && rand_scaled(3 * terrains[gi].move_cost) > U.moves) {
                                    U.moves = 0;
                                    break;
                                }
                                U.moves -= 3 * terrains[gi].move_cost;
                            }
                        } else
                            U.moves -= 3;
                        if (U.moves < 0)
                            U.moves = 0;
                        if (civ != civnow) {
                            if (U.f8 != -1 && (U.f8 ^ 4) == ms && stack_len(civ, jf, 2) <= 2) {
                                U.goto_x = U.f8 = -1;
                                U.moves = 0;
                                break;
                            } else
                                U.f8 = ms;
                        }
                        if (qo != -1 && civ == stk_plr && civ != civnow && !(fz & 1) && (utypes[U.type].domain == 0 || (utypes[U.type].domain == 1 && U.fuel != 0))) {
                            hh = stack_len(civ, qo, 2);
                            if ((U.moves != 0 ? 4 : 2) <= hh)
                                break;
                        }
                        yg = pixel_at(2, oq + 80, mc);
                        if (yg != 0 && yg < 8) {
                            if (gi != 10 && iw != 10 && U.type < 26 && (fz & 15) && yg != civ && (civ == civnow || yg == civnow) && utypes[U.type].domain == 0 && map_owner(oq, mc) != civ) {
                                if (civ == civnow) {
                                    U.goto_x = -1;
                                    if (attack_ok(yg) == -1)
                                        break;
                                    accord(civ, yg, 2);
                                } else {
                                    if (yg == civnow && (treaties[yg][civ] & 2)) {
                                        U.moves = 0;
                                        break;
                                    }
                                }
                            }
                            if (civ != yg && (map_flags(oq, mc) & 14))
                                U.visibility |= 1 << yg;
                        }
                        cm = map_city(oq, mc);
                        if (civ == 0 && gi != 10 && ((1 << civnow) & civ_vis_grid[ku * 50 + jv]) && !((1 << civnow) & U.visibility) && townbuf[near_city(ku, jv)].owner == civnow)
                            U.visibility |= civ_vis_grid[ku * 50 + jv];
                        if ((fz & 1) && cm != civ && (civ_vis_grid[oq * 50 + mc] & (1 << civnow)) && ((treaties[civnow][civ] & 64) || (treaties[civnow][cm] & 64)))
                            U.visibility |= 1 << civnow;
                        if (gi == 10 && (map_flags(oq, mc) & 2))
                            U.visibility |= 1 << cm;
                        if ((U.visibility & (1 << civnow)) || civ == civnow || (xray_on && civ != 0)) {
                            if ((debug & 32) || civ == civnow) {
                                map_center(civnow, oq, mc);
                                anim_unit(civ, jf, ms);
                            } else
                                seen_map(ku, jv);
                        }
                        sm = U.next;
                        lift_unit(civ, jf, ku, jv);
                        nb = ku;
                        ld = jv;
                        put_unit(civ, jf, oq, mc);
                        U.x = oq;
                        U.y = mc;
                        U.visibility = 0;
                        if (civ != civnow && utypes[U.type].domain != 1)
                            U.status &= 0x30;
                        if (yg != 0 && yg < 8 && yg != civ)
                            map_claim(oq + 80, mc, 0, 2);
                        if ((unsigned char) U.goto_x == oq && (unsigned char) U.goto_y == mc) {
                            U.goto_x = U.f8 = -1;
                            if (civ != civnow)
                                U.moves = 0;
                        }
                        if (iw == 10 && gi != 10 && utypes[U.type].domain != 1)
                            U.moves = 0;
                        if ((utypes[U.type].capacity != 0 || U.type == 23) && sm != -1) {
                            gy = bn = 0;
                            do {
                                ao = armydata[civ][sm].next;
                                if ((utypes[U.type].capacity == 0) == utypes[armydata[civ][sm].type].domain && ((armydata[civ][sm].status & 1) || iw == 10 || (civ != civnow && !(armydata[civ][sm].status & 12) && armydata[civ][sm].next != -1))) {
                                    lift_unit(civ, sm, nb, ld);
                                    put_unit(civ, sm, oq, mc);
                                    armydata[civ][sm].x = oq;
                                    armydata[civ][sm].y = mc;
                                    armydata[civ][sm].visibility = 0;
                                    armydata[civ][sm].f8 = -1;
                                    if (utypes[U.type].capacity != 0)
                                        armydata[civ][sm].status |= 1;
                                    if (utypes[armydata[civ][sm].type].fuel != 0)
                                        armydata[civ][sm].fuel = utypes[armydata[civ][sm].type].fuel - 1;
                                    ++gy;
                                }
                                sm = ao;
                            } while (++bn < 20 && sm != -1 && (utypes[U.type].capacity > gy || U.type == 23 || civ == 0));

                        }
                        if ((fz & 1) && cm != civ) {
                            U.visibility |= 1 << cm;
                            uk = city_at(oq, mc);
                            capunit(civ, uk, 0L);
                            occupy(oq, mc);
                        }
                        map_refresh(civ, oq, mc);
                        unit_sight(oq, mc, civ, jf, 1L);
                        if (has_hut(gi, oq, mc) != 0) {
                            enter_hut(civ, jf);
                            civ_vis_grid[oq * 50 + mc] |= 1;
                        }
                        if (gi == 10 && utypes[U.type].domain != 2)
                            each_stack(civ, jf, reset_state);
                        if (civ == 0 && gi != 10 && ((1 << civnow) & civ_vis_grid[oq * 50 + mc]) && !((1 << civnow) & U.visibility) && townbuf[near_city(oq, mc)].owner == civnow)
                            U.visibility |= civ_vis_grid[oq * 50 + mc];
                        if ((U.visibility & (1 << civnow)) || civ == civnow || (xray_on && civ != 0)) {
                            if (civ == civnow || (debug & 32)) {
                                map_center(civnow, oq, mc);
                                unit_redraw(civ, jf);
                                if (civ != civnow && still != 1)
                                    flip(30);
                            } else {
                                if (unit_redraw(civ, jf) != 0)
                                    flip(10);
                            }
                        }
                        wi = 0;
                        if ((fz & 1) && utypes[U.type].fuel != 0) {
                            U.status &= ~2;
                            U.moves = 0;
                            U.goto_x = -1;
                        }
                        if (utypes[U.type].fuel != 0 && utypes[U.type].domain == 1 && count_u(civ, jf, 23) != 0) {
                            U.status &= ~2;
                            U.moves = 0;
                            U.fuel = utypes[U.type].fuel;
                        }
                    }
                }
gc:                            /* global command */
                switch (ek) {
                case 0x3b00:
                    if (xray_on)
                        view_army();
                    else
                        city_rpt(civnow);
                    break;
                case 0x3c00:
                    mil_rpt(civnow);
                    break;
                case 0x3d00:
                    if (xray_on) {
                        mouse_off();
                        set_page(2);
                        getch();
                        set_page(0);
                        mouse_on();
                    } else
                        spy_rpt(civnow);
                    break;
                case 0x3e00:
                    mood_rpt(civnow);
                    break;
                case 0x3f00:
                    trade_rpt(civnow);
                    break;
                case 0x4000:
                    sci_rpt(civnow);
                    break;
                case 0x4100:
                    if (xray_on) {
                        for (gy = 1; gy < 8; gy++)
                            civ_info(gy);
                    } else
                        wonders();
                    break;
                case 0x4200:
                    if (xray_on) {
                        power_graph();
                        replay();
                    } else
                        top_five();
                    break;
                case 0x4300:
                    if (xray_on)
                        cont_table();
                    else
                        civ_score(civnow, 1L);
                    break;
                case 0x4400:
                    world_map(1L);
                    break;
                case 0x4838:
                    ms = 1;
                    goto sc;
                case 0x4939:
                    ms = 2;
                    goto sc;
                case 0x4d36:
                    ms = 3;
                    goto sc;
                case 0x5133:
                    ms = 4;
                    goto sc;
                case 0x5032:
                    ms = 5;
                    goto sc;
                case 0x4f31:
                    ms = 6;
                    goto sc;
                case 0x4b34:
                    ms = 7;
                    goto sc;
                case 0x4737:
                    ms = 8;
                    goto sc;
sc:                            /* scroll map */
                    map_x0 += spiral_x[ms] * 4;
                    map_y0 += spiral_y[ms] * 4;
                    reveal(civnow, map_x0, map_y0);
                    break;
                case 13:
                case 32:
                    if (jf == 128)
                        goto fn;
                    break;
                case '/':
                case '?':
                    goto_city();
                    break;
                case '%':
                    if (key_cmd() == '^')
                        xray_on ^= 1;
                    break;
                case 'S':
                    if (turn_tick != 0)
                        save_game(-1);
                    break;
                case -2:
                    msgtext[0] = 0;
                    named_msg("*REV");
                    if (show_msg(msgtext, 64, 80) != 1)
                        break;
                    civ_gov[civ] = 0;
                    strcpy(msgtext, "The ");
                    strcat(msgtext, civ_names[civ]);
                    strcat(msgtext, " are\nrevolting! Citizens\ndemand new govt.\n");
                    news_box(-1);
                    cabinet();
                    sel_player();
                    break;
                case 0x3200:
                    init_mouse();
                    break;
                case '+':
                case '=':
                    strcpy(msgtext, "Select new Tax rate:\n ");
                    for (hh = 0; hh <= civ_toll[civ] + sci[civ]; hh++) {
                        strcat(msgtext, itoa(10 * hh, nstr, 10));
                        strcat(msgtext, "% Tax, (");
                        strcat(msgtext, itoa(10 * (civ_toll[civ] + sci[civ] - hh), nstr, 10));
                        strcat(msgtext, "% Science)\n ");
                    }
                    editing = civ_toll[civ];
                    hh = show_msg(msgtext, 100, 80);
                    if (hh != -1) {
                        sci[civ] += civ_toll[civ] - hh;
                        civ_toll[civ] = hh;
                        status_draw();
                    }
                    break;
                case '-':
                case '_':
                    strcpy(msgtext, "Select new Luxuries rate:\n ");
                    for (hh = 0; hh <= 10 - civ_toll[civ]; hh++) {
                        strcat(msgtext, itoa(10 * hh, nstr, 10));
                        strcat(msgtext, "% Luxuries, (");
                        strcat(msgtext, itoa(10 * (-(civ_toll[civ] + hh - 10)), nstr, 10));
                        strcat(msgtext, "% Science)\n ");
                    }
                    editing = -(civ_toll[civ] + sci[civ] - 10);
                    hh = show_msg(msgtext, 100, 80);
                    if (hh != -1) {
                        sci[civ] = -(civ_toll[civ] + hh - 10);
                        status_draw();
                    }
                    break;
                case 't':
                    bare_flag = 1;
                    reveal(civ, map_x0, map_y0);
                    ad_wait();
                    bare_flag = 0;
                    reveal(civ, map_x0, map_y0);
                    break;
                case 0x1300:
                    for (hh = 0; hh < 16; hh++) {
                        civt[hh].f0 = rand_scaled(3) - 1;
                        civt[hh].f1 = rand_scaled(3) - 1;
                        civt[hh].f2 = rand_scaled(3) - 1;
                    }
                    show_msg("Leaders randomized.\n", 100, 80);
                    break;
                case 0x2f00:
                    debug ^= 16;
                    strcpy(msgtext, "Sounds ");
                    strcat(msgtext, debug & 16 ? "ON\n" : "OFF\n");
                    show_msg(msgtext, 100, 80);
                    break;
                case 0x2300:
                    notice_a("*HELP1");
                    notice_a("*HELP2");
                    break;
                case 0x1000:
                    strcpy(msgtext, "Are you sure you\nwant to Quit?\n Keep Playing\n Yes, Quit\n");
                    if (show_msg(msgtext, 100, 80) != 1) {
                        leave = 0;
                        break;
                    }
                    if (leave == 0)
                        leave = 1;
                    jf = 128;
                    break;
                case 27:
                    if (has_mouse == 0 && xx != 0) {
                        xx = 0;
                        if (civ_vis_grid[vj * 50 + tl] & (1 << civ))
                            map_note(vj, tl);
                        vj = -1;
                    }
                    break;
                case 9:
                    if (has_mouse == 0) {
                        if (xx != 0) {
                            if (civ_vis_grid[vj * 50 + tl] & (1 << civ))
                                map_note(vj, tl);
                            vj = -1;
                        } else {
                            if (jf != 128 && in_view(U.x, U.y) != 0) {
                                vj = U.x;
                                tl = U.y;
                            } else {
                                vj = map_x0 + 7;
                                tl = map_y0 + 6;
                            }
                            if (civ_vis_grid[vj * 50 + tl] & (1 << civ))
                                map_note(vj, tl);
                        }
                        xx ^= 1;
                    }
                    break;
                case 0x2200:
                    cmd_menu(civ, jf, 0);
                    goto mn;
                case 0x1800:
                    cmd_menu(civ, jf, 1);
                    goto mn;
                case 0x1e00:
                    cmd_menu(civ, jf, 2);
                    goto mn;
                case 0x1100:
                    cmd_menu(civ, jf, 3);
                    goto mn;
                case 0x2e00:
                    cmd_menu(civ, jf, 4);
                    goto mn;
mn:                            /* menu bar */
                    if (cmd_code != -1) {
                        ek = cmd_code;
                        goto uc;
                    }
                    break;
                }

dn:            ;               /* unit finished */
            } while (jf < 128 && U.moves > 0 && lt == 0 && leave == 0);
            if (jf < 128) {
                if (U.type == -1)
                    continue;
                if (U.moves > 0)
                    oa = 0;
                else {
                    if (utypes[U.type].fuel != 0) {
                        if ((map_flags(U.x, U.y) & 1) || count_u(civ, jf, 23) != 0)
                            U.fuel = utypes[U.type].fuel;
                        if (--U.fuel < 0) {
                            drop_u_stack(civ, jf);
                            human_msg("*FUEL");
                        }
                    }
                    if (U.type == 16 && civ == civnow && rand_scaled(2) != 0) {
                        hx = 0;
                        for (gy = 1; gy <= 8; gy++) {
                            if (terrain_at(U.x + spiral_x[gy], U.y + spiral_y[gy]) != 10) {
                                hx = 1;
                                break;
                            }
                        }
                        if (hx == 0) {
                            drop_u_stack(civ, jf);
                            human_msg("*TRIREME");
                        }
                    }
                }
            }

            if (human_set & (1 << civ))
                uinit(civ, jf);
        }
    } while (oa == 0 && leave == 0);
    if (leave == 0 && (human_set & (1 << civ)) && (xh == 0 || (debug & 4))) {
        jf = 128;
        oa = 0;
        wy = 1;
        goto rd;
    }
fn:                            /* end of turn */
    if (civ == civnow) {
        mouse_off();
        panel_prep(0, 97, 80, 103);
        mouse_on();
    }
    return;

}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

#undef U

extern struct unit_type utypes[];

extern char civ_vis_grid[];
extern struct unit armydata[4][128];

extern int map_flags(int, int), terrain_at(int, int);
extern void map_note(int, int);

void note_map(int x, int y)
{
    if ((civ_vis_grid[x * 50 + y] & (1 << civnow)) || xray_on)
        map_note(x, y);
}

void unit_note(int player, int unit)
{
    if (player == civnow || (armydata[player][unit].visibility & (1 << civnow)))
        map_note(armydata[player][unit].x, armydata[player][unit].y);
}

int work_tile(int x, int y)
{
    /* Five unused words retain the original twelve-byte local frame. */
    int terrain19, spare0, spare1, spare2, spare3, spare4;
    if (map_flags(x, y) & 7)
        return 0;
    terrain19 = terrain_at(x, y);
    if (work_rules[terrain19].mining_change <= -3)
        return 2;
    if (work_rules[terrain19].irrigation_change == -2 && build_ok(x, y))
        return 1;
    return 0;
}

int build_ok(int x, int y)
{
    int nx_k, ny_k, terrain_n, direction_c, result_f, flags_i;
    if (terrain_at(x, y) == 11)
        return 1;
    result_f = 0;
    for (direction_c = 1; direction_c <= 8; direction_c += 2) {
        nx_k = spiral_x[direction_c] + x;
        ny_k = spiral_y[direction_c] + y;
        terrain_n = terrain_at(nx_k, ny_k);
        flags_i = map_flags(nx_k, ny_k);
        if (flags_i & 1)
            continue;
        if (terrain_n == 11 || terrain_n == 10)
            return 1;
        if (flags_i & 2)
            return 1;
    }
    return result_f;
}

#include <string.h>
#include <stdlib.h>

extern struct terrain_control terrains[];
extern struct city townbuf[];

extern int city_at(int, int);

void uinit(int player, int unit)
{
    int city_h, next_c, index_k, x29, y29, fraction_p, moves_m, flags_k;
    char cargo_m;
    mouse_off();
    panel_prep(0, 97, 80, 103);
    if (unit == 128) {
        draw_string("End of Turn", 4, 124, 0);
        draw_string("Press Enter", 4, 136, 0);
        draw_string("to continue", 4, 144, 0);
    } else {
        y29 = 99;
        strcpy(msgtext, civ_adjectives[player]);
        draw_string(msgtext, 4, 99, 0);
        y29 += 8;
        strcpy(msgtext, utypes[armydata[player][unit].type].name);
        draw_string(msgtext, 4, y29, 0);
        y29 += 8;
        if (armydata[player][unit].status & 32) {
            draw_string("Veteran", 8, y29, 0);
            y29 += 8;
        }
        moves_m = clamp(armydata[player][unit].moves / 3, 0, 99);
        strcpy(msgtext, "Moves: ");
        strcat(msgtext, itoa(moves_m, nstr, 10));
        fraction_p = armydata[player][unit].moves % 3;
        if (fraction_p) {
            strcat(msgtext, ".");
            strcat(msgtext, itoa(fraction_p, nstr, 10));
        }
        if (utypes[armydata[player][unit].type].fuel != 0) {
            strcat(msgtext, "(");
            moves_m += armydata[player][unit].fuel * utypes[armydata[player][unit].type].moves;
            strcat(msgtext, itoa(moves_m, nstr, 10));
            strcat(msgtext, ")");
        }
        draw_string(msgtext, 4, y29, 0);
        y29 += 8;
        msgtext[0] = 0;
        append_place_name(armydata[player][unit].home_city);
        draw_string(msgtext, 4, y29, 0);
        y29 += 8;
        strcpy(msgtext, "(");
        strcat(msgtext, terrains[terrain_at(armydata[player][unit].x, armydata[player][unit].y)].name);
        strcat(msgtext, ")");
        draw_string(msgtext, 4, y29, 0);
        y29 += 8;
        flags_k = map_flags(armydata[player][unit].x, armydata[player][unit].y);
        if (flags_k & 16) {
            draw_string("(RailRoad)", 4, y29, 0);
            y29 += 8;
        } else if (flags_k & 8) {
            draw_string("(Road)", 4, y29, 0);
            y29 += 8;
        }
        if (flags_k & 2) {
            draw_string("(Irrigation)", 4, y29, 0);
            y29 += 8;
        } else if (flags_k & 4) {
            draw_string("(Mining)", 4, y29, 0);
            y29 += 8;
        }
        if (flags_k & 64) {
            draw_string("(Pollution)", 4, y29, 0);
            y29 += 8;
        }
        y29 += 4;
        next_c = armydata[player][unit].next;
        x29 = 8;
        if (map_flags(armydata[player][unit].x, armydata[player][unit].y) & 1) {
            city_h = city_at(armydata[player][unit].x, armydata[player][unit].y);
            for (index_k = 0; index_k < 2; index_k++) {
                cargo_m = townbuf[city_h].garrison[index_k];
                if (cargo_m != -1) {
                    /* Reserved slot: preserve status/goto fields and partial reset. */
                    armydata[player][127].type = cargo_m & 63;
                    armydata[player][127].status = 8;
                    armydata[player][127].goto_x = -1;
                    draw_unit(player, 127, x29, y29);
                    armydata[player][127].type = -1;
                    x29 += 16;
                }
            }
        }
        while (next_c != -1) {
            if (next_c == unit)
                break;
            if (y29 >= 184)
                break;
            draw_unit(player, next_c, x29, y29);
            x29 += 16;
            if (x29 > 64) {
                x29 = 8;
                y29 += 16;
            }
            next_c = armydata[player][next_c].next;
        }
        if (next_c != -1 && next_c != unit)
            draw_string("+", 74, 192, 0);
    }
    mouse_on();
}

/* Original final control helpers; human_notice crosses physical F0866. */

extern struct unit armydata[4][128];
extern struct unit_type utypes[];
extern int kbhit(void);

int in_view(int x, int y)
{
    if (x < 16 && map_x0 >= 65)
        x += 80;
    if (x < map_x0 || x >= map_x0 + 15 || y < map_y0 || y >= map_y0 + 12)
        return 0;
    return 1;
}

void repaint(void)
{
    while (kbhit())
        key_cmd();
    do {
        poll_mouse();
    } while (mse_held != 0);
}

int land_unit(int player, int unit)
{
    int best_c, distance_g, next_d, limit_e, count_l;
    next_d = armydata[player][unit].next;
    if (next_d == -1)
        return unit;
    count_l = 0;
    best_c = unit;
    limit_e = 999;
    do {
        if (utypes[armydata[player][next_d].type].domain != 2) {
            distance_g = next_d > unit ? next_d - unit : next_d - unit + 128;
            if (distance_g < limit_e) {
                limit_e = distance_g;
                best_c = next_d;
            }
        }
        next_d = armydata[player][next_d].next;
    } while (next_d != unit && ++count_l < 16);
    return best_c;
}

void human_msg(char *message)
{
    if (civ_id == civnow)
        notice_b(message);
}
