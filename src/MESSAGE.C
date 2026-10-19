/* Text and news messages. */
#include <stdio.h>
#include <string.h>
#include "GLOBALS.H"
#include "RESIDENT.H"
#include "DRAW.H"
#include "MESSAGE.H"

extern int font_height(int), char_width(int, int);

void news_wrap(int columns)
{
    int width, index;
    width = 0;
    index = 0;
    while (index < (int) strlen(msgtext)) {
        if (msgtext[index] == '^') {
            msgtext[index] = ' ';
            msgtext[index + 1] = '\n';
            width = -1;
        }
        if (msgtext[index] == '\n')
            width = -1;
        ++width;
        if (width > columns && msgtext[index] == ' ' && msgtext[index + 1] != ' ') {
            msgtext[index] = '\n';
            width = 0;
        }
        ++index;
    }
    if (width > 0)
        strcat(msgtext, "\n");
}

int news_effect(int columns, int x, int y, int colour)
{
    int saved, line_width, index, gap, height, start;
    line_width = start = gap = 0;
    height = font_height(scrbuf->font);
    for (index = 0; index < (int) strlen(msgtext); ++index) {
        line_width += char_width(scrbuf->font, msgtext[index]);
        if (msgtext[index] == ' ' || msgtext[index] == '\n' || msgtext[index] == '^')
            gap = index;
        if (msgtext[index] == '\n' || msgtext[index] == '^' || line_width > columns * 8) {
            saved = msgtext[gap];
            msgtext[gap] = 0;
            draw_string(msgtext + start, x, y, colour);
            msgtext[gap] = saved;
            y += height;
            line_width = 0;
            start = gap + 1;
            if (msgtext[index] == '^')
                ++start;
            index = gap;
            if (y > 192)
                break;
        }
    }
    if (line_width > 0 && y <= 192) {
        draw_string(msgtext + start, x, y, colour);
        y += height;
    }
    return y;
}

int news_name(char *name, char *key)
{
    char text_buf[256];
    int slot, idx, offset, hits, in_cache, matched;
    unsigned int table_word;
    FILE *stream;
    idx = slot = in_cache = 0;
    for (; idx < (int) strlen(key); ++idx)
        slot += key[idx] * idx;
    slot &= 255;
    for (;;) {
        /* Binary offset table followed by named text sections. */
        stream = fopen(name, "rb");
        fseek(stream, (long) (slot + slot / 32) * 2, 0);
        fread(&table_word, 2, 1, stream);
        fclose(stream);
        stream = fopen(name, "rt");
        if (table_word > 511 && table_word != 0x3030) {
            fseek(stream, (long) table_word, 0);
            fscanf(stream, "%[^\n]\n", text_buf);
            if (!stricmp(text_buf, key))
                in_cache = 1;
            else {
                ++slot;
                slot = (unsigned char) slot;
                fclose(stream);
                continue;
            }
        }
        hits = offset = 0;
        if (!in_cache) {
            fseek(stream, 512L, 0);
            do {
                table_word = ftell(stream);
                offset = fscanf(stream, "%[^\n]\n", text_buf);
            } while (stricmp(text_buf, key) && offset != -1);
        }
        if (offset != -1) {
            do {
                fscanf(stream, "%[^\n]\n", text_buf);
            } while (text_buf[0] == '*');
            do {
                strcat(msgtext, text_buf);
                strcat(msgtext, " ");
                fscanf(stream, "%[^\n]\n", text_buf);
                ++hits;
            } while (text_buf[0] != '*');
            matched = 1;
            goto done;
        } else
            break;
    }
    matched = 0;
done:
    fclose(stream);
    /* Save the newly discovered offset for subsequent lookups. */
    if (!in_cache && matched && offset != -1) {
        stream = fopen(name, "r+b");
        fseek(stream, (long) (slot + slot / 32) * 2, 0);
        fwrite(&table_word, 2, 1, stream);
        fclose(stream);
    }
    return matched ? hits : -1;
}

char *news_tokens[] = { "$US", "$THEM", "$BUCKS", "$RPLC1", "$RPLC2" };
char *news_values[] = { "                     ", "                     ", "                     ", "                     ", "                     " };

void named_msg(char *key)
{
    news_name("king.txt", key);
    help_open();
    news_wrap(80);
}

void help_open(void)
{
    int n;
    char *p;
    char buffer[512];
    for (n = 0; n < 5; ++n) {
        do {
            p = strstr(msgtext, news_tokens[n]);
            if (p) {
                strcpy(buffer, p);
                *p = 0;
                strcat(msgtext, news_values[n]);
                strcat(msgtext, buffer + strlen(news_tokens[n]));
            }
        } while (p);
    }
}

void truncate_text(char *s, int width)
{
    while (string_width(s) > width) {
        if (s[strlen(s) - 3] != ' ')
            s[strlen(s) - 2] = '.';
        s[strlen(s) - 1] = 0;
    }
}
