/* TURN.H: shared declarations of symbols defined in TURN.C. */

extern int palace_stage;
extern int next_event_turn;
extern int show_menu(char *, int, int);
extern void game_turn(void);
extern int winner;
/* Calls use differing argument counts; retain the unprototyped interface. */
extern void show_hall_of_fame();
extern void show_ad_screen(void);
extern void uprising(void);
extern void rate_civs(void);
extern void draw_palace(int, int, int, int);
extern int cost_sum(int);
extern void format_year(void);
extern void ai_event(void);
extern void sel_player(void);
extern void panel_prep(int, int, int, int);
extern void load_player_palette(void);
extern int steps_for(int);
extern void draw_banner(void);
extern void status_draw(void);
extern int show_msg(char *, int, int);
