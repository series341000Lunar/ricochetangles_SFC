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

Assert-SourcePattern 'typedef\s+enum\s*\{\s*DRIVE_MODE_PC_LIKE\s*=\s*0\s*,\s*DRIVE_MODE_STICK\s*=\s*1\s*\}\s*DriveMode\s*;' 'DriveMode contract is missing or reordered.'
Assert-SourcePattern 'static\s+u8\s+menuOpen\s*;' 'Menu open state is missing.'
Assert-SourcePattern 'static\s+u8\s+menuSelection\s*;' 'Menu row selection state is missing.'
Assert-SourcePattern 'driveMode\s*=\s*DRIVE_MODE_PC_LIKE\s*;' 'Cold-boot Drive mode must be PC-LIKE.'
Assert-SourcePattern 'aimMode\s*=\s*AIM_MODE_MOUSE\s*;' 'Cold-boot Aim mode must be MOUSE.'
Assert-SourcePattern 'consoleDrawText\(9,\s*2,\s*"RicochetAngles"\)' 'Menu title must remain exactly RicochetAngles.'
Assert-SourcePattern 'consoleDrawText\(9,\s*27,\s*"CHANNEL A BNC"\)' 'Footer must use the exact required text on the bottom tile row.'
Assert-SourcePattern 'if\s*\(\(padDown\s*&\s*KEY_START\)\s*!=\s*0\)\s*\{\s*menuOpen\s*=\s*menuOpen\s*==\s*0\s*\?\s*1\s*:\s*0\s*;' 'P1 START edge must toggle the menu.'
Assert-SourcePattern 'if\s*\(menuOpen\s*!=\s*0\)\s*\{\s*updateControlMenu\(padDown\)\s*;\s*continue\s*;' 'Open menu must stop gameplay fall-through.'
Assert-SourcePattern 'menuOpen\s*=\s*menuOpen\s*==\s*0\s*\?\s*1\s*:\s*0\s*;\s*resetMainGunInput\(\)' 'Opening and closing the menu must disarm fire input.'
Assert-SourcePattern 'resolveHullInputPcLike\(pad,\s*&hull\)' 'PC-LIKE resolver is not connected.'
Assert-SourcePattern 'resolveHullInputStick\(pad,\s*&hull\)' 'STICK resolver is not connected.'
Assert-SourcePattern 'resolvePadHeading\(pad,\s*&hull->targetHeading\)' 'STICK mode must use the shared independent-axis heading resolver.'
Assert-SourcePattern 'hull->throttle\s*=\s*1\s*;' 'Valid STICK direction must request forward throttle.'
Assert-SourcePattern 'if\s*\(resolvePadHeading\(pad,\s*&hull->targetHeading\)\s*==\s*0\)\s*\{\s*hull->throttle\s*=\s*0\s*;' 'Released or fully neutral STICK must coast.'
Assert-SourcePattern 'delta\s*=\s*\(s8\)\(hull->targetHeading\s*-\s*hull->heading\)' 'STICK heading must use wrap-around shortest-path delta.'
Assert-SourcePattern 'hull->heading\s*=\s*\(u8\)\(hull->heading\s*\+\s*TURN_RATE\)' 'STICK clockwise turn must retain TURN_RATE.'
Assert-SourcePattern 'hull->heading\s*=\s*\(u8\)\(hull->heading\s*-\s*TURN_RATE\)' 'STICK counter-clockwise turn must retain TURN_RATE.'
Assert-SourcePattern 'if\s*\(\(padDown\s*&\s*KEY_SELECT\)\s*!=\s*0\)\s*\{\s*initializeProjectiles\(\)\s*;\s*initializeEnemy\(\)' 'DEV Enemy reset must move to SELECT.'

if ([regex]::IsMatch($source, 'updateAimMode\s*\(')) {
    throw 'The old SELECT Aim quick-toggle path must be removed.'
}

