# Plan: The Minish Cap

Estado (2026-11-10): solo las carpetas. La investigación (proyectos existentes y los tres caminos) está en `..\gba-recomp\docs\investigacion.md`.

## Siguiente

1. Decidir el enfoque con la investigación (recomendado: un spike propio en `gba-recomp` y comparar con `mstan/gbarecomp`).
2. Spike: leer la ROM, desensamblar ARM/Thumb desde el punto de entrada ($08000000), localizar el bucle principal y el manejador de VBlank.
3. Perfil `games\minishcap_usa.toml`: entradas, tablas de saltos, ganchos.
4. Runtime mínimo: memoria, I/O, PPU de modo 0, entrada, hasta ver el título.

## Ideas (cuando ya se juegue)

Las mismas mejoras de calidad de vida que en alttp-recomp, apagadas por defecto y sin romper la dificultad; pantalla ancha; traducción al español del texto (propio, nunca copia del volcado de la ROM).
