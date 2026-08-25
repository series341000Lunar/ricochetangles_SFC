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
    throw "Heavy Tank Test source is missing: $sourceFullPath"
}
if (-not (Test-Path -LiteralPath $gfx4SnesFullPath -PathType Leaf)) {
    throw "gfx4snes is missing: $gfx4SnesFullPath"
}

$profile = @{}
foreach ($rawLine in [System.IO.File]::ReadAllLines($sourceFullPath)) {
    $line = $rawLine.Trim()
    if ($line.Length -eq 0 -or $line.StartsWith('#')) {
        continue
    }
    if ($line -notmatch '^([A-Z0-9_]+)=(.+)$') {
        throw "Invalid Heavy Tank Test profile line: $line"
    }
    $profile[$Matches[1]] = $Matches[2].Trim()
}

function Get-RequiredProfileValue {
    param([Parameter(Mandatory)][string]$Name)

    if (-not $profile.ContainsKey($Name)) {
        throw "Heavy Tank Test profile is missing $Name."
    }
    return [string]$profile[$Name]
}

function Convert-HexColor {
    param([Parameter(Mandatory)][string]$Value)

    if ($Value -notmatch '^#([0-9A-Fa-f]{6})$') {
        throw "Invalid RGB color: $Value"
    }
    $hex = $Matches[1]
    return @(
        [Convert]::ToInt32($hex.Substring(0, 2), 16),
        [Convert]::ToInt32($hex.Substring(2, 2), 16),
        [Convert]::ToInt32($hex.Substring(4, 2), 16)
    )
}

$frameCount = [int](Get-RequiredProfileValue 'FRAME_COUNT')
$frameSize = [int](Get-RequiredProfileValue 'FRAME_SIZE')
$moduleCenterOffset = [int](Get-RequiredProfileValue 'MODULE_CENTER_OFFSET')
$segmentHalfLength = [double](Get-RequiredProfileValue 'SEGMENT_HALF_LENGTH')
$segmentHalfWidth = [double](Get-RequiredProfileValue 'SEGMENT_HALF_WIDTH')
$sideArmorStart = [double](Get-RequiredProfileValue 'SIDE_ARMOR_START')
$upperHalfWidth = [double](Get-RequiredProfileValue 'UPPER_HALF_WIDTH')
$frontArmorStart = [double](Get-RequiredProfileValue 'FRONT_ARMOR_START')
$rearArmorEnd = [double](Get-RequiredProfileValue 'REAR_ARMOR_END')

if ($frameCount -ne 16 -or $frameSize -ne 32) {
    throw 'Heavy Tank Test must remain 16 frames of 32x32 pixels.'
}
if ($moduleCenterOffset -ne 17) {
    throw 'Heavy Tank Test box composition requires a 17-pixel segment offset.'
}

$palette = @(
    @(0, 0, 0),
    (Convert-HexColor (Get-RequiredProfileValue 'SHADOW')),
    (Convert-HexColor (Get-RequiredProfileValue 'BASE')),
    (Convert-HexColor (Get-RequiredProfileValue 'LIGHT')),
    (Convert-HexColor (Get-RequiredProfileValue 'HIGHLIGHT'))
)

New-Item -ItemType Directory -Path $outputFullPath -Force | Out-Null

$width = 8 * $frameSize
$height = 2 * $frameSize
$center = ($frameSize - 1) / 2.0

