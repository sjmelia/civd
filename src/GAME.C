/* Main entry point, rule tables, screen and mouse helpers, and game menus. */
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "GLOBALS.H"
#include "AUDIENCE.H"
#include "DIALOG.H"
#include "F0DATA.H"
#include "RESIDENT.H"
#include "CORE.H"
#include "FAME.H"
#include "FILEIO.H"
#include "GAME.H"
#include "INTRO.H"
#include "MESSAGE.H"
#include "NEWGAME.H"
#include "SAVE.H"
#include "SETUP.H"
#include "TURN.H"
#include "WORLDGEN.H"
#include "DRAW.H"
#include "DRIVER.H"
#include "FADE.H"
#include "FCHECK.H"
#include "LOADER.H"
#include "LOADTAIL.H"
#include "MOUSE.H"

extern union REGS bios_call_reg_block_io;





extern void (interrupt far * _dos_getvect(int)) ();
extern void _dos_setvect(int, void (interrupt far *) ());

extern void show_help(char *);

struct terrain_rec {
    char name[12];
    unsigned char f0, f1, f2, f3, f4, f5, f6;
};

struct word_rec {
    int a, b, c, d;
};

/* 0-11 the terrain types, 12-23 the same with their special resource. */
struct terrain_rec terrains[24] = {
    { "Desert", 1, 2, 0, 1, 0, 1, 14 },
    { "Plains", 1, 2, 1, 1, 0, 1, 6 },
    { "Grassland", 1, 2, 2, 1, 0, 1, 10 },
    { "Forest", 2, 3, 1, 2, 0, 2, 2 },
    { "Hills", 2, 4, 1, 0, 0, 2, 12 },
    { "Mountains", 3, 6, 0, 1, 0, 3, 13 },
    { "Tundra", 1, 2, 1, 0, 0, 0, 7 },
    { "Arctic", 2, 2, 0, 0, 0, 0, 15 },
    { "Swamp", 2, 3, 1, 0, 0, 0, 3 },
    { "Jungle", 2, 3, 1, 0, 0, 0, 11 },
    { "Ocean", 1, 2, 1, 0, 2, 0, 1 },
    { "River", 1, 3, 2, 1, 1, 2, 9 },
    { "Oasis", 1, 2, 3, 1, 0, 1, 14 },
    { "Horses", 1, 2, 1, 3, 0, 1, 6 },
    { "Grassland", 1, 2, 2, 1, 0, 1, 10 },
    { "Game", 2, 3, 3, 2, 0, 2, 2 },
    { "Coal", 2, 4, 1, 2, 0, 2, 12 },
    { "Gold", 3, 6, 0, 1, 6, 3, 13 },
    { "Game", 1, 2, 3, 0, 0, 0, 7 },
    { "Seals", 2, 2, 2, 0, 0, 0, 15 },
    { "Oil", 2, 3, 1, 4, 0, 0, 3 },
    { "Gems", 2, 3, 1, 0, 4, 0, 11 },
    { "Fish", 1, 2, 3, 0, 2, 0, 1 },
    { "River", 1, 3, 2, 1, 1, 2, 9 },
};

struct word_rec work_rules[18] = {
    { -2, 5, -2, 5 },
    { 0, 0, -2, 5 },
    { 2, 15, 1, 1 },
    { -2, 5, 2, 10 },
    { 0, 1, 6, 5 },
    { -1, 5, 0, 0 },
    { -2, 10, -4, 10 },
    { 1, 1, -1, 0 },
    { -2, 10, 0, 0 },
    { -1, 0, -1, 0 },
    { 0, 0, -1, 0 },
    { -1, 0, 0, 0 },
    { 10, 15, 2, 15 },
    { 0, 0, 10, 15 },
    { 2, 15, 0, 0 },
    { -1, 0, -1, 0 },
    { 0, 0, -2, 5 },
    { -1, 0, 0, 1 },
};

