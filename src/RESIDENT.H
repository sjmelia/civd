/* Shared declarations for storage and routines in RESIDENT.ASM. */
#include "GRAPHICS.H"

extern struct page *scrbuf;

/* Names for other variables, or for entries of them. */
extern char *news_values[];     /* MESSAGE.C: strings the news messages fill in */
#define name_buf (news_values[0])
#define message_slot_a (news_values[1])
#define message_slot_b (news_values[2])
#define message_slot (news_values[3])
extern int fame_slot;           /* FAME.C */
#define load_state fame_slot
extern int advisor;             /* DIALOG.C */
#define message_mode advisor
extern int errno;               /* the C library's */
#define image_error errno


extern int target_value;
extern int match_count;
extern int mouse_hide_limit;
extern void restore_timer(void);
extern int menu_result;
extern int hilite_colour;
extern int line_count;
extern int line_off[];
extern char table1[];
extern char table2[];
extern char table3[];
extern int edit_cursor;
extern int edit_cursor_w;
extern int edit_cursor_x;
extern void timer_reset(void);
extern int timer_read(void);
extern int save_error;
extern int save_fd;
extern int save_index;
extern int save_mode;
extern int save_player;
extern int save_turn;
extern int save_year;
extern int selected_tile;
extern int nearest_dist;
extern int replay_auto;
extern int replay_civ;
extern int replay_file;
extern int replay_mode;
extern int ship_state[];
extern int ship_tiles[];
extern unsigned char star_on[];
extern unsigned char star_x[];
extern unsigned char star_y[];
extern int pedia_more;
extern int archive_index;
extern int image_file;
extern int picture_bytes;
extern int picture_height;
extern int picture_width;
extern int source_offset;
extern int source_segment;
extern int path_cache_x;
extern int path_cache_y;
extern int path_limit;
extern char path_queue_x[256];
extern char path_queue_y[256];
extern int path_target_x;
extern int path_target_y;
extern int city_wonder_tiles[];
extern int pop_tiles[];
extern int shown_face;          /* advisor face last drawn */
extern int faces_loaded;        /* leader face tiles allocated */
extern int modern_court;        /* leader has Invention: modern pictures */
extern int no_portraits;        /* portrait disk absent: text only */
extern int audience_civ;
extern int anim_step;           /* index into the face_* animation tables */
extern int talk_hold;           /* frames before idle animation resumes */
extern int portrait;            /* leader portrait number */
extern int advisors_loaded;     /* advisor head tiles cut */
extern int leader_font;
extern int excess_unhappy;      /* unhappy citizens beyond city size */
extern int selected_city;
extern int selected_owner;
extern int radius_tiles;        /* tiles in a city radius (21) */
extern int units_abroad;        /* military units away, causing unhappiness */
extern int land_count;
extern int story_clock;
extern int story_line;
extern int story_skip;
extern int story_wait;
extern int group_total;
extern int selected_score;
extern int selected_stack_index;
extern int total_limit;
extern int total_mode;

/* Graphics driver gateways defined by RESIDENT.ASM. */
extern void blit(struct page *, int, int, int, int, struct page *, int, int);
extern void sprite_draw(struct page *, int, int, int);
extern int in_view(int, int);
