# Plan: The Minish Cap

Estado (2026-11-10): enfoque decidido: motor propio y estático (`..\gba-recomp`). El motor ya lee la ROM y descubre desde la entrada 3998 instrucciones y 125 funciones, y con los punteros de los datos, las tablas de `switch` y el bloque de IWRAM, 262 764 instrucciones (567 KB) y 7470 funciones (`gba-recomp trace roms\minishcap_usa.gba --pointers --overlay 030056F0:080B197C:135C`). La investigación (proyectos existentes y los tres caminos) está en `..\gba-recomp\docs\investigacion.md`.

## Siguiente

1. ~~Decidir el enfoque~~ hecho: propio y estático.
2. Resolver los saltos indirectos (punteros a funciones, tablas de saltos del bucle principal y del manejador de VBlank) para descubrir el resto del código.
3. Perfil `games\minishcap_usa.toml` (hay un borrador con el bloque de IWRAM): entradas, tablas de saltos, ganchos; que el motor lo lea. Falta localizar el origen del mezclador de sonido (`0x0300404C`, copiado a IWRAM por el código de M4A).
4. Runtime mínimo: memoria, I/O, PPU de modo 0, entrada, hasta ver el título.

## Ideas (cuando ya se juegue)

Las mismas mejoras de calidad de vida que en alttp-recomp, apagadas por defecto y sin romper la dificultad; pantalla ancha; traducción al español del texto (propio, nunca copia del volcado de la ROM).
