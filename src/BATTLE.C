#include <string.h>
#include <stdlib.h>
#include "BATTLE.H"
#include "CITYMGT.H"
#include "DIPLOMAC.H"
#include "DIPLOMAT.H"
#include "MESSAGE.H"
#include "UNITAI.H"
#include "CAPTURE.H"
#include "TURN.H"

#include "CIV.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "MAP.H"
#include "RESIDENT.H"
#include "UNITS.H"

extern struct unit armydata[][128];
extern struct unit_type utypes[];
extern struct city townbuf[];
extern struct terrain_type terrains[];
extern int treaties[8][8];

extern int terrain_at(int, int), map_flags(int, int), seen_map(int, int), map_note();
extern int map_stack(int, int), wrap_x(int), city_at(int, int), rand_scaled(int);
extern int each_stack();
extern int unit_redraw(int, int);

extern int label_with_style(int, int, int), flip(int), sound();

/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int battle(attacker_civ_b, attacker_unit_m, defender_civ_e, defender_unit_p, flag)
int attacker_civ_b, attacker_unit_m, defender_civ_e, defender_unit_p, flag;
{
    /* Letter suffixes retain MSC allocation buckets. vet and def are reused later. */
    char vet;
    int attacker_wins;
    int stack_losses_b;
    int defender_terrain_o;
    int barbarian_reroll_o;
    int anim_frame;
    int def;
    int step_y_l;
    int strong_unit_d;
    int dir;
    int dx;
    int attack_power_i;
    int defender_type_o;
    int sy;
    int sx;
    int attacker_type_j;
    int city;
    int unused_a_n;
    int unused_b_l;

    attacker_type_j = armydata[attacker_civ_b][attacker_unit_m].type;
    defender_type_o = armydata[defender_civ_e][defender_unit_p].type;
    defender_terrain_o = terrain_at(armydata[defender_civ_e][defender_unit_p].x, armydata[defender_civ_e][defender_unit_p].y);
    barbarian_reroll_o = 0;
    attack_power_i = utypes[attacker_type_j].attack * 8;
    if (map_flags(armydata[defender_civ_e][defender_unit_p].x, armydata[defender_civ_e][defender_unit_p].y) & 0x20)
        def = terrains[defender_terrain_o].defense * utypes[defender_type_o].defense * 8;
    else if (armydata[defender_civ_e][defender_unit_p].status & UNIT_FORTIFIED)
        def = terrains[defender_terrain_o].defense * utypes[defender_type_o].defense * 12 / 2;
    else
        def = terrains[defender_terrain_o].defense * utypes[defender_type_o].defense * 4;
    if (attacker_civ_b == 0) {
        if (defender_civ_e == civnow)
            attack_power_i = attack_power_i * (cur_tier + 1) / 4;
        else
            attack_power_i /= 2;
    }
    if (utypes[defender_type_o].domain == 1 || utypes[defender_type_o].domain == 2)
        def = utypes[defender_type_o].defense * 8;
    city = -1;
    if (map_flags(armydata[defender_civ_e][defender_unit_p].x, armydata[defender_civ_e][defender_unit_p].y) & 1) {
        city = city_at(armydata[defender_civ_e][defender_unit_p].x, armydata[defender_civ_e][defender_unit_p].y);
        if ((townbuf[city].buildings & HAS_CITY_WALLS) && utypes[attacker_type_j].attack != 12 && utypes[defender_type_o].domain != 1)
            def = terrains[defender_terrain_o].defense * utypes[defender_type_o].defense * 12;
        if (attacker_civ_b == 0) {
            if (city_count[defender_civ_e] == 1)
                attack_power_i = 0;
            if (townbuf[city].buildings & HAS_PALACE)
                attack_power_i /= 2;
            barbarian_reroll_o = 1;
        }
    }
    if (armydata[attacker_civ_b][attacker_unit_m].status & UNIT_VETERAN)
        attack_power_i += attack_power_i / 2;
    if (armydata[defender_civ_e][defender_unit_p].status & UNIT_VETERAN)
        def += def / 2;
    vet = armydata[attacker_civ_b][attacker_unit_m].moves;
    if (vet < 3)
        attack_power_i = vet * attack_power_i / 3;
    if (flag == 0)
        return attack_power_i * 8 / (def + 1);
    if (attacker_civ_b != 0) {
        if (cur_tier <= 1 && defender_civ_e == civnow)
            attack_power_i /= 2;
        if (cur_tier == 0 && attacker_civ_b == civnow)
            attack_power_i *= 2;
    }
    if (attacker_civ_b == civnow && attack_ok(defender_civ_e) == -1) {
        armydata[attacker_civ_b][attacker_unit_m].goto_x = -1;
        return -1;
    }
    attacker_wins = 0;
    if (rand_scaled(attack_power_i) > rand_scaled(def))
        attacker_wins = 0x10;
    /* Barbarians must win a second roll when attacking a city. */
    if (barbarian_reroll_o && attacker_wins) {
        if (rand_scaled(attack_power_i) < rand_scaled(def))
            attacker_wins = 0;
    }

    if ((attacker_civ_b == civnow || defender_civ_e == civnow || (armydata[attacker_civ_b][attacker_unit_m].visibility & (1 << civnow))
         || (armydata[defender_civ_e][defender_unit_p].visibility & (1 << civnow)) || xray_on) && still != 1) {
        map_center(civnow, armydata[defender_civ_e][defender_unit_p].x, armydata[defender_civ_e][defender_unit_p].y);
        mouse_off();
        if (seen_map(armydata[attacker_civ_b][attacker_unit_m].x, armydata[attacker_civ_b][attacker_unit_m].y)) {
            if (attacker_wins)
                strong_unit_d = utypes[attacker_type_j].attack > 4 || utypes[attacker_type_j].defense > 2;
            else
                strong_unit_d = utypes[defender_type_o].attack > 4 || utypes[defender_type_o].defense > 2;
            sound(attacker_wins && attacker_type_j == 15 ? 0x2b : (attacker_wins && attacker_civ_b == civnow) || (!attacker_wins && defender_civ_e == civnow)
                  ? (strong_unit_d == 0 ? 0x26 : 0x28) : (strong_unit_d == 0 ? 0x27 : 0x29));
            unit_redraw(defender_civ_e, defender_unit_p);
            vet = armydata[attacker_civ_b][attacker_unit_m].next;
            if (vet != -1)
                unit_redraw(attacker_civ_b, vet);
            blit(scrbuf, 0x50, 0, 0xf0, 0xc8, back_page, 0x50, 0);
            sx = (wrap_x(armydata[attacker_civ_b][attacker_unit_m].x - map_x0) << 4) + 0x50;
            sy = ((armydata[attacker_civ_b][attacker_unit_m].y - map_y0) << 4) + 8;
            dx = map_random(armydata[defender_civ_e][defender_unit_p].x - armydata[attacker_civ_b][attacker_unit_m].x);
            step_y_l = map_random(armydata[defender_civ_e][defender_unit_p].y - armydata[attacker_civ_b][attacker_unit_m].y);
            if ((armydata[defender_civ_e][defender_unit_p].x == 0 && armydata[attacker_civ_b][attacker_unit_m].x == 0x4f)
                || (armydata[defender_civ_e][defender_unit_p].x == 0x4f && armydata[attacker_civ_b][attacker_unit_m].x == 0))
                dx = -dx;
            for (anim_frame = 0; anim_frame <= 10; anim_frame++) {
                label_with_style(armydata[attacker_civ_b][attacker_unit_m].type + (attacker_civ_b << 5) + 0x40, anim_frame * dx + sx + 1, anim_frame * step_y_l + sy + 1);
                flip(2);
                blit(back_page, anim_frame * dx + sx, anim_frame * step_y_l + sy, 0x10, 0x10, scrbuf, anim_frame * dx + sx, anim_frame * step_y_l + sy);
            }
            map_note(armydata[attacker_civ_b][attacker_unit_m].x, armydata[attacker_civ_b][attacker_unit_m].y);
            map_note(armydata[defender_civ_e][defender_unit_p].x, armydata[defender_civ_e][defender_unit_p].y);
            blit(scrbuf, 0, 0, 0x100, 0xc8, back_page, 0, 0);
            for (anim_frame = 0; anim_frame < 8; anim_frame++) {
                label_with_style(anim_frame + 0x20, attacker_wins * dx + sx + 1, attacker_wins * step_y_l + sy + 1);
                flip(4);
                blit(back_page, attacker_wins * dx + sx, attacker_wins * step_y_l + sy, 0x10, 0x10, scrbuf, attacker_wins * dx + sx, attacker_wins * step_y_l + sy);
            }
        }
        mouse_on();
    }
    treaties[attacker_civ_b][defender_civ_e] |= 0x20;
    treaties[defender_civ_e][attacker_civ_b] |= 0x20;
    if (attacker_civ_b && defender_civ_e)
        player_peace_run = 0;
    if ((defender_civ_e == civnow && (treaties[defender_civ_e][attacker_civ_b] & 2)) || (treaties[attacker_civ_b][defender_civ_e] & 0x200)) {
        message_mode = 3;
        strcpy(msgtext, "Sneak attack by\n");
        strcat(msgtext, civ_adjectives[attacker_civ_b]);
        strcat(msgtext, " forces!\n");
        show_msg(msgtext, 0x64, 0x50);
        treaties[attacker_civ_b][defender_civ_e] &= ~0x200;
    }
    if ((treaties[defender_civ_e][attacker_civ_b] & 2) || (treaties[attacker_civ_b][defender_civ_e] & 2)) {
        if (attacker_civ_b == civnow || defender_civ_e == civnow) {
            message_mode = 5;
            strcpy(msgtext, civ_names[defender_civ_e]);
            strcat(msgtext, " cancel\npeace treaty\nwith ");
            strcat(msgtext, civ_names[attacker_civ_b]);
            strcat(msgtext, ".\n");
            show_msg(msgtext, 0x64, 0x50);
        }
        if (attacker_civ_b == civnow && (treaties[defender_civ_e][attacker_civ_b] & 2))
            treaties[defender_civ_e][attacker_civ_b] |= 8;
        accord(attacker_civ_b, defender_civ_e, 2);
    }
    if (attacker_civ_b == civnow)
        treaties[defender_civ_e][attacker_civ_b] &= ~0x200;
    if (attacker_type_j == 0x19) {
        atom(attacker_civ_b, armydata[defender_civ_e][defender_unit_p].x, armydata[defender_civ_e][defender_unit_p].y);
        return 1;
    }
    if (attacker_wins) {
        if (defender_civ_e == 0 && armydata[defender_civ_e][defender_unit_p].type == 0x1a) {
            money[attacker_civ_b] += 100;
            if (attacker_civ_b == civnow) {
                msgtext[0] = 0;
                named_msg("*LEADER");
                show_msg(msgtext, 0x50, 0x50);
            }
        }
        if (city == -1 && !(map_flags(armydata[defender_civ_e][defender_unit_p].x, armydata[defender_civ_e][defender_unit_p].y) & 0x20)) {
            /* Outside cities and fortresses, defeat destroys the whole stack. */
            stack_losses_b = stack_len(defender_civ_e, defender_unit_p, 2);
            killed[attacker_civ_b][defender_civ_e] += stack_losses_b;
            each_stack(defender_civ_e, defender_unit_p, drop_u_stack);
            if (stack_losses_b > 1 && (attacker_civ_b == civnow || defender_civ_e == civnow)) {
                msgtext[0] = 0;
                strcat(msgtext, itoa(stack_losses_b, nstr, 10));
                strcat(msgtext, " units destroyed.\n");
                show_msg(msgtext, 0x50, 0x50);
            }
        } else {
            if (city != -1 && armydata[defender_civ_e][defender_unit_p].next == -1) {
                for (dir = 8; dir >= 0; dir--) {
                    sx = armydata[defender_civ_e][defender_unit_p].x + spiral_x[dir];
                    sy = armydata[defender_civ_e][defender_unit_p].y + spiral_y[dir];
                    def = map_stack(sx, sy);
                    if (def != -1 && the_enemy != civnow)
                        each_stack(the_enemy, def, kill_cb);
                }
                if (townbuf[city].owner == civnow)
                    city_captured(city);
            }
            drop_u_stack(defender_civ_e, defender_unit_p);
            killed[attacker_civ_b][defender_civ_e]++;
        }
        if (rand_scaled(2))
            armydata[attacker_civ_b][attacker_unit_m].status |= UNIT_VETERAN;
        return 1;
    }
    killed[defender_civ_e][attacker_civ_b]++;
    drop_u_stack(attacker_civ_b, attacker_unit_m);
    if (rand_scaled(2))
        armydata[defender_civ_e][defender_unit_p].status |= UNIT_VETERAN;
    return 0;

}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int stack_add(int p, int i, int head)
{
    if (armydata[p][head].next == -1) {
        armydata[p][head].next = i;
        armydata[p][i].next = head;
    } else {
        armydata[p][i].next = armydata[p][head].next;
        armydata[p][head].next = i;
    }
}

