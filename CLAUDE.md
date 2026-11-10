# minishcap-recomp

*The Legend of Zelda: The Minish Cap* (USA, SHA-1 `b4bd50e4131b027c334547b4524e2dbbd4227130`, 16 MiB, guardado EEPROM) recompilado con el motor `gba-recomp` (repositorio hermano `..\gba-recomp`). Este repositorio es el **juego**: perfil, C propio y scripts. Repositorios hermanos de referencia: `..\snes-recomp` y `..\alttp-recomp` (misma estructura: motor general + perfil `games\<id>.toml` + `games\<id>\*.c`).

## Estructura

- `roms\minishcap_usa.gba`: la ROM del usuario (en `.gitignore`).
- `games\`: perfil (direcciones, nombres, tamaños; ningún byte del juego) y C propio. Vacío por ahora.
- `scripts\`: comprobaciones y empaquetado (por hacer).
- `docs\`: notas.
- `plan.md`: el plan y el estado.

## Reglas

- Nunca la ROM ni nada derivado: `roms/`, `out/`, `*.gba`, `*.srm`, `*.sav`, `*.sst` están en `.gitignore`. Antes de cada push hay que comprobar que no se sube nada de la ROM.
- El C es original (clean-room). La decompilación `zeldaret/tmc` y los proyectos que la usan (`999sian/tmc`, `mstan/MinishCapRecomp`) solo son referencia; de la decompilación, como mucho nombres de símbolos, nunca su C.
- Lo del traductor, la PPU, el sonido o el runtime va en `gba-recomp`; aquí solo lo específico de este juego.
- Documentación y mensajes de commit en español.