struct advance advances[72] = {
    { "Alphabet", -1, -1 },
    { "Code of Laws", 0, -1 },
    { "Currency", 17, -1 },
    { "Atomic Theory", 51, 11 },
    { "Democracy", 28, 30 },
    { "Monarchy", 25, 1 },
    { "Astronomy", 26, 9 },
    { "MapMaking", 0, -1 },
    { "Navigation", 7, 6 },
    { "Mathematics", 0, 16 },
    { "Medicine", 28, 24 },
    { "Physics", 9, 8 },
    { "Engineering", 33, 45 },
    { "University", 9, 28 },
    { "Magnetism", 8, 11 },
    { "Electronics", 12, 54 },
    { "Masonry", -1, -1 },
    { "Bronze Working", -1, -1 },
    { "Iron Working", 17, -1 },
    { "Bridge Building", 18, 45 },
    { "Invention", 12, 30 },
    { "Computers", 9, 15 },
    { "Writing", 0, -1 },
    { "Steam Engine", 11, 20 },
    { "Trade", 2, 1 },
    { "Ceremonial Burial", -1, -1 },
    { "Mysticism", 25, -1 },
    { "Nuclear Fission", 41, 3 },
    { "Philosophy", 26, 30 },
    { "Religion", 28, 22 },
    { "Literacy", 22, 1 },
    { "Horseback Riding", -1, -1 },
    { "Feudalism", 16, 5 },
    { "The Wheel", -1, -1 },
    { "Gunpowder", 20, 18 },
    { "Industrialization", 49, 53 },
    { "Chemistry", 13, 10 },
    { "Combustion", 55, 56 },
    { "Flight", 37, 11 },
    { "Advanced Flight", 38, 54 },
    { "Space Flight", 21, 46 },
    { "Mass Production", 58, 47 },
    { "Pottery", -1, -1 },
    { "Communism", 28, 35 },
    { "The Republic", 1, 30 },
    { "Construction", 16, 2 },
    { "Rocketry", 39, 15 },
    { "The Corporation", 53, 35 },
    { "Metallurgy", 34, 13 },
    { "RailRoad", 23, 19 },
    { "Nuclear Power", 27, 15 },
    { "Theory of Gravity", 6, 13 },
    { "Steel", 48, 35 },
    { "Banking", 24, 44 },
    { "Electricity", 48, 14 },
    { "Refining", 36, 47 },
    { "Explosives", 34, 36 },
    { "SuperConductor", 60, 41 },
    { "Automobile", 37, 52 },
    { "Genetic Engineering", 10, 47 },
    { "Plastics", 55, 40 },
    { "Recycling", 41, 4 },
    { "Chivalry", 32, 31 },
    { "Robotics", 60, 21 },
    { "Conscription", 44, 56 },
    { "Labor Union", 41, 43 },
    { "Fusion Power", 50, 57 },
    { "1", 127, 127 },
    { "2", 127, 127 },
    { "3", 127, 127 },
    { "4", 127, 127 },
    { "Future Tech.", 127, 127 }
};

