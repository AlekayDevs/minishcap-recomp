/* Opciones de Minish Cap (ver docs/opciones-del-juego.md del motor). Direcciones: la decompilación zeldaret/tmc las da como
   referencia (linker.ld); cada una se ha comprobado contra la ROM USA jugando antes de usarla. */
#include "gba_sys.h"

/* gTextRender (EWRAM 0x02022780): el texto del diálogo. delay y typeSpeed son lo que TextDispUpdate mira cada fotograma. */
#define TEXT_RENDER 0x22780
#define TR_MSG_STATUS 0x88
#define TR_RENDER_STATUS 0x89
#define TR_NEWLINE_DELAY 0x8A
#define TR_TYPE_SPEED 0x92
#define TR_DELAY 0x95

const GameOpt game_opts[] = {
    {"fast_text", "Fast text", "Texto r\xC3\xA1pido",
     "Skips the pauses and types the text almost at once, while you hold B or always. Does not change the game, only how long you wait.",
     "Salta las pausas y escribe el texto casi de golpe, mientras mantienes B o siempre. No cambia el juego, solo cu\xC3\xA1nto esperas.",
     0, 0, 2, "Off,Hold B,Always", "No,Con B,Siempre"},
};
const int game_opt_count = 1;

void game_frame(void) {
    int mode = rt_opt("fast_text");
    if (mode == 0 || (mode == 1 && !(sys_keys & 2))) return;
    /* solo mientras se escribe un mensaje (msgStatus 2 = MSG_UPDATE, renderStatus 1 = RENDER_UPDATE): fuera de eso la estructura
       la usan otras pantallas (el nombre del jugador) y escribir en ella las cuelga */
    if (rt_ewram[TEXT_RENDER + TR_MSG_STATUS] != 2 || rt_ewram[TEXT_RENDER + TR_RENDER_STATUS] != 1) return;
    rt_ewram[TEXT_RENDER + TR_DELAY] = 0;
    rt_ewram[TEXT_RENDER + TR_NEWLINE_DELAY] = 0;
    rt_ewram[TEXT_RENDER + TR_TYPE_SPEED] = (unsigned char)-40;
}
