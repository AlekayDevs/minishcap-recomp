# Plan: The Minish Cap

Estado (2026-11-10): enfoque decidido: motor propio y estático (`..\gba-recomp`). El motor ya lee la ROM y descubre desde la entrada 3998 instrucciones y 125 funciones, y con los punteros de los datos, las tablas de `switch` y el bloque de IWRAM, 258 487 instrucciones (559 KB), 7088 funciones y solo 15 saltos indirectos dinámicos sin resolver (`gba-recomp trace roms\minishcap_usa.gba --pointers --overlay 030056F0:080B197C:135C`). La investigación (proyectos existentes y los tres caminos) está en `..\gba-recomp\docs\investigacion.md`.

El plan completo del motor (analizador, generación de C, runtime, herramientas y riesgos) está en `..\gba-recomp\plan.md`; aquí va lo específico del juego.

## Siguiente

1. ~~Decidir el enfoque~~ hecho: propio y estático.
2. Validar los punteros candidatos (7 «indefinidas» en datos) y los punteros a funciones ARM; luego, el despacho dinámico para los 15 saltos que no se resuelven en estático.
3. Perfil `games\minishcap_usa.toml` (hay un borrador con el bloque de IWRAM): entradas, tablas de saltos, ganchos; que el motor lo lea. Ya incluye el bloque del mezclador de sonido (`0x0300404C`).
4. Runtime mínimo: memoria, I/O, PPU de modo 0, entrada, hasta ver el título.

## Lanzador (como alttp-recomp)

Meta: descomprimir, doble clic, elegir la ROM una vez y **Jugar**. Se hace en el motor (`..\gba-recomp`), copiando el diseño de `..\snes-recomp` (`port.rs`, `launcher.rs`, `launcher_win.rs`, `update.rs`).
- [x] `gba-port <rom.gba>` (2026-11-15, consola): identifica la ROM por CRC32 con `games\*.toml`, genera el C, compila con TinyCC + runtime + SDL2 (2,7 s), caché por juego/CRC/versión, recuerda la última ROM. Prueba: `GBA_PORT_CACHE=<dir> gba-port roms\minishcap_usa.gba [--no-launch]`.
- [ ] Ventana de Windows (elegir ROM, Jugar, progreso, errores, idioma es/en) y subsistema `windows` con consola enganchada.
- [ ] Partidas en `saves\` junto al exe (hoy `play` escribe `<rom>.sav` junto a la ROM).
- [ ] Paquete: `scripts\release.ps1` (motor + `games\` + TinyCC + SDL2, sin ROM ni C generado) y `scripts\update-engine.ps1`.
- [ ] Aviso de versión nueva, icono, README del juego (es/en), `scripts\check.ps1`.

## Ideas (cuando ya se juegue)

Las mismas mejoras de calidad de vida que en alttp-recomp, apagadas por defecto y sin romper la dificultad; pantalla ancha; traducción al español del texto (propio, nunca copia del volcado de la ROM).

## Pendiente específico del juego

- Perfil `games\minishcap_usa.toml`: pasar de borrador a lo que lea el motor; añadir las tablas de saltos conocidas, los ganchos y los nombres de símbolos propios.
- Confirmar con el desensamblado qué SWI de la BIOS usa el juego y cuáles hay que implementar primero (los que salieron: 0, 1, 3, 5, 6, 8, 11, 14, 15, 17, 18, 25).
- Guardado EEPROM del juego (512 B u 8 KiB: comprobarlo con los accesos DMA del juego).
- Otras regiones (EU, JP) más adelante; la ROM de esta carpeta es la USA (SHA-1 `b4bd50e4...`, CRC32 `ABCEBBB1`).
- Mejoras de calidad de vida (solo ahorran pasos, apagadas por defecto), pantalla ancha, traducción propia al español del texto (nunca copia del volcado de la ROM), menú del port con estilo propio.
- `scripts\check.ps1` y `scripts\check.sh`; `scripts\release.ps1` y `scripts\update-engine.ps1` como en alttp-recomp.
- README del juego con cómo poner la ROM y cómo jugar.
