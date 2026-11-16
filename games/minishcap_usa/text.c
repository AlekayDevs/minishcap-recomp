/* The Minish Cap (USA): the text of the game, as a file the player can read and edit.
 *
 * The game keeps its messages in one bank: a table of 80 groups at ROM 0x9B1D90, each group a table of offsets and then its messages.
 * Seven pointers at 0x109214 (one per language, all the same in this ROM) point to the bank. With language=es this file builds a
 * second bank in the free space at the end of the ROM image (in memory: the file on disk is never touched), putting in each message
 * the text of <game folder>/lang/es.txt when the file has one and the game's own English when it does not, and points those seven
 * pointers to it. With language=dump the English messages are written to <game folder>/lang/en.dump.txt in the same format and the
 * program ends: the dump comes from the player's own ROM and is not for sharing (it is in .gitignore).
 *
 * Format of lang/es.txt: a line `@G:M` (group and message, decimal) opens a message and the text after it goes on until the next `@`;
 * lines that start with `#` are comments; line breaks (and carriage returns) of the file are not part of the text. A letter, a digit,
 * a space or a sign of the keyboard is that character; `\n` is the game's line break (0A); any other byte is written in hexadecimal
 * between braces: {06 00} is two bytes (a command with its parameter), spaces inside the braces are ignored. The final 00 is not
 * written. The file is UTF-8 and may have a e i o u with an acute accent, u with a diaeresis, n with a tilde and the opening ? and !.
 *
 * Accents: the ROM already has the European letters. Codes 0x80-0xFF read their glyphs from a second table (ROM 0x697CE0 + 64 * (code
 * - 0x90)) laid out like Latin-1, so a letter with an acute accent, a diaeresis or a tilde, and the opening ? and !, are the byte
 * of their Unicode code point (U+00A0-U+00FF): a is E1, n with a tilde is F1, the opening ? is BF. The first table, ROM 0x692F60 + 64 *
 * code (two 4bpp tiles of 8x16 pixels), is the one of the plain ASCII. Nothing of the font is copied here. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gba_sys.h"

#define TOP 0x9B1D90u        /* table of groups */
#define N_GROUPS 80
#define BANK_PTR_FIRST 0x109214u
#define BANK_PTRS 7
#define NEW_BANK 0xE00000u   /* free space (FF) from 0xDE7DA4 to the end of the 16 MiB image */
#define ROM_END 0x1000000u
#define BANK_ADDR(off) (0x08000000u + (off))

static u32 rd32le(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24); }
static void wr32le(uint8_t *p, u32 v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }

/* ---- the messages of the ROM ---- */

typedef struct { u32 start, end; } Span;   /* a message in the ROM image */
static Span *spans[N_GROUPS];
static int counts[N_GROUPS];
static u32 group_start[N_GROUPS], group_end[N_GROUPS];

static int read_bank(const uint8_t *rom, u32 len) {
    if (TOP + N_GROUPS * 4 > len) return 0;
    for (int g = 0; g < N_GROUPS; g++) group_start[g] = TOP + rd32le(rom + TOP + 4 * g);
    for (int g = 0; g < N_GROUPS; g++) {
        u32 gs = group_start[g];
        if (gs + 4 > len) return 0;
        int n = (int)(rd32le(rom + gs) / 4);
        if (n < 1 || n > 4000) return 0;
        counts[g] = n;
        spans[g] = calloc((size_t)n, sizeof(Span));
        for (int m = 0; m < n; m++) spans[g][m].start = gs + rd32le(rom + gs + 4 * m);
        if (g + 1 < N_GROUPS) group_end[g] = group_start[g + 1];
        else { u32 e = spans[g][n - 1].start; while (e < len && rom[e]) e++; group_end[g] = e + 1; }
        for (int m = 0; m < n; m++) spans[g][m].end = m + 1 < n ? spans[g][m + 1].start : group_end[g];
    }
    return 1;
}

/* ---- the file of the translation ---- */

typedef struct { uint8_t *b; int n; } Msg;
static Msg *over[N_GROUPS];   /* over[g][m]: the translated message (b == NULL: none) */

