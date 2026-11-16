# Arma out\release\minishcap-recomp-<version>-win64.zip: gba-port.exe y gba-recomp.exe del motor, su runtime en C, SDL2,
# TinyCC (para que quien solo quiere jugar no instale nada) y games\ de este repositorio. El usuario descomprime, hace
# doble clic en gba-port.exe, elige su ROM una vez y juega.
# No lleva ROMs, partidas guardadas ni C generado: si algo así acaba en la carpeta, el script falla y no crea el zip.
#
#   powershell -File scripts\release.ps1 [-Engine ..\gba-recomp] [-Sdl2 <carpeta con include\ y lib\x64>] [-Tcc <carpeta de TinyCC>] [-Version 0.1.0]
#
# Compila antes el motor (cargo build --release en -Engine). Sin -Sdl2 / -Tcc busca sdl2\ y toolchain\tcc\ en un
# repositorio hermano (alttp-recomp), que son los mismos que usa el motor de SNES.
param(
    [string]$Engine = "",
    [string]$Sdl2 = "",
    [string]$Tcc = "",
    [string]$Version = "0.1.0"
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
if ($Engine -eq "") { $Engine = Join-Path (Split-Path -Parent $root) "gba-recomp" }
$Engine = (Resolve-Path $Engine).Path

foreach ($f in "target\release\gba-port.exe", "target\release\gba-recomp.exe", "runtime\gba_rt.h", "LICENSE") {
    if (-not (Test-Path (Join-Path $Engine $f))) { throw "al motor le falta $f (cargo build --release en $Engine)" }
}
function Find-Sibling($rel) {
    foreach ($d in Get-ChildItem (Split-Path -Parent $root) -Directory) {
        $p = Join-Path $d.FullName $rel
        if (Test-Path $p) { return $p }
    }
    return ""
}
if ($Sdl2 -eq "") { $Sdl2 = Find-Sibling "sdl2" }
if ($Tcc -eq "") { $Tcc = Find-Sibling "toolchain\tcc" }
if (-not $Sdl2 -or -not (Test-Path "$Sdl2\lib\x64\SDL2.dll")) { throw "no encuentro SDL2 (usa -Sdl2 <carpeta con include\ y lib\x64\SDL2.dll>)" }
if (-not $Tcc -or -not (Test-Path "$Tcc\tcc.exe")) { throw "no encuentro TinyCC (usa -Tcc <carpeta con tcc.exe>)" }

$name = "minishcap-recomp-$Version-win64"
$stage = Join-Path $root "out\release\$name"
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force $stage | Out-Null

Copy-Item "$Engine\target\release\gba-port.exe", "$Engine\target\release\gba-recomp.exe" $stage
Copy-Item "$Engine\runtime" $stage -Recurse
Copy-Item "$Engine\LICENSE" "$stage\LICENSE-engine"
Copy-Item $Sdl2 "$stage\sdl2" -Recurse
New-Item -ItemType Directory -Force "$stage\toolchain" | Out-Null
Copy-Item $Tcc "$stage\toolchain\tcc" -Recurse
$aviso = Join-Path (Split-Path -Parent $Tcc) "AVISO-TINYCC.txt"
if (Test-Path $aviso) { Copy-Item $aviso "$stage\toolchain" }
Copy-Item README.md, LICENSE $stage
if (Test-Path README.es.md) { Copy-Item README.es.md $stage }
Copy-Item games $stage -Recurse
# del motor solo va lo que se ejecuta o se compila: fuera las pruebas y utilidades del runtime
Get-ChildItem "$stage\runtime" -File | Where-Object { $_.Name -in "bios_test.c", "gba_test.c", "run_headless.c" } | Remove-Item

# la guarda: nada de ROMs, partidas guardadas ni C generado
$bad = Get-ChildItem $stage -Recurse -File | Where-Object {
    $_.Extension -in ".gba", ".sav", ".srm", ".sst", ".wav", ".ppm" -or $_.Name -match '\.s[1-9]$' -or $_.Name -in "gen.c", "rom.c"
}
if ($bad) {
    $bad | ForEach-Object { Write-Error "prohibido en el paquete: $($_.FullName)" -ErrorAction Continue }
    Remove-Item -Recurse -Force $stage
    throw "el paquete llevaria ficheros de ROM o C generado: no se crea el zip"
}
# y nada grande que no sea del motor (un volcado de la ROM mide megas)
$big = Get-ChildItem "$stage\games", "$stage\runtime" -Recurse -File | Where-Object { $_.Length -gt 2MB }
if ($big) { Remove-Item -Recurse -Force $stage; throw "fichero sospechosamente grande en games\ o runtime\: $($big[0].FullName)" }

$zip = Join-Path $root "out\release\$name.zip"
if (Test-Path $zip) { Remove-Item -Force $zip }
Compress-Archive -Path $stage -DestinationPath $zip
Write-Host "creado $zip ($([math]::Round((Get-Item $zip).Length / 1MB, 1)) MiB)"
