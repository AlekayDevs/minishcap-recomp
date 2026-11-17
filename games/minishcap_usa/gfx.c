/* Gráficos en español (con language=es). Los rótulos del título y los menús son imágenes de la ROM, no mensajes: aquí se dibujan
   con un alfabeto propio (nada se copia de la ROM) y se escriben en la VRAM cuando la pantalla correspondiente los ha cargado.
   gfx_frame() se llama una vez por fotograma desde game_frame() (options.c). */
#include <string.h>
#include "gba_sys.h"

int gfx_es; /* lo pone game_init (text.c) */

/* Letras de 6x10 con trazo de dos píxeles; 'X' es relleno. */
static const char *glyph(char c) {
    switch (c) {
    case 'P': return "XXXXX. XXXXXX XX..XX XX..XX XXXXXX XXXXX. XX.... XX.... XX.... XX....";
    case 'U': return "XX..XX XX..XX XX..XX XX..XX XX..XX XX..XX XX..XX XX..XX XXXXXX .XXXX.";
    case 'L': return "XX.... XX.... XX.... XX.... XX.... XX.... XX.... XX.... XXXXXX XXXXXX";
    case 'S': return ".XXXXX XXXXXX XX.... XX.... XXXXX. .XXXXX ....XX ....XX XXXXXX XXXXX.";
    case 'A': return ".XXXX. XXXXXX XX..XX XX..XX XX..XX XXXXXX XXXXXX XX..XX XX..XX XX..XX";
    case 'T': return "XXXXXX XXXXXX ..XX.. ..XX.. ..XX.. ..XX.. ..XX.. ..XX.. ..XX.. ..XX..";
    case 'R': return "XXXXX. XXXXXX XX..XX XX..XX XXXXX. XXXXX. XX.XX. XX..XX XX..XX XX..XX";
    }
    return NULL;
}

/* Lienzo de 96x16 con los índices de la paleta 14 del rótulo (6..2 relleno amarillo-verde, 14 contorno blanco, 9 sombra). */
static void draw_title(unsigned char px[16][96], const char *text) {
    unsigned char fill[16][96]; memset(fill, 0, sizeof fill); memset(px, 0, 16 * 96);
    int x0 = 6, n = (int)strlen(text);
    for (int i = 0; i < n; i++) {
        const char *g = glyph(text[i]);
        if (!g) continue;
        for (int r = 0; r < 10; r++) for (int c = 0; c < 6; c++)
            if (g[r * 7 + c] == 'X') { int x = x0 + i * 8 + c; if (x < 96) { fill[3 + r][x] = 1; px[3 + r][x] = (unsigned char)(6 - r / 2); } }
    }
    for (int y = 0; y < 16; y++) for (int x = 0; x < 96; x++) {
        if (fill[y][x]) continue;
        for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++) {
            int yy = y + dy, xx = x + dx;
            if (yy >= 0 && yy < 16 && xx >= 0 && xx < 96 && fill[yy][xx]) px[y][x] = 14;
        }
    }
    for (int y = 1; y < 16; y++) for (int x = 0; x < 96; x++)
        if (!px[y][x] && ((px[y - 1][x] == 14) || (y >= 8 && x + 1 < 96 && px[y][x + 1] == 14 && !fill[y][x + 1]))) px[y][x] = 9;
}

/* "PRESS START": tres sprites de 32x16 (4x2 teselas, 8 teselas cada uno) en la tesela 0, 8 y 16 de los objetos, paleta 14. */
static void title_press_start(void) {
    int found = 0;
    for (int i = 0; i < 128; i++) {
        const unsigned char *o = rt_oam + i * 8;
        unsigned a0 = o[0] | o[1] << 8, a1 = o[2] | o[3] << 8, a2 = o[4] | o[5] << 8;
        if (a0 == 0x407C && a2 == 0xE000 && a1 == 0x8048) found |= 1;
        if (a0 == 0x407C && a2 == 0xE008 && a1 == 0x8068) found |= 2;
        if (a0 == 0x407C && a2 == 0xE010 && a1 == 0x8088) found |= 4;
    }
    if (found != 7) return;
    static unsigned char px[16][96], want[768];
    static int built;
    if (!built) {
        draw_title(px, "PULSA START");
        for (int s = 0; s < 3; s++) for (int ty = 0; ty < 2; ty++) for (int tx = 0; tx < 4; tx++)
            for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c += 2) {
                int y = ty * 8 + r, x = s * 32 + tx * 8 + c;
                want[(s * 8 + ty * 4 + tx) * 32 + r * 4 + c / 2] = (unsigned char)(px[y][x] | px[y][x + 1] << 4);
            }
        built = 1;
    }
    if (memcmp(rt_vram + 0x10000, want, sizeof want)) memcpy(rt_vram + 0x10000, want, sizeof want);
}


/* ---- Cabeceras azules de los menús (CHOOSE A FILE, ENTER A NAME): cuatro sprites de texto seguidos (32x16 x3 + 8x16) ---- */

