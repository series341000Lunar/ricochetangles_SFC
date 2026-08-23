[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$msysRoot = 'C:\msys64'
$bashPath = Join-Path $msysRoot 'usr\bin\bash.exe'
$makePath = Join-Path $msysRoot 'usr\bin\make.exe'
$pvsneslibWindowsPath = 'C:\snesdev\pvsneslib-4.6.0'
$pvsneslibMsysPath = '/c/snesdev/pvsneslib-4.6.0'

function ConvertTo-MsysPath {
    param([Parameter(Mandatory)][string]$WindowsPath)

    $fullPath = [System.IO.Path]::GetFullPath($WindowsPath)
    if ($fullPath -notmatch '^([A-Za-z]):\\(.*)$') {
        throw "Cannot convert path to MSYS2 form: $fullPath"
    }

    $drive = $Matches[1].ToLowerInvariant()
    $tail = $Matches[2].Replace('\\', '/')
    return "/$drive/$tail"
}

function Restore-ProcessEnvironmentVariable {
    param(
        [Parameter(Mandatory)][string]$Name,
        [AllowNull()][string]$Value
    )

    [System.Environment]::SetEnvironmentVariable($Name, $Value, 'Process')
}

foreach ($requiredPath in @(
    $bashPath,
    $makePath,
    (Join-Path $pvsneslibWindowsPath 'devkitsnes\snes_rules')
)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "Required S0-01 tool is missing: $requiredPath"
    }
}

$repoRootOutput = & git rev-parse --show-toplevel
if ($LASTEXITCODE -ne 0) {
    throw 'Unable to resolve the Git repository root.'
}

$repoRoot = [System.IO.Path]::GetFullPath(($repoRootOutput | Select-Object -First 1).Trim())
$buildRoot = [System.IO.Path]::GetFullPath((Join-Path $repoRoot 'build'))
$expectedPrefix = $repoRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar
if (-not $buildRoot.StartsWith($expectedPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Unsafe build path: $buildRoot"
}

$workRoot = Join-Path $buildRoot 'work'
$workSourceRoot = Join-Path $workRoot 'src'
$romRoot = Join-Path $buildRoot 'rom'
$romName = 'ricochetangles_s0_hello.sfc'
$builtRom = Join-Path $workRoot $romName
$finalRom = Join-Path $romRoot $romName

if (Test-Path -LiteralPath $buildRoot) {
    Remove-Item -LiteralPath $buildRoot -Recurse -Force
}

New-Item -ItemType Directory -Path $workSourceRoot -Force | Out-Null
New-Item -ItemType Directory -Path $romRoot -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot 'Makefile') -Destination $workRoot
Copy-Item -LiteralPath (Join-Path $repoRoot 'src\main.c') -Destination $workSourceRoot

$originalMsystem = [System.Environment]::GetEnvironmentVariable('MSYSTEM', 'Process')
$originalPvsneslibHome = [System.Environment]::GetEnvironmentVariable('PVSNESLIB_HOME', 'Process')

try {
    $env:MSYSTEM = 'UCRT64'
    $env:PVSNESLIB_HOME = $pvsneslibMsysPath

    $workMsysPath = ConvertTo-MsysPath -WindowsPath $workRoot
    & $bashPath -c "export PATH=/ucrt64/bin:/usr/bin; cd '$workMsysPath' && make"
    $makeExitCode = $LASTEXITCODE
}
finally {
    Restore-ProcessEnvironmentVariable -Name 'MSYSTEM' -Value $originalMsystem
    Restore-ProcessEnvironmentVariable -Name 'PVSNESLIB_HOME' -Value $originalPvsneslibHome
}

if ($makeExitCode -ne 0) {
    throw "PVSnesLib build failed with exit code $makeExitCode."
}

if (-not (Test-Path -LiteralPath $builtRom)) {
    throw "Build reported success but ROM is missing: $builtRom"
}

Copy-Item -LiteralPath $builtRom -Destination $finalRom

& (Join-Path $repoRoot 'tools\verify-rom.ps1') -RomPath $finalRom
if ($LASTEXITCODE -ne 0) {
    throw "ROM verification failed with exit code $LASTEXITCODE."
}

Write-Output "BUILD=PASS"
Write-Output "ROM=$finalRom"
