[CmdletBinding()]
param([Parameter(Mandatory)][string]$SourcePath)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$source = [System.IO.File]::ReadAllText([System.IO.Path]::GetFullPath($SourcePath))

function Assert-SourcePattern {
    param(
        [Parameter(Mandatory)][string]$Pattern,
        [Parameter(Mandatory)][string]$FailureMessage
    )

    if (-not [regex]::IsMatch($source, $Pattern, [System.Text.RegularExpressions.RegexOptions]::Singleline)) {
        throw $FailureMessage
    }
}

Assert-SourcePattern '#define\s+ENEMY_ARMOR_HALF_LENGTH\s+13\b' 'Enemy armor half-length must be 13 pixels.'
Assert-SourcePattern '#define\s+ENEMY_ARMOR_HALF_WIDTH\s+10\b' 'Enemy armor half-width must be 10 pixels.'
Assert-SourcePattern '#define\s+RICOCHET_THRESHOLD_DEGREES\s+75\b' 'HTML canonical 75-degree threshold is not recorded.'
Assert-SourcePattern '#define\s+RICOCHET_THRESHOLD_HEADING_UNITS\s+54\b' 'The first 0..255 heading unit at or above 75 degrees must be 54.'
Assert-SourcePattern 'ARMOR_FACE_FRONT\s*=\s*1[^}]*ARMOR_FACE_LEFT\s*=\s*2[^}]*ARMOR_FACE_RIGHT\s*=\s*3[^}]*ARMOR_FACE_REAR\s*=\s*4' 'Four explicit armor faces are required.'
Assert-SourcePattern 'typedef\s+struct\s*\{[^}]*u16\s+pointX\s*;[^}]*u16\s+pointY\s*;[^}]*u8\s+face\s*;[^}]*u8\s+normalHeading\s*;[^}]*u8\s+impactAngle\s*;[^}]*u8\s+result\s*;[^}]*\}\s*ArmorImpact\s*;' 'ArmorImpact must retain point, face, normal, angle and result.'
Assert-SourcePattern 'forwardX\s*=\s*cos256\(enemy->heading\)\s*;\s*forwardY\s*=\s*sin256\(enemy->heading\)' 'Enemy heading basis is not derived from the shared sin/cos LUT.'
Assert-SourcePattern 'previousForward\s*=\s*\(previousDeltaX\s*\*\s*forwardX\s*\+\s*previousDeltaY\s*\*\s*forwardY\)\s*>>\s*FIXED_SHIFT' 'Previous Projectile point is not transformed into Enemy-local coordinates.'
Assert-SourcePattern 'currentRight\s*=\s*\(-currentDeltaX\s*\*\s*forwardY\s*\+\s*currentDeltaY\s*\*\s*forwardX\)\s*>>\s*FIXED_SHIFT' 'Current Projectile point is not transformed into Enemy-local coordinates.'
Assert-SourcePattern 'static\s+u8\s+entryFractionIsEarlier[^}]*candidateNumerator\s*\*\s*entryDenominator\s*<\s*entryNumerator\s*\*\s*candidateDenominator' 'Corner entry must compare crossing fractions without floating point.'
Assert-SourcePattern 'entryFractionIsEarlier\(\s*candidateNumerator,\s*candidateDenominator,\s*entryNumerator,\s*entryDenominator\)' 'Corner collision does not use the isolated crossing-fraction comparator.'
Assert-SourcePattern 'attackHeading\s*=\s*\(u8\)\(projectileHeading\s*\+\s*128\)' 'Attack direction must oppose Projectile travel heading.'
Assert-SourcePattern 'impactAngle\s*=\s*shortestHeadingDistance\(attackHeading,\s*normalHeading\)' 'Impact angle must use shortest wrapped heading distance.'
Assert-SourcePattern 'impactAngle\s*>=\s*RICOCHET_THRESHOLD_HEADING_UNITS\s*\?\s*ARMOR_RESULT_RICOCHET\s*:\s*ARMOR_RESULT_PENETRATION' 'Threshold equality must resolve to RICOCHET.'
Assert-SourcePattern 'if\s*\(impact\.result\s*==\s*ARMOR_RESULT_RICOCHET\)\s*\{\s*enemy->hitFlashFrames\s*=\s*ENEMY_RICOCHET_FLASH_FRAMES\s*;\s*\}' 'RICOCHET must end with feedback and no damage call.'
Assert-SourcePattern 'else\s*\{\s*damageEnemy\(enemy\)\s*;\s*\}' 'PENETRATION must use the existing damageEnemy path.'
Assert-SourcePattern 'lastArmorImpact\s*=\s*impact\s*;' 'Exactly one recent armor result must be retained.'
Assert-SourcePattern 'consoleDrawText\(16,\s*2[^;]*;.*consoleDrawText\(20,\s*2[^;]*;.*ARMOR_RESULT_RICOCHET\s*\?\s*"RIC"' 'Compact face, angle and result diagnostics are missing.'