$menuPauseMatch = [regex]::Match(
    $source,
    'if\s*\(menuOpen\s*!=\s*0\)\s*\{(?<body>.*?)\}\s*if\s*\(\(padDown\s*&\s*KEY_SELECT\)',
    [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (-not $menuPauseMatch.Success -or -not $menuPauseMatch.Groups['body'].Value.Contains('continue')) {
    throw 'Unable to prove that Menu frames stop before simulation updates.'
}

function Resolve-PadHeading {
    param([bool]$Up, [bool]$Down, [bool]$Left, [bool]$Right, [int]$Previous)

    $horizontal = 0
    $vertical = 0
    if ($Left -ne $Right) { $horizontal = if ($Left) { -1 } else { 1 } }
    if ($Up -ne $Down) { $vertical = if ($Up) { -1 } else { 1 } }
    if ($horizontal -gt 0) { return $(if ($vertical -lt 0) { 0xE0 } elseif ($vertical -gt 0) { 0x20 } else { 0x00 }) }
    if ($horizontal -lt 0) { return $(if ($vertical -lt 0) { 0xA0 } elseif ($vertical -gt 0) { 0x60 } else { 0x80 }) }
    if ($vertical -lt 0) { return 0xC0 }
    if ($vertical -gt 0) { return 0x40 }
    return $Previous
}

$tests = @(
    @{ U = $false; D = $false; L = $false; R = $true; H = 0x00 },
    @{ U = $false; D = $true; L = $false; R = $true; H = 0x20 },
    @{ U = $false; D = $true; L = $false; R = $false; H = 0x40 },
    @{ U = $false; D = $true; L = $true; R = $false; H = 0x60 },
    @{ U = $false; D = $false; L = $true; R = $false; H = 0x80 },
    @{ U = $true; D = $false; L = $true; R = $false; H = 0xA0 },
    @{ U = $true; D = $false; L = $false; R = $false; H = 0xC0 },
    @{ U = $true; D = $false; L = $false; R = $true; H = 0xE0 }
)
foreach ($test in $tests) {
    $actual = Resolve-PadHeading -Up $test.U -Down $test.D -Left $test.L -Right $test.R -Previous 0x55
    if ($actual -ne $test.H) { throw "STICK heading fixture failed: expected $($test.H), got $actual." }
}
if ((Resolve-PadHeading -Up $true -Down $true -Left $false -Right $true -Previous 0x55) -ne 0x00) {
    throw 'STICK UP+DOWN+RIGHT must resolve RIGHT.'
}
if ((Resolve-PadHeading -Up $true -Down $false -Left $true -Right $true -Previous 0x55) -ne 0xC0) {
    throw 'STICK LEFT+RIGHT+UP must resolve UP.'
}
if ((Resolve-PadHeading -Up $true -Down $true -Left $true -Right $true -Previous 0x55) -ne 0x55) {
    throw 'Fully opposed STICK input must preserve heading and coast.'
}

$paused = @{ X = 1234; Heading = 42; Speed = 300; Turret = 77; ShellX = 900; Cooldown = 9; Flash = 4 }
$snapshot = $paused.Clone()
if (@($paused.Keys | Where-Object { $paused[$_] -ne $snapshot[$_] }).Count -ne 0) {
    throw 'Pause fixture mutated simulation state.'
}

Write-Output 'S3_CONTROL_MENU_VERIFY=PASS'
Write-Output 'MENU_TOGGLE=P1_START_DOWN'
Write-Output 'MENU_ROWS=DRIVE,AIM'
Write-Output 'DRIVE_MODES=PC-LIKE,STICK'
Write-Output 'AIM_MODES=MOUSE,P2_PAD'
Write-Output 'RUNTIME_COMBINATIONS=4'
Write-Output 'STICK_HEADINGS=00,20,40,60,80,A0,C0,E0'
Write-Output 'STICK_OPPOSITE_AXIS_POLICY=INDEPENDENT_NEUTRAL'
Write-Output 'STICK_THROTTLE=FORWARD_OR_COAST'
Write-Output 'PAUSE_SIMULATION_FALLTHROUGH=BLOCKED'
Write-Output 'RESUME_FIRE_POLICY=DISARM_UNTIL_RELEASE'
Write-Output 'DEV_ENEMY_RESET=P1_SELECT_DOWN'
Write-Output 'FOOTER=CHANNEL_A_BNC_ROW_27'
