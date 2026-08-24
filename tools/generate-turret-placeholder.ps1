[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$SourcePath,
    [Parameter(Mandatory)][string]$OutputDirectory,
    [Parameter(Mandatory)][string]$Gfx4SnesPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$sourceFullPath = [System.IO.Path]::GetFullPath($SourcePath)
$outputFullPath = [System.IO.Path]::GetFullPath($OutputDirectory)
$gfx4SnesFullPath = [System.IO.Path]::GetFullPath($Gfx4SnesPath)

foreach ($requiredPath in @($sourceFullPath, $gfx4SnesFullPath)) {
    if (-not (Test-Path -LiteralPath $requiredPath -PathType Leaf)) {
        throw "Required turret asset input is missing: $requiredPath"
    }
}

$vectors = @{}
foreach ($rawLine in [System.IO.File]::ReadAllLines($sourceFullPath)) {
    $line = $rawLine.Trim()
    if ($line.Length -eq 0 -or $line.StartsWith('#')) {
        continue
    }
    if ($line -notmatch '^FRAME ([0-9A-Fa-f]{2}) (-?[0-9]+) (-?[0-9]+)$') {
        throw "Invalid turret vector line: $line"
    }
    $frame = [Convert]::ToInt32($Matches[1], 16)
    if ($frame -lt 0 -or $frame -ge 16 -or $vectors.ContainsKey($frame)) {
        throw "Invalid or duplicate turret frame: $line"
    }
    $vectors[$frame] = @([int]$Matches[2], [int]$Matches[3])
}

for ($frame = 0; $frame -lt 16; $frame++) {
    if (-not $vectors.ContainsKey($frame)) {
        throw "Missing turret frame: $($frame.ToString('X2'))"
    }
}

New-Item -ItemType Directory -Path $outputFullPath -Force | Out-Null
$width = 128
$height = 32
$pixels = [byte[]]::new($width * $height)

function Set-TurretPixel {
    param([int]$X, [int]$Y, [byte]$Color)
    if ($X -ge 0 -and $X -lt $width -and $Y -ge 0 -and $Y -lt $height) {
        $pixels[($Y * $width) + $X] = $Color
    }
}

function Draw-Line {
    param([int]$X0, [int]$Y0, [int]$X1, [int]$Y1, [byte]$Color)
    $dx = [Math]::Abs($X1 - $X0)
    $sx = if ($X0 -lt $X1) { 1 } else { -1 }
    $dy = -[Math]::Abs($Y1 - $Y0)
    $sy = if ($Y0 -lt $Y1) { 1 } else { -1 }
    $errorValue = $dx + $dy
    while ($true) {
        Set-TurretPixel -X $X0 -Y $Y0 -Color $Color
        if ($X0 -eq $X1 -and $Y0 -eq $Y1) { break }
        $twiceError = 2 * $errorValue
        if ($twiceError -ge $dy) { $errorValue += $dy; $X0 += $sx }
        if ($twiceError -le $dx) { $errorValue += $dx; $Y0 += $sy }
    }
}

for ($frame = 0; $frame -lt 16; $frame++) {
    $originX = ($frame % 8) * 16
    $originY = [Math]::Floor($frame / 8) * 16
    $centerX = $originX + 7
    $centerY = $originY + 7
    $endX = $centerX + $vectors[$frame][0]
    $endY = $centerY + $vectors[$frame][1]

    Draw-Line -X0 $centerX -Y0 $centerY -X1 $endX -Y1 $endY -Color 3
    for ($y = -3; $y -le 3; $y++) {
        for ($x = -3; $x -le 3; $x++) {
            if ([Math]::Abs($x) + [Math]::Abs($y) -le 4) {
                Set-TurretPixel -X ($centerX + $x) -Y ($centerY + $y) -Color 1
            }
        }
    }
    Set-TurretPixel -X $centerX -Y ($centerY - 2) -Color 2
    Set-TurretPixel -X $endX -Y $endY -Color 4
}

$bmpPath = Join-Path $outputFullPath 'turret_placeholder.bmp'
$pixelOffset = 14 + 40 + (256 * 4)
$imageSize = $width * $height
$fileSize = $pixelOffset + $imageSize
$stream = [System.IO.File]::Open($bmpPath, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write, [System.IO.FileShare]::None)
$writer = [System.IO.BinaryWriter]::new($stream)
try {
    $writer.Write([uint16]0x4D42)
    $writer.Write([uint32]$fileSize)
    $writer.Write([uint32]0)
    $writer.Write([uint32]$pixelOffset)
    $writer.Write([uint32]40)
    $writer.Write([int32]$width)
    $writer.Write([int32]$height)
    $writer.Write([uint16]1)
    $writer.Write([uint16]8)
    $writer.Write([uint32]0)
    $writer.Write([uint32]$imageSize)
    $writer.Write([int32]0)
    $writer.Write([int32]0)
    $writer.Write([uint32]256)
    $writer.Write([uint32]5)

    $palette = @(
        @(0, 0, 0),
        @(126, 116, 61),
        @(216, 205, 115),
        @(232, 218, 92),
        @(255, 247, 174)
    )
    for ($index = 0; $index -lt 256; $index++) {
        $color = if ($index -lt $palette.Count) { $palette[$index] } else { @(0, 0, 0) }
        $writer.Write([byte]$color[2])
        $writer.Write([byte]$color[1])
        $writer.Write([byte]$color[0])
        $writer.Write([byte]0)
    }
    for ($y = $height - 1; $y -ge 0; $y--) {
        $writer.Write($pixels, $y * $width, $width)
    }
}
finally {
    $writer.Dispose()
    $stream.Dispose()
}

Push-Location $outputFullPath
try {
    & $gfx4SnesFullPath -s 8 -o 16 -u 16 -p -t bmp -i $bmpPath
    if ($LASTEXITCODE -ne 0) { throw "gfx4snes failed with exit code $LASTEXITCODE." }
}
finally {
    Pop-Location
}

$picturePath = Join-Path $outputFullPath 'turret_placeholder.pic'
$palettePath = Join-Path $outputFullPath 'turret_placeholder.pal'
if ((Get-Item -LiteralPath $picturePath).Length -ne 2048) {
    throw "Turret graphics size is not 2048 bytes: $picturePath"
}
if ((Get-Item -LiteralPath $palettePath).Length -ne 32) {
    throw "Turret palette size is not 32 bytes: $palettePath"
}

$includeText = @'
#ifndef TURRET_PLACEHOLDER_INC
#define TURRET_PLACEHOLDER_INC

#include <snes.h>

extern unsigned char turretPlaceholderTiles, turretPlaceholderTilesEnd;
extern unsigned char turretPlaceholderPalette, turretPlaceholderPaletteEnd;

#endif
'@
[System.IO.File]::WriteAllText((Join-Path $outputFullPath 'turret_placeholder.inc'), $includeText, [System.Text.Encoding]::ASCII)

$assemblyText = @'
.include "hdr.asm"

.bank 7
.section ".roturretplaceholder"

turretPlaceholderTiles:
.incbin "turret_placeholder.pic"
turretPlaceholderTilesEnd:

turretPlaceholderPalette:
.incbin "turret_placeholder.pal"
turretPlaceholderPaletteEnd:

.ends
'@
[System.IO.File]::WriteAllText((Join-Path $outputFullPath 'turret_placeholder_data.asm'), $assemblyText, [System.Text.Encoding]::ASCII)

Write-Output 'TURRET_ASSET_BUILD=PASS'
Write-Output 'TURRET_FRAMES=16'
Write-Output 'TURRET_DIMENSIONS=16x16'
Write-Output 'TURRET_GRAPHICS_BYTES=2048'
Write-Output 'TURRET_PALETTE_COLORS=5'
Write-Output 'TURRET_PALETTE_BYTES=32'
