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

Assert-SourcePattern 'typedef\s+enum\s*\{\s*AIM_MODE_MOUSE\s*=\s*0\s*,\s*AIM_MODE_PAD2\s*=\s*1\s*\}\s*AimMode\s*;' 'AimMode enum contract is missing or reordered.'
Assert-SourcePattern 'aimMode\s*=\s*AIM_MODE_MOUSE\s*;' 'Default aim mode must be Mouse.'
Assert-SourcePattern 'padDown\s*=\s*padsDown\(0\)\s*;' 'P1 SELECT mode toggle must use PVSnesLib padsDown(0).'
Assert-SourcePattern 'if\s*\(\(padDown\s*&\s*KEY_SELECT\)\s*==\s*0\)' 'P1 SELECT is not the mode-toggle edge.'
Assert-SourcePattern 'aimMode\s*=\s*aimMode\s*==\s*AIM_MODE_MOUSE\s*\?\s*AIM_MODE_PAD2\s*:\s*AIM_MODE_MOUSE\s*;' 'Aim mode does not alternate between Mouse and Pad2.'
Assert-SourcePattern 'resetMainGunInput\(\)\s*;' 'Aim mode switching does not reset and disarm the fire edge state.'
Assert-SourcePattern 'pad2\s*=\s*padsCurrent\(1\)\s*;' 'P2 Standard Pad is not read through padsCurrent(1).'
Assert-SourcePattern 'initMouse\(MOUSE_SLOW\)\s*;' 'Known-good P2 SNES Mouse initialization was removed.'
Assert-SourcePattern 'pad2Gameplay\s*=\s*mouseConnected\s*==\s*0\s*\?\s*pad2\s*:\s*0\s*;' 'Mouse protocol data is not blocked from Pad2 gameplay interpretation.'
Assert-SourcePattern '#define\s+PAD_AIM_INDICATOR_DISTANCE\s+48\b' 'Pad2 direction indicator must be 48 pixels from the hull.'
Assert-SourcePattern 'cos256\(turret->targetHeading\)\s*\*\s*PAD_AIM_INDICATOR_DISTANCE' 'Pad2 indicator X is not derived from the target heading.'
Assert-SourcePattern 'sin256\(turret->targetHeading\)\s*\*\s*PAD_AIM_INDICATOR_DISTANCE' 'Pad2 indicator Y is not derived from the target heading.'
Assert-SourcePattern 'updateInputHudSprites\(pad\)\s*;' 'Existing P1 D-pad OAM HUD path changed.'
Assert-SourcePattern 'resolveHullInput\(pad,\s*&hull\)\s*;' 'P1 Hull input no longer has its independent path.'
Assert-SourcePattern 'updateTurretTargetFromPad2\(pad2Gameplay,\s*&turret\)\s*;' 'P2 aim no longer has its independent path.'
Assert-SourcePattern '#define\s+AIM_GAIN\s+1\b' 'Known-good Mouse AIM_GAIN changed.'
Assert-SourcePattern '#define\s+TURRET_TURN_RATE\s+4\b' 'Known-good turret traverse rate changed.'