int stack_remove(int p, int i)
{
    int c, h;

    h = armydata[p][i].next;
    if (h == -1)
        return;
    if (armydata[p][h].next == i)
        armydata[p][h].next = -1;
    else {
        c = 0;
        do {
            h = armydata[p][h].next;
            c++;
        } while (c < 0x20 && h != -1 && armydata[p][h].next != i);
        if (c < 0x20 && h != -1)
            armydata[p][h].next = armydata[p][i].next;
    }
    armydata[p][i].next = -1;
}

int attack_ok(int pl)
{
    if (treaties[civnow][pl] & 2) {
        strcpy(msgtext, "We have signed a\npeace treaty with\nthe ");
        strcat(msgtext, civ_names[pl]);
        strcat(msgtext, "!\n Cancel action.\n Break treaty.\n");
        message_mode = 5;
        if (show_msg(msgtext, 0x50, 0x50) != 1)
            return -1;
        if (civ_gov[civnow] >= 4) {
            strcpy(msgtext, "Overruled by\nthe Senate.\nAction canceled.\n");
            message_mode = 4;
            show_msg(msgtext, 0x64, 0x50);
            return -1;
        }
        return 1;
    }
    return 0;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int atom(int pl, int x, int y)
{
    int c, nx, ny, k, id, n;

    if (pl != civnow) {
        message_mode = 3;
        strcpy(msgtext, civ_names[pl]);
        strcat(msgtext, " use\nnuclear weapons!\n");
        show_msg(msgtext, 0x64, 0x50);
    }
    nuke_animation(x, y);
    for (k = 8; k >= 0; k--) {
        nx = spiral_x[k] + x;
        ny = spiral_y[k] + y;
        id = map_stack(nx, ny);
        if (map_flags(nx, ny) & 1) {
            c = city_at(nx, ny);
            if (townbuf[c].buildings & HAS_SDI) {
                strcpy(msgtext, "SDI protects ");
                append_place_name(c);
                strcat(msgtext, ".\n");
                show_msg(msgtext, 0x50, 0x64);
                id = -1;
            }
        }
        if (id != -1) {
            n = stack_len(the_enemy, id, 2);
            killed[pl][the_enemy] += n;
            each_stack(the_enemy, id, drop_u_stack);
            if (pl != the_enemy)
                treaties[the_enemy][pl] |= 0x88;
        }
    }
    nuke_fallout(x, y);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int nuke_fallout(int x, int y)
{
    int c, nx, ny, k;

    for (k = 0; k <= 8; k++) {
        nx = wrap_x(spiral_x[k] + x);
        ny = spiral_y[k] + y;
        if (map_flags(nx, ny) & 1) {
            c = city_at(nx, ny);
            townbuf[c].size -= townbuf[c].size / 2;
        } else if (rand_scaled(3) && terrain_at(nx, ny) != 10)
            add_pollution(nx, ny);
        map_note(nx, ny);
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */
