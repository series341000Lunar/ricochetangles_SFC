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

Assert-SourcePattern '#define\s+ENEMY_INITIAL_X\s+208\b' 'Static Enemy initial X must be 208.'
Assert-SourcePattern '#define\s+ENEMY_INITIAL_Y\s+144\b' 'Static Enemy initial Y must be 144.'
Assert-SourcePattern '#define\s+ENEMY_INITIAL_HEADING\s+128\b' 'Static Enemy must face LEFT at heading 128.'
Assert-SourcePattern '#define\s+ENEMY_MAX_HP\s+3\b' 'Static Enemy verification HP must be 3.'
Assert-SourcePattern '#define\s+ENEMY_ARMOR_HALF_LENGTH\s+13\b' 'Enemy armor half-length must preserve the S3-01 longitudinal extent.'
Assert-SourcePattern '#define\s+ENEMY_ARMOR_HALF_WIDTH\s+10\b' 'Enemy armor half-width must define a non-square local hull rectangle.'
Assert-SourcePattern '#define\s+ENEMY_HIT_FLASH_FRAMES\s+6\b' 'Enemy hit blink must last 6 frames.'
Assert-SourcePattern '#define\s+SHELL_SPEED_PIXELS\s+4\b' 'Known-good shell speed changed.'
Assert-SourcePattern 'typedef\s+struct\s*\{[^}]*u16\s+positionX\s*;[^}]*u16\s+positionY\s*;[^}]*u8\s+heading\s*;[^}]*u8\s+hp\s*;[^}]*u8\s+maxHp\s*;[^}]*u8\s+active\s*;[^}]*u8\s+hitFlashFrames\s*;[^}]*\}\s*EnemyState\s*;' 'Minimal EnemyState contract is missing.'
Assert-SourcePattern 'static\s+EnemyState\s+enemy\s*;' 'S3-01 must use exactly one static Enemy State.'
Assert-SourcePattern 'static\s+u8\s+enemyHitCount\s*;' 'Diagnostic hit counter is missing.'
Assert-SourcePattern 'enemy\.positionX\s*=\s*\(u16\)ENEMY_INITIAL_X\s*<<\s*FIXED_SHIFT\s*;' 'Enemy reset does not restore initial X.'
Assert-SourcePattern 'enemy\.positionY\s*=\s*\(u16\)ENEMY_INITIAL_Y\s*<<\s*FIXED_SHIFT\s*;' 'Enemy reset does not restore initial Y.'
Assert-SourcePattern 'projectiles\[index\]\.active\s*=\s*0\s*;\s*lastArmorImpact\s*=\s*impact\s*;' 'An armor interaction must deactivate its shell before resolving the outcome.'
Assert-SourcePattern 'else\s*\{\s*damageEnemy\(enemy\)\s*;\s*\}\s*continue\s*;' 'The penetration branch must apply one damage and leave the slot loop.'
Assert-SourcePattern 'enemy->hp--\s*;\s*enemyHitCount\+\+\s*;' 'A valid hit must decrement HP and increment the diagnostic count exactly once.'
Assert-SourcePattern 'if\s*\(enemy->hp\s*==\s*0\)\s*\{\s*enemy->active\s*=\s*0\s*;' 'HP 0 must deactivate the Enemy.'
Assert-SourcePattern 'if\s*\(\(padDown\s*&\s*KEY_SELECT\)\s*!=\s*0\)\s*\{\s*initializeProjectiles\(\)\s*;\s*initializeEnemy\(\)\s*;' 'P1 SELECT must reset the Enemy and clear all active shells.'
Assert-SourcePattern 'oamSetGfxOffset\(ENEMY_OAM_ID,\s*hullVisualTile\(' 'Enemy visual must reuse the existing Hull graphics.'
Assert-SourcePattern 'oamSetEx\(ENEMY_OAM_ID,\s*OBJ_LARGE,\s*OBJ_SHOW\)' 'Enemy must use one 32x32 large OBJ.'
Assert-SourcePattern 'updateProjectiles\(&enemy\)\s*;' 'Projectile update is not connected to the static Enemy collision state.'

$collisionMatch = [regex]::Match(
    $source,
    'static\s+u8\s+resolveEnemyArmorImpact\s*\([^)]*\)\s*\{(?<body>.*?)\}\s*static\s+void\s+damageEnemy',
    [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (-not $collisionMatch.Success) {
    throw 'Unable to isolate the S3-02 heading-aware armor collision test.'
}
$collisionBody = $collisionMatch.Groups['body'].Value
foreach ($requiredToken in @('ENEMY_ARMOR_HALF_LENGTH', 'ENEMY_ARMOR_HALF_WIDTH', 'enemy->heading', 'previousForward', 'previousRight', 'currentForward', 'currentRight')) {
    if (-not $collisionBody.Contains($requiredToken)) {
        throw "Enemy collision test is missing $requiredToken."
    }
}

$updateMatch = [regex]::Match(
    $source,
    'static\s+void\s+updateProjectiles\s*\([^)]*\)\s*\{(?<body>.*?)\}\s*static\s+void\s+updateEnemyHitFlash',
    [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (-not $updateMatch.Success) {
    throw 'Unable to isolate the Projectile update order.'
}
$updateBody = $updateMatch.Groups['body'].Value
$moveIndex = $updateBody.IndexOf('addProjectileDelta', [System.StringComparison]::Ordinal)
$hitIndex = $updateBody.IndexOf('resolveEnemyArmorImpact', [System.StringComparison]::Ordinal)
if ($moveIndex -lt 0 -or $hitIndex -lt 0 -or $moveIndex -ge $hitIndex) {
    throw 'Projectile collision must be tested after the Projectile moves.'
}

$oamIds = @(0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48)
if (($oamIds | Sort-Object -Unique).Count -ne 13) {
    throw 'S3-01 OAM IDs overlap.'
}

function Invoke-FixtureFrame {
    param(
        [Parameter(Mandatory)][hashtable]$Fixture,
        [Parameter(Mandatory)][hashtable]$Shell
    )

    if (-not $Shell.Active) { return }
    $Shell.X += 4
    $inside = $Fixture.Active -and
        $Shell.X -ge (208 - 13) -and $Shell.X -le (208 + 13) -and
        $Shell.Y -ge (144 - 13) -and $Shell.Y -le (144 + 13)
    if ($inside) {
        $Shell.Active = $false
        $Fixture.Hp--
        $Fixture.Hits++
        if ($Fixture.Hp -eq 0) { $Fixture.Active = $false }
    }
}

$fixture = @{ Hp = 3; Active = $true; Hits = 0 }
$missShell = @{ X = 192; Y = 130; Active = $true }
Invoke-FixtureFrame -Fixture $fixture -Shell $missShell
if ($fixture.Hp -ne 3 -or $fixture.Hits -ne 0 -or -not $missShell.Active) {
    throw 'Deterministic miss fixture changed Enemy HP or removed the shell.'
}

$hitShell = @{ X = 192; Y = 144; Active = $true }
Invoke-FixtureFrame -Fixture $fixture -Shell $hitShell
if ($fixture.Hp -ne 2 -or $fixture.Hits -ne 1 -or $hitShell.Active) {
    throw 'Deterministic direct-hit fixture did not apply exactly one damage and remove its shell.'
}
Invoke-FixtureFrame -Fixture $fixture -Shell $hitShell
if ($fixture.Hp -ne 2 -or $fixture.Hits -ne 1) {
    throw 'An inactive hit shell applied repeated damage.'
}

foreach ($shot in 1..2) {
    $shell = @{ X = 192; Y = 144; Active = $true }
    Invoke-FixtureFrame -Fixture $fixture -Shell $shell
}
if ($fixture.Hp -ne 0 -or $fixture.Active -or $fixture.Hits -ne 3) {
    throw 'Three valid hits did not destroy the Enemy exactly once per shell.'
}

$fixture = @{ Hp = 3; Active = $true; Hits = 0 }
if ($fixture.Hp -ne 3 -or -not $fixture.Active -or $fixture.Hits -ne 0) {
    throw 'Enemy reset fixture did not restore HP, active state and hit count.'
}

Write-Output 'S3_TARGET_VERIFY=PASS'
Write-Output 'ENEMY_POSITION=208,144'
Write-Output 'ENEMY_HEADING=128'
Write-Output 'ENEMY_HP=3'
Write-Output 'ENEMY_COLLISION=SEGMENT_ORIENTED_RECT_26x20'
Write-Output 'ENEMY_DAMAGE_PER_HIT=1'
Write-Output 'ENEMY_HIT_FLASH_FRAMES=6'
Write-Output 'ENEMY_RESET=P1_SELECT_DOWN'
Write-Output 'ENEMY_GFX_BYTES_ADDED=0'
Write-Output 'ENEMY_PALETTE_BYTES_ADDED=0'
Write-Output 'ENEMY_OAM_ID=48'
Write-Output 'TOTAL_ACTIVE_OBJ_MAX=13'
Write-Output 'WORST_CASE_PROJECTILE_OBJ=4'
