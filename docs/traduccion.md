# Traducción al español: criterios y glosario

Texto propio, escrito a mano a partir de lo que dice el juego (nunca copiado de otra traducción ni del volcado). Se traduce el **sentido** y el tono, no palabra por palabra: si una frase literal suena rara, se reescribe como lo diría alguien en español. Tuteo siempre. Los nombres propios no se traducen; los apodos o nombres descriptivos sí.

## Reglas técnicas

- Los códigos entre llaves (`{06}{00}` nombre del jugador, `{02}{01}` color, `{05}{03}..` cursor de opciones, etc.) se copian tal cual y en el mismo orden relativo; el byte que sigue a un código puede parecer una letra (`!`, `%`): es un parámetro, no se toca.
- Los saltos `\n` se rehacen para que cada línea quepa (el inglés llega a unos 36 caracteres por línea; en español se intenta no pasar de 34) y se conserva el número de líneas del cuadro cuando se puede.
- Letras con acento, `ñ`, `¿` y `¡` van directas en UTF-8.

## Glosario

| Inglés | Español |
|---|---|
| Minish / Picori | Minish / Picori (festival: Festival Picori) |
| Kinstone / Kinstone Piece / Kinstone Bag | piedra de fusión / fragmento de fusión / bolsa de fusión |
| Heart Container / Piece of Heart | Contenedor de Corazón / Pieza de Corazón |
| Rupee | rupia |
| Picolyte | picolito |
| Gust Jar | Vasija Vendaval |
| Cane of Pacci | Bastón de Pacci |
| Mole Mitts | Guantes de Topo |
| Roc's Cape | Capa del Roc |
| Pegasus Boots | Botas de Pegaso |
| Fire Rod | Vara de Fuego |
| Ocarina of Wind | Ocarina del Viento |
| Flame Lantern | Linterna de Fuego |
| Power Bracelets / Grip Ring / Flippers | Brazaletes de Poder / Anillo de Agarre / Aletas |
| Boomerang | Bumerán |
| Four Sword | Espada Cuádruple |
| Smith's Sword | Espada del Herrero |
| Nayru's / Farore's / Din's Charm | Amuleto de Nayru / de Farore / de Din |
| Spin / Roll / Dash Attack | Ataque Giratorio / Rodante / Veloz |
| Rock Breaker / Sword Beam / Down Thrust | Rompepiedras / Rayo de Espada / Estocada Descendente |
| Mt. Crenel / Lon Lon Ranch | Monte Crenel / Rancho Lon Lon |
| Cucco | cuco |
| Swordsman's newsletter | Boletín del Espadachín |

## Rótulos que son imágenes (`games/minishcap_usa/gfx.c`)

"PRESS START" y las cabeceras de los menús ("CHOOSE A FILE", "ENTER A NAME") no son mensajes sino sprites de la ROM. Con
`language=es`, `gfx_frame()` (una vez por fotograma, desde `game_frame()`) los reconoce en la OAM —y, las cabeceras, por el
CRC32 de sus teselas en inglés— y escribe encima teselas dibujadas con un alfabeto propio (nada se copia de la ROM):
"PULSA START", "ELIGE ARCHIVO" y "TU NOMBRE". Pendiente: los botones ABC/abc/END de la pantalla de nombre, el menú de copiar
y borrar archivos, el logotipo (se deja) y el resto de imágenes con texto.
