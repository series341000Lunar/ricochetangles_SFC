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

Assert-SourcePattern '#define\s+ENEMY_CLASS_HEAVY_TEST\s+1\b' 'Heavy Test Enemy class ID must be explicit and minimal.'
Assert-SourcePattern '#define\s+ENEMY_INITIAL_X\s+223\b' 'Heavy Test Enemy initial X must preserve range without clipping.'
Assert-SourcePattern '#define\s+ENEMY_INITIAL_Y\s+144\b' 'Static Enemy initial Y must be 144.'
Assert-SourcePattern '#define\s+ENEMY_INITIAL_HEADING\s+128\b' 'Static Enemy must face LEFT at heading 128.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_MAX_HP\s+20\b' 'Heavy Test Enemy verification HP must be decimal 20.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_ARMOR_HALF_LENGTH\s+28\b' 'Heavy Test armor half-length must be 28 pixels.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_ARMOR_HALF_WIDTH\s+16\b' 'Heavy Test armor half-width must be 16 pixels.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_ARMOR_BOUNDING_RADIUS\s+33\b' 'Heavy Test broad-phase radius must contain the oriented armor rectangle.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_MODULE_OFFSET\s+17\b' 'Heavy Test three-segment composition offset must be 17 pixels.'
Assert-SourcePattern '#define\s+ENEMY_HIT_FLASH_FRAMES\s+6\b' 'Enemy hit blink must last 6 frames.'
Assert-SourcePattern '#define\s+SHELL_SPEED_PIXELS\s+4\b' 'Known-good shell speed changed.'
Assert-SourcePattern 'typedef\s+struct\s*\{[^}]*u16\s+positionX\s*;[^}]*u16\s+positionY\s*;[^}]*u8\s+heading\s*;[^}]*u8\s+hp\s*;[^}]*u8\s+maxHp\s*;[^}]*u8\s+active\s*;[^}]*u8\s+hitFlashFrames\s*;[^}]*u8\s+classId\s*;[^}]*\}\s*EnemyState\s*;' 'Minimal EnemyState Heavy Test class field is missing.'
Assert-SourcePattern 'static\s+EnemyState\s+enemy\s*;' 'S3-01 must use exactly one static Enemy State.'
Assert-SourcePattern 'static\s+u8\s+enemyHitCount\s*;' 'Diagnostic hit counter is missing.'
Assert-SourcePattern 'enemy\.positionX\s*=\s*\(u16\)ENEMY_INITIAL_X\s*<<\s*FIXED_SHIFT\s*;' 'Enemy reset does not restore initial X.'
Assert-SourcePattern 'enemy\.positionY\s*=\s*\(u16\)ENEMY_INITIAL_Y\s*<<\s*FIXED_SHIFT\s*;' 'Enemy reset does not restore initial Y.'
Assert-SourcePattern 'enemy\.hp\s*=\s*HEAVY_TANK_TEST_MAX_HP\s*;\s*enemy\.maxHp\s*=\s*HEAVY_TANK_TEST_MAX_HP\s*;' 'Enemy reset does not restore Heavy Test HP 20.'
Assert-SourcePattern 'enemy\.classId\s*=\s*ENEMY_CLASS_HEAVY_TEST\s*;' 'Enemy reset does not restore the Heavy Test class.'
Assert-SourcePattern 'projectiles\[index\]\.active\s*=\s*0\s*;\s*lastArmorImpact\s*=\s*impact\s*;' 'An armor interaction must deactivate its shell before resolving the outcome.'
Assert-SourcePattern 'else\s*\{\s*damageEnemy\(enemy\)\s*;\s*\}\s*continue\s*;' 'The penetration branch must apply one damage and leave the slot loop.'
Assert-SourcePattern 'enemy->hp--\s*;\s*enemyHitCount\+\+\s*;' 'A valid hit must decrement HP and increment the diagnostic count exactly once.'
Assert-SourcePattern 'if\s*\(enemy->hp\s*==\s*0\)\s*\{\s*enemy->active\s*=\s*0\s*;' 'HP 0 must deactivate the Enemy.'
Assert-SourcePattern 'if\s*\(\(padDown\s*&\s*KEY_SELECT\)\s*!=\s*0\)\s*\{\s*initializeProjectiles\(\)\s*;\s*initializeEnemy\(\)\s*;' 'P1 SELECT must reset the Enemy and clear all active shells.'
Assert-SourcePattern 'oamSetGfxOffset\(ENEMY_FRONT_OAM_ID,\s*HEAVY_TANK_TEST_FRONT_TILE_BASE\)' 'Heavy Test front module must use its fixed resident tile window.'
Assert-SourcePattern 'oamSetGfxOffset\(ENEMY_CENTER_OAM_ID,\s*HEAVY_TANK_TEST_CENTER_TILE_BASE\)' 'Heavy Test center module must use its fixed resident tile window.'
Assert-SourcePattern 'oamSetGfxOffset\(ENEMY_REAR_OAM_ID,\s*HEAVY_TANK_TEST_REAR_TILE_BASE\)' 'Heavy Test rear module must use its distinct fixed resident tile window.'
Assert-SourcePattern 'oamSetEx\(ENEMY_FRONT_OAM_ID,\s*OBJ_LARGE,\s*OBJ_SHOW\)' 'Heavy Test front module must use one 32x32 large OBJ.'
Assert-SourcePattern 'oamSetEx\(ENEMY_CENTER_OAM_ID,\s*OBJ_LARGE,\s*OBJ_SHOW\)' 'Heavy Test center module must use one 32x32 large OBJ.'
Assert-SourcePattern 'oamSetEx\(ENEMY_REAR_OAM_ID,\s*OBJ_LARGE,\s*OBJ_SHOW\)' 'Heavy Test rear module must use one 32x32 large OBJ.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_FRONT_GFX_VRAM_ADDRESS\s+0x1A00\b' 'Heavy Test front VRAM base must be 0x1A00.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_CENTER_GFX_VRAM_ADDRESS\s+0x1A40\b' 'Heavy Test center VRAM base must be 0x1A40.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_REAR_GFX_VRAM_ADDRESS\s+0x1A80\b' 'Heavy Test rear VRAM base must be 0x1A80.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_GFX_BYTES\s+24576\b' 'Heavy Test three-segment graphics must occupy 24576 bytes.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_FRAME_BYTES\s+1536\b' 'One Heavy Test resident front/center/rear frame set must occupy 1536 bytes.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_COMPONENT_BYTES\s+512\b' 'Each Heavy Test box segment must occupy 512 bytes per frame.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_FRONT_TILE_BASE\s+416\b' 'Heavy Test front tile base must match VRAM word address 0x1A00.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_CENTER_TILE_BASE\s+420\b' 'Heavy Test center tile base must match VRAM word address 0x1A40.'
Assert-SourcePattern '#define\s+HEAVY_TANK_TEST_REAR_TILE_BASE\s+424\b' 'Heavy Test rear tile base must match VRAM word address 0x1A80.'
Assert-SourcePattern 'loadHeavyTankTestVisualFrame\(headingVisualFrame\(ENEMY_INITIAL_HEADING\)\)' 'Heavy Test initial 16-direction frame upload is missing.'
Assert-SourcePattern 'for\s*\(tileRow\s*=\s*0;\s*tileRow\s*<\s*4;\s*tileRow\+\+\)' 'Heavy Test frame upload must cover four 32x32 tile rows.'
Assert-SourcePattern 'dmaCopyVram\(&heavyTankTestTiles\s*\+\s*sourceOffset,\s*frontVramAddress,\s*HEAVY_TANK_TEST_TILE_ROW_BYTES\)' 'Heavy Test front half frame row DMA is missing.'
Assert-SourcePattern 'dmaCopyVram\(&heavyTankTestTiles\s*\+\s*sourceOffset\s*\+\s*HEAVY_TANK_TEST_COMPONENT_BYTES,\s*centerVramAddress,\s*HEAVY_TANK_TEST_TILE_ROW_BYTES\)' 'Heavy Test center segment frame row DMA is missing.'
Assert-SourcePattern 'dmaCopyVram\(&heavyTankTestTiles\s*\+\s*sourceOffset\s*\+\s*\(HEAVY_TANK_TEST_COMPONENT_BYTES\s*\*\s*2\),\s*rearVramAddress,\s*HEAVY_TANK_TEST_TILE_ROW_BYTES\)' 'Heavy Test rear segment frame row DMA is missing.'
Assert-SourcePattern 'dmaCopyCGram\(&heavyTankTestPalette,\s*176,' 'Heavy Test palette must use independent OBJ palette slot 3.'
Assert-SourcePattern 'formatDecimal2\(enemy\.hp,\s*statusValue\)' 'Enemy HP HUD must use two-digit decimal formatting.'
Assert-SourcePattern 'updateProjectiles\(&enemy\)\s*;' 'Projectile update is not connected to the static Enemy collision state.'

