/* MESSAGE.H: shared declarations of symbols defined in MESSAGE.C. */

extern void news_wrap(int);
extern void help_open(void);
extern void named_msg(char *);
extern int news_effect(int, int, int, int);
extern int news_name(char *, char *);
extern void truncate_text(char *, int);
