[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$OutputDirectory,
    [Parameter(Mandatory)][string]$Gfx4SnesPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$outputFullPath = [System.IO.Path]::GetFullPath($OutputDirectory)
$gfx4SnesFullPath = [System.IO.Path]::GetFullPath($Gfx4SnesPath)

if (-not (Test-Path -LiteralPath $gfx4SnesFullPath -PathType Leaf)) {
    throw "gfx4snes is missing: $gfx4SnesFullPath"
}

New-Item -ItemType Directory -Path $outputFullPath -Force | Out-Null

$width = 128
$height = 48
$pixels = [byte[]]::new($width * $height)

function Set-IndexedPixel {
    param(
        [Parameter(Mandatory)][int]$X,
        [Parameter(Mandatory)][int]$Y,
        [Parameter(Mandatory)][byte]$Color
    )

    if ($X -lt 0 -or $X -ge $width -or $Y -lt 0 -or $Y -ge $height) {
        throw "HUD pixel is outside the sheet: $X,$Y"
    }
    $pixels[($Y * $width) + $X] = $Color
}

$arrowPatterns = @{
    'UP' = @(
        '...#...',
        '..###..',
        '.#####.',
        '#######',
        '...#...',
        '...#...',
        '...#...'
    )
    'DOWN' = @(
        '...#...',
        '...#...',
        '...#...',
        '#######',
        '.#####.',
        '..###..',
        '...#...'
    )
    'LEFT' = @(
        '...#...',
        '..##...',
        '.###...',
        '#######',
        '.###...',
        '..##...',
        '...#...'
    )
    'RIGHT' = @(
        '...#...',
        '...##..',
        '...###.',
        '#######',
        '...###.',
        '...##..',
        '...#...'
    )
}

function Draw-ButtonFrame {
    param(
        [Parameter(Mandatory)][int]$OriginX,
        [Parameter(Mandatory)][string]$Direction,
        [Parameter(Mandatory)][bool]$Pressed
    )

    $fillColor = if ($Pressed) { [byte]4 } else { [byte]1 }
    $borderColor = if ($Pressed) { [byte]5 } else { [byte]2 }
    $arrowColor = if ($Pressed) { [byte]6 } else { [byte]3 }

    for ($y = 1; $y -le 14; $y++) {
        for ($x = 1; $x -le 14; $x++) {
            $isCorner = (($x -eq 1 -or $x -eq 14) -and ($y -eq 1 -or $y -eq 14))
            if (-not $isCorner) {
                $isBorder = $x -eq 1 -or $x -eq 14 -or $y -eq 1 -or $y -eq 14
                Set-IndexedPixel -X ($OriginX + $x) -Y $y -Color $(if ($isBorder) { $borderColor } else { $fillColor })
            }
        }
    }

    $pattern = $arrowPatterns[$Direction]
    for ($y = 0; $y -lt 7; $y++) {
        for ($x = 0; $x -lt 7; $x++) {
            if ($pattern[$y][$x] -eq '#') {
                Set-IndexedPixel -X ($OriginX + 4 + $x) -Y (4 + $y) -Color $arrowColor
            }
        }
    }
}

$directions = @('UP', 'DOWN', 'LEFT', 'RIGHT')
for ($directionIndex = 0; $directionIndex -lt $directions.Count; $directionIndex++) {
    Draw-ButtonFrame -OriginX (($directionIndex * 2) * 16) -Direction $directions[$directionIndex] -Pressed $false
    Draw-ButtonFrame -OriginX ((($directionIndex * 2) + 1) * 16) -Direction $directions[$directionIndex] -Pressed $true
}

$markerPattern = @(
    '...#...',
    '..###..',
    '.#####.',
    '#######',
    '.#####.',
    '..###..',
    '...#...'
)
for ($y = 0; $y -lt 7; $y++) {
    for ($x = 0; $x -lt 7; $x++) {
        if ($markerPattern[$y][$x] -eq '#') {
            Set-IndexedPixel -X (4 + $x) -Y (36 + $y) -Color 7
        }
    }
}

$cursorOriginX = 16
$cursorOriginY = 32
foreach ($offset in 1..5) {
    Set-IndexedPixel -X ($cursorOriginX + 7) -Y ($cursorOriginY + $offset) -Color 8
    Set-IndexedPixel -X ($cursorOriginX + 7) -Y ($cursorOriginY + 14 - $offset) -Color 8
    Set-IndexedPixel -X ($cursorOriginX + $offset) -Y ($cursorOriginY + 7) -Color 8
    Set-IndexedPixel -X ($cursorOriginX + 14 - $offset) -Y ($cursorOriginY + 7) -Color 8
}

$shellPattern = @(
    '...##...',
    '..####..',
    '.######.',
    '########',
    '########',
    '.######.',
    '..####..',
    '...##...'
)
$shellOriginX = 32
$shellOriginY = 32
for ($y = 0; $y -lt 8; $y++) {
    for ($x = 0; $x -lt 8; $x++) {
        if ($shellPattern[$y][$x] -eq '#') {
            Set-IndexedPixel -X ($shellOriginX + 4 + $x) -Y ($shellOriginY + 4 + $y) -Color 7
        }
    }
}

$bmpPath = Join-Path $outputFullPath 'input_hud.bmp'
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
    $writer.Write([uint32]9)

    $palette = @(
        @(0, 0, 0),
        @(24, 45, 31),
        @(83, 132, 91),
        @(190, 230, 190),
        @(220, 180, 40),
        @(255, 235, 100),
        @(45, 38, 20),
        @(255, 232, 64),
        @(80, 240, 255)
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
    & $gfx4SnesFullPath -s 8 -o 16 -u 16 -p -t bmp -i $bmpPath
    if ($LASTEXITCODE -ne 0) {
        throw "gfx4snes failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}

$picturePath = Join-Path $outputFullPath 'input_hud.pic'
$palettePath = Join-Path $outputFullPath 'input_hud.pal'
if ((Get-Item -LiteralPath $picturePath).Length -ne 3072) {
    throw "Input HUD graphics size is not 3072 bytes: $picturePath"
}
if ((Get-Item -LiteralPath $palettePath).Length -ne 32) {
    throw "Input HUD palette size is not 32 bytes: $palettePath"
}

$includeText = @'
#ifndef INPUT_HUD_INC
#define INPUT_HUD_INC

#include <snes.h>

extern unsigned char inputHudTiles, inputHudTilesEnd;
extern unsigned char inputHudPalette, inputHudPaletteEnd;

#endif
'@
[System.IO.File]::WriteAllText((Join-Path $outputFullPath 'input_hud.inc'), $includeText, [System.Text.Encoding]::ASCII)

$assemblyText = @'
.include "hdr.asm"

.bank 6
.section ".roinputhud"

inputHudTiles:
.incbin "input_hud.pic"
inputHudTilesEnd:

inputHudPalette:
.incbin "input_hud.pal"
inputHudPaletteEnd:

.ends
'@
[System.IO.File]::WriteAllText((Join-Path $outputFullPath 'input_hud_data.asm'), $assemblyText, [System.Text.Encoding]::ASCII)

Write-Output 'INPUT_HUD_ASSET_BUILD=PASS'
Write-Output 'INPUT_HUD_BUTTONS=4'
Write-Output 'INPUT_HUD_STATES=IDLE,PRESSED'
Write-Output 'INPUT_HUD_DIMENSIONS=16x16'
Write-Output 'INPUT_HUD_GRAPHICS_BYTES=3072'
Write-Output 'INPUT_HUD_PALETTE_BYTES=32'
Write-Output 'AIM_CURSOR_DIMENSIONS=16x16'
Write-Output 'AIM_CURSOR_GRAPHICS_BYTES=128'
Write-Output 'PROJECTILE_VISUAL_DIMENSIONS=8x8'
Write-Output 'PROJECTILE_TILE_FOOTPRINT=16x16'
Write-Output 'PROJECTILE_GRAPHICS_BYTES=128'