/* Alfabeto de 5x7 (cada trazo se engrosa a 2 píxeles al dibujar). */
static const char *font5(char c) {
    switch (c) {
    case 'A': return "01110 10001 10001 11111 10001 10001 10001";
    case 'B': return "11110 10001 10001 11110 10001 10001 11110";
    case 'C': return "01110 10001 10000 10000 10000 10001 01110";
    case 'D': return "11110 10001 10001 10001 10001 10001 11110";
    case 'E': return "11111 10000 10000 11110 10000 10000 11111";
    case 'F': return "11111 10000 10000 11110 10000 10000 10000";
    case 'G': return "01111 10000 10000 10011 10001 10001 01110";
    case 'H': return "10001 10001 10001 11111 10001 10001 10001";
    case 'I': return "01110 00100 00100 00100 00100 00100 01110";
    case 'J': return "00111 00010 00010 00010 00010 10010 01100";
    case 'K': return "10001 10010 10100 11000 10100 10010 10001";
    case 'L': return "10000 10000 10000 10000 10000 10000 11111";
    case 'M': return "10001 11011 10101 10101 10001 10001 10001";
    case 'N': return "10001 11001 10101 10011 10001 10001 10001";
    case 'O': return "01110 10001 10001 10001 10001 10001 01110";
    case 'P': return "11110 10001 10001 11110 10000 10000 10000";
    case 'Q': return "01110 10001 10001 10001 10101 10010 01101";
    case 'R': return "11110 10001 10001 11110 10100 10010 10001";
    case 'S': return "01111 10000 10000 01110 00001 00001 11110";
    case 'T': return "11111 00100 00100 00100 00100 00100 00100";
    case 'U': return "10001 10001 10001 10001 10001 10001 01110";
    case 'V': return "10001 10001 10001 10001 10001 01010 00100";
    case 'W': return "10001 10001 10001 10101 10101 11011 10001";
    case 'X': return "10001 10001 01010 00100 01010 10001 10001";
    case 'Y': return "10001 10001 01010 00100 00100 00100 00100";
    case 'Z': return "11111 00001 00010 00100 01000 10000 11111";
    }
    return NULL;
}

#define BW 104
/* Lienzo de BWx16 con los índices de la paleta 7: fondo 5, fondo oscuro 6, anillos c y e (blanco) y relleno 7. */
static void draw_banner(unsigned char px[16][BW], const char *text) {
    static unsigned char fill[16][BW];
    memset(fill, 0, sizeof fill); memset(px, 5, 16 * BW);
    int n = (int)strlen(text), x0 = (BW - (n * 8 - 2)) / 2 - 1;
    for (int i = 0; i < n; i++) {
        const char *g = font5(text[i]);
        if (!g) continue;
        for (int r = 0; r < 7; r++) for (int c = 0; c < 5; c++)
            if (g[r * 6 + c] == '1')
                for (int dy = 0; dy < 2; dy++) for (int dx = 0; dx < 2; dx++) {
                    int x = x0 + i * 8 + c + dx, y = 3 + r + dy;
                    if (x >= 0 && x < BW && y < 16) fill[y][x] = 1;
                }
    }
    for (int y = 0; y < 16; y++) for (int x = 0; x < BW; x++) {
        int d = 9;
        for (int dy = -3; dy <= 3; dy++) for (int dx = -3; dx <= 3; dx++) {
            int yy = y + dy, xx = x + dx;
            if (yy < 0 || yy >= 16 || xx < 0 || xx >= BW || !fill[yy][xx]) continue;
            int m = (dx < 0 ? -dx : dx) > (dy < 0 ? -dy : dy) ? (dx < 0 ? -dx : dx) : (dy < 0 ? -dy : dy);
            if (m < d) d = m;
        }
        px[y][x] = fill[y][x] ? (y == 3 ? 6 : 7) : d == 1 ? 12 : d == 2 ? 14 : d == 3 ? 6 : 5;
    }
}

static unsigned crc32_of(const unsigned char *p, int n) {
    unsigned c = 0xFFFFFFFFu;
    while (n--) { c ^= *p++; for (int k = 0; k < 8; k++) c = c & 1 ? (c >> 1) ^ 0xEDB88320u : c >> 1; }
    return ~c;
}

/* Las 26 teselas del texto empiezan en 'tile' (sprites en tile, +8, +16 y +24 de 32x16, 32x16, 32x16 y 8x16). */
static void banner(const char *text, unsigned crc_en) {
    int tile = -1;
    for (int i = 0; i < 128 && tile < 0; i++) {
        const unsigned char *o = rt_oam + i * 8;
        unsigned a0 = o[0] | o[1] << 8, a1 = o[2] | o[3] << 8, a2 = o[4] | o[5] << 8;
        if (a0 == 0x4000 && (a1 & 0x1FF) == 32 && (a2 >> 12) == 7 && (a1 >> 14) == 2) tile = a2 & 0x3FF;
    }
    if (tile < 0 || tile > 1024 - 26 || crc32_of(rt_vram + 0x10000 + tile * 32, 26 * 32) != crc_en) return;
    static unsigned char px[16][BW];
    draw_banner(px, text);
    unsigned char *dst = rt_vram + 0x10000 + tile * 32;
    for (int s = 0; s < 4; s++) {
        int tiles_w = s < 3 ? 4 : 1;
        for (int ty = 0; ty < 2; ty++) for (int tx = 0; tx < tiles_w; tx++) {
            int t = s < 3 ? s * 8 + ty * 4 + tx : 24 + ty;
            for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c += 2) {
                int y = ty * 8 + r, x = s * 32 + tx * 8 + c;
                dst[t * 32 + r * 4 + c / 2] = (unsigned char)(px[y][x] | px[y][x + 1] << 4);
            }
        }
    }
}

void gfx_frame(void) {
    if (!gfx_es) return;
    title_press_start();
    banner("ELIGE ARCHIVO", 0xD2B12865u);
    banner("TU NOMBRE", 0x648BB2C5u);
}