static int hexval(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Turns the text of one message into its bytes (with the final 00). Returns -1 and a reason when it cannot be read. */
static int encode(const char *s, size_t n, uint8_t *out, int cap, const char **why) {
    int o = 0;
    for (size_t i = 0; i < n;) {
        unsigned char c = (unsigned char)s[i];
        if (o + 2 >= cap) { *why = "message too long"; return -1; }
        if (c == '\r' || c == '\n') { i++; continue; }
        if (c == '\\') {
            if (i + 1 < n && s[i + 1] == 'n') { out[o++] = 0x0A; i += 2; continue; }
            *why = "a backslash that is not \\n"; return -1;
        }
        if (c == '{') {
            size_t j = i + 1; int digits = 0, hi = -1;
            for (; j < n && s[j] != '}'; j++) {
                if (s[j] == ' ') continue;
                int h = hexval((unsigned char)s[j]);
                if (h < 0) { *why = "a brace with something that is not hexadecimal"; return -1; }
                if (hi < 0) hi = h; else { if (o + 2 >= cap) { *why = "message too long"; return -1; } out[o++] = (uint8_t)(hi * 16 + h); hi = -1; }
                digits++;
            }
            if (j >= n || hi >= 0 || !digits) { *why = "an unclosed or odd brace"; return -1; }
            i = j + 1; continue;
        }
        if (c >= 0x20 && c < 0x7F) { out[o++] = c; i++; continue; }
        unsigned cp = 0; int len = 0;
        if ((c & 0xE0) == 0xC0 && i + 1 < n) { cp = ((c & 0x1Fu) << 6) | ((unsigned char)s[i + 1] & 0x3Fu); len = 2; }
        if (len && cp >= 0xA0 && cp <= 0xFF) { out[o++] = (uint8_t)cp; i += (size_t)len; continue; } /* Latin-1 */
        *why = "a character the font does not have"; return -1;
    }
    out[o++] = 0;
    return o;
}

static int load_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)sz + 1);
    if (!buf || fread(buf, 1, (size_t)sz, f) != (size_t)sz) { fclose(f); free(buf); return 0; }
    fclose(f); buf[sz] = 0;
    for (int g = 0; g < N_GROUPS; g++) over[g] = calloc((size_t)counts[g], sizeof(Msg));
    char *p = buf; int line = 1, loaded = 0, bad = 0;
    int g = -1, m = -1; char *text = NULL; size_t tl = 0;
    #define FLUSH() do { if (g >= 0) { \
        uint8_t tmp[2048]; const char *why = ""; int k = encode(text, tl, tmp, (int)sizeof tmp, &why); \
        if (k < 0) { fprintf(stderr, "%s: message %d:%d: %s\n", path, g, m, why); bad++; } \
        else { over[g][m].b = malloc((size_t)k); memcpy(over[g][m].b, tmp, (size_t)k); over[g][m].n = k; loaded++; } \
        g = -1; } } while (0)
    while (*p) {
        char *e = strchr(p, '\n'); size_t ll = e ? (size_t)(e - p) : strlen(p);
        if (ll && p[ll - 1] == '\r') ll--;
        if (p[0] == '#') { /* comment */ }
        else if (p[0] == '@') {
            FLUSH();
            int gg, mm;
            if (sscanf(p + 1, "%d:%d", &gg, &mm) == 2 && gg >= 0 && gg < N_GROUPS && mm >= 0 && mm < counts[gg]) {
                g = gg; m = mm;
                char *c = p + 1; while (*c && *c != ' ' && *c != '\n' && *c != '\r') c++;
                if (*c == ' ') c++;
                text = c; tl = ll - (size_t)(c - p);
            } else { fprintf(stderr, "%s:%d: not a message number\n", path, line); bad++; g = -1; }
        } else if (g >= 0) {
            /* the text goes on (line breaks of the file are not part of it): join the lines by moving them next to the first */
            memmove(text + tl, p, ll); tl += ll;
        }
        if (!e) break;
        p = e + 1; line++;
    }
    FLUSH();
    #undef FLUSH
    fprintf(stderr, "text: %s: %d messages%s\n", path, loaded, bad ? " (some left out, see above)" : "");
    return 1;
}

/* ---- the bank in the free space ---- */

static int build_bank(uint8_t *rom, u32 len) {
    if (len < ROM_END) { fprintf(stderr, "text: the ROM image is smaller than 16 MiB: the translation needs the free space at its end\n"); return 0; }
    u32 pos = NEW_BANK + N_GROUPS * 4;
    for (int g = 0; g < N_GROUPS; g++) {
        u32 gs = (pos + 3) & ~3u; u32 table = gs, body = gs + (u32)counts[g] * 4;
        if (body + 8192 > ROM_END) { fprintf(stderr, "text: the translation does not fit in the free space\n"); return 0; }
        u32 w = body;
        wr32le(rom + NEW_BANK + 4 * g, gs - NEW_BANK);
        for (int m = 0; m < counts[g]; m++) {
            wr32le(rom + table + 4 * m, w - gs);
            if (over[g] && over[g][m].b) { memcpy(rom + w, over[g][m].b, (size_t)over[g][m].n); w += (u32)over[g][m].n; }
            else { u32 n = spans[g][m].end - spans[g][m].start; memcpy(rom + w, rom + spans[g][m].start, n); w += n; }
            if (w + 2048 > ROM_END) { fprintf(stderr, "text: the translation does not fit in the free space\n"); return 0; }
        }
        pos = w;
    }
    for (int i = 0; i < BANK_PTRS; i++) wr32le(rom + BANK_PTR_FIRST + 4 * i, BANK_ADDR(NEW_BANK));
    return 1;
}

/* ---- the dump of the English text ---- */

static int dump_english(const uint8_t *rom, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return 0; }
    fprintf(f, "# The English text of your ROM, in the format of es.txt. It comes from your copy of the game: do not share it.\n");
    for (int g = 0; g < N_GROUPS; g++) {
        for (int m = 0; m < counts[g]; m++) {
            fprintf(f, "@%d:%d ", g, m);
            u32 e = spans[g][m].end;
            if (e > spans[g][m].start && rom[e - 1] == 0) e--; /* the terminator is not written */
            for (u32 i = spans[g][m].start; i < e; i++) {
                unsigned char c = rom[i];
                if (c == 0x0A) fputs("\\n", f);
                else if (c >= 0x20 && c < 0x7F && c != '{' && c != '\\' && c != '@') fputc(c, f);
                else fprintf(f, "{%02X}", c);
            }
            fputc('\n', f);
        }
    }
    fclose(f);
    return 1;
}

void game_init(uint8_t *rom, u32 len, const char *game_dir, const char *language) {
    int es = !strcmp(language, "es"), dump = !strcmp(language, "dump");
    if (!es && !dump) return;
    if (!read_bank(rom, len)) { fprintf(stderr, "text: the message bank is not where it should be (is this the USA ROM?)\n"); return; }
    char path[1024];
    if (dump) {
        snprintf(path, sizeof path, "%s/lang/en.dump.txt", game_dir);
        if (dump_english(rom, path)) fprintf(stderr, "text: English text written to %s\n", path);
        exit(0);
    }
    snprintf(path, sizeof path, "%s/lang/es.txt", game_dir);
    if (!load_file(path)) { fprintf(stderr, "text: %s not found: the game stays in English\n", path); return; }
    build_bank(rom, len);
}
