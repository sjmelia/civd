/* Overlay 11: save and load menus, disk handling and state transfers. */
#include <dos.h>
#include <bios.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "CIV.H"
#include "CORE.H"
#include "DIALOG.H"
#include "FAME.H"
#include "FILEIO.H"
#include "GAME.H"
#include "GLOBALS.H"
#include "INTRO.H"
#include "RESIDENT.H"
#include "SPACESHP.H"
#include "TURN.H"
#include "UNITS.H"
#include "WORLDGEN.H"
#include "MESSAGE.H"
#include "SAVE.H"

extern char filename[];
extern int debug;

extern int treaties[8][8];
extern struct city townbuf[];
extern struct unit_type utypes[];
extern struct unit armydata[8][128];
extern char replay_length[], block_ground_way[];
extern char ship_success[], ai_ship_success[];
extern char far power_history[], far war_record[];
extern char civ_vis_grid[];
extern int far advance_owners[];
extern char far place_name[][13], far replay_log[];
extern char far unit_losses[];
extern char far adv_source[], far spaceships[];
extern char far city_site_x[];
extern char far city_site_y[];
extern unsigned int drive_choice;
extern void disk_insert();

extern int image_screen(), open(), read(), write(), close();
extern long lseek();
int read_save_header();         /* NOLINT(clang-diagnostic-deprecated-non-prototype) */

