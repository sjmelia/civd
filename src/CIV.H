/* Shared game records. Field names are inferred; sizes and signedness
 * preserve the DOS representation. */
#ifndef CIV_H
#define CIV_H

/* DGROUP 0282: 12 terrain types, then the same 12 with their special
 * resource, 19 bytes each. */
struct terrain_type {
    char name[12];
    signed char move_cost;
    signed char defense;        /* defense multiplier, in halves */
    signed char food, shields, trade, f5, f6;
};

/* Columns of work_rules. Negative change values are a yield bonus of
 * -1 - value (-1 for none); others name the terrain the tile turns into. */
#define WORK_IRRIGATE 0
#define WORK_IRRIGATE_TURNS 1
#define WORK_MINE 2
#define WORK_MINE_TURNS 3
#define WORK_ROAD 4             /* AI builds roads here */
#define WORK_ROAD_BRIDGES 5     /* the same with Bridge Building */

/* DGROUP 112A: 28 unit types, 34 bytes each. */
struct unit_type {
    char name[12];
    int obsolete_by;
    int domain;                 /* 0 land, 1 air, 2 sea */
    int moves;
    int fuel;
    int attack;
    int defense;
    int cost;
    int sight;
    int capacity;
    int role;
    int prereq;
};

/* Unit type IDs (indices into utypes table) */
#define UTYPE_SETTLERS      0
#define UTYPE_MILITIA       1
#define UTYPE_PHALANX       2
#define UTYPE_LEGION        3
#define UTYPE_MUSKETEERS    4
#define UTYPE_RIFLEMEN      5
#define UTYPE_CAVALRY       6
#define UTYPE_KNIGHTS       7
#define UTYPE_CATAPULT      8
#define UTYPE_CANNON        9
#define UTYPE_CHARIOT       10
#define UTYPE_ARMOR         11
#define UTYPE_MECH_INF      12
#define UTYPE_ARTILLERY     13
#define UTYPE_FIGHTER       14
#define UTYPE_BOMBER        15
#define UTYPE_TRIREME       16
#define UTYPE_SAIL          17
#define UTYPE_FRIGATE       18
#define UTYPE_IRONCLAD      19
#define UTYPE_CRUISER       20
#define UTYPE_BATTLESHIP    21
#define UTYPE_SUBMARINE     22
#define UTYPE_CARRIER       23
#define UTYPE_TRANSPORT     24
#define UTYPE_NUCLEAR       25
#define UTYPE_DIPLOMAT      26
#define UTYPE_CARAVAN       27

/* DGROUP 70EC: 128 cities, 28 bytes each. */
struct city {
    long buildings;             /* bit (n-1) set: improvement n is built */
    unsigned char x, y;
    char status;
    signed char size;
    char visible_size;
    char production;
    char trade;
    signed char owner;
    int food_box;               /* accumulated food */
    int shield_box;             /* accumulated production */
    long workers;               /* worked tiles 0..20, specialist count 26..31 */
    int specialists;            /* eight two-bit specialist types */
    unsigned char name_id;      /* index into place_name */
    char routes[3];             /* destination city indices, -1 if empty */
    char garrison[2];           /* unit type plus veteran bit 0x40, -1 if empty */
};

/* City improvement bits (improvement number minus one). */
#define HAS_PALACE          0x00000001L
#define HAS_BARRACKS        0x00000002L
#define HAS_GRANARY         0x00000004L
#define HAS_TEMPLE          0x00000008L
#define HAS_MARKETPLACE     0x00000010L
#define HAS_LIBRARY         0x00000020L
#define HAS_COURTHOUSE      0x00000040L
#define HAS_CITY_WALLS      0x00000080L
#define HAS_AQUEDUCT        0x00000100L
#define HAS_BANK            0x00000200L
#define HAS_CATHEDRAL       0x00000400L
#define HAS_UNIVERSITY      0x00000800L
#define HAS_MASS_TRANSIT    0x00001000L
#define HAS_COLOSSEUM       0x00002000L
#define HAS_FACTORY         0x00004000L
#define HAS_MFG_PLANT       0x00008000L
#define HAS_SDI             0x00010000L
#define HAS_RECYCLING_CNTR  0x00020000L
#define HAS_POWER_PLANT     0x00040000L
#define HAS_HYDRO_PLANT     0x00080000L
#define HAS_NUCLEAR_PLANT   0x00100000L

/* DGROUP 81D4: 128 units per civilization, 12 bytes each. */
struct unit {
    char status;                /* 0x08 fortified, 0x20 veteran */
    unsigned char x, y;
    signed char type;
    signed char moves;          /* remaining moves, in thirds */
    char fuel;
    char goto_x;
    char goto_y;
    char last_dir;              /* last direction moved (1-8), or -1 */
    signed char visibility;     /* one bit per civilization */
    signed char next;           /* next unit in the same stack, or -1 */
    char home_city;
};

/* Unit status flags */
#define UNIT_SENTRY         0x01
#define UNIT_ROAD           0x02
#define UNIT_FORTIFYING     0x04
#define UNIT_FORTIFIED      0x08
#define UNIT_ASSIGNED       0x10
#define UNIT_VETERAN        0x20
#define UNIT_IRRIGATING     0x40
#define UNIT_MINING         0x80
#define UNIT_FORTRESS       (UNIT_IRRIGATING | UNIT_MINING)
#define UNIT_POLLUTION      (UNIT_MINING | UNIT_ROAD)

/* City status flags */
#define CITY_DISORDER       0x01
#define CITY_COASTAL        0x02
#define CITY_GARRISON       0x04
#define CITY_RIVER          0x08
#define CITY_AUTO_BUILD     0x10
#define CITY_TECH_STOLEN    0x20
#define CITY_CELEBRATING    0x40
#define CITY_SOLD_BUILDING  0x80
#define CITY_CAPTURE_MASK   0xae
#define CITY_TRANSFER_MASK  0x9b

#endif
