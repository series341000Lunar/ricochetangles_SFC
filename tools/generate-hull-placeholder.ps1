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

if (-not (Test-Path -LiteralPath $sourceFullPath -PathType Leaf)) {
    throw "Hull placeholder source is missing: $sourceFullPath"
}
if (-not (Test-Path -LiteralPath $gfx4SnesFullPath -PathType Leaf)) {
    throw "gfx4snes is missing: $gfx4SnesFullPath"
}

New-Item -ItemType Directory -Path $outputFullPath -Force | Out-Null

$frames = @{}
$currentFrame = -1
foreach ($rawLine in [System.IO.File]::ReadAllLines($sourceFullPath)) {
    $line = $rawLine.Trim()
    if ($line.Length -eq 0 -or $line.StartsWith('#')) {
        continue
    }

    if ($line -match '^FRAME ([0-9A-Fa-f]{2})$') {
        $currentFrame = [Convert]::ToInt32($Matches[1], 16)
        if ($currentFrame -lt 0 -or $currentFrame -ge 16) {
            throw "Frame index is outside 00..0F: $line"
        }
        if ($frames.ContainsKey($currentFrame)) {
            throw "Duplicate frame: $line"
        }
        $frames[$currentFrame] = [System.Collections.Generic.List[string]]::new()
        continue
    }

    if ($currentFrame -lt 0) {
        throw "Pixel row appears before a FRAME declaration: $line"
    }
    if ($line -notmatch '^[.dgl]{16}$') {
        throw "Frame $currentFrame has an invalid 16-pixel row: $line"
    }
    $frames[$currentFrame].Add($line)
}

for ($frame = 0; $frame -lt 16; $frame++) {
    if (-not $frames.ContainsKey($frame)) {
        throw "Missing frame: $($frame.ToString('X2'))"
    }
    if ($frames[$frame].Count -ne 16) {
        throw "Frame $($frame.ToString('X2')) has $($frames[$frame].Count) rows; expected 16."
    }
}

$sourceFrameSize = 16
$scale = 2
$outputFrameSize = $sourceFrameSize * $scale
$width = 8 * $outputFrameSize
$height = 2 * $outputFrameSize
$pixels = [byte[]]::new($width * $height)
$colorIndex = @{
    '.' = [byte]0
    'd' = [byte]1
    'g' = [byte]3
    'l' = [byte]4
}

for ($frame = 0; $frame -lt 16; $frame++) {
    $frameOriginX = ($frame % 8) * $outputFrameSize
    $frameOriginY = [Math]::Floor($frame / 8) * $outputFrameSize
    for ($y = 0; $y -lt $sourceFrameSize; $y++) {
        $row = $frames[$frame][$y]
        for ($x = 0; $x -lt $sourceFrameSize; $x++) {
            $pixel = $colorIndex[[string]$row[$x]]
            for ($scaleY = 0; $scaleY -lt $scale; $scaleY++) {
                for ($scaleX = 0; $scaleX -lt $scale; $scaleX++) {
                    $targetX = $frameOriginX + ($x * $scale) + $scaleX
                    $targetY = $frameOriginY + ($y * $scale) + $scaleY
                    $pixels[($targetY * $width) + $targetX] = $pixel
                }
            }
        }
    }
}

$bmpPath = Join-Path $outputFullPath 'hull_placeholder.bmp'
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
        @(36, 92, 43),
        @(255, 232, 64),
        @(67, 183, 80),
        @(146, 235, 134)
    )
    for ($index = 0; $index -lt 256; $index++) {
        if ($index -lt $palette.Count) {
            $red = $palette[$index][0]
            $green = $palette[$index][1]
            $blue = $palette[$index][2]
        }
        else {
            $red = 0
            $green = 0
            $blue = 0
        }
        $writer.Write([byte]$blue)
        $writer.Write([byte]$green)
        $writer.Write([byte]$red)
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
    & $gfx4SnesFullPath -s 32 -o 16 -u 16 -p -t bmp -i $bmpPath
    if ($LASTEXITCODE -ne 0) {
        throw "gfx4snes failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

$picturePath = Join-Path $outputFullPath 'hull_placeholder.pic'
$palettePath = Join-Path $outputFullPath 'hull_placeholder.pal'
if ((Get-Item -LiteralPath $picturePath).Length -ne 8192) {
    throw "Hull graphics size is not 8192 bytes: $picturePath"
}
if ((Get-Item -LiteralPath $palettePath).Length -ne 32) {
    throw "Hull palette size is not 32 bytes: $palettePath"
}

$includeText = @'
#ifndef HULL_PLACEHOLDER_INC
#define HULL_PLACEHOLDER_INC

#include <snes.h>

extern unsigned char hullPlaceholderTiles, hullPlaceholderTilesEnd;
extern unsigned char hullPlaceholderPalette, hullPlaceholderPaletteEnd;

#endif
'@
[System.IO.File]::WriteAllText((Join-Path $outputFullPath 'hull_placeholder.inc'), $includeText, [System.Text.Encoding]::ASCII)

$assemblyText = @'
.include "hdr.asm"

.bank 5
.section ".rohullplaceholder"

hullPlaceholderTiles:
.incbin "hull_placeholder.pic"
hullPlaceholderTilesEnd:

hullPlaceholderPalette:
.incbin "hull_placeholder.pal"
hullPlaceholderPaletteEnd:

.ends
'@
[System.IO.File]::WriteAllText((Join-Path $outputFullPath 'hull_placeholder_data.asm'), $assemblyText, [System.Text.Encoding]::ASCII)

Write-Output 'HULL_ASSET_BUILD=PASS'
Write-Output "HULL_FRAMES=16"
Write-Output "HULL_DIMENSIONS=32x32"
Write-Output "HULL_GRAPHICS_BYTES=8192"
Write-Output "HULL_PALETTE_BYTES=32"
