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
extern int advisor;             /* DIALOG.C */
#define message_mode advisor

extern void timer_reset(void);
extern int timer_read(void);
extern int land_count;

/* Graphics driver gateways defined by RESIDENT.ASM. */
extern void blit(struct page *, int, int, int, int, struct page *, int, int);
extern void sprite_draw(struct page *, int, int, int);
extern int in_view(int, int);
