#!/usr/bin/env bash
# Equivalente en Linux de scripts/check.ps1, de menos a más costoso; falla (código 1) en la primera que no pasa:
#   1. Nada de la ROM ni derivados en lo que git sigue ni en su historial (tools/scan.py del motor).
#   2. El C propio de games/ compila contra el runtime del motor y el perfil es TOML válido.
#   3. Las pruebas del motor (cargo test).
#   4. Construcción real: gba-port construye el juego desde roms/minishcap_usa.gba y deja el reproductor en una caché nueva
#      (sin ROM, sin gcc o con --no-build se omite).
#
#   scripts/check.sh [--no-build] [ruta/a/gba-recomp]     (el motor se busca en ../gba-recomp)
set -u
cd "$(dirname "$0")/.." || exit 1
root=$PWD
nobuild=0
engine=""
for a in "$@"; do
    case "$a" in --no-build) nobuild=1 ;; *) engine=$a ;; esac
done
engine=$(readlink -f "${engine:-../gba-recomp}")
[ -f "$engine/runtime/gba_rt.h" ] || { echo "error: no encuentro el motor en $engine" >&2; exit 1; }
step() { echo "== $1"; }
fail() { echo "FALLA: $1" >&2; exit 1; }

step "sin ROM ni derivados en git"
python3 -I "$engine/tools/scan.py" --history || fail "scan.py"

step "perfil TOML y C propio"
python3 -c "import tomllib; tomllib.load(open('games/minishcap_usa.toml','rb'))" || fail "el perfil no es TOML válido"
if command -v gcc >/dev/null 2>&1; then
    for f in games/minishcap_usa/*.c; do
        gcc -std=gnu11 -Wall -Wno-comment -Werror -fsyntax-only -I"$engine/runtime" -DRT_GAME -DRT_GAME_OPTS "$f" || fail "$f no compila"
    done
else
    echo "omitido: no hay gcc"
fi

step "pruebas del motor"
(cd "$engine" && cargo test --release 2>&1 | grep -E "test result|FAILED|error"; exit "${PIPESTATUS[0]}") || fail "cargo test"

rom=roms/minishcap_usa.gba
if [ "$nobuild" = 1 ] || [ ! -f "$rom" ]; then
    echo "== construcción real: omitida"
else
    step "construcción real desde la ROM"
    (cd "$engine" && cargo build --release 2>&1 | tail -1)
    cache="$root/out/check-cache"
    rm -rf "$cache"
    exe=$("$engine/target/release/gba-port" "$rom" --no-launch --cache "$cache" --games "$root/games" | tail -1) || fail "gba-port"
    [ -x "$exe" ] || fail "no se construyó el reproductor"
    echo "construido: $exe"
    rm -rf "$cache" # lo generado del juego no se queda: deriva de la ROM
fi
echo "todo bien"
