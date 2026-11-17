"""Dibuja el icono del lanzador (arte propio, ningún gráfico del juego) y escribe games/minishcap_usa/icon.ico.

    python scripts/make_icon.py [--preview out.png]

Necesita Pillow. El .ico se sube al repositorio, así que compilar o empaquetar no necesita Python.
Un gorro verde de punta caída sobre un fondo verde oscuro con aro dorado.
"""
import pathlib, sys
from PIL import Image, ImageDraw

ROOT = pathlib.Path(__file__).resolve().parent.parent
N = 1024


def lerp(a, b, t):
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))


def bez(p0, p1, p2, n=40):
    return [((1 - t) ** 2 * p0[0] + 2 * (1 - t) * t * p1[0] + t * t * p2[0], (1 - t) ** 2 * p0[1] + 2 * (1 - t) * t * p1[1] + t * t * p2[1]) for t in [i / n for i in range(n + 1)]]


def draw():
    s = N / 256.0
    S = lambda pts: [(x * s, y * s) for x, y in pts]
    img = Image.new("RGBA", (N, N), (0, 0, 0, 0))
    grad = Image.new("RGBA", (N, N))
    gd = ImageDraw.Draw(grad)
    for y in range(N):
        gd.line([(0, y), (N, y)], fill=lerp((22, 58, 46), (10, 30, 24), y / N) + (255,))
    mask = Image.new("L", (N, N), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, N - 1, N - 1], radius=int(N * 0.22), fill=255)
    img.paste(grad, (0, 0), mask)
    d = ImageDraw.Draw(img)
    gold, gold_dark = (232, 197, 71), (160, 124, 32)
    d.rounded_rectangle([N * 0.03, N * 0.03, N * 0.97, N * 0.97], radius=int(N * 0.19), outline=gold + (255,), width=int(N * 0.028))
    # sombra
    sh = Image.new("RGBA", (N, N), (0, 0, 0, 0))
    # cuerpo del gorro: borde izquierdo hasta la punta, punta caída, borde derecho hasta la base
    left = bez((66, 176), (70, 70), (176, 46))
    tip = bez((176, 46), (226, 50), (226, 112))
    right = bez((226, 112), (190, 92), (190, 176))
    body = S(left + tip[1:] + right[1:])
    ImageDraw.Draw(sh).polygon([(x, y + 10 * s) for x, y in body], fill=(0, 0, 0, 100))
    img = Image.alpha_composite(img, sh)
    d = ImageDraw.Draw(img)
    green, green_dark, green_light = (88, 184, 72), (38, 120, 54), (150, 224, 110)
    d.polygon(body, fill=green + (255,))
    # sombra propia de la mitad derecha y brillo de la izquierda
    shade = S(bez((176, 46), (200, 80), (190, 176)) + [(190, 176), (130, 176)] + bez((130, 176), (150, 110), (176, 46))[1:])
    d.polygon(shade, fill=green_dark + (255,))
    hi = S(bez((84, 150), (88, 84), (158, 56)) + bez((158, 56), (110, 80), (104, 150))[1:])
    d.polygon(hi, fill=green_light + (255,))
    edge = lerp(green_dark, (0, 0, 0), 0.5) + (255,)
    d.line(body + [body[0]], fill=edge, width=int(N * 0.012), joint="curve")
    # ala dorada
    d.rounded_rectangle([60 * s, 168 * s, 196 * s, 196 * s], radius=int(14 * s), fill=gold + (255,), outline=lerp(gold_dark, (0, 0, 0), 0.3) + (255,), width=int(N * 0.01))
    d.rounded_rectangle([66 * s, 188 * s, 190 * s, 194 * s], radius=int(3 * s), fill=gold_dark + (255,))
    return img


def main():
    img = draw()
    out = ROOT / "games" / "minishcap_usa" / "icon.ico"
    out.parent.mkdir(parents=True, exist_ok=True)
    img.save(out, sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    print("escrito", out)
    # el emblema del menú del reproductor: 32x32, BMP de 24 bits con magenta como color transparente (SDL lo lee sin más)
    small = img.resize((32, 32), Image.LANCZOS)
    bmp = Image.new("RGB", (32, 32), (255, 0, 255))
    px, src = bmp.load(), small.load()
    for y in range(32):
        for x in range(32):
            r, g, b, a = src[x, y]
            if a >= 128:
                px[x, y] = (r, g, b) if (r, g, b) != (255, 0, 255) else (254, 0, 255)
    bmp.save(out.parent / "emblem.bmp")
    print("escrito", out.parent / "emblem.bmp")
    if "--preview" in sys.argv:
        img.resize((256, 256), Image.LANCZOS).save(sys.argv[sys.argv.index("--preview") + 1])


main()