char filename[] = "a:CIVIL0.SVE";

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int load_game(int slot)
{
    int i, drive, reserve;
    save_mode = 1;
    mouse_off();
    drive = choose_drive();
    if (drive != -1) {
        if (slot == -1) {
            strcpy(msgtext, "\214Select Load File...\n");
            saved_found = 0;
            for (i = 0; (unsigned int) i < 10; i++) {
                filename[7] = i + '0';
                if (read_save_header(filename, 1L))
                    saved_found |= 1 << i;
            }
            mouse_on();
            slotsel = show_menu(msgtext, 48, 65);
            mouse_off();
            if (!(saved_found & (1 << slotsel)))
                slotsel = -1;
        } else
            slotsel = slot;
        if (slotsel != -1) {
            filename[7] = slotsel + '0';
            if (!read_save_header(filename, 0L))
                slotsel = -1;
        }
        disk_insert(0, 1L);
        mouse_on();
        return slotsel;
    }
    mouse_on();
    return -1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Callers pass long arguments to int parameters; preserve their stack layout; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int read_save_header(file, preview)
char *file;
int preview;
{
    char leader[16];
    strcpy(file + 9, "SVE");
    if (preview) {
        save_fd = open(file, O_BINARY);
        if (save_fd != -1) {
            lseek(save_fd, 2L, 0);
            read(save_fd, &save_turn, 2);
            lseek(save_fd, 8L, 0);
            read(save_fd, &save_year, 2);
            strcat(msgtext, " ");
            lseek(save_fd, 10L, 0);
            read(save_fd, &save_player, 2);
            strcat(msgtext, rank_title[save_player]);
            strcat(msgtext, " ");
            lseek(save_fd, (long) (save_turn * 14) + 16L, 0);
            read(save_fd, leader, 14);
            strcat(msgtext, leader);
            lseek(save_fd, (long) (save_turn * 12) + 128L, 0);
            read(save_fd, leader, 12);
            strcat(msgtext, ", ");
            strcat(msgtext, leader);
            strcat(msgtext, "/");
            strcat(msgtext, itoa(abs(save_year), nstr, 10));
            strcat(msgtext, save_year < 0 ? " BC\n" : " AD\n");
        } else
            strcat(msgtext, " (EMPTY)\n");
        close(save_fd);
        return save_fd != -1 ? 1 : 0;
    }
    load_save(file);
    disk_insert(1, 1L);
    scrbuf->number = 2;
    link_seas(0, 0);
    scrbuf->number = 0;
    for (save_index = 1; save_index < 8; save_index++);
    reload = slotsel < 4 ? 1 : 2;
    fame_slot = -3;
    game_flags &= ~1;
    return 1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void save_game(int slot)
{
    int i, drive;
    save_mode = save_error = 0;
    mouse_off();
    menu_begin();
    drive = choose_drive();
    if (drive != -1) {
        strcpy(msgtext, "\214Select Save File...\n");
        for (i = 0; i < 4; i++) {
            filename[7] = i + '0';
            read_save_header(filename, 1L);
        }
        editing = clamp(slotsel, 0, 3);
        if (slot == -1) {
            mouse_on();
            slot = show_menu(msgtext, 48, 33);
            mouse_off();
        }
        if (slot != -1) {
            slotsel = slot;
            filename[7] = slot + '0';
            if (write_save(filename)) {
                if ((long) save_error == 0L)
                    strcpy(msgtext, " Game has been saved.\n");
                else {
                    strcpy(msgtext, " Game NOT saved.\n");
                    rect_fill(scrbuf, 64, 127, 192, 34, 12);
                }
                if (save_error == 13)
                    strcat(msgtext, " Write access denied.\n");
                if (save_error == 28)
                    strcat(msgtext, " Disk Full.\n");
                strcat(msgtext, " Press key to continue.\n");
                box_info_flags = 1;
                show_menu(msgtext, 64, 127);
            }
        }
    }
    disk_insert(0, 1L);
    menu_end();
    mouse_on();
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int write_save(char *file)
{
    int reserve, ret, fd, flag;
    abort_flag = 1;
    strcpy(msgtext, " ");
    strcat(msgtext, file);
    strcat(msgtext, "\n ");
    strcat(msgtext, rank_title[cur_tier]);
    strcat(msgtext, " ");
    strcat(msgtext, leader_names[civnow]);
    strcat(msgtext, "\n ");
    strcat(msgtext, civ_names[civnow]);
    strcat(msgtext, "/");
    format_year();
    strcat(msgtext, "\n ... save in progress.\n");
    box_info_flags = 1;
    show_menu(msgtext, 64, 86);
    read_save(file);
    return 1;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void save_write(int fd, char *buf, int length)
{
    if (write(fd, buf, length) == -1)
        save_error = image_error;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

unsigned int drive_choice = -1;

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int choose_drive(void)
{
    int input, unused, spare;
    if (drive_choice == -1) {
        _dos_getdrive(&drive_choice);
        drive_choice--;
    }
    rect_fill(scrbuf, 0, 0, 320, 200, 15);
    if (!save_mode)
        menu_clear();
    do {
        strcpy(msgtext, save_mode ? "  Which drive contains your\n    saved game files?\n\n            " : "  Which drive contains your\n     Save Game disk?\n\n            ");
        msgtext[strlen(msgtext) - 1] = drive_choice + 'A';
        strcat(msgtext, ":\n\n    Press drive letter and\nReturn when disk is inserted.\n");
        strcat(msgtext, "    Press Escape to cancel.\n");
        news_effect(99, 80, 72, 0);
        input = key_cmd();
        if (input == 'A' || input == 'a')
            drive_choice = 0;
        if (input == 'B' || input == 'b')
            drive_choice = 1;
        if (input == 'C' || input == 'c')
            drive_choice = 2;
        if (input == 'D' || input == 'd')
            drive_choice = 3;
        if (input == 'E' || input == 'e')
            drive_choice = 4;
        if (input == 'F' || input == 'f')
            drive_choice = 5;
        if (input == 27)
            drive_choice = -1;
        rect_fill(scrbuf, 80, 88, 160, 24, 15);
    } while (input != 13 && input != 27);
    rect_fill(scrbuf, 8, 8, 304, 184, 15);
    if (drive_choice != -1 && !disk_ready(drive_choice)) {
        strcpy(msgtext, "No Disk in Drive A.\n");
        msgtext[strlen(msgtext) - 3] = drive_choice + 'A';
        show_menu(msgtext, 100, 80);
        return -1;
    }
    filename[0] = drive_choice + 'a';
    return drive_choice;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

int disk_ready(int drive)
{
    struct diskinfo_t disk;
    int error, tries;
    if (drive > 1)
        return 1;
    disk.drive = drive;
    disk.head = 0;
    disk.track = 0;
    disk.sector = 2;
    disk.nsectors = 1;
    for (tries = 0; tries < 3; tries++) {
        error = _bios_disk(_DISK_VERIFY, &disk);
        if (!(error & 0xff00))
            return 1;
        _bios_disk(_DISK_RESET, 0);
    }
    return 0;
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int load_save(char *file)
{
    strcpy(file + 9, "SVE");
    save_fd = open(file, O_BINARY);
    if (save_fd == -1) {
        strcpy(msgtext, "File Error: ");
        strcat(msgtext, file);
        strcat(msgtext, "\n");
        show_msg(msgtext, 100, 80);
        return 0;
    }
    save_mode = 1;
    transfer_game();
    close(save_fd);
    mouse_off();
    strcpy(file + 9, "map");
    load_screen(2, file);
    mouse_on();
    return 1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
int read_save(char *file)
{
    strcpy(file + 9, "map");
    if (image_screen(2, file)) {
        strcpy(msgtext, "Error writing map file.\n");
        show_menu(msgtext, 4, 64);
        save_error = 1;
        return 0;
    }
    strcpy(file + 9, "SVE");
    save_fd = open(file, 0x8301, 128);
    if (save_fd == -1) {
        strcpy(msgtext, "File Error: ");
        strcat(msgtext, file);
        strcat(msgtext, "\n");
        show_msg(msgtext, 100, 80);
        return 0;
    }
    save_mode = 0;
    transfer_game();
    close(save_fd);
    if (save_error)
        return 0;
    return 1;
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

void transfer_game(void)
{
    int unused;
    transfer_near((char *) &turn_tick, 0x2);
    transfer_near((char *) &civnow, 0x2);
    transfer_near((char *) &human_set, 0x2);
    transfer_near((char *) &map_rng_state, 0x2);
    transfer_near((char *) &year, 0x2);
    transfer_near((char *) &cur_tier, 0x2);
    transfer_near((char *) &players, 0x2);
    transfer_near((char *) &selected_index, 0x2);
    transfer_near(leader_names[0], 0x70);
    transfer_near(civ_names[0], 0x60);
    transfer_near(civ_adjectives[0], 0x58);
    transfer_near((char *) money, 0x10);
    transfer_near((char *) civ_research_tally, 0x10);
    transfer_near((char *) forces_present, 0x1c0);
    transfer_near((char *) civ_units_ordered, 0x1c0);
    transfer_near((char *) tech_count, 0x10);
    transfer_near((char *) advances_known, 0x50);
    transfer_near((char *) civ_gov, 0x10);
    transfer_near((char *) civ_area_plan, 0x100);
    transfer_near((char *) treaties, 0x80);
    transfer_near((char *) city_count, 0x10);
    transfer_near((char *) troops_total, 0x10);
    transfer_near((char *) civ_held_cells, 0x10);
    transfer_near((char *) civ_settlers_n, 0x10);
    transfer_near((char *) citizens, 0x10);
    transfer_near((char *) military_table, 0x10);
    transfer_near((char *) nation_ratings, 0x10);
    transfer_near((char *) civ_toll, 0x10);
    transfer_near((char *) civ_ratings, 0x10);
    transfer_near((char *) last_met_turn, 0x10);
    transfer_near((char *) x_cap, 0x10);
    transfer_near((char *) civ_tribe_of, 0x10);
    transfer_near((char *) defenders_island, 0x100);
    transfer_near((char *) civ_cont_attackers, 0x100);
    transfer_near((char *) ai_continent_towns, 0x100);
    transfer_near((char *) continent_span, 0x100);
    transfer_near((char *) ai_zone_places, 0x20);
    transfer_far((char far *) power_history, 0x4b0);
    transfer_far((char far *) war_record, 0x4b0);
    transfer_near((char *) townbuf, 0xe00);
    transfer_near((char *) utypes, 0x3b8);
    transfer_near((char *) armydata, 0x3000);
    transfer_near(civ_vis_grid, 0xfa0);
    transfer_near((char *) spot_classes, 0x80);
    transfer_near((char *) ai_camp_site_weights, 0x80);
    transfer_near((char *) camp_xpos, 0x80);
    transfer_near((char *) civ_lot_y, 0x80);
    transfer_far((char far *) advance_owners, 0x90);
    transfer_near((char *) killed, 0x80);
    transfer_far((char far *) place_name, 0xd00);
    transfer_near((char *) replay_length, 0x2);
    transfer_far((char far *) replay_log, 0x1000);
    transfer_near((char *) marvel_site, 0x2c);
    transfer_far((char far *) unit_losses, 0x1c0);
    transfer_far((char far *) adv_source, 0x240);
    transfer_near((char *) &polluted, 0x2);
    transfer_near((char *) &warm_levels, 0x2);
    transfer_near((char *) &warming, 0x2);
    transfer_near((char *) &debug, 0x2);
    transfer_near((char *) block_ground_way, 0x104);
    transfer_near((char *) &max_rating, 0x2);
    transfer_near((char *) &advances_future, 0x2);
    transfer_near((char *) &rule_options, 0x2);
    transfer_near((char *) sci, 0x10);
    transfer_near((char *) &next_event_turn, 0x2);
    transfer_near((char *) history_rating, 0x10);
    transfer_far((char far *) spaceships, 0x5a0);
    transfer_near((char *) &game_flags, 0x2);
    transfer_near((char *) ship_success, 0x2);
    transfer_near((char *) ai_ship_success, 0x2);
    transfer_near((char *) spaceship_arrive_dates, 0x10);
    transfer_near((char *) floors_palace, 0x18);
    transfer_near((char *) section_grades, 0x18);
    transfer_far((char far *) city_site_x, 0x100);
    transfer_far((char far *) city_site_y, 0x100);
    transfer_near((char *) &palace_stage, 0x2);
    transfer_near((char *) &player_peace_run, 0x2);
    transfer_near((char *) &civ_total, 0x2);
    transfer_near((char *) space_ship_habitation, 0x10);
    transfer_near((char *) liftoff_when, 0x10);
    transfer_near((char *) &civ_flags_bits, 0x2);
}

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void transfer_far(char far *buffer, int length)
{
    char far *scratch = msgtext;
    int bytes;
    while (length > 0) {
        bytes = clamp(length, 0, 512);
        if (save_mode) {
            read(save_fd, msgtext, bytes);
            movedata(FP_SEG(scratch), FP_OFF(scratch), FP_SEG(buffer), FP_OFF(buffer), bytes);
        } else {
            movedata(FP_SEG(buffer), FP_OFF(buffer), FP_SEG(scratch), FP_OFF(scratch), bytes);
            save_write(save_fd, msgtext, bytes);
        }
        length -= bytes;
        buffer += bytes;
    }
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */

/* Non-prototype ABI calls below; NOLINTBEGIN(clang-diagnostic-deprecated-non-prototype) */
void transfer_near(char *buffer, int length)
{
    if (save_mode)
        read(save_fd, buffer, length);
    else
        save_write(save_fd, buffer, length);
}

/* NOLINTEND(clang-diagnostic-deprecated-non-prototype) */
