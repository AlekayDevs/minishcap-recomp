# Reajusta las líneas de es.txt que pasan de $Max caracteres visibles (corta en el último espacio y pasa el resto a la línea siguiente).
# Salta los grupos 1 y 3 (créditos y boletín, pantallas propias). Uso: powershell -File scripts\rewrap.ps1 [-Max 42]
param([int]$Max = 42)
$p = Join-Path $PSScriptRoot '..\games\minishcap_usa\lang\es.txt'
$raw = [IO.File]::ReadAllText($p, [Text.Encoding]::UTF8)
$crlf = $raw.Contains("`r`n")
$lines = $raw -split "`r?`n"
function Vis($s) { [regex]::Replace($s, '\{[0-9A-Fa-f ]+\}', '').Length }
$out = New-Object System.Collections.Generic.List[string]
foreach ($l in $lines) {
  if (-not ($l -match '^@(\d+):\d+ ') -or $Matches[1] -eq '1' -or $Matches[1] -eq '3') { $out.Add($l); continue }
  $i = $l.IndexOf(' '); $head = $l.Substring(0, $i); $body = $l.Substring($i + 1)
  $parts = New-Object System.Collections.ArrayList
  [void]$parts.AddRange(@($body -split '\n'))
  $j = 0
  while ($j -lt $parts.Count) {
    $s = [string]$parts[$j]
    if ((Vis $s) -gt $Max) {
      $cut = -1
      for ($c = $s.Length - 1; $c -gt 0; $c--) {
        if ($s[$c] -eq ' ' -and (Vis $s.Substring(0, $c)) -le $Max -and $s.Substring(0, $c).Trim().Length -gt 0) { $cut = $c; break }
      }
      if ($cut -gt 0) {
        $rest = $s.Substring($cut + 1); $parts[$j] = $s.Substring(0, $cut)
        if ($j + 1 -lt $parts.Count -and ([string]$parts[$j + 1]).Trim().Length -gt 0 -and -not ([string]$parts[$j + 1]).StartsWith('{01}') -and -not $s.Contains('{01}')) { $parts[$j + 1] = $rest + ' ' + $parts[$j + 1] }
        else { $parts.Insert($j + 1, $rest) }
      }
    }
    $j++
  }
  $out.Add($head + ' ' + ($parts -join '\n'))
}
$nl = if ($crlf) { "`r`n" } else { "`n" }
[IO.File]::WriteAllText($p, ($out -join $nl), (New-Object Text.UTF8Encoding $false))