struct advance_attr advance_attrs[72] = {
    { 5, 0 },
    { 4, 1 },
    { 4, 1 },
    { 4, 0 },
    { 3, 2 },
    { 6, 1 },
    { 4, 0 },
    { 6, -1 },
    { 6, -1 },
    { 6, -1 },
    { 4, 0 },
    { 3, 0 },
    { 4, 0 },
    { 3, 1 },
    { 3, -2 },
    { 3, 0 },
    { 3, 1 },
    { 6, -1 },
    { 5, -2 },
    { 3, 0 },
    { 4, 0 },
    { 3, 0 },
    { 3, 1 },
    { 4, -1 },
    { 4, 1 },
    { 5, 0 },
    { 4, 0 },
    { 8, -2 },
    { 3, 1 },
    { 3, 2 },
    { 3, 2 },
    { 4, -1 },
    { 5, -1 },
    { 4, -2 },
    { 8, -2 },
    { 5, -1 },
    { 5, -1 },
    { 6, -1 },
    { 4, -1 },
    { 5, -1 },
    { 4, 1 },
    { 6, 0 },
    { 6, 1 },
    { 2, 0 },
    { 5, 1 },
    { 4, 0 },
    { 6, -2 },
    { 4, 0 },
    { 6, -2 },
    { 4, 0 },
    { 3, 0 },
    { 3, 1 },
    { 4, -1 },
    { 4, 1 },
    { 3, 1 },
    { 4, 0 },
    { 5, -1 },
    { 2, 1 },
    { 8, -1 },
    { 2, 2 },
    { 4, 1 },
    { 2, 1 },
    { 5, -2 },
    { 5, -1 },
    { 6, -1 },
    { 4, -1 },
    { 3, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
};

struct improvement improvements[46] = {
    { "NONE", 8, 0, -1 },
    { "Palace", 20, 5, 16 },
    { "Barracks", 4, 0, -1 },
    { "Granary", 6, 1, 42 },
    { "Temple", 4, 1, 25 },
    { "MarketPlace", 8, 1, 2 },
    { "Library", 8, 1, 22 },
    { "Courthouse", 8, 1, 1 },
    { "City Walls", 12, 2, 16 },
    { "Aqueduct", 12, 2, 45 },
    { "Bank", 12, 3, 53 },
    { "Cathedral", 16, 3, 29 },
    { "University", 16, 3, 13 },
    { "Mass Transit", 16, 4, 41 },
    { "Colosseum", 10, 4, 45 },
    { "Factory", 20, 4, 35 },
    { "Mfg. Plant", 32, 6, 63 },
    { "SDI Defense", 20, 4, 57 },
    { "Recycling Cntr.", 20, 2, 61 },
    { "Power Plant", 16, 4, 55 },
    { "Hydro Plant", 24, 4, 15 },
    { "Nuclear Plant", 16, 2, 50 },
    { "SS Structural", 8, 0, 40 },
    { "SS Component", 16, 0, 60 },
    { "SS Module", 32, 0, 63 },
    { "Pyramids", 30, 0, 16 },
    { "Hanging Gardens", 30, 0, 42 },
    { "Colossus", 20, 0, 17 },
    { "Lighthouse", 20, 0, 7 },
    { "Great Library", 30, 0, 30 },
    { "Oracle", 30, 0, 26 },
    { "Great Wall", 30, 0, 16 },
    { "Magellan's Expedition", 40, 0, 8 },
    { "Michelangelo's Chapel", 30, 0, 29 },
    { "Copernicus' Observatory", 30, 0, 6 },
    { "Shakespeare's Theatre", 40, 0, 10 },
    { "Isaac Newton's College", 40, 0, 51 },
    { "J.S.Bach's Cathedral", 40, 0, 29 },
    { "Darwin's Voyage", 30, 0, 49 },
    { "Hoover Dam", 60, 0, 15 },
    { "Women's Suffrage", 60, 0, 35 },
    { "Manhattan Project", 60, 0, 27 },
    { "United Nations", 60, 0, 43 },
    { "Apollo Program", 60, 0, 40 },
    { "SETI Program", 60, 0, 21 },
    { "Cure for Cancer", 60, 0, 59 }
};

int improvement_words[22] = {
    -1, 43, 20, 54, 14, 13, 29, 34, 127, 43, 127, 15, 27, 127, 127, 127, 127, 127, 127, 127, 127, 127
};

/* Unit types and civilization/leader tables. */

struct unit_type {
    char name[12];
    int obsolete_by;
    int domain;
    int moves, fuel, attack, defense, cost, sight, capacity, role, prereq;
};

struct unit_type utypes[28] = {
    { "Settlers", 127, 0, 1, 0, 0, 1, 4, 0, 0, 0, -1 },
    { "Militia", 34, 0, 1, 0, 1, 1, 1, 0, 0, 2, -1 },
    { "Phalanx", 34, 0, 1, 0, 1, 2, 2, 0, 0, 2, 17 },
    { "Legion", 64, 0, 1, 0, 3, 1, 2, 0, 0, 1, 18 },
    { "Musketeers", 64, 0, 1, 0, 2, 3, 3, 0, 0, 2, 34 },
    { "Riflemen", 127, 0, 1, 0, 3, 5, 3, 0, 0, 2, 64 },
    { "Cavalry", 64, 0, 2, 0, 2, 1, 2, 0, 0, 1, 31 },
    { "Knights", 58, 0, 2, 0, 4, 2, 4, 0, 0, 1, 62 },
    { "Catapult", 48, 0, 1, 0, 6, 1, 4, 0, 0, 1, 9 },
    { "Cannon", 63, 0, 1, 0, 8, 1, 4, 0, 0, 1, 48 },
    { "Chariot", 62, 0, 2, 0, 4, 1, 4, 0, 0, 1, 33 },
    { "Armor", 127, 0, 3, 0, 10, 5, 8, 0, 0, 1, 58 },
    { "Mech. Inf.", 127, 0, 3, 0, 6, 6, 5, 0, 0, 2, 65 },
    { "Artillery", 127, 0, 2, 0, 12, 2, 6, 0, 0, 1, 63 },
    { "Fighter", 127, 1, 10, 1, 4, 2, 6, 2, 0, 4, 38 },
    { "Bomber", 127, 1, 8, 2, 12, 1, 12, 2, 0, 1, 39 },
    { "Trireme", 8, 2, 3, 0, 1, 0, 4, 0, 2, 5, 7 },
    { "Sail", 14, 2, 3, 0, 1, 1, 4, 0, 3, 5, 8 },
    { "Frigate", 35, 2, 3, 0, 2, 2, 4, 0, 4, 5, 14 },
    { "Ironclad", 37, 2, 4, 0, 4, 4, 6, 0, 0, 3, 23 },
    { "Cruiser", 127, 2, 6, 0, 6, 6, 8, 3, 0, 3, 37 },
    { "Battleship", 127, 2, 4, 0, 18, 12, 16, 3, 0, 3, 52 },
    { "Submarine", 127, 2, 3, 0, 8, 2, 5, 3, 0, 3, 41 },
    { "Carrier", 127, 2, 5, 0, 1, 12, 16, 3, 0, 3, 39 },
    { "Transport", 127, 2, 4, 0, 0, 3, 5, 0, 8, 5, 35 },
    { "Nuclear", 127, 1, 16, 1, 99, 0, 16, 0, 0, 1, 46 },
    { "Diplomat", 127, 0, 2, 0, 0, 0, 3, 0, 0, 6, 22 },
    { "Caravan", 127, 0, 1, 0, 0, 1, 5, 0, 0, 6, 24 },
};

struct civ_type civt[16] = {
    { "", "", "", 0, 0, 0, 36, 36 },
    { "Caesar", "", "Roman", 0, 1, 1, 24, 10 },
    { "Hammurabi", "", "Babylonian", -1, -1, 1, 28, 14 },
    { "Frederick", "Germans", "German", 1, -1, 1, 32, 18 },
    { "Ramesses", "", "Egyptian", 0, 0, 1, 21, 7 },
    { "Abe Lincoln", "", "American", -1, 0, 1, 19, 5 },
    { "Alexander", "", "Greek", 0, 1, -1, 26, 12 },
    { "M.Gandhi", "", "Indian", -1, -1, 0, 31, 17 },
    { "", "", "", 0, 0, 0, 36, 36 },
    { "Stalin", "", "Russian", 1, 0, -1, 25, 11 },
    { "Shaka", "", "Zulu", 1, 0, 0, 22, 8 },
    { "Napoleon", "French", "French", 1, 1, 1, 23, 9 },
    { "Montezuma", "", "Aztec", 0, -1, 1, 20, 6 },
    { "Mao Tse Tung", "Chinese", "Chinese", 0, 0, 1, 29, 15 },
    { "Elizabeth I", "English", "English", 0, 1, 0, 27, 13 },
    { "Genghis Khan", "", "Mongol", 1, 1, -1, 30, 16 },
};

int spiral_x[49] = {
    0, 0, 1, 1, 1, 0, -1, -1, -1, 0, 1, 2, 2, 2, 1, 0,
    -1, -2, -2, -2, -1, 2, 2, -2, -2, 0, 1, 2, 3, 3, 3, 3,
    3, 2, 1, 0, -1, -2, -3, -3, -3, -3, -3, -2, -1, 3, 3, -3,
    -3
};

int spiral_y[49] = {
    0, -1, -1, 0, 1, 1, 1, 0, -1, -2, -2, -1, 0, 1, 2, 2,
    2, 1, 0, -1, -2, 2, -2, -2, 2, -3, -3, -3, -2, -1, 0, 1,
    2, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, 3, -3, 3,
    -3
};

int civ_colours[8] = {
    12, 15, 10, 9, 14, 11, 13, 7
};

int civ_shadows[8] = {
    4, 7, 2, 1, 10, 3, 4, 8
};

char *government_names[6] = {
    "Anarchy",
    "Despotism",
    "Monarchy",
    "Communist",
    "Republic",
    "Democratic"
};

char *trade_goods[8] = {
    "Silk",
    "Silver",
    "Wine",
    "Copper",
    "Gems",
    "Dye",
    "Salt",
    "Spice"
};

char *civ_adjectives[8] = {
    "Barbarian ",
    "Roman     ",
    "Zulu      ",
    "Mongol    ",
    "Aztec     ",
    "Persian   ",
    "Muslim    ",
    "Hittite   "
};

char *civ_names[8] = {
    "Barbarians ",
    "Romans     ",
    "Babylonians",
    "Germans    ",
    "Egyptians  ",
    "Americans  ",
    "Greeks     ",
    "Mongols    "
};

char *leader_names[8] = {
    "Attila       ",
    "Caesar       ",
    "Shaka        ",
    "Genghis Khan ",
    "Montezuma    ",
    "Xerxes       ",
    "Saladin      ",
    "Suppiluliumas"
};

char *leader_titles[6] = {
    "Mr.",
    "Emperor",
    "King ",
    "Comrade",
    "President",
    "President"
};

int selected_index = -1;
int debug = 2;

struct page page1 = { 1, 0, 0, 319, 199, 0, 15, 4, 0 };

struct page *back_page = &page1;
struct page page2 = { 2, 0, 0, 319, 199, 0, 15, 4, 0 };

struct page *tile_page = &page2;
struct page page3 = { 3, 0, 0, 319, 199, 0, 15, 4, 0 };

struct page *spare_page = &page3;

char ega_palette[17] = { 0, 1, 2, 3, 4, 0, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0 };
char ega_default_palette[17] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0 };