function New-HullSegmentPixels {
    param([Parameter(Mandatory)][ValidateSet('Front', 'Center', 'Rear')][string]$Component)

    $pixels = [byte[]]::new($width * $height)
    for ($frame = 0; $frame -lt $frameCount; $frame++) {
        $angle = $frame * [Math]::PI / 8.0
        $cosine = [Math]::Cos($angle)
        $sine = [Math]::Sin($angle)
        $frameOriginX = ($frame % 8) * $frameSize
        $frameOriginY = [Math]::Floor($frame / 8) * $frameSize

        for ($y = 0; $y -lt $frameSize; $y++) {
            for ($x = 0; $x -lt $frameSize; $x++) {
                $deltaX = $x - $center
                $deltaY = $y - $center
                $forward = ($deltaX * $cosine) + ($deltaY * $sine)
                $right = (-$deltaX * $sine) + ($deltaY * $cosine)
                $absoluteForward = [Math]::Abs($forward)
                $absoluteRight = [Math]::Abs($right)

                if ($absoluteForward -gt $segmentHalfLength -or $absoluteRight -gt $segmentHalfWidth) {
                    continue
                }

                if ($absoluteRight -ge $sideArmorStart -or
                    ($segmentHalfWidth - $absoluteRight) -lt 1.0) {
                    $pixel = [byte]1
                }
                elseif ($Component -eq 'Front' -and $forward -ge $frontArmorStart) {
                    $pixel = [byte]4
                }
                elseif ($Component -eq 'Rear' -and $forward -le $rearArmorEnd) {
                    $pixel = [byte]1
                }
                elseif ($absoluteRight -le $upperHalfWidth) {
                    $pixel = [byte]3
                }
                else {
                    $pixel = [byte]2
                }

                $targetX = $frameOriginX + $x
                $targetY = $frameOriginY + $y
                $pixels[($targetY * $width) + $targetX] = $pixel
            }
        }
    }
    return $pixels
}

function Assert-FrameEdgesClear {
    param(
        [Parameter(Mandatory)][byte[]]$Pixels,
        [Parameter(Mandatory)][string]$Component
    )

    for ($frame = 0; $frame -lt $frameCount; $frame++) {
        $frameOriginX = ($frame % 8) * $frameSize
        $frameOriginY = [Math]::Floor($frame / 8) * $frameSize
        for ($offset = 0; $offset -lt $frameSize; $offset++) {
            $edgeIndexes = @(
                (($frameOriginY * $width) + $frameOriginX + $offset),
                ((($frameOriginY + $frameSize - 1) * $width) + $frameOriginX + $offset),
                ((($frameOriginY + $offset) * $width) + $frameOriginX),
                ((($frameOriginY + $offset) * $width) + $frameOriginX + $frameSize - 1)
            )
            foreach ($edgeIndex in $edgeIndexes) {
                if ($Pixels[$edgeIndex] -ne 0) {
                    throw "$Component Heavy Tank frame $frame touches its 32x32 edge."
                }
            }
        }
    }
}

function Write-IndexedBitmap {
    param(
        [Parameter(Mandatory)][string]$Path,
        [Parameter(Mandatory)][byte[]]$Pixels
    )

    $pixelOffset = 14 + 40 + (256 * 4)
    $imageSize = $width * $height
    $fileSize = $pixelOffset + $imageSize
    $stream = [System.IO.File]::Open($Path, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write, [System.IO.FileShare]::None)
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
            $writer.Write($Pixels, $y * $width, $width)
        }
    }
    finally {
        $writer.Dispose()
        $stream.Dispose()
    }
}

$frontBmpPath = Join-Path $outputFullPath 'heavy_tank_test_front.bmp'
$centerBmpPath = Join-Path $outputFullPath 'heavy_tank_test_center.bmp'
$rearBmpPath = Join-Path $outputFullPath 'heavy_tank_test_rear.bmp'
$frontPixels = New-HullSegmentPixels 'Front'
$centerPixels = New-HullSegmentPixels 'Center'
$rearPixels = New-HullSegmentPixels 'Rear'
Assert-FrameEdgesClear $frontPixels 'Front'
Assert-FrameEdgesClear $centerPixels 'Center'
Assert-FrameEdgesClear $rearPixels 'Rear'
Write-IndexedBitmap $frontBmpPath $frontPixels
Write-IndexedBitmap $centerBmpPath $centerPixels
Write-IndexedBitmap $rearBmpPath $rearPixels

Push-Location $outputFullPath
try {
    foreach ($bmpPath in @($frontBmpPath, $centerBmpPath, $rearBmpPath)) {
        & $gfx4SnesFullPath -s 32 -o 16 -u 16 -p -t bmp -i $bmpPath
        if ($LASTEXITCODE -ne 0) {
            throw "gfx4snes failed with exit code $LASTEXITCODE."
        }
    }
}
finally {
    Pop-Location
}

