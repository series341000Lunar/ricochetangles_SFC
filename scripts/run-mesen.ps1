[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$mesenPath = 'C:\Users\LunarGagarin\Documents\MesenCE\Mesen.exe'
$repoRootOutput = & git rev-parse --show-toplevel
if ($LASTEXITCODE -ne 0) {
    throw 'Unable to resolve the Git repository root.'
}

$repoRoot = [System.IO.Path]::GetFullPath(($repoRootOutput | Select-Object -First 1).Trim())
$romPath = Join-Path $repoRoot 'build\rom\ricochetangles_s0_hello.sfc'

if (-not (Test-Path -LiteralPath $mesenPath)) {
    throw "MesenCE is missing: $mesenPath"
}
if (-not (Test-Path -LiteralPath $romPath)) {
    throw "ROM is missing; run scripts/build.ps1 first: $romPath"
}

& (Join-Path $repoRoot 'tools\verify-rom.ps1') -RomPath $romPath
if ($LASTEXITCODE -ne 0) {
    throw "ROM verification failed with exit code $LASTEXITCODE."
}

$process = Start-Process -FilePath $mesenPath -ArgumentList @($romPath) -PassThru
Write-Output "MESEN_PID=$($process.Id)"
Write-Output "ROM=$romPath"

