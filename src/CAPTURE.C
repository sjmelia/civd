/* City capture, unrest, technology theft, place names and caravan trade. */
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "AUDIENCE.H"
#include "CITYVIEW.H"
#include "CIVWAR.H"
#include "CONTROL.H"
#include "CORE.H"
#include "ECONOMY.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "MAP.H"
#include "NEWGAME.H"
#include "NEWS.H"
#include "RESIDENT.H"
#include "SPACESHP.H"
#include "TURN.H"
#include "UNITAI.H"
#include "UNITS.H"
#include "CAPTURE.H"
#include "PATH.H"
#include "CITYMGT.H"

int incited = 0;                /* city taken by a diplomat: capture, never destroy */

/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void capunit(a, b, c)
int a, b, c;
{
    int old, n, l, sy, sx, y, x, val, r;

    old = townbuf[b].owner;
    x = townbuf[b].x;
    y = townbuf[b].y;
    if (a == civnow) {
        last_met_turn[old] = -2;
        old = old;
    }
    if (old != civnow) {
        if (((townbuf[b].buildings & HAS_PALACE) || townbuf[b].production == -1) && c == 0) {
            if ((townbuf[b].buildings & HAS_PALACE) && city_count[old] > 4 && nation_ratings[civnow] < nation_ratings[old]) {
                if (civil_war(old) == 0) {
                    townbuf[b].owner = a;
                    r = central_city(old);
                    townbuf[r].production = -1;
                    townbuf[b].owner = old;
                }
            }
        }
    }
    if (townbuf[b].buildings & HAS_PALACE) {
        strcpy(msgtext, civ_adjectives[old]);
        strcat(msgtext, " spaceship\n");
        if (game_flags & (1 << (old + 8))) {
            strcat(msgtext, (game_flags & (1 << old)) ? "returns to Earth.\n" : "construction cancelled.\n");
            show_msg(msgtext, 100, 80);
        }
        game_flags &= ~(1 << old);
        game_flags &= ~(1 << (old + 8));
        reset_spaceship(old);
    }
    if (old == civnow)
        last_met_turn[a] = -1;
    val = clamp(unrest_chance(b), 0, money[old]);
    if (a == 0 && utypes[armydata[a][map_stack(x, y)].type].attack <= 2 && incited == 0) {
        vanish(b, x, y);
        townbuf[b].size = 0;
    } else {
        townbuf[b].shield_box = 0;
        townbuf[b].status &= CITY_CAPTURE_MASK;
        townbuf[b].buildings &= ~(HAS_PALACE | HAS_TEMPLE | HAS_COURTHOUSE | HAS_CATHEDRAL);
        if (c == 0)
            townbuf[b].buildings &= 0xaaaau >> rand_scaled(2);
        if (c == 0 || townbuf[b].size > 1)
            if (--townbuf[b].size == 0)
                vanish(b, x, y);
    }
    money[old] -= val;
    if (a)
        money[a] += val;
    if (townbuf[b].size != 0) {
        post_event(9, 4, a, townbuf[b].name_id, x, y);
        clear_sites_near(a, 0, x, y, 4);
        townbuf[b].status &= CITY_TRANSFER_MASK;
        n = 0;
        do {
            sx = wrap_x(spiral_x[n] + x);
            sy = spiral_y[n] + y;
            map_claim(sx + 80, sy, a, 2);
            ++n;
        } while (n < 20);
    }
    townbuf[b].owner = a;
    if (a == civnow || old == civnow) {
        map_mark(x, y);
        if (old == civnow)
            map_center(civnow, x, y);
        seen_map(x, y);
        strcpy(msgtext, civ_names[a]);
        strcat(msgtext, " capture\n");
        append_place_name(b);
        strcat(msgtext, ". ");
        strcat(msgtext, itoa(val, nstr, 10));
        strcat(msgtext, " gold\npieces plundered.\n");
        sound(civt[civ_tribe_of[a]].motif);
        if (debug & 8) {
            mouse_off();
            city_scene(b, -2);
            mouse_on();
            disk_insert(1, 1L);
            invasion_scene(a);
            disk_insert(0, 1L);
            sel_player();
        } else
            news_box(b);
        sound(1);
        townbuf[b].visible_size = (a == civnow) ? 0 : townbuf[b].size;
    } else if ((treaty_low(civnow, a) & 0x40)
               || (treaty_low(civnow, old) & 0x40)) {
        message_mode = 0;
        strcpy(msgtext, "Spies report:\n");
        strcat(msgtext, civ_names[a]);
        strcat(msgtext, townbuf[b].size ? " capture\nthe " : " destroy\nthe ");
        strcat(msgtext, civ_adjectives[old]);
        strcat(msgtext, " city\nof ");
        append_place_name(b);
        strcat(msgtext, ".\n");
        show_msg(msgtext, 100, 80);
    }
    steal_advance(a, old);
    map_refresh(a, x, y);
    n = 0;
    do {
        if (armydata[old][n].type != -1 && armydata[old][n].home_city == b && (armydata[old][n].x != townbuf[b].x || armydata[old][n].y != townbuf[b].y || townbuf[b].size == 0))
            drop_u_stack(old, n);
        ++n;
    } while (n < 128);
    if (a == civnow && civ_gov[civnow] < 4)
        townbuf[b].status |= CITY_DISORDER;
    if (townbuf[b].size != 0) {
        if (townbuf[b].production >= 0)
            --civ_units_ordered[old][townbuf[b].production];
        if (a == civnow) {
            city_process(b, 1L);
            sel_player();
        } else {
            townbuf[b].production = UTYPE_SETTLERS;
            townbuf[b].production = find_slot(a, b);
            if (townbuf[b].production >= 0)
                ++civ_units_ordered[a][townbuf[b].production];
        }
    }
    kill_civ(old, a);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int unrest_chance(int i)
{
    int owner;

    owner = townbuf[i].owner;
    if (32000 / townbuf[i].size > money[owner])
        return money[owner] * townbuf[i].size / (citizens[owner] + 1);
    return money[owner] / (citizens[owner] + 1) * townbuf[i].size;
}

void steal_advance(int a, int b)
{
    int num, top, opts[72], bv, i;
    unsigned int choice;

    if (a == 0)
        return;
    if (b == 0)
        return;
    num = 0;
    strcpy(msgtext, "Select one...\n ");
    bv = -1;
    i = 0x47;
    do {
        if (!has_tech(a, i) && has_tech(b, i)) {
            strcat(msgtext, advances[i].name);
            strcat(msgtext, "\n ");
            opts[num++] = i;
            if (advance_value(a, i) > bv) {
                bv = advance_value(a, i);
                top = i;
            }
        }
    } while (--i >= 0);
    if (num == 0)
        return;
    if (a == civnow) {
        do {
            if (mouse_mode == -1)
                choice = show_msg(msgtext, 80, 32);
            else
                choice = leader_menu(msgtext, 20, 139);
        } while (choice == -1);
        give_advance(a, opts[choice], b);
        return;
    }
    if (b == civnow) {
        strcpy(msgtext, civ_names[a]);
        strcat(msgtext, " take\n");
        strcat(msgtext, advances[top].name);
        strcat(msgtext, ".\n");
        if (mouse_mode == -1)
            show_msg(msgtext, 80, 80);
        else
            leader_menu(msgtext, 20, 139);
    }
    give_advance(a, top, b);
    retarget(a, -1);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void append_place_name(int i)
{
    if (i == -1)
        strcat(msgtext, "NONE");
    else
        copy_from_far(place_name[townbuf[i].name_id], msgtext + strlen(msgtext), 13);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void ad_wait(void)
{
    repaint();
    do
        poll_mouse();
    while (!mse_held && !kbhit());
    if (!mse_held && !getch())
        getch();
}

int rpath(int a, int b, int c)
{
    int r, s, t, w, j, k, best, n1;

    t = armydata[a][b].home_city;
    r = map_dist(townbuf[c].x, townbuf[t].x, townbuf[c].y, townbuf[t].y) + 10;
    r *= townbuf[c].trade + townbuf[t].trade;
    r /= 24;
    n1 = townbuf[t].x;
    if (region_key(townbuf[c].x, townbuf[c].y) == region_key(n1, n1))
        r /= 2;
    if (townbuf[c].owner == a)
        r /= 2;
    if (has_tech(townbuf[c].owner, 0x31))
        r -= r / 3;
    if (has_tech(townbuf[c].owner, 0x26))
        r -= r / 3;
    money[a] += r;
    civ_research_tally[a] += r;
    if (a == civnow) {
        strcpy(msgtext, trade_goods[b & 7]);
        strcat(msgtext, " caravan from ");
        append_place_name(t);
        strcat(msgtext, "\narrives in ");
        append_place_name(c);
        strcat(msgtext, "\nTrade route established\nRevenue: $");
        strcat(msgtext, itoa(r, nstr, 10));
        strcat(msgtext, ".\n");
        show_msg(msgtext, 80, 80);
        status_draw();
    }
    drop_u_stack(a, b);
    s = townbuf[c].trade;
    if (townbuf[c].owner != a)
        s <<= 1;
    best = -1;
    for (j = 0; j < 3; j++) {
        k = (unsigned char) townbuf[t].routes[j];
        if (k == c)
            return 1;
        if (k == 0xff)
            w = -1;
        else {
            w = townbuf[k].trade;
            if (townbuf[k].owner != a)
                w <<= 1;
        }
        if (w < s) {
            s = w;
            best = j;
        }
    }
    if (best != -1)
        townbuf[t].routes[best] = c;
    return 1;
}

/* City names, twelve characters padded with spaces; original far-data block. */
char far place_name[256][13] = {
    "Rome        ", "Caesarea    ", "Carthage    ", "Nicopolis   ",
    "Byzantium   ", "Brundisium  ", "Syracuse    ", "Antioch     ",
    "Palmyra     ", "Cyrene      ", "Gordion     ", "Tyrus       ",
    "Jerusalem   ", "Seleucia    ", "Ravenna     ", "Artaxata    ",
    "Babylon     ", "Sumer       ", "Uruk        ", "Nineveh     ",
    "Ashur       ", "Ellipi      ", "Akkad       ", "Eridu       ",
    "Kish        ", "Nippur      ", "Shuruppak   ", "Zariqum     ",
    "Izibia      ", "Nimrud      ", "Arbela      ", "Zamua       ",
    "Berlin      ", "Leipzig     ", "Hamburg     ", "Bremen      ",
    "Frankfurt   ", "Bonn        ", "Nuremberg   ", "Cologne     ",
    "Hannover    ", "Munich      ", "Stuttgart   ", "Heidelburg  ",
    "Salzburg    ", "Konigsberg  ", "Dortmund    ", "Brandenburg ",
    "Thebes      ", "Memphis     ", "Oryx        ", "Heliopolis  ",
    "Gaza        ", "Alexandria  ", "Byblos      ", "Cairo       ",
    "Coptos      ", "Edfu        ", "Pithom      ", "Busiris     ",
    "Athribis    ", "Mendes      ", "Tanis       ", "Abydos      ",
    "Washington  ", "New York    ", "Boston      ", "Philadelphia",
    "Atlanta     ", "Chicago     ", "Buffalo     ", "St.Louis    ",
    "Detroit     ", "New Orleans ", "Baltimore   ", "Denver      ",
    "Cincinnati  ", "Dallas      ", "Los Angeles ", "Las Vegas   ",
    "Athens      ", "Sparta      ", "Corinth     ", "Delphi      ",
    "Eretria     ", "Pharsalos   ", "Argos       ", "Mycenae     ",
    "Herakleia   ", "Antioch     ", "Ephesos     ", "Rhodes      ",
    "Knossos     ", "Troy        ", "Pergamon    ", "Miletos     ",
    "Delhi       ", "Bombay      ", "Madras      ", "Bangalore   ",
    "Calcutta    ", "Lahore      ", "Karachi     ", "Kolhapur    ",
    "Jaipur      ", "Hyderabad   ", "Bengal      ", "Chittagong  ",
    "Punjab      ", "Dacca       ", "Indus       ", "Ganges      ",
    "Moscow      ", "Leningrad   ", "Kiev        ", "Minsk       ",
    "Smolensk    ", "Odessa      ", "Sevastopol  ", "Tblisi      ",
    "Sverdlovsk  ", "Yakutsk     ", "Vladivostok ", "Novograd    ",
    "Krasnoyarsk ", "Riga        ", "Rostov      ", "Astrakhan   ",
    "Zimbabwe    ", "Ulundi      ", "Bapedi      ", "Hlobane     ",
    "Isandhlwana ", "Intombe     ", "Mpondo      ", "Ngome       ",
    "Swazi       ", "Tugela      ", "Umtata      ", "Umfolozi    ",
    "Ibabanago   ", "Isipezi     ", "Amatikulu   ", "Zunguin     ",
    "Paris       ", "Orleans     ", "Lyons       ", "Tours       ",
    "Chartres    ", "Bordeaux    ", "Rouen       ", "Avignon     ",
    "Marseilles  ", "Grenoble    ", "Dijon       ", "Amiens      ",
    "Cherbourg   ", "Poitiers    ", "Toulouse    ", "Bayonne     ",
    "Tenochtitlan", "Chiauhtia   ", "Chapultepec ", "Coatepec    ",
    "Ayotzinco   ", "Itzapalapa  ", "Iztapam     ", "Mitxcoac    ",
    "Tacubaya    ", "Tecamac     ", "Tepezinco   ", "Ticoman     ",
    "Tlaxcala    ", "Xaltocan    ", "Xicalango   ", "Zumpanco    ",
    "Peking      ", "Shanghai    ", "Canton      ", "Nanking     ",
    "Tsingtao    ", "Hangchow    ", "Tientsin    ", "Tatung      ",
    "Macao       ", "Anyang      ", "Shantung    ", "Chinan      ",
    "Kaifeng     ", "Ningpo      ", "Paoting     ", "Yangchow    ",
    "London      ", "Coventry    ", "Birmingham  ", "Dover       ",
    "Nottingham  ", "York        ", "Liverpool   ", "Brighton    ",
    "Oxford      ", "Reading     ", "Exeter      ", "Cambridge   ",
    "Hastings    ", "Canterbury  ", "Banbury     ", "Newcastle   ",
    "Samarkand   ", "Bokhara     ", "Nishapur    ", "Karakorum   ",
    "Kashgar     ", "Tabriz      ", "Aleppo      ", "Kabul       ",
    "Ormuz       ", "Basra       ", "Khanbalyk   ", "Khorasan    ",
    "Shangtu     ", "Kazan       ", "Quinsay     ", "Kerman      ",
    "Mecca       ", "Naples      ", "Sidon       ", "Tyre        ",
    "Tarsus      ", "Issus       ", "Cunaxa      ", "Cremona     ",
    "Cannae      ", "Capua       ", "Turin       ", "Genoa       ",
    "Utica       ", "Crete       ", "Damascus    ", "Verona      ",
    "Salamis     ", "Lisbon      ", "Hamburg     ", "Prague      ",
    "Salzburg    ", "Bergen      ", "Venice      ", "Milan       ",
    "Ghent       ", "Pisa        ", "Cordoba     ", "Seville     ",
    "Dublin      ", "Toronto     ", "Melbourne   ", "Sydney      "
};
