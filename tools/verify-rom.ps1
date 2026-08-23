[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string]$RomPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Read-U16LittleEndian {
    param(
        [Parameter(Mandatory)][byte[]]$Bytes,
        [Parameter(Mandatory)][int]$Offset
    )

    return [uint16](([int]$Bytes[$Offset]) -bor (([int]$Bytes[$Offset + 1]) -shl 8))
}

function Assert-RomField {
    param(
        [Parameter(Mandatory)][bool]$Condition,
        [Parameter(Mandatory)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$resolvedRomPath = [System.IO.Path]::GetFullPath($RomPath)
Assert-RomField (Test-Path -LiteralPath $resolvedRomPath -PathType Leaf) "ROM does not exist: $resolvedRomPath"

$rom = [System.IO.File]::ReadAllBytes($resolvedRomPath)
Assert-RomField ($rom.Length -ge 0x8000) "ROM is smaller than one LoROM bank: $($rom.Length) bytes"
Assert-RomField (($rom.Length % 0x8000) -eq 0) "ROM size is not aligned to a 32 KiB LoROM bank: $($rom.Length) bytes"
Assert-RomField (($rom.Length -band ($rom.Length - 1)) -eq 0) "ROM size is not a power of two: $($rom.Length) bytes"

$headerOffset = 0x7FC0
Assert-RomField ($rom.Length -ge ($headerOffset + 0x40)) 'ROM is too small to contain a complete LoROM header and vectors.'

$title = [System.Text.Encoding]::ASCII.GetString($rom, $headerOffset, 21).TrimEnd([char]0, [char]32)
$mapMode = $rom[$headerOffset + 0x15]
$cartridgeType = $rom[$headerOffset + 0x16]
$romSizeCode = $rom[$headerOffset + 0x17]
$sramSizeCode = $rom[$headerOffset + 0x18]
$countryCode = $rom[$headerOffset + 0x19]
$version = $rom[$headerOffset + 0x1B]
$checksumComplement = Read-U16LittleEndian -Bytes $rom -Offset ($headerOffset + 0x1C)
$checksum = Read-U16LittleEndian -Bytes $rom -Offset ($headerOffset + 0x1E)
$resetVector = Read-U16LittleEndian -Bytes $rom -Offset 0x7FFC

Assert-RomField ($title -eq 'RICOCHETANGLES S0') "Unexpected ROM title: '$title'"
Assert-RomField ($mapMode -eq 0x20) ('Expected LoROM/SlowROM map mode 0x20, got 0x{0:X2}' -f $mapMode)
Assert-RomField ($cartridgeType -eq 0x00) ('Expected ROM-only cartridge type 0x00, got 0x{0:X2}' -f $cartridgeType)
Assert-RomField ($romSizeCode -eq 0x08) ('Expected 256 KiB ROM size code 0x08, got 0x{0:X2}' -f $romSizeCode)
Assert-RomField ($sramSizeCode -eq 0x00) ('Expected no-SRAM code 0x00, got 0x{0:X2}' -f $sramSizeCode)
Assert-RomField ($countryCode -eq 0x00) ('Expected Japan/SFC destination code 0x00, got 0x{0:X2}' -f $countryCode)
Assert-RomField ($version -eq 0x00) ('Expected ROM version 0x00, got 0x{0:X2}' -f $version)
Assert-RomField ($rom.Length -eq ((1KB) -shl $romSizeCode)) "Header ROM size does not match file size: $($rom.Length) bytes"
Assert-RomField (($checksum -bxor $checksumComplement) -eq 0xFFFF) 'Checksum and complement do not XOR to 0xFFFF.'
Assert-RomField ($resetVector -ge 0x8000) ('Reset vector is outside the LoROM executable area: 0x{0:X4}' -f $resetVector)

$byteSum = [uint64]0
foreach ($value in $rom) {
    $byteSum += $value
}
$calculatedChecksum = [uint16]($byteSum % 0x10000)
Assert-RomField ($calculatedChecksum -eq $checksum) ('Stored checksum 0x{0:X4} does not match byte sum 0x{1:X4}' -f $checksum, $calculatedChecksum)

$sha256 = (Get-FileHash -LiteralPath $resolvedRomPath -Algorithm SHA256).Hash.ToLowerInvariant()

Write-Output 'ROM_VERIFY=PASS'
Write-Output "PATH=$resolvedRomPath"
Write-Output "SIZE_BYTES=$($rom.Length)"
Write-Output "TITLE=$title"
Write-Output ('MAP_MODE=0x{0:X2} (LoROM/SlowROM)' -f $mapMode)
Write-Output ('CARTRIDGE_TYPE=0x{0:X2} (ROM only)' -f $cartridgeType)
Write-Output ('SRAM_SIZE=0x{0:X2} (none)' -f $sramSizeCode)
Write-Output ('CHECKSUM=0x{0:X4}' -f $checksum)
Write-Output ('RESET_VECTOR=0x{0:X4}' -f $resetVector)
Write-Output "SHA256=$sha256"

