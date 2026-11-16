# El texto del juego (USA) y cómo traducirlo

Notas de la investigación del 2026-11-15. Solo direcciones, tamaños y formato: ningún texto ni byte de la ROM.

## Dónde está y cómo se organiza

- Los mensajes son ASCII con códigos de control, terminados en `00`, en `0x9B1D90`..`0x9FB000` de la ROM (≈ 330 KB). Los códigos de control (`01`..`1F`, p. ej. `0A` = salto de línea) llevan parámetros; hay bytes altos (`B7`..`BA`, `F8`..`FF`) que son parámetros o símbolos, no letras.
- **Tabla superior** en `0x9B1D90`: 80 entradas `u32`; cada una es el desplazamiento, desde `0x9B1D90`, de un **grupo** (la primera entrada, `0x140`, da el grupo de `0x9B1ED0`).
- **Grupo**: una tabla de `u32` (desplazamientos, desde el inicio del grupo, de cada mensaje; el primero es `4 × n`) seguida de los mensajes. En total 3699 mensajes; el más largo mide 428 bytes.
- Un mensaje se identifica por (grupo, índice). Todo es relativo, así que el banco entero se puede reconstruir en otro sitio.
- En `0x109214` hay 7 punteros iguales (`0x089B1D90`), probablemente uno por idioma: redirigir ese puntero (o los 7) a un banco nuevo cambia el texto sin tocar el código del juego.
- La ROM acaba con `0x21825C` bytes libres (`FF`) desde `0xDE7DA4`: cabe un banco en español sin ampliar la ROM ni tocar la decodificación de la EEPROM del bus.

## Idea de implementación (sin ganchos)

Al cargar la ROM, el runtime llama a un C del juego (`games/minishcap_usa/text.c`) que:
1. lee `lang/es.txt` (texto propio, UTF-8, clave `@GG:MMM`, códigos de control entre llaves);
2. reconstruye el banco: el texto traducido si lo hay y, si no, el inglés de la ROM del usuario (en memoria);
3. lo escribe en el hueco libre del final y cambia el puntero de `0x109214`.

Nada de esto se guarda en el repositorio salvo `es.txt`. Un volcado del inglés (`gba-recomp text-dump`, por hacer) se escribe junto a la ROM del usuario y no se sube.

## La fuente (hallada el 2026-11-15 con el trazador del motor)

- Tabla de glifos en la ROM: el glifo del código `c` está en `0x692F60 + 64·c`. Cada glifo son **dos teselas de 4bpp** (32 bytes cada una, la de arriba y la de abajo): 8 px de ancho y 16 de alto. Comprobado: espacio (`0x20`) en `0x693760`, `A` (`0x41`) en `0x693FA0`, `a` (`0x61`) en `0x6947A0`, que son justo los bloques que el juego lee al dibujar.
- Orden de la tabla: los códigos `0x20`..`0x7F` siguen el ASCII (con kana ocupando los signos entre `9` y `A`, como `:`, y otros), y la tabla sigue con kana y símbolos hasta `0xFF` (se lee hasta `0x696F60`). Ningún glifo está en blanco.
- El juego compone el texto en un búfer de EWRAM (`0x02036AD0`..) y luego lo copia a VRAM (BG0, bloque de caracteres 3, `0xC000`); no escribe los glifos directamente en VRAM. Los colores (tinta 14, fondo 15, 3 para el verde) salen de la paleta.
- Los anchos no vienen de una tabla (por confirmar): el juego los mide por columnas vacías, así que un glifo dibujado a mano encaja solo.
- **Los acentos ya están en la ROM USA**: los códigos `>= 0x80` leen sus glifos de una segunda tabla (`0x697CE0 + 64·(código − 0x90)`) con la disposición de Latin-1: `À`…`ß` en `0xC0`–`0xDF`, `à`…`ÿ` en `0xE0`–`0xFF`, y también `¡` (`0xA1`), `¿` (`0xBF`), `ª º « »`. Así que un carácter U+00A0–U+00FF se escribe con el byte de su punto de código; no hay que dibujar nada (comprobado en pantalla en el cuadro de diálogo). El texto del intro (vidrieras) usa otra fuente que no se ha mirado.

## Herramienta del motor para repetirlo

`run_headless --romstat DESDE:HASTA` (compilado con `-DRT_ROMSTAT -DROM_SHIFT=6`) cuenta las lecturas de ROM por bloque; con `VTRACE=lo:hi:n` imprime las escrituras de VRAM y con `WTRACE=valor:n` las de RAM (o `FFFFFFFF:n` para el búfer de texto), cada una con las últimas lecturas de ROM (sin el audio, que es `>= 0xA00000`).

## Cómo se aplica (`games/minishcap_usa/text.c`, hecho)

- El motor compila `games/<id>/*.c` con `-DRT_GAME` y el runtime llama a `game_init(rom, len, carpeta, idioma)` tras cargar la ROM en memoria (`--game-dir` y `--language`; `gba-port` los pasa, con `GBA_PORT_LANGUAGE=es` por ahora).
- `--language dump` escribe el inglés de la ROM del usuario en `games/minishcap_usa/lang/en.dump.txt` (ignorado por git) en el formato de `es.txt` y termina. Es la base para traducir y no se sube.
- `--language es` lee `lang/es.txt` (`@GRUPO:MENSAJE texto`, `
` salto de línea, `{06 00}` bytes sueltos, `#` comentarios; UTF-8 con `á é í ó ú ñ ü ¿ ¡` y mayúsculas acentuadas), reconstruye el banco en el hueco libre (`0xE00000`) con el español donde lo haya y el inglés donde no, y redirige los 7 punteros de `0x109214`.
- Los mensajes no se separan por el primer `00` (los códigos de control llevan parámetros `00`): los límites los dan las tablas de desplazamientos.

## Lo que falta saber

- **Anchos y líneas**: el cuadro del diálogo tiene dos líneas visibles y el ancho está limitado (el español es más largo); hace falta un comprobador (como `scripts/check_text.py` de alttp-recomp) con los anchos de la fuente. Los anchos de los glifos parecen medirse en ejecución.
- **Códigos de control**: qué hace cada uno y sus parámetros (nombre del jugador `{06 00}`, colores `{02 02}`/`{02 00}`, pausas, opciones, objetos).
- **Texto fuera de los mensajes**: menús, pantalla de título, nombres de objetos, vidrieras del intro (otra fuente) y mapa pueden venir de gráficos o de otras tablas.
- **Selector de idioma** en el lanzador (por ahora `GBA_PORT_LANGUAGE=es`).
- **Volumen**: 3699 mensajes, a traducir por grupos empezando por lo que se ve primero.