char graphics_driver[] = "mgraphic.exe";
char sound_driver[] = "nsound.cvl";
int has_mouse = 1;
int low_memory = 0;

/*
 * ## main
 *
 * Game entry point.
 * Initializes the runtime environment, drivers, screen, memory,
 * displays the opening menu, and runs the main turn loop.
 */
int main(void)
{
    int j, k, n, memory, i;

    run_program(load_program("misc.exe", 0));
    setup_main();
    *(char far *) 0x00400017L &= 0xdf;
    install_break_handlers();
    game_startup();
    disk_insert(0, 1L);
    memory = free_paragraphs();
    if (small_space_check)
        memory = 0;
    if ((vga_modes ? 8000 : 4000) > memory) {
        rect_fill(scrbuf, 0, 0, 320, 200, 12);
        msgtext[0] = 0;
        strcat(msgtext, itoa(memory, nstr, 10));
        strcpy(message_slot, msgtext);
        msgtext[0] = 0;
        named_msg("*LOMEM");
        show_menu(msgtext, 64, 49);
    }
    if (vga_modes && memory < 9150)
        low_memory = 1;
    sprite_arena_init();
    mouse_cursor = load_cursor("torch.pic");
    sprite_arena_release();
    if (has_mouse)
        has_mouse = init_mouse();
    if (has_mouse) {
        set_mousecur(0, 0, mouse_cursor);
        mouse_on();
    }
    game_menu();
    start_game();
    if (main_option_result == 1) {
        map_x0 = x_cap[civnow] - 7;
        map_y0 = 19;
        sel_player();
    }
    leave = 0;
    do
        game_turn();
    while (!leave);
    if (has_mouse)
        mouse_close();
    sound(0);
    sound_shutdown();
    restore_timer();
    restore_break_handlers();
    bios_call_reg_block_io.h.ah = 0;
    bios_call_reg_block_io.h.al = 3;
    int86(0x10, &bios_call_reg_block_io, &bios_call_reg_block_io);
}

