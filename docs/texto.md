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

## Lo que falta saber

- **La fuente**: dónde están los glifos y la tabla de anchos, y si hay sitio para `á é í ó ú ñ ¿ ¡` (alttp dibuja los acentos en tiempo de ejecución con las marcas sobre las letras de la propia fuente). El diálogo no está en el bloque de caracteres de BG0 (`BGCNT` 1F0C); está en otra capa o en OBJ.
- **Anchos**: las líneas del cuadro de diálogo tienen un límite en píxeles; el español es más largo y hay que comprobarlo con la tabla de anchos de la ROM (como `scripts/check_text.py` de alttp-recomp).
- **Códigos de control**: qué hace cada uno y sus parámetros (nombre del jugador, colores, pausas, opciones Sí/No, objetos).
- **Texto fuera de los mensajes**: menús, pantalla de título, nombres de objetos y mapa pueden venir de gráficos o de otras tablas.
- **Volumen**: 3699 mensajes. Hay que traducirlos por grupos, empezando por lo que se ve primero (menú de archivo, inicio, primeras zonas).
