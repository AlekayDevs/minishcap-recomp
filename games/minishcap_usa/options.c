/* Opciones de Minish Cap (ver docs/opciones-del-juego.md del motor). Direcciones: la decompilación zeldaret/tmc las da como
   referencia (linker.ld); cada una se ha comprobado contra la ROM USA jugando antes de usarla. */
#include "gba_sys.h"
#ifdef OPT_DEBUG
#include <stdio.h>
#endif

/* gTextRender (EWRAM 0x02022780): el texto del diálogo. delay y typeSpeed son lo que TextDispUpdate mira cada fotograma. */
#define TEXT_RENDER 0x22780
#define TR_MSG_STATUS 0x88
#define TR_RENDER_STATUS 0x89
#define TR_NEWLINE_DELAY 0x8A
#define TR_TYPE_SPEED 0x92
#define TR_DELAY 0x95

/* gFadeControl (IWRAM 0x03000FD0): los fundidos de pantalla. FadeMain resta speed a progress (de 256 a sustain) cada fotograma. */
#define FADE_CONTROL 0xFD0
#define FC_ACTIVE 0x00
#define FC_SPEED 0x0A
#define FC_PROGRESS 0x0C
#define FC_SUSTAIN 0x0E

static unsigned fc16(int off) { return rt_iwram[FADE_CONTROL + off] | (unsigned)rt_iwram[FADE_CONTROL + off + 1] << 8; }

const GameOpt game_opts[] = {
    {"fast_text", "Fast text", "Texto r\xC3\xA1pido",
     "Skips the pauses and types the text almost at once, while you hold B or always. Does not change the game, only how long you wait.",
     "Salta las pausas y escribe el texto casi de golpe, mientras mantienes B o siempre. No cambia el juego, solo cu\xC3\xA1nto esperas.",
     0, 0, 2, "Off,Hold B,Always", "No,Con B,Siempre"},
    {"fast_fades", "Fast transitions", "Transiciones rápidas",
     "Screen fades (doors, rooms, the title) run two or four times faster. Only the waiting changes: the game is not played during them.",
     "Los fundidos de pantalla (puertas, salas, el título) van dos o cuatro veces más rápido. Solo cambia la espera: durante ellos no se juega.",
     0, 0, 2, "Off,x2,x4", "No,x2,x4"},
};
const int game_opt_count = 2;

/* Cada fotograma resta de progress el mismo paso que FadeMain ya ha restado: el fundido corre 2 o 4 veces mas rapido. Se para en
   sustain, como hace FadeMain, y es FadeMain quien termina el fundido en el fotograma siguiente. */
static void fast_fades(void) {
    int times = rt_opt("fast_fades") ? 2 * rt_opt("fast_fades") : 1; /* 1, 2, 4 */
#ifdef OPT_DEBUG
    static int was, len;
    if (rt_iwram[FADE_CONTROL + FC_ACTIVE]) len++;
    if (was && !rt_iwram[FADE_CONTROL + FC_ACTIVE]) { fprintf(stderr, "fade ended at frame %u after %d frames\n", sys_frames, len); len = 0; }
    was = rt_iwram[FADE_CONTROL + FC_ACTIVE];
#endif
    if (times == 1 || !rt_iwram[FADE_CONTROL + FC_ACTIVE]) return;
    int speed = (short)fc16(FC_SPEED), progress = (short)fc16(FC_PROGRESS), sustain = (short)fc16(FC_SUSTAIN);
    if (speed <= 0 || progress <= sustain) return; /* a negative speed is a deliberate effect of the game: left alone */
    progress -= speed * (times - 1);
    if (progress < sustain) progress = sustain;
    rt_iwram[FADE_CONTROL + FC_PROGRESS] = (unsigned char)progress;
    rt_iwram[FADE_CONTROL + FC_PROGRESS + 1] = (unsigned char)(progress >> 8);
}

extern void gfx_frame(void);
void game_frame(void) {
    gfx_frame();
    fast_fades();
    int mode = rt_opt("fast_text");
    if (mode == 0 || (mode == 1 && !(sys_keys & 2))) return;
    /* solo mientras se escribe un mensaje (msgStatus 2 = MSG_UPDATE, renderStatus 1 = RENDER_UPDATE): fuera de eso la estructura
       la usan otras pantallas (el nombre del jugador) y escribir en ella las cuelga */
    if (rt_ewram[TEXT_RENDER + TR_MSG_STATUS] != 2 || rt_ewram[TEXT_RENDER + TR_RENDER_STATUS] != 1) return;
    rt_ewram[TEXT_RENDER + TR_DELAY] = 0;
    rt_ewram[TEXT_RENDER + TR_NEWLINE_DELAY] = 0;
    rt_ewram[TEXT_RENDER + TR_TYPE_SPEED] = (unsigned char)-40;
}