$collisionMatch = [regex]::Match(
    $source,
    'static\s+u8\s+resolveEnemyArmorImpact\s*\([^)]*\)\s*\{(?<body>.*?)\}\s*static\s+void\s+damageEnemy',
    [System.Text.RegularExpressions.RegexOptions]::Singleline)
if (-not $collisionMatch.Success) {
    throw 'Unable to isolate the S3-02 heading-aware armor collision test.'
}
$collisionBody = $collisionMatch.Groups['body'].Value
foreach ($requiredToken in @('HEAVY_TANK_TEST_ARMOR_HALF_LENGTH', 'HEAVY_TANK_TEST_ARMOR_HALF_WIDTH', 'enemy->heading', 'previousForward', 'previousRight', 'currentForward', 'currentRight')) {
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

$oamIds = @(0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56)
if (($oamIds | Sort-Object -Unique).Count -ne 15) {
    throw 'S3-02R1 OAM IDs overlap.'
}

function Invoke-FixtureFrame {
    param(
        [Parameter(Mandatory)][hashtable]$Fixture,
        [Parameter(Mandatory)][hashtable]$Shell
    )

    if (-not $Shell.Active) { return }
    $Shell.X += 4
    $inside = $Fixture.Active -and
        $Shell.X -ge (223 - 28) -and $Shell.X -le (223 + 28) -and
        $Shell.Y -ge (144 - 16) -and $Shell.Y -le (144 + 16)
    if ($inside) {
        $Shell.Active = $false
        $Fixture.Hp--
        $Fixture.Hits++
        if ($Fixture.Hp -eq 0) { $Fixture.Active = $false }
    }
}

$fixture = @{ Hp = 20; Active = $true; Hits = 0 }
$missShell = @{ X = 192; Y = 126; Active = $true }
Invoke-FixtureFrame -Fixture $fixture -Shell $missShell
if ($fixture.Hp -ne 20 -or $fixture.Hits -ne 0 -or -not $missShell.Active) {
    throw 'Deterministic miss fixture changed Enemy HP or removed the shell.'
}

$hitShell = @{ X = 192; Y = 144; Active = $true }
Invoke-FixtureFrame -Fixture $fixture -Shell $hitShell
if ($fixture.Hp -ne 19 -or $fixture.Hits -ne 1 -or $hitShell.Active) {
    throw 'Deterministic direct-hit fixture did not apply exactly one damage and remove its shell.'
}
Invoke-FixtureFrame -Fixture $fixture -Shell $hitShell
if ($fixture.Hp -ne 19 -or $fixture.Hits -ne 1) {
    throw 'An inactive hit shell applied repeated damage.'
}

foreach ($shot in 2..20) {
    $shell = @{ X = 192; Y = 144; Active = $true }
    Invoke-FixtureFrame -Fixture $fixture -Shell $shell
}
if ($fixture.Hp -ne 0 -or $fixture.Active -or $fixture.Hits -ne 20) {
    throw 'Twenty valid hits did not destroy the Heavy Test Enemy exactly once per shell.'
}

$fixture = @{ Hp = 20; Active = $true; Hits = 0 }
if ($fixture.Hp -ne 20 -or -not $fixture.Active -or $fixture.Hits -ne 0) {
    throw 'Enemy reset fixture did not restore HP, active state and hit count.'
}

Write-Output 'S3_TARGET_VERIFY=PASS'
Write-Output 'ENEMY_CLASS=HEAVY_TANK_TEST'
Write-Output 'ENEMY_POSITION=223,144'
Write-Output 'ENEMY_HEADING=128'
Write-Output 'ENEMY_HP=20'
Write-Output 'ENEMY_COLLISION=SEGMENT_ORIENTED_RECT_56x32'
Write-Output 'ENEMY_BROAD_PHASE_RADIUS=33'
Write-Output 'ENEMY_DAMAGE_PER_HIT=1'
Write-Output 'ENEMY_HIT_FLASH_FRAMES=6'
Write-Output 'ENEMY_RESET=P1_SELECT_DOWN'
Write-Output 'ENEMY_GFX_BYTES_ADDED=24576'
Write-Output 'ENEMY_PALETTE_BYTES_ADDED=32'
Write-Output 'ENEMY_ROM_GFX_BYTES=24576'
Write-Output 'ENEMY_RESIDENT_FRAME_BYTES=1536'
Write-Output 'ENEMY_VRAM_ROWS=0x1A00..0x1ABF,0x1B00..0x1BBF,0x1C00..0x1CBF,0x1D00..0x1DBF_WORDS'
Write-Output 'ENEMY_OAM_IDS=48,52,56'
Write-Output 'HEAVY_TANK_OBJ_COUNT=3'
Write-Output 'PLAYER_SIDE_OBJ_COUNT=12'
Write-Output 'TOTAL_ACTIVE_OBJ_MAX=15'
Write-Output 'WORST_CASE_PROJECTILE_OBJ=4'
Write-Output 'WORST_CASE_GAMEPLAY_SCANLINE_TILES=30_OF_34'