$frontPicturePath = Join-Path $outputFullPath 'heavy_tank_test_front.pic'
$centerPicturePath = Join-Path $outputFullPath 'heavy_tank_test_center.pic'
$rearPicturePath = Join-Path $outputFullPath 'heavy_tank_test_rear.pic'
$frameMajorPicturePath = Join-Path $outputFullPath 'heavy_tank_test_frames.pic'
$palettePath = Join-Path $outputFullPath 'heavy_tank_test_front.pal'
foreach ($picturePath in @($frontPicturePath, $centerPicturePath, $rearPicturePath)) {
    if ((Get-Item -LiteralPath $picturePath).Length -ne 8192) {
        throw "Heavy Tank Test component graphics size is not 8192 bytes: $picturePath"
    }
}
if ((Get-Item -LiteralPath $palettePath).Length -ne 32) {
    throw "Heavy Tank Test palette size is not 32 bytes: $palettePath"
}

$frontNativePicture = [System.IO.File]::ReadAllBytes($frontPicturePath)
$centerNativePicture = [System.IO.File]::ReadAllBytes($centerPicturePath)
$rearNativePicture = [System.IO.File]::ReadAllBytes($rearPicturePath)
$frameMajorPicture = [byte[]]::new(24576)
for ($frame = 0; $frame -lt 16; $frame++) {
    $sourceTile = (($frame -shr 2) -shl 6) + (($frame -band 3) -shl 2)
    for ($tileRow = 0; $tileRow -lt 4; $tileRow++) {
        [System.Buffer]::BlockCopy(
            $frontNativePicture,
            ($sourceTile + ($tileRow * 16)) * 32,
            $frameMajorPicture,
            ($frame * 1536) + ($tileRow * 128),
            128)
        [System.Buffer]::BlockCopy(
            $centerNativePicture,
            ($sourceTile + ($tileRow * 16)) * 32,
            $frameMajorPicture,
            ($frame * 1536) + 512 + ($tileRow * 128),
            128)
        [System.Buffer]::BlockCopy(
            $rearNativePicture,
            ($sourceTile + ($tileRow * 16)) * 32,
            $frameMajorPicture,
            ($frame * 1536) + 1024 + ($tileRow * 128),
            128)
    }
}
[System.IO.File]::WriteAllBytes($frameMajorPicturePath, $frameMajorPicture)

$includeText = @'
#ifndef HEAVY_TANK_TEST_INC
#define HEAVY_TANK_TEST_INC

#include <snes.h>

extern unsigned char heavyTankTestTiles, heavyTankTestTilesEnd;
extern unsigned char heavyTankTestPalette, heavyTankTestPaletteEnd;

#endif
'@
[System.IO.File]::WriteAllText((Join-Path $outputFullPath 'heavy_tank_test.inc'), $includeText, [System.Text.Encoding]::ASCII)

$assemblyText = @'
.include "hdr.asm"

.bank 4
.section ".roheavytanktest"

heavyTankTestTiles:
.incbin "heavy_tank_test_frames.pic"
heavyTankTestTilesEnd:

heavyTankTestPalette:
.incbin "heavy_tank_test_front.pal"
heavyTankTestPaletteEnd:

.ends
'@
[System.IO.File]::WriteAllText((Join-Path $outputFullPath 'heavy_tank_test_data.asm'), $assemblyText, [System.Text.Encoding]::ASCII)

Write-Output 'HEAVY_TANK_TEST_ASSET_BUILD=PASS'
Write-Output 'HEAVY_TANK_TEST_FRAMES=16'
Write-Output 'HEAVY_TANK_TEST_MODULES=FRONT_32x32,CENTER_32x32,REAR_32x32'
Write-Output 'HEAVY_TANK_TEST_COMPOSITION=3x32x32_OBJ_BOX_SEGMENTS'
Write-Output 'HEAVY_TANK_TEST_EFFECTIVE_AXIS=52x26_APPROX'
Write-Output 'HEAVY_TANK_TEST_FRAME_EDGE_CLEAR=PASS'
Write-Output 'HEAVY_TANK_TEST_GRAPHICS_BYTES=24576'
Write-Output 'HEAVY_TANK_TEST_RESIDENT_FRAME_BYTES=1536'
Write-Output 'HEAVY_TANK_TEST_PALETTE_BYTES=32'