void empty_hook(void)
{
}

void set_screen_mode(int mode)
{
    if (page_ready(0)) {
        back_page->number = mode;
        blit(scrbuf, 0, 0, 320, 200, back_page, 0, 0);
        set_page(mode);
        back_page->number = scrbuf->number ^ 1;
        flip(1);
    }
}

void poll_mouse(void)
{
    if (has_mouse) {
        mse_held = mouse_poll() | mouse_info_a;
        ptr_x = mouse_info_b;
        ptr_y = mouse_info_c;
    } else {
        mse_held = ptr_x = ptr_y = 0;
    }
}

void mouse_on(void)
{
    pointer_hide_count++;
    if (has_mouse && pointer_hide_count == 1)
        mouse_show();
}

void mouse_off(void)
{
    if (has_mouse && pointer_hide_count == 1)
        mouse_hide();
    pointer_hide_count--;
}

void mouse_force_on(void)
{
    mouse_hide_limit = pointer_hide_count;
    while (pointer_hide_count < 1)
        mouse_on();
}

void mouse_restore(void)
{
    while (pointer_hide_count > mouse_hide_limit)
        mouse_off();
}

char *file_names[8] = {
    "civ.exe",
    "docker.pic",
    "govt0a.pic",
    "blurb0.txt",
    "king00.pic",
    "king08.pic",
    "fonts.cv",
    "birth2.pic"
};

