# Comprueba games/minishcap_usa/lang/es.txt contra el volcado del inglés (lang/en.dump.txt, de tu ROM):
# 1) cada mensaje traducido conserva los mismos códigos de control {..} en el mismo orden;
# 2) ninguna línea pasa de $Max caracteres visibles. Uso: powershell -File scripts\check_text.ps1 [-Max 34] [-Group N]
param([int]$Max = 42, [int]$Group = -1)
$dir = Join-Path $PSScriptRoot '..\games\minishcap_usa\lang'
function Load($p) {
  $h = @{}; $k = $null
  foreach ($l in [IO.File]::ReadAllLines($p, [Text.Encoding]::UTF8)) {
    if ($l.StartsWith('@')) { $i = $l.IndexOf(' '); if ($i -lt 0) { $k = $l; $h[$k] = '' } else { $k = $l.Substring(0, $i); $h[$k] = $l.Substring($i + 1) } }
  }
  $h
}
function Codes($t) {
  # tokens {..} sin las letras latinas ({C0}..{F7}, que en es.txt pueden ir como letra directa)
  $t = [regex]::Replace($t, '\{02\}\{01\}\s*\{02\}\{00\}', '')
  ([regex]::Matches($t, '\{[0-9A-Fa-f ]+\}') | ForEach-Object { $_.Value.ToUpper() -replace ' ', '' } |
    Where-Object { -not ($_ -match '^\{(C[0-9A-F]|D[0-9A-F]|E[0-9A-F]|F[0-7]|A1|BF)\}$') }) -join ''
}
$en = Load (Join-Path $dir 'en.dump.txt'); foreach ($q in @($en.Keys)) { $en[$q] = $en[$q] -replace '(\{00\})(\{FF\})+$', '' }; $es = Load (Join-Path $dir 'es.txt')
$bad = 0; $n = 0
foreach ($k in $es.Keys | Sort-Object { [int]($_.Substring(1).Split(':')[0]) * 10000 + [int]$_.Split(':')[1] }) {
  if ($Group -ge 0 -and -not $k.StartsWith("@$($Group):")) { continue }
  $n++
  if (-not $en.ContainsKey($k)) { "$k no existe en el inglés"; $bad++; continue }
  if ((Codes $en[$k]) -ne (Codes $es[$k])) { "$k códigos distintos: EN [$(Codes $en[$k])] ES [$(Codes $es[$k])]"; $bad++ }
  foreach ($ln in ($es[$k] -split '\\n')) {
    $v = [regex]::Replace($ln, '\{[0-9A-Fa-f ]+\}', '')
    if ($v.Length -gt $Max) { "$k línea de $($v.Length): $v"; $bad++ }
  }
}
"$n mensajes comprobados, $bad avisos"
