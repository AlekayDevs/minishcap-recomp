# Comprobaciones de este repositorio y del motor, de menos a mas costosas. Falla (codigo 1) en la primera que no pasa.
#
#   powershell -File scripts\check.ps1 [-Engine ..\gba-recomp] [-NoBuild]
#
# 1. Nada de la ROM ni derivados en lo que git sigue ni en su historial (tools\scan.py del motor).
# 2. Las pruebas del motor (cargo test): decodificadores, perfil, lanzador, aviso de version, SHA-256.
# 3. Pruebas C del runtime (BIOS y estados), si hay TinyCC o GCC.
# 4. Construccion real: gba-port construye el juego desde roms\minishcap_usa.gba y el reproductor queda en la cache
#    (sin ROM a mano se omite; -NoBuild la omite siempre). Usa una cache nueva para que se construya de verdad.
param(
    [string]$Engine = "",
    [string]$Cc = "",
    [switch]$NoBuild
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
if ($Engine -eq "") { $Engine = Join-Path (Split-Path -Parent $root) "gba-recomp" }
$Engine = (Resolve-Path $Engine).Path

function Step($name, [scriptblock]$body) {
    Write-Host "== $name"
    & $body
    if ($LASTEXITCODE -ne 0) { Write-Host "FALLA: $name"; exit 1 }
}

Step "sin ROM ni derivados en git" { python "$Engine\tools\scan.py" --history }
Step "pruebas del motor" {
    Push-Location $Engine
    try {
        # por cmd: cargo escribe en stderr y con $ErrorActionPreference = Stop PowerShell lo tomaria por un error
        $out = cmd /c "cargo test --release 2>&1"
        $code = $LASTEXITCODE
        $out | Select-String "test result|FAILED|error" | ForEach-Object { $_.Line }
    } finally { Pop-Location }
    $global:LASTEXITCODE = $code
}

if (-not $Cc) {
    $candidates = @($env:GBA_PORT_CC, $env:TCC, (Join-Path $root '..\alttp-recomp\toolchain\tcc\tcc.exe'), 'tcc', 'gcc')
    foreach ($candidate in $candidates) {
        if (-not $candidate) { continue }
        if (Test-Path -LiteralPath $candidate -PathType Leaf) { $Cc = (Resolve-Path -LiteralPath $candidate).Path; break }
        $command = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($command) { $Cc = $command.Source; break }
    }
}
if ($Cc) {
    Step "pruebas C del runtime" {
        $runtimeDir = Join-Path $Engine 'runtime'
        $testDir = Join-Path $root 'out\runtime-check'
        New-Item -ItemType Directory -Force -Path $testDir | Out-Null
        $sources = @('gba_cpu.c','gba_bus.c','gba_sys.c','gba_ppu.c','gba_apu.c','gba_state.c','gba_save.c','gba_bios.c') |
            ForEach-Object { Join-Path $runtimeDir $_ }
        foreach ($test in @('bios_test', 'state_test')) {
            $source = Join-Path $runtimeDir "$test.c"
            if (-not (Test-Path -LiteralPath $source)) { Write-Host "omitido: $source no existe en este motor"; continue }
            $program = Join-Path $testDir "$test.exe"
            & $Cc "-I$runtimeDir" -o $program $source @sources
            if ($LASTEXITCODE -ne 0) { return }
            Push-Location $testDir
            try { & $program; $code = $LASTEXITCODE } finally { Pop-Location }
            if ($code -ne 0) { $global:LASTEXITCODE = $code; return }
        }
        $global:LASTEXITCODE = 0
    }
} else { Write-Host '== pruebas C del runtime: omitidas (sin TinyCC/GCC)' }

$rom = Join-Path $root "roms\minishcap_usa.gba"
if ($NoBuild -or -not (Test-Path $rom)) {
    Write-Host "== construccion real: omitida"
} else {
    Step "construccion real desde la ROM" {
        Push-Location $Engine
        try { cmd /c "cargo build --release 2>&1" | Select-Object -Last 1 } finally { Pop-Location }
        $cache = Join-Path $root "out\check-cache"
        if (Test-Path $cache) { Remove-Item -Recurse -Force $cache }
        $exe = & "$Engine\target\release\gba-port.exe" $rom --no-launch --cache $cache --games (Join-Path $root "games") | Select-Object -Last 1
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path $exe)) { $global:LASTEXITCODE = 1; return }
        Write-Host "construido: $exe"
        # lo generado del juego no se queda: es derivado de la ROM
        Remove-Item -Recurse -Force $cache
    }
}
Write-Host "todo bien"