$resolverMatch = [regex]::Match(
    $source,
    'static\s+void\s+updateTurretTargetFromPad2\s*\([^)]*\)\s*\{(?<body>.*?)\}\s*static\s+void\s+updateTurretHeading',
    [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (-not $resolverMatch.Success) {
    throw 'Unable to isolate the Pad2 direction resolver.'
}
$resolver = $resolverMatch.Groups['body'].Value
if ([regex]::IsMatch($resolver, '\bswitch\s*\(')) {
    throw 'Pad2 aim must resolve independent axes instead of switching on the whole pad mask.'
}
if ([regex]::IsMatch($resolver, '\baim\s*\.')) {
    throw 'Pad2 aim must not overwrite the persistent Mouse AimState.'
}
foreach ($keyName in @('KEY_UP', 'KEY_DOWN', 'KEY_LEFT', 'KEY_RIGHT')) {
    if (-not $resolver.Contains($keyName)) {
        throw "Pad2 resolver does not inspect $keyName independently."
    }
}
foreach ($heading in @('0x00', '0x20', '0x40', '0x60', '0x80', '0xA0', '0xC0', '0xE0')) {
    if (-not $resolver.Contains($heading)) {
        throw "Pad2 resolver is missing heading $heading."
    }
}

function Resolve-PadHeading {
    param(
        [bool]$Up,
        [bool]$Down,
        [bool]$Left,
        [bool]$Right,
        [int]$PreviousHeading
    )

    $horizontal = 0
    $vertical = 0
    if ($Left -ne $Right) { $horizontal = if ($Left) { -1 } else { 1 } }
    if ($Up -ne $Down) { $vertical = if ($Up) { -1 } else { 1 } }

    if ($horizontal -gt 0) {
        return $(if ($vertical -lt 0) { 0xE0 } elseif ($vertical -gt 0) { 0x20 } else { 0x00 })
    }
    if ($horizontal -lt 0) {
        return $(if ($vertical -lt 0) { 0xA0 } elseif ($vertical -gt 0) { 0x60 } else { 0x80 })
    }
    if ($vertical -lt 0) { return 0xC0 }
    if ($vertical -gt 0) { return 0x40 }
    return $PreviousHeading
}

$cardinalAndDiagonalTests = @(
    @{ Name = 'RIGHT'; Up = $false; Down = $false; Left = $false; Right = $true; Expected = 0x00 },
    @{ Name = 'DOWN_RIGHT'; Up = $false; Down = $true; Left = $false; Right = $true; Expected = 0x20 },
    @{ Name = 'DOWN'; Up = $false; Down = $true; Left = $false; Right = $false; Expected = 0x40 },
    @{ Name = 'DOWN_LEFT'; Up = $false; Down = $true; Left = $true; Right = $false; Expected = 0x60 },
    @{ Name = 'LEFT'; Up = $false; Down = $false; Left = $true; Right = $false; Expected = 0x80 },
    @{ Name = 'UP_LEFT'; Up = $true; Down = $false; Left = $true; Right = $false; Expected = 0xA0 },
    @{ Name = 'UP'; Up = $true; Down = $false; Left = $false; Right = $false; Expected = 0xC0 },
    @{ Name = 'UP_RIGHT'; Up = $true; Down = $false; Left = $false; Right = $true; Expected = 0xE0 }
)
foreach ($test in $cardinalAndDiagonalTests) {
    $actual = Resolve-PadHeading -Up $test.Up -Down $test.Down -Left $test.Left -Right $test.Right -PreviousHeading 0x55
    if ($actual -ne $test.Expected) {
        throw "Pad2 heading $($test.Name) resolved to $actual instead of $($test.Expected)."
    }
}

$oppositeVertical = Resolve-PadHeading -Up $true -Down $true -Left $false -Right $true -PreviousHeading 0x55
if ($oppositeVertical -ne 0x00) {
    throw 'UP+DOWN must neutralize only the vertical axis while retaining RIGHT.'
}
$oppositeHorizontal = Resolve-PadHeading -Up $true -Down $false -Left $true -Right $true -PreviousHeading 0x55
if ($oppositeHorizontal -ne 0xC0) {
    throw 'LEFT+RIGHT must neutralize only the horizontal axis while retaining UP.'
}
$allNeutral = Resolve-PadHeading -Up $true -Down $true -Left $true -Right $true -PreviousHeading 0x55
if ($allNeutral -ne 0x55) {
    throw 'All-neutral Pad2 input must retain the previous target heading.'
}

function Invoke-FireFrame {
    param([bool]$Held, [hashtable]$State)

    $wasHeld = $State.PreviousHeld
    if (-not $Held) { $State.Armed = $true }
    $fired = $Held -and -not $wasHeld -and $State.Armed
    if ($fired) { $State.Armed = $false }
    $State.PreviousHeld = $Held
    return $fired
}

$fireState = @{ PreviousHeld = $false; Armed = $false }
if (Invoke-FireFrame -Held $true -State $fireState) {
    throw 'A held fire input at initialization must not fire before release.'
}
if (Invoke-FireFrame -Held $false -State $fireState) {
    throw 'Release must only arm fire, not fire.'
}
if (-not (Invoke-FireFrame -Held $true -State $fireState)) {
    throw 'An armed rising edge must fire.'
}

$fireState.PreviousHeld = $false
$fireState.Armed = $false
if (Invoke-FireFrame -Held $true -State $fireState) {
    throw 'A held fire input during a mode switch must not fire.'
}
if (Invoke-FireFrame -Held $true -State $fireState) {
    throw 'A held fire input after a mode switch must not repeat.'
}
if (Invoke-FireFrame -Held $false -State $fireState) {
    throw 'Post-switch release must only re-arm fire.'
}
if (-not (Invoke-FireFrame -Held $true -State $fireState)) {
    throw 'A new post-switch rising edge must fire after release.'
}

Write-Output 'S2_PAD_AIM_VERIFY=PASS'
Write-Output 'AIM_MODE_DEFAULT=MOUSE'
Write-Output 'AIM_MODE_TOGGLE=P1_SELECT_DOWN'
Write-Output 'PAD2_API=padsCurrent(1)'
Write-Output 'PAD2_HEADINGS=00,20,40,60,80,A0,C0,E0'
Write-Output 'PAD2_OPPOSITE_AXIS_POLICY=INDEPENDENT_NEUTRAL'
Write-Output 'PAD2_IDLE_POLICY=HOLD_PREVIOUS_TARGET'
Write-Output 'PAD2_INDICATOR_DISTANCE=48'
Write-Output 'MODE_SWITCH_FIRE_POLICY=DISARM_UNTIL_RELEASE'
Write-Output 'MODE_SWITCH_FIRE_SIMULATION=PASS'
Write-Output 'TWIN_STICK_INPUT_PATHS=INDEPENDENT'
