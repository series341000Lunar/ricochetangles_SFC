[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$msysRoot = 'C:\msys64'
$bashPath = Join-Path $msysRoot 'usr\bin\bash.exe'
$makePath = Join-Path $msysRoot 'usr\bin\make.exe'
$pvsneslibWindowsPath = 'C:\snesdev\pvsneslib-4.6.0'
$pvsneslibMsysPath = '/c/snesdev/pvsneslib-4.6.0'
$gfx4SnesPath = Join-Path $pvsneslibWindowsPath 'devkitsnes\tools\gfx4snes.exe'

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
    $gfx4SnesPath,
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
$hullAssetSource = Join-Path $repoRoot 'assets\hull_placeholder\hull_16dir.txt'
$hullAssetGenerator = Join-Path $repoRoot 'tools\generate-hull-placeholder.ps1'
$turretAssetSource = Join-Path $repoRoot 'assets\turret_placeholder\turret_16dir.txt'
$turretAssetGenerator = Join-Path $repoRoot 'tools\generate-turret-placeholder.ps1'
$inputHudGenerator = Join-Path $repoRoot 'tools\generate-input-hud.ps1'
$s2AngleVerifier = Join-Path $repoRoot 'tools\verify-s2-angle.ps1'
$s2ProjectileVerifier = Join-Path $repoRoot 'tools\verify-s2-projectile.ps1'
$s2PadAimVerifier = Join-Path $repoRoot 'tools\verify-s2-pad-aim.ps1'
foreach ($requiredProjectPath in @($hullAssetSource, $hullAssetGenerator, $turretAssetSource, $turretAssetGenerator, $inputHudGenerator, $s2AngleVerifier, $s2ProjectileVerifier, $s2PadAimVerifier)) {
    if (-not (Test-Path -LiteralPath $requiredProjectPath -PathType Leaf)) {
        throw "Required S2 project file is missing: $requiredProjectPath"
    }
}

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
$symbolPath = Join-Path $workRoot 'ricochetangles_s0_hello.sym'

if (Test-Path -LiteralPath $buildRoot) {
    Remove-Item -LiteralPath $buildRoot -Recurse -Force
}

New-Item -ItemType Directory -Path $workSourceRoot -Force | Out-Null
New-Item -ItemType Directory -Path $romRoot -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot 'Makefile') -Destination $workRoot
Copy-Item -LiteralPath (Join-Path $repoRoot 'src\main.c') -Destination $workSourceRoot

& $hullAssetGenerator `
    -SourcePath $hullAssetSource `
    -OutputDirectory $workRoot `
    -Gfx4SnesPath $gfx4SnesPath

& $inputHudGenerator `
    -OutputDirectory $workRoot `
    -Gfx4SnesPath $gfx4SnesPath

& $turretAssetGenerator `
    -SourcePath $turretAssetSource `
    -OutputDirectory $workRoot `
    -Gfx4SnesPath $gfx4SnesPath

& $s2AngleVerifier -SourcePath (Join-Path $repoRoot 'src\main.c')
& $s2ProjectileVerifier -SourcePath (Join-Path $repoRoot 'src\main.c')
& $s2PadAimVerifier -SourcePath (Join-Path $repoRoot 'src\main.c')

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

if (-not (Test-Path -LiteralPath $symbolPath -PathType Leaf)) {
    throw "Build symbol file is missing: $symbolPath"
}

$symbols = @{}
foreach ($line in Get-Content -LiteralPath $symbolPath) {
    if ($line -match '^([0-9A-Fa-f]{8})\s+(.+)$') {
        $symbols[$Matches[2]] = [Convert]::ToUInt32($Matches[1], 16)
    }
}

$sameBankRuntimeSymbols = @(
    'main',
    'consoleInitDefaultText',
    'bgSetGfxPtr',
    'bgSetMapPtr',
    'oamInit',
    'oamInitGfxAttr',
    'dmaCopyVram',
    'dmaCopyCGram',
    'oamSet',
    'oamSetEx',
    'oamSetGfxOffset',
    'oamSetXY',
    'oamSetVisible',
    'setMode',
    'bgSetDisable',
    'consoleDrawText',
    'initMouse',
    'WaitForVBlank',
    'setScreenOn'
)
foreach ($symbolName in $sameBankRuntimeSymbols) {
    if (-not $symbols.ContainsKey($symbolName)) {
        throw "Required runtime symbol is missing: $symbolName"
    }
    $symbolBank = $symbols[$symbolName] -shr 16
    if ($symbolBank -ne 0) {
        throw ("Runtime symbol {0} escaped code bank 00 (actual bank {1:X2}); PVSnesLib RTS calls must remain same-bank." -f $symbolName, $symbolBank)
    }
}

$sourceRomSymbols = $symbols.GetEnumerator() | Where-Object {
    $_.Key.StartsWith('tccs_src/main.asm_', [System.StringComparison]::Ordinal) -and
    (($_.Value -shr 16) -ne 0x7E)
}
foreach ($symbol in $sourceRomSymbols) {
    $symbolBank = $symbol.Value -shr 16
    if ($symbolBank -ne 0) {
        throw ("Compiled source symbol {0} escaped code bank 00 (actual bank {1:X2})." -f $symbol.Key, $symbolBank)
    }
}

$assetBanks = @{
    hullPlaceholderTiles = 5
    inputHudTiles = 6
    turretPlaceholderTiles = 7
}
foreach ($entry in $assetBanks.GetEnumerator()) {
    if (-not $symbols.ContainsKey($entry.Key)) {
        throw "Required asset symbol is missing: $($entry.Key)"
    }
    $actualBank = $symbols[$entry.Key] -shr 16
    if ($actualBank -ne $entry.Value) {
        throw ("Asset symbol {0} is in bank {1:X2}; expected bank {2:X2}." -f $entry.Key, $actualBank, $entry.Value)
    }
}

Write-Output 'S2_BANK_LAYOUT_VERIFY=PASS'
Write-Output 'CODE_BANK=00'
Write-Output 'HULL_ASSET_BANK=05'
Write-Output 'INPUT_HUD_ASSET_BANK=06'
Write-Output 'TURRET_ASSET_BANK=07'

Copy-Item -LiteralPath $builtRom -Destination $finalRom

& (Join-Path $repoRoot 'tools\verify-rom.ps1') -RomPath $finalRom
if ($LASTEXITCODE -ne 0) {
    throw "ROM verification failed with exit code $LASTEXITCODE."
}

Write-Output "BUILD=PASS"
Write-Output "ROM=$finalRom"
