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

Assert-SourcePattern '#define\s+MAX_PLAYER_SHELLS\s+4\b' 'Projectile pool must contain exactly four static slots.'
Assert-SourcePattern '#define\s+FIRE_COOLDOWN_FRAMES\s+18\b' 'Main-gun cooldown must be 18 frames.'
Assert-SourcePattern '#define\s+MUZZLE_DISTANCE\s+16\b' 'Muzzle distance must be 16 pixels.'
Assert-SourcePattern '#define\s+SHELL_SPEED_PIXELS\s+4\b' 'Shell speed must be 4 pixels per frame.'
Assert-SourcePattern 'static\s+ProjectileState\s+projectiles\[MAX_PLAYER_SHELLS\]' 'Projectile pool is not statically allocated.'
Assert-SourcePattern 'initializeProjectiles\s*\(\s*\)\s*;' 'Projectile pool is not explicitly initialized for emulator and hardware RAM consistency.'
Assert-SourcePattern 'projectiles\[index\]\.active\s*=\s*0\s*;' 'Projectile active flags are not explicitly cleared.'
Assert-SourcePattern 'u8\s+shotHeading\s*=\s*turret->heading\s*;' 'Projectile heading is not snapped from the current turret heading.'
Assert-SourcePattern 'projectiles\[index\]\.heading\s*=\s*shotHeading\s*;' 'Projectile heading snapshot is not stored in the pool.'
Assert-SourcePattern 'velocityX\s*=\s*\(s16\)\(directionX\s*\*\s*SHELL_SPEED_PIXELS\)' 'Projectile X velocity does not use the turret forward vector.'
Assert-SourcePattern 'velocityY\s*=\s*\(s16\)\(directionY\s*\*\s*SHELL_SPEED_PIXELS\)' 'Projectile Y velocity does not use the turret forward vector.'
Assert-SourcePattern 'leftHeld\s*!=\s*0\s*&&\s*leftWasHeld\s*==\s*0\s*&&\s*fireInputArmed\s*!=\s*0' 'Left mouse fire is not armed rising-edge triggered.'
Assert-SourcePattern 'if\s*\(leftHeld\s*==\s*0\)\s*\{\s*fireInputArmed\s*=\s*1\s*;' 'Main gun is not armed by an observed released state.'
Assert-SourcePattern 'previousMouseButtons\s*=\s*mouseButtons\s*;' 'Previous mouse state is not retained for edge detection.'
Assert-SourcePattern 'mouseButtons\s*&\s*mouse_L' 'P2 SNES Mouse Left is not used for main-gun input.'
Assert-SourcePattern 'PROJECTILE_OAM_BASE\s+32\b' 'Projectile OAM range must begin after the S2-01 sprites.'
Assert-SourcePattern 'INPUT_HUD_SHELL_TILE\s*\(INPUT_HUD_TILE_BASE\s*\+\s*68\)' 'Projectile tile must use the reserved HUD tile region.'
Assert-SourcePattern 'pixelX\s*<\s*PROJECTILE_MIN_X\s*\|\|\s*pixelX\s*>\s*PROJECTILE_MAX_X' 'Projectile X cleanup bounds are missing.'

if ([regex]::IsMatch($source, 'spawnProjectile\s*\([^)]*mouse_R', [System.Text.RegularExpressions.RegexOptions]::Singleline)) {
    throw 'P2 SNES Mouse Right must not feed projectile spawning.'
}

$quarterSinMatch = [regex]::Match(
    $source,
    'static const s16 quarterSin\[65\] = \{(?<values>.*?)\};',
    [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (-not $quarterSinMatch.Success) {
    throw 'Unable to find the existing quarterSin table.'
}
$quarterSin = @([regex]::Matches($quarterSinMatch.Groups['values'].Value, '-?\d+') | ForEach-Object { [int]$_.Value })
if ($quarterSin.Count -ne 65) {
    throw "Invalid quarterSin table count: $($quarterSin.Count)."
}

function Resolve-Sin256 {
    param([int]$Heading)
    $quadrant = $Heading -shr 6
    $index = $Heading -band 0x3F
    if ($quadrant -eq 0) { return $quarterSin[$index] }
    if ($quadrant -eq 1) { return $quarterSin[64 - $index] }
    if ($quadrant -eq 2) { return -$quarterSin[$index] }
    return -$quarterSin[64 - $index]
}

$maximumSpeedError = 0.0
foreach ($heading in 0..255) {
    $velocityX = (Resolve-Sin256 -Heading (($heading + 64) -band 255)) * 4
    $velocityY = (Resolve-Sin256 -Heading $heading) * 4
    $speed = [Math]::Sqrt(($velocityX * $velocityX) + ($velocityY * $velocityY)) / 256.0
    $maximumSpeedError = [Math]::Max($maximumSpeedError, [Math]::Abs($speed - 4.0))
}
if ($maximumSpeedError -gt 0.02) {
    throw "Shell speed error is too large: $maximumSpeedError pixels/frame."
}

$oamIds = @(0, 4, 8, 12, 16, 20, 24, 28)
foreach ($index in 0..3) {
    $oamIds += 32 + ($index * 4)
}
if (($oamIds | Sort-Object -Unique).Count -ne 12) {
    throw 'S2-02 OAM IDs overlap.'
}
if (($oamIds | Measure-Object -Maximum).Maximum -ne 44) {
    throw 'S2-02 OAM range exceeds the expected slot 11.'
}

Write-Output 'S2_PROJECTILE_VERIFY=PASS'
Write-Output 'FIRE_INPUT=P2_MOUSE_LEFT_RISING_EDGE'
Write-Output 'PROJECTILE_POOL_SIZE=4'
Write-Output 'FIRE_COOLDOWN_FRAMES=18'
Write-Output 'MUZZLE_DISTANCE_PIXELS=16'
Write-Output 'SHELL_SPEED_PIXELS_PER_FRAME=4'
Write-Output ('MAX_SHELL_SPEED_ERROR={0:F6}' -f $maximumSpeedError)
Write-Output 'PLAYER_OAM_OBJECTS_MAX=12'
Write-Output 'PROJECTILE_OAM_IDS=32,36,40,44'