char disk_digit = 'A';
int disk_swap = 0;

/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
disk_insert(file, mode)
int file, mode;
{
    int old_style;
    char buffer[256];
    if (mode == 0) {
        if (file_exists(file_names[file], 0) != -1) {
            which_data = file;
            return 1;
        }
        file = 0;
    }
    while (file_exists(file_names[file], 0) == -1 || mode == -1) {
        set_page(scrbuf->number);
        if (mode == -1)
            mode = 1;
        mouse_off();
        rect_fill(scrbuf, 0, 0, 320, 200, 15);
        disk_digit = file / found_floppy + '1';
        strcpy(buffer, "Please insert Disk ");
        strcat(buffer, &disk_digit);
        strcat(buffer, ".\n \n    Press Return\n    to continue.\n");
        old_style = style;
        style = -1;
        box_info_flags = 1;
        show_menu(buffer, 100, 81);
        style = old_style;
        rect_fill(scrbuf, 0, 0, 320, 200, 15);
        disk_swap = 1;
        mouse_on();
    }
    which_data = file;
    return mode;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void interrupt far break_handler(void)
{
}

void install_break_handlers(void)
{
    orig_break_isr = _dos_getvect(0x1b);
    int_23 = _dos_getvect(0x23);
    _dos_setvect(0x1b, break_handler);
    _dos_setvect(0x23, break_handler);
}

void restore_break_handlers(void)
{
    _dos_setvect(0x1b, orig_break_isr);
    _dos_setvect(0x23, int_23);
}

/* Calls use differing argument counts; retain this interface; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
sound(a, b)
int a, b;
{
    if (debug & 0x10)
        sound_command(a, b);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void game_menu(void)
{
    int seed, i, j;

    beginner_help = 0;
    goto test;
    do {
        strcpy(msgtext, " Start a New Game\n Load a Saved Game\n EARTH\n Customize World\n View Hall of Fame\n");
        main_option_result = show_menu(msgtext, 100, 140);
test:  ;
    } while (main_option_result == -1);
    do
        seed = sound_quiet_ticks();
    while (seed > 5269 && (seed - 5269) % 288 != 0);
    sound(1);
    switch (main_option_result) {
    case 0:
        mouse_off();
        for (i = 0; i < 4; i++)
            custom_params[i] = 1;
start:
        make_world();
        scrbuf->number = 0;
        set_page(0);
        load_screen(1, "sp299.pic");
        blit(back_page, 160, 50, 160, 150, tile_page, 160, 50);
        disk_insert(0, 1L);
        break;
    case 3:
        mouse_off();
        disk_insert(7, 1L);
        if (vga_modes)
            set_palette(5, 0, 0, 0);
        rect_fill(scrbuf, 0, 0, 320, 200, 0);
        load_picture(1, "custom.pic");
        disk_insert(0, 1L);
        blit(back_page, 0, 0, 320, 200, scrbuf, 0, 0);
        mouse_on();
        editing = 1;
        for (;;) {
            set_text_pos(210, 11);
            strcpy(msgtext, "LAND MASS:\n Small\n Normal\n Large\n");
            custom_params[0] = show_menu(msgtext, 200, 1);
            if (custom_params[0] == -1)
                continue;
            if (edit_mode) {
                show_help("Land");
                editing = custom_params[0];
                continue;
            }
            break;
        }
        editing = 1;
        for (;;) {
            set_text_pos(210, 61);
            strcpy(msgtext, "TEMPERATURE:\n Cool\n Temperate\n Warm\n");
            custom_params[1] = show_menu(msgtext, 200, 51);
            if (custom_params[1] == -1)
                continue;
            if (edit_mode) {
                show_help("Temperature");
                editing = custom_params[1];
                continue;
            }
            break;
        }
        editing = 1;
        for (;;) {
            set_text_pos(210, 111);
            strcpy(msgtext, "CLIMATE:\n Arid\n Normal\n Wet\n");
            custom_params[2] = show_menu(msgtext, 200, 101);
            if (custom_params[2] == -1)
                continue;
            if (edit_mode) {
                show_help("Climate");
                editing = custom_params[2];
                continue;
            }
            editing = 1;
            do {
                set_text_pos(210, 161);
                strcpy(msgtext, "AGE:\n 3 billion years\n 4 billion years\n 5 billion years\n");
                custom_params[3] = show_menu(msgtext, 200, 151);
            } while (custom_params[3] == -1);
            if (edit_mode) {
                show_help("Age");
                editing = custom_params[3];
                continue;
            }
            break;
        }
        mouse_off();
        goto start;
    case 2:
        mouse_off();
        beginner_help = 1;
        make_world();
        scrbuf->number = 0;
        set_page(0);
        load_screen(1, "sp299.pic");
        blit(back_page, 160, 50, 160, 150, tile_page, 160, 50);
        disk_insert(0, 1L);
        break;
    case 1:
        if (load_game(-1) == -1)
            goto reset;
        mouse_off();
        rect_fill(scrbuf, 0, 0, 320, 200, 7);
        break;
    case 4:
        mouse_off();
        fame_load();
        show_fame(-1);
        mouse_on();
reset:
        main_option_result = -1;
        goto test;
    }
    setup_art();
    busy_flag = 1;
    if (vga_modes && (main_option_result == 0 || main_option_result == 2 || main_option_result == 3))
        set_palette(5, 0, 0, 0);
    rect_fill(scrbuf, 0, 0, 320, 200, 0);
    load_back("sp257.pic");
    setup_cycles();
    mouse_on();
}

void start_game(void)
{
    int a, b, c;

    if (has_mouse)
        set_mousecur(0, 0, splashhand);
    if (!turn_tick) {
        load_picture(1, "diffs.pic");
        disk_insert(0, 1L);
        new_game();
    }
    cabinet();
    disk_insert(0, 1L);
}

void set_cursor(int row, int column)
{
    bios_call_reg_block_io.h.ah = 2;
    bios_call_reg_block_io.h.bh = 0;
    bios_call_reg_block_io.h.dh = row;
    bios_call_reg_block_io.h.dl = column;
    int86(0x10, &bios_call_reg_block_io, &bios_call_reg_block_io);
}

/* Far world state. These uninitialized far arrays are communals, which LINK
   lays out in FAR_BSS by its symbol hash bucket, (length + sum of
   (char | 0x20) << (position & 3)) & 0xFF, not in declaration order. The
   original names are unknown; these ascend in that order. Other modules
   declare them extern with the types they need. */
char far power_history[150][8];
int far advance_owners[72];
char far city_site_x[256];
char far city_site_y[256];
char far spaceships[8][15][12];
int far pedia_topics[100];
signed char far adv_source[8][72];
char far war_record[150][8];
char far fog_map[20][13];
int far unit_losses[8][28];
unsigned char far replay_log[4096];
int far topic_class[84];
char far fame_ranks[332];
