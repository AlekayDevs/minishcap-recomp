# Plan: The Minish Cap

Estado (2026-11-10): enfoque decidido: motor propio y estático (`..\gba-recomp`). El motor ya lee la ROM y descubre 3998 instrucciones y 125 funciones desde la entrada (`gba-recomp trace roms\minishcap_usa.gba`). La investigación (proyectos existentes y los tres caminos) está en `..\gba-recomp\docs\investigacion.md`.

## Siguiente

1. ~~Decidir el enfoque~~ hecho: propio y estático.
2. Resolver los saltos indirectos (punteros a funciones, tablas de saltos del bucle principal y del manejador de VBlank) para descubrir el resto del código.
3. Perfil `games\minishcap_usa.toml`: entradas, tablas de saltos, ganchos.
4. Runtime mínimo: memoria, I/O, PPU de modo 0, entrada, hasta ver el título.

## Ideas (cuando ya se juegue)

Las mismas mejoras de calidad de vida que en alttp-recomp, apagadas por defecto y sin romper la dificultad; pantalla ancha; traducción al español del texto (propio, nunca copia del volcado de la ROM).