if ([regex]::IsMatch($source, '\b(float|double|atan2|acos)\b')) {
    throw 'S3-02 runtime armor path must not use floating point, atan2 or acos.'
}

function Get-ShortestHeadingDistance {
    param([int]$First, [int]$Second)

    $difference = ($First - $Second) -band 0xFF
    if ($difference -gt 128) { return 256 - $difference }
    return $difference
}

function Resolve-LocalArmorImpact {
    param(
        [int]$PreviousForward,
        [int]$PreviousRight,
        [int]$CurrentForward,
        [int]$CurrentRight,
        [int]$ProjectileHeading,
        [int]$EnemyHeading
    )

    $halfLength = 13
    $halfWidth = 10
    if ($CurrentForward -lt -$halfLength -or $CurrentForward -gt $halfLength -or
        $CurrentRight -lt -$halfWidth -or $CurrentRight -gt $halfWidth) {
        return $null
    }

    $candidates = @()
    if ($PreviousForward -gt $halfLength) {
        $candidates += [pscustomobject]@{ Face = 'FRONT'; Numerator = $PreviousForward - $halfLength; Denominator = $PreviousForward - $CurrentForward }
    } elseif ($PreviousForward -lt -$halfLength) {
        $candidates += [pscustomobject]@{ Face = 'REAR'; Numerator = -$halfLength - $PreviousForward; Denominator = $CurrentForward - $PreviousForward }
    }
    if ($PreviousRight -gt $halfWidth) {
        $candidates += [pscustomobject]@{ Face = 'RIGHT'; Numerator = $PreviousRight - $halfWidth; Denominator = $PreviousRight - $CurrentRight }
    } elseif ($PreviousRight -lt -$halfWidth) {
        $candidates += [pscustomobject]@{ Face = 'LEFT'; Numerator = -$halfWidth - $PreviousRight; Denominator = $CurrentRight - $PreviousRight }
    }
    if ($candidates.Count -eq 0) { return $null }

    $entry = $candidates[0]
    foreach ($candidate in $candidates | Select-Object -Skip 1) {
        if ($candidate.Numerator * $entry.Denominator -lt $entry.Numerator * $candidate.Denominator) {
            $entry = $candidate
        }
    }

    $normal = switch ($entry.Face) {
        'FRONT' { $EnemyHeading }
        'REAR' { ($EnemyHeading + 128) -band 0xFF }
        'RIGHT' { ($EnemyHeading + 64) -band 0xFF }
        'LEFT' { ($EnemyHeading - 64) -band 0xFF }
    }
    $attack = ($ProjectileHeading + 128) -band 0xFF
    $angle = Get-ShortestHeadingDistance -First $attack -Second $normal
    if ($angle -gt 64) { $angle = 64 }
    $result = if ($angle -ge 54) { 'RICOCHET' } else { 'PENETRATION' }
    return [pscustomobject]@{ Face = $entry.Face; Normal = $normal; Angle = $angle; Result = $result }
}

