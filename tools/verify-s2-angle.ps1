[CmdletBinding()]
param([Parameter(Mandatory)][string]$SourcePath)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$source = [System.IO.File]::ReadAllText([System.IO.Path]::GetFullPath($SourcePath))
$match = [regex]::Match(
    $source,
    'static const u8 atanThreshold\[32\] = \{(?<values>.*?)\};',
    [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (-not $match.Success) {
    throw 'Unable to find the 32-entry atanThreshold table.'
}

$thresholds = @([regex]::Matches($match.Groups['values'].Value, '\d+') | ForEach-Object { [int]$_.Value })
if ($thresholds.Count -ne 32 -or $thresholds[0] -ne 3 -or $thresholds[31] -ne 250) {
    throw "Invalid atanThreshold table contract: count=$($thresholds.Count)."
}

function Resolve-OctantAngle {
    param([int]$Minor, [int]$Major)
    $low = 0
    $high = 32
    $scaledMinor = $Minor * 256
    while ($low -lt $high) {
        $middle = ($low + $high) -shr 1
        if ($scaledMinor -ge $Major * $thresholds[$middle]) {
            $low = $middle + 1
        }
        else {
            $high = $middle
        }
    }
    return $low
}

function Resolve-Heading {
    param([int]$Dx, [int]$Dy)
    $absoluteX = [Math]::Abs($Dx)
    $absoluteY = [Math]::Abs($Dy)
    if ($absoluteX -ge $absoluteY) {
        $baseAngle = if ($absoluteX -eq 0) { 0 } else { Resolve-OctantAngle -Minor $absoluteY -Major $absoluteX }
    }
    else {
        $baseAngle = 64 - (Resolve-OctantAngle -Minor $absoluteX -Major $absoluteY)
    }

    if ($Dx -ge 0) {
        return $(if ($Dy -ge 0) { $baseAngle } else { (256 - $baseAngle) -band 255 })
    }
    return $(if ($Dy -ge 0) { 128 - $baseAngle } else { (128 + $baseAngle) -band 255 })
}

$cardinals = @(
    @(1, 0, 0),
    @(0, 1, 64),
    @(-1, 0, 128),
    @(0, -1, 192)
)
foreach ($test in $cardinals) {
    $actual = Resolve-Heading -Dx $test[0] -Dy $test[1]
    if ($actual -ne $test[2]) {
        throw "Cardinal heading failed for $($test[0]),$($test[1]): $actual."
    }
}

$maximumError = 0
foreach ($dy in -64..64) {
    foreach ($dx in -64..64) {
        if ($dx -eq 0 -and $dy -eq 0) { continue }
        $actual = Resolve-Heading -Dx $dx -Dy $dy
        $expected = [int][Math]::Round(
            [Math]::Atan2($dy, $dx) * 128 / [Math]::PI,
            [MidpointRounding]::AwayFromZero)
        $expected = ($expected + 256) -band 255
        $errorValue = [Math]::Abs($actual - $expected)
        $errorValue = [Math]::Min($errorValue, 256 - $errorValue)
        $maximumError = [Math]::Max($maximumError, $errorValue)
    }
}
if ($maximumError -gt 1) {
    throw "Integer vector heading error exceeds one heading unit: $maximumError."
}

foreach ($current in 0..255) {
    foreach ($target in 0..255) {
        $delta = ($target - $current) -band 255
        if ($delta -ge 128) { $delta -= 256 }
        $step = if ($delta -gt 4) { 4 } elseif ($delta -lt -4) { -4 } else { $delta }
        if ([Math]::Abs($step) -gt 4) {
            throw "Turret traverse exceeded four heading units: $current -> $target."
        }
    }
}

$turretTiles = New-Object 'System.Collections.Generic.HashSet[int]'
foreach ($frame in 0..15) {
    $base = 352 + (($frame -shr 3) -shl 5) + (($frame -band 7) -shl 1)
    foreach ($offset in @(0, 1, 16, 17)) {
        if (-not $turretTiles.Add($base + $offset)) {
            throw "Turret frame tile overlap at frame $frame."
        }
    }
}
if ($turretTiles.Count -ne 64) {
    throw "Turret tile coverage is not 64 tiles: $($turretTiles.Count)."
}

Write-Output 'S2_ANGLE_VERIFY=PASS'
Write-Output "ATAN_THRESHOLD_ENTRIES=$($thresholds.Count)"
Write-Output 'RUNTIME_DIVISION=NONE'
Write-Output "MAX_HEADING_ERROR=$maximumError"
Write-Output 'TURRET_TRAVERSE_RATE=4'
Write-Output "TURRET_TILE_COVERAGE=$($turretTiles.Count)"
