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
- [x] Ventana de Windows (2026-11-15; `launcher.rs` y `launcher_win.rs` del motor): elegir ROM, Jugar, progreso, errores con «Copiar detalle», es/en según Windows, subsistema `windows` con consola enganchada. Probada: sin argumentos abre la ventana; soltar una ROM construye, oculta la ventana y abre el juego; un fichero que no es ROM muestra el error. Falta probar a mano el diálogo de elegir ROM y la pantalla «Jugar» con ROM recordada; sin icono todavía.
- [x] Partidas y estados en `saves\` junto a `games\` (`play --saves DIR`; copia una vez el `.sav` que hubiera junto a la ROM). El título de la ventana del juego es el del perfil.
- [x] Diseño del lanzador (2026-11-15), al estilo de alttp-recomp: tema por juego en `[launcher]` del perfil (nombre, colores, icono), marco dorado con rombos, emblema de gorro verde (`scripts\make_icon.py`, arte propio), botones redondeados, errores en rojo.
- [x] Paquete: `scripts\release.ps1` (gba-port.exe, gba-recomp.exe, runtime, SDL2, TinyCC y `games\`; guarda contra ROM, `.sav` y C generado). Probado el zip (4,1 MiB) en una carpeta limpia: construye en 2,8 s con su TinyCC, abre el juego y guarda en `saves\`. SDL2 y TinyCC salen de un repositorio hermano (alttp-recomp) o de `-Sdl2`/`-Tcc`. Falta probarlo en un Windows sin herramientas de desarrollo.
- [x] README del juego (es/en).
- [x] Icono del `.exe` (2026-11-15): `build.rs` del motor escribe el recurso; `release.ps1` compila el motor en `out\engine-target` con `GBA_PORT_ICON` = el icono del juego (comprobado: el exe del paquete lleva el gorro). El manifiesto da DPI y estilo.
- [x] Aviso de versión nueva (`update.rs` del motor): casilla «Buscar actualizaciones» (apagada por defecto), aviso Sí/No/Omitir, descarga del zip de la release, comprobación contra `SHA256SUMS`, instalación sobre la carpeta (las partidas se conservan). `release.ps1` escribe `update.info` (repo de `git remote origin`, versión) y `out\release\SHA256SUMS`. Probado de punta a punta con un servidor local que imita la API (`GBA_PORT_UPDATE_API`): 0.1.0 → 0.2.0 con `gba-port --update`. No probado: hash que no coincide de punta a punta (solo SHA-256 con vectores conocidos), ni contra GitHub de verdad.
- [x] `scripts\check.ps1`: guarda de la ROM (`scan.py --history`), pruebas del motor y construcción real desde la ROM.
- [ ] **Publicar: NO todavía** (decisión del dueño, 2026-11-15). Antes faltan, sin orden fijo:
  - Seguir revisando el juego: jugarlo más tiempo, raíces dinámicas nuevas (`tools\complete.sh`), fallos de PPU, oír el sonido, estados guardados y teclas a mano, rebobinado.
  - Traducción al español del texto (propia, nunca copia del volcado de la ROM).
  - Versión Linux (lanzador nativo con TinyCC y SDL2 dentro, como `..\snes-recomp\linux\`) y paquete `.tar.gz`.
  - Versión Android (como `..\snes-recomp\android\`), con aviso de actualización propio.
  - Web de descarga en GitHub Pages (como alekaydevs.github.io/alttp-recomp).
  - Cuando todo esté: release `v0.1.0` con los tres paquetes (el zip de Windows acaba en `-win64.zip`) y `SHA256SUMS`.

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