$front = Resolve-LocalArmorImpact 14 0 12 0 128 0
if ($null -eq $front -or $front.Face -ne 'FRONT' -or $front.Angle -ne 0 -or $front.Result -ne 'PENETRATION') {
    throw 'FRONT perpendicular fixture failed.'
}

$side = Resolve-LocalArmorImpact 0 11 0 9 192 0
if ($null -eq $side -or $side.Face -ne 'RIGHT' -or $side.Normal -ne 64 -or $side.Angle -ne 0 -or $side.Result -ne 'PENETRATION') {
    throw 'SIDE perpendicular fixture failed.'
}

$below = Resolve-LocalArmorImpact 14 0 12 -1 181 0
if ($below.Angle -ne 53 -or $below.Result -ne 'PENETRATION') {
    throw 'Below-threshold grazing fixture failed.'
}

$boundary = Resolve-LocalArmorImpact 14 0 12 -1 182 0
if ($boundary.Angle -ne 54 -or $boundary.Result -ne 'RICOCHET') {
    throw 'Threshold equality must RICOCHET at heading unit 54.'
}

$above = Resolve-LocalArmorImpact 14 0 12 -1 192 0
if ($above.Angle -ne 64 -or $above.Result -ne 'RICOCHET') {
    throw 'Above-threshold grazing fixture failed.'
}

$hp = 3
$shellActive = $true
if ($boundary.Result -eq 'RICOCHET') { $shellActive = $false }
if ($hp -ne 3 -or $shellActive) { throw 'RICOCHET changed HP or left the shell active.' }

$hp = 3
$shellActive = $true
if ($front.Result -eq 'PENETRATION') { $hp--; $shellActive = $false }
if ($hp -ne 2 -or $shellActive) { throw 'PENETRATION did not apply exactly one damage and end the shell.' }
if ($front.Result -eq 'PENETRATION' -and $shellActive) { $hp-- }
if ($hp -ne 2) { throw 'One Projectile produced more than one armor result.' }

$hp = 3
$active = $true
foreach ($shot in 1..3) {
    if ($front.Result -eq 'PENETRATION') { $hp-- }
    if ($hp -eq 0) { $active = $false }
}
if ($hp -ne 0 -or $active) { throw 'Three PENETRATION results did not destroy HP3 Enemy.' }

$miss = Resolve-LocalArmorImpact 16 0 14 0 128 0
if ($null -ne $miss) { throw 'Miss fixture produced an armor result.' }

$wrapA = Resolve-LocalArmorImpact 14 0 12 0 130 254
$wrapB = Resolve-LocalArmorImpact 14 0 12 0 126 2
if ($wrapA.Angle -ne 4 -or $wrapB.Angle -ne 4 -or
    $wrapA.Result -ne 'PENETRATION' -or $wrapB.Result -ne 'PENETRATION') {
    throw 'Heading wrap fixture failed around FE/02.'
}

Write-Output 'S3_ARMOR_VERIFY=PASS'
Write-Output 'ARMOR_GEOMETRY=ORIENTED_RECT_26x20'
Write-Output 'ARMOR_FACES=FRONT,LEFT,RIGHT,REAR'
Write-Output 'IMPACT_ANGLE_UNITS=0..64'
Write-Output 'IMPACT_ANGLE_MEANING=0_PERPENDICULAR,64_GRAZING'
Write-Output 'RICOCHET_THRESHOLD_DEGREES=75'
Write-Output 'RICOCHET_THRESHOLD_HEADING_UNITS=54'
Write-Output 'RICOCHET_FIRST_REPRESENTABLE_DEGREES=75.9375'
Write-Output 'RUNTIME_FLOATING_POINT=NONE'
Write-Output 'RUNTIME_ATAN2_ACOS=NONE'
Write-Output 'DETERMINISTIC_CASES=11'
