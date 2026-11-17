# Ejecuta el juego con entradas por fotograma y captura su PPU, sin teclado ni ventana.
# Todos los derivados y la partida de prueba quedan en out/translation-visual/<Session>.
# Ejemplo: .\scripts\review_translation.ps1 -Press start@2300+20,a@3100+12 -ShotAt 2250,3000,3500
param(
    [ValidateRange(1, 1000000)][int]$Frames = 3600,
    [string[]]$Press = @('start@2300+20', 'a@3100+12'),
    [int[]]$ShotAt = @(2250, 3000, 3500),
    [ValidatePattern('^[a-zA-Z0-9_-]+$')][string]$Session = 'menus',
    [ValidateSet('es', 'en')][string]$Language = 'es',
    [string]$InputScript = '',
    [string[]]$SaveState = @(),
    [string[]]$LoadState = @(),
    [string[]]$GameOpt = @(),
    [string]$Engine = '',
    [string]$Tcc = ''
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if ($ShotAt.Count -gt 16) { throw 'El motor admite hasta 16 capturas por ejecucion.' }
if ($SaveState.Count + $LoadState.Count -gt 8) { throw 'El motor admite hasta 8 operaciones de estado.' }
foreach ($stateSpec in @($SaveState) + @($LoadState)) {
    if ($stateSpec -notmatch '^(\d+):([a-zA-Z0-9_-]+)$' -or [int]$Matches[1] -lt 1 -or [int]$Matches[1] -gt $Frames) {
        throw "Estado no valido (FOTOGRAMA:nombre): $stateSpec"
    }
}
foreach ($frame in $ShotAt) {
    if ($frame -lt 1 -or $frame -gt $Frames) { throw "Captura fuera de rango: $frame" }
}
foreach ($keyPress in $Press) {
    if ($keyPress -notmatch '^(a|b|select|start|right|left|up|down|r|l)@\d+(\+\d+)?$') {
        throw "Entrada no valida: $keyPress"
    }
}
if ($Engine -eq '') { $Engine = Join-Path $root '..\gba-recomp' }
$Engine = (Resolve-Path -LiteralPath $Engine).Path
if ($Tcc -eq '') { $Tcc = $env:GBA_PORT_CC }
if (-not $Tcc) { $Tcc = Join-Path $root '..\alttp-recomp\toolchain\tcc\tcc.exe' }
$Tcc = (Resolve-Path -LiteralPath $Tcc).Path
if ($InputScript) { $InputScript = (Resolve-Path -LiteralPath $InputScript).Path }
$generator = Join-Path $Engine 'target\release\gba-recomp.exe'
if (-not (Test-Path -LiteralPath $generator)) { throw 'Compila el motor con cargo build --release.' }
$work = Join-Path $root "out\translation-visual\$Session"
New-Item -ItemType Directory -Force -Path $work | Out-Null
$rom = Join-Path $work 'minishcap_usa.gba'
# La ROM y la EEPROM originales no se usan como destino de escritura.
Copy-Item -LiteralPath (Join-Path $root 'roms\minishcap_usa.gba') -Destination $rom
$gen = Join-Path $work 'gen.c'
$run = Join-Path $work 'run.exe'
$gameDir = Join-Path $root 'games\minishcap_usa'
$runtimeDir = Join-Path $Engine 'runtime'
& $generator gen $rom $gen --pointers --profile (Join-Path $root 'games\minishcap_usa.toml')
if ($LASTEXITCODE -ne 0) { throw 'Fallo al generar el juego.' }
$sources = @('gba_cpu.c', 'gba_bus.c', 'gba_sys.c', 'gba_ppu.c', 'gba_apu.c', 'gba_state.c', 'gba_save.c', 'gba_bios.c', 'run_headless.c') |
    ForEach-Object { Join-Path $runtimeDir $_ }
$own = Get-ChildItem -LiteralPath $gameDir -Filter '*.c' | ForEach-Object { $_.FullName }
& $Tcc "-I$runtimeDir" -DRT_GAME -DRT_GAME_OPTS -o $run $gen @sources @own
if ($LASTEXITCODE -ne 0) { throw 'Fallo al compilar el ejecutor de pruebas.' }
$runArgs = @($rom, "$Frames", '--language', $Language, '--game-dir', $gameDir, '--watch', '1000')
foreach ($keyPress in $Press) { $runArgs += @('--press', $keyPress) }
if ($InputScript) { $runArgs += @('--script', $InputScript) }
foreach ($stateSpec in $SaveState) {
    $at, $name = $stateSpec.Split(':'); $runArgs += @('--save-state', "${at}:$work\$name.sst")
}
foreach ($stateSpec in $LoadState) {
    $at, $name = $stateSpec.Split(':'); $runArgs += @('--load-state', "${at}:$work\$name.sst")
}
foreach ($option in $GameOpt) { $runArgs += @('--game-opt', $option) }
foreach ($frame in $ShotAt) { $runArgs += @('--shot-at', "${frame}:$work\frame-$frame.ppm") }
& $run @runArgs
if ($LASTEXITCODE -ne 0) { throw 'El juego no completo la prueba; revisa el error del motor.' }
# Pillow solo convierte las capturas locales; el juego las dibuja con su PPU real.
foreach ($frame in $ShotAt) {
    $ppm = Join-Path $work "frame-$frame.ppm"
    & python -c 'from PIL import Image; import sys; p=sys.argv[1]; Image.open(p).resize((960,640), Image.Resampling.NEAREST).save(p[:-4]+".png")' $ppm
    if ($LASTEXITCODE -ne 0) { throw 'No se pudo convertir la captura; hace falta Python con Pillow.' }
}
Write-Host "Capturas y partida de prueba: $work"
