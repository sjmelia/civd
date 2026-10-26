#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GAME.H"
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "SETUP.H"
#include "DRIVER.H"
#include "STRBIOS.H"

extern union REGS bios_regs;


extern char graphics_driver;
extern char sound_driver;

char *gfx_names[4] = { ") VGA (256 color)", ") MCGA (256 color)", ") EGA (16 color)", ") Tandy 1000 (16 color)" };
char *sound_names[6] = { ") No sounds please", ") IBM sounds", ") Tandy sounds", ") AdLib/Sound Blaster", ") Roland MT-32 MIDI board", ") Custom sound driver" };
char sound_codes[6] = { 'N', 'I', 'T', 'A', 'R', 'X' };
char gfx_codes[4] = { 'M', 'M', 'E', 'T' };

char *clear_line = "                                                                               \r\n$";

void setup_main(void)
{
    /* Unused words/arrays retain the original frame; suffixes retain MSC buckets. */
    char unused_ready_i;
    char input[128];
    char option_text_e[58];
    int intro_line_p;
    char await_choice_d;
    char unused_a[4];
    unsigned char option_m;
    unsigned char key_n;
    char unused_b_c[16];
    char ask_sound_b;
    char unused_c_d[4];
    FILE *intro_file_p;
    int option_row_l;
    int unused_limit_k;
    char unused_d_g[20];
    int option_count_n;
    int unused_e_h;

    bios_regs.x.ax = 3;
    int86(0x10, &bios_regs, &bios_regs);
    print_at(0, 0, "\xff\x1e");
    intro_file_p = fopen("intro.txt", "rt");
    for (intro_line_p = 1; intro_line_p <= 10; intro_line_p++) {
        fscanf(intro_file_p, "%[^\n]\n", msgtext);
        /* Replace the printed version digit in the bundled introduction. */
        if (intro_line_p == 3) {
            msgtext[0x36] = '5';
            msgtext[0x37] = ' ';
        }
        print_at(intro_line_p, 0, msgtext);
    }
    unused_ready_i = 1;
    ask_sound_b = 1;
    set_cursor(13, 0);
    for (option_m = 1; option_m <= 10; option_m++) {
        dos_print(clear_line);
    }
    set_cursor(11, 0);
    dos_print(clear_line);
    dos_print(clear_line);
    strcpy(msgtext, "\xff\x0e");
    fscanf(intro_file_p, "%[^\n]\n", input);
    strcat(msgtext, input);
    print_at(13, 0, msgtext);
    dos_print(clear_line);

    /* Tables map displayed numeric choices to driver letters. */
    option_count_n = 1;
    for (option_m = 0; option_m < 4; option_m++) {
        itoa(option_count_n, option_text_e, 10);
        table1[option_count_n] = gfx_codes[option_m];
        strcat(option_text_e, gfx_names[option_m]);
        print_at(option_count_n++ + 14, 27, option_text_e);
    }
    await_choice_d = 1;
    do {
        set_cursor(13, 49);
        key_n = key_echo();
        if (key_n >= '1' && key_n < option_count_n + '0') {
            graphics_driver = table1[key_n - '0'];
            await_choice_d = 0;
        }
    } while (await_choice_d);

    if (ask_sound_b == 1) {
        set_cursor(12, 0);
        for (option_m = 1; option_m <= 12; option_m++) {
            dos_print(clear_line);
        }
        strcpy(msgtext, "\xff\x0e");
        fscanf(intro_file_p, "%[^\n]\n", input);
        strcat(msgtext, input);
        print_at(13, 0, msgtext);
        option_count_n = 1;
        option_row_l = 1;
        unused_limit_k = 7;
        for (option_m = 0; option_m < 6; option_m++) {
            itoa(option_count_n, option_text_e, 10);
            table2[option_count_n] = sound_codes[option_m];
            /* Dormant custom-driver entries are outside the six built-in choices. */
            if (option_m >= 7) {
                sound_names[option_m][15] = table3[option_m];
                table2[option_count_n] = table3[option_m];
            }
            strcat(option_text_e, sound_names[option_m]);
            print_at(option_row_l++ + 13, 27, option_text_e);
            option_count_n++;
        }
        await_choice_d = 1;
        do {
            set_cursor(13, 50);
            key_n = key_echo();
            if (key_n >= '1' && key_n < option_count_n + '0') {
                sound_driver = table2[key_n - '0'];
                await_choice_d = 0;
            }
        } while (await_choice_d);
    }

    set_cursor(12, 0);
    for (option_m = 1; option_m <= 12; option_m++) {
        dos_print(clear_line);
    }
    print_at(13, 27, "1. Mouse and Keyboard");
    print_at(14, 27, "2. Keyboard only");
    await_choice_d = 1;
    do {
        set_cursor(13, 50);
        key_n = key_echo();
        if (key_n >= '1' && key_n <= '2') {
            has_mouse = (key_n == '1') ? 1 : 0;
            await_choice_d = 0;
        }
    } while (await_choice_d);

    fclose(intro_file_p);
}
