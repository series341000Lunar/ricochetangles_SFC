#include <snes.h>
#include "hull_placeholder.inc"
#include "input_hud.inc"
#include "turret_placeholder.inc"
#include "heavy_tank_test.inc"

#define FIXED_SHIFT 8

#define MAX_FORWARD_SPEED 0x0180
#define MAX_REVERSE_SPEED 0x00C0
#define FORWARD_ACCELERATION 0x0008
#define REVERSE_ACCELERATION 0x0008
#define COAST_DECELERATION 0x0006
#define TURN_RATE 2
#define TURRET_TURN_RATE 4

#define MAX_PLAYER_SHELLS 4
#define FIRE_COOLDOWN_FRAMES 18
#define MUZZLE_DISTANCE 16
#define SHELL_SPEED_PIXELS 4
#define PROJECTILE_MIN_X 4
#define PROJECTILE_MAX_X 251
#define PROJECTILE_MIN_Y 56
#define PROJECTILE_MAX_Y 223

#define ENEMY_CLASS_HEAVY_TEST 1
#define ENEMY_INITIAL_X 223
#define ENEMY_INITIAL_Y 144
#define ENEMY_INITIAL_HEADING 128
#define HEAVY_TANK_TEST_MAX_HP 20
#define HEAVY_TANK_TEST_ARMOR_HALF_LENGTH 28
#define HEAVY_TANK_TEST_ARMOR_HALF_WIDTH 16
#define HEAVY_TANK_TEST_ARMOR_BOUNDING_RADIUS 33
#define HEAVY_TANK_TEST_MODULE_OFFSET 17
#define ENEMY_HIT_FLASH_FRAMES 6
#define ENEMY_RICOCHET_FLASH_FRAMES 4
#define RICOCHET_THRESHOLD_DEGREES 75
#define RICOCHET_THRESHOLD_HEADING_UNITS 54

#define AIM_GAIN 1
#define AIM_DEADZONE 5
#define INITIAL_AIM_DISTANCE 48
#define PAD_AIM_INDICATOR_DISTANCE 48
#define AIM_MIN_X 8
#define AIM_MAX_X 247
#define AIM_MIN_Y 64
#define AIM_MAX_Y 215

#define HULL_MIN_X 16
#define HULL_MAX_X 239
#define HULL_MIN_Y 80
#define HULL_MAX_Y 207

#define HULL_OAM_ID 0
#define MARKER_OAM_ID 4
#define HUD_UP_OAM_ID 8
#define HUD_DOWN_OAM_ID 12
#define HUD_LEFT_OAM_ID 16
#define HUD_RIGHT_OAM_ID 20
#define TURRET_OAM_ID 24
#define CURSOR_OAM_ID 28
#define PROJECTILE_OAM_BASE 32
#define ENEMY_FRONT_OAM_ID 48
#define ENEMY_CENTER_OAM_ID 52
#define ENEMY_REAR_OAM_ID 56
#define HULL_GFX_VRAM_ADDRESS 0x0000
#define HULL_GFX_BYTES 8192
#define INPUT_HUD_GFX_VRAM_ADDRESS 0x1000
#define INPUT_HUD_GFX_BYTES 3072
#define INPUT_HUD_TILE_BASE 256
#define INPUT_HUD_MARKER_TILE (INPUT_HUD_TILE_BASE + 64)
#define INPUT_HUD_CURSOR_TILE (INPUT_HUD_TILE_BASE + 66)
#define INPUT_HUD_SHELL_TILE (INPUT_HUD_TILE_BASE + 68)
#define TURRET_GFX_VRAM_ADDRESS 0x1600
#define TURRET_GFX_BYTES 2048
#define TURRET_TILE_BASE 352
#define HEAVY_TANK_TEST_FRONT_GFX_VRAM_ADDRESS 0x1A00
#define HEAVY_TANK_TEST_CENTER_GFX_VRAM_ADDRESS 0x1A40
#define HEAVY_TANK_TEST_REAR_GFX_VRAM_ADDRESS 0x1A80
#define HEAVY_TANK_TEST_GFX_BYTES 24576
#define HEAVY_TANK_TEST_FRAME_BYTES 1536
#define HEAVY_TANK_TEST_COMPONENT_BYTES 512
#define HEAVY_TANK_TEST_TILE_ROW_BYTES 128
#define HEAVY_TANK_TEST_FRONT_TILE_BASE 416
#define HEAVY_TANK_TEST_CENTER_TILE_BASE 420
#define HEAVY_TANK_TEST_REAR_TILE_BASE 424
#define MARKER_DISTANCE 20

typedef struct
{
    u16 positionX;
    u16 positionY;
    s16 speed;
    u8 heading;
    u8 targetHeading;
    s8 throttle;
    s8 turn;
} HullState;

typedef struct
{
    u8 heading;
    u8 targetHeading;
} TurretState;

typedef struct
{
    u16 x;
    u16 y;
} AimState;

typedef struct
{
    u16 positionX;
    u16 positionY;
    s16 velocityX;
    s16 velocityY;
    u8 heading;
    u8 active;
} ProjectileState;

typedef struct
{
    u16 positionX;
    u16 positionY;
    u8 heading;
    u8 hp;
    u8 maxHp;
    u8 active;
    u8 hitFlashFrames;
    u8 classId;
} EnemyState;

typedef enum
{
    ARMOR_FACE_NONE = 0,
    ARMOR_FACE_FRONT = 1,
    ARMOR_FACE_LEFT = 2,
    ARMOR_FACE_RIGHT = 3,
    ARMOR_FACE_REAR = 4
} ArmorFace;

typedef enum
{
    ARMOR_RESULT_NONE = 0,
    ARMOR_RESULT_RICOCHET = 1,
    ARMOR_RESULT_PENETRATION = 2
} ArmorResult;

typedef struct
{
    u16 pointX;
    u16 pointY;
    u8 face;
    u8 normalHeading;
    u8 impactAngle;
    u8 result;
} ArmorImpact;

typedef enum
{
    AIM_MODE_MOUSE = 0,
    AIM_MODE_PAD2 = 1
} AimMode;

typedef enum
{
    DRIVE_MODE_PC_LIKE = 0,
    DRIVE_MODE_STICK = 1
} DriveMode;

#define MENU_ROW_DRIVE 0
#define MENU_ROW_AIM 1

static const char hexDigits[] = "0123456789ABCDEF";
static const char armorFaceLetters[] = "-FLRB";
static HullState hull;
static TurretState turret;
static AimState aim;
static AimMode aimMode;
static DriveMode driveMode;
static u8 menuOpen;
static u8 menuSelection;
static ProjectileState projectiles[MAX_PLAYER_SHELLS];
static EnemyState enemy;
static ArmorImpact lastArmorImpact;
static u8 enemyHitCount;
static u8 fireCooldown;
static u8 previousFireHeld;
static u8 fireInputArmed;
static char statusValue[3];

static const s16 quarterSin[65] = {
    0, 6, 13, 19, 25, 31, 38, 44,
    50, 56, 62, 68, 74, 80, 86, 92,
    98, 104, 109, 115, 121, 126, 132, 137,
    142, 147, 152, 157, 162, 167, 172, 177,
    181, 185, 190, 194, 198, 202, 206, 209,
    213, 216, 220, 223, 226, 229, 231, 234,
    237, 239, 241, 243, 245, 247, 248, 250,
    251, 252, 253, 254, 255, 255, 256, 256,
    256
};

static const u8 atanThreshold[32] = {
    3, 9, 16, 22, 28, 35, 41, 48,
    54, 61, 67, 74, 81, 88, 95, 102,
    110, 117, 125, 133, 141, 149, 158, 167,
    176, 185, 195, 205, 215, 226, 238, 250
};

static void formatHex8(u8 value, char *output)
{
    output[0] = hexDigits[(value >> 4) & 0x0F];
    output[1] = hexDigits[value & 0x0F];
}

static void formatImpactDegrees(u8 headingUnits, char *output)
{
    u8 degrees = (u8)(((u16)headingUnits * 90) >> 6);
    u8 tens = 0;

    while (degrees >= 10)
    {
        degrees -= 10;
        tens++;
    }
    output[0] = (char)('0' + tens);
    output[1] = (char)('0' + degrees);
}

static void formatDecimal2(u8 value, char *output)
{
    u8 tens = 0;

    while (value >= 10)
    {
        value -= 10;
        tens++;
    }
    output[0] = (char)('0' + tens);
    output[1] = (char)('0' + value);
}

static s16 sin256(u8 angle)
{
    u8 quadrant = angle >> 6;
    u8 index = angle & 0x3F;

    if (quadrant == 0)
    {
        return quarterSin[index];
    }
    if (quadrant == 1)
    {
        return quarterSin[64 - index];
    }
    if (quadrant == 2)
    {
        return -quarterSin[index];
    }
    return -quarterSin[64 - index];
}

static s16 cos256(u8 angle)
{
    return sin256((u8)(angle + 64));
}

static u8 headingVisualFrame(u8 heading)
{
    return (u8)((((u16)heading + 8) >> 4) & 0x0F);
}

static u16 hullVisualTile(u8 visualFrame)
{
    return (u16)(((u16)(visualFrame >> 2) << 6) + ((u16)(visualFrame & 0x03) << 2));
}

static void loadHeavyTankTestVisualFrame(u8 visualFrame)
{
    u16 sourceOffset = ((u16)visualFrame << 10) + ((u16)visualFrame << 9);
    u16 frontVramAddress = HEAVY_TANK_TEST_FRONT_GFX_VRAM_ADDRESS;
    u16 centerVramAddress = HEAVY_TANK_TEST_CENTER_GFX_VRAM_ADDRESS;
    u16 rearVramAddress = HEAVY_TANK_TEST_REAR_GFX_VRAM_ADDRESS;
    u8 tileRow;

    for (tileRow = 0; tileRow < 4; tileRow++)
    {
        dmaCopyVram(&heavyTankTestTiles + sourceOffset, frontVramAddress, HEAVY_TANK_TEST_TILE_ROW_BYTES);
        dmaCopyVram(&heavyTankTestTiles + sourceOffset + HEAVY_TANK_TEST_COMPONENT_BYTES, centerVramAddress, HEAVY_TANK_TEST_TILE_ROW_BYTES);
        dmaCopyVram(&heavyTankTestTiles + sourceOffset + (HEAVY_TANK_TEST_COMPONENT_BYTES * 2), rearVramAddress, HEAVY_TANK_TEST_TILE_ROW_BYTES);
        sourceOffset += HEAVY_TANK_TEST_TILE_ROW_BYTES;
        frontVramAddress += 0x0100;
        centerVramAddress += 0x0100;
        rearVramAddress += 0x0100;
    }
}

static u16 inputButtonTile(u8 buttonIndex, u8 pressed)
{
    return (u16)(INPUT_HUD_TILE_BASE + ((u16)buttonIndex << 2) + (pressed != 0 ? 2 : 0));
}

static u16 turretVisualTile(u8 visualFrame)
{
    return (u16)(TURRET_TILE_BASE + ((u16)(visualFrame >> 3) << 5) + ((u16)(visualFrame & 0x07) << 1));
}

static s16 decodeMouseDelta(u8 rawDelta)
{
    s16 magnitude = (s16)(rawDelta & 0x7F);
    return (rawDelta & 0x80) != 0 ? -magnitude : magnitude;
}

static u16 addClampedCoordinate(u16 value, s16 delta, u16 minimum, u16 maximum)
{
    u16 magnitude;

    if (delta < 0)
    {
        magnitude = (u16)(-delta);
        return value <= minimum + magnitude ? minimum : (u16)(value - magnitude);
    }

    magnitude = (u16)delta;
    return value >= maximum - magnitude ? maximum : (u16)(value + magnitude);
}

static void updateAimCursor(u8 mouseConnected, u8 mouseRawX, u8 mouseRawY)
{
    if (mouseConnected == 0)
    {
        return;
    }

    aim.x = addClampedCoordinate(
        aim.x,
        (s16)(decodeMouseDelta(mouseRawX) * AIM_GAIN),
        AIM_MIN_X,
        AIM_MAX_X);
    aim.y = addClampedCoordinate(
        aim.y,
        (s16)(decodeMouseDelta(mouseRawY) * AIM_GAIN),
        AIM_MIN_Y,
        AIM_MAX_Y);
}

static u16 absoluteS16(s16 value)
{
    return value < 0 ? (u16)(-value) : (u16)value;
}

static u8 resolveOctantAngle(u16 minor, u16 major)
{
    u8 low = 0;
    u8 high = 32;
    u8 middle;
    u16 scaledMinor = (u16)(minor << 8);

    while (low < high)
    {
        middle = (u8)((low + high) >> 1);
        if (scaledMinor >= (u16)(major * atanThreshold[middle]))
        {
            low = (u8)(middle + 1);
        }
        else
        {
            high = middle;
        }
    }
    return low;
}

static u8 vectorToHeading(s16 dx, s16 dy)
{
    u16 absoluteX = absoluteS16(dx);
    u16 absoluteY = absoluteS16(dy);
    u8 baseAngle;

    if (absoluteX >= absoluteY)
    {
        baseAngle = absoluteX == 0 ? 0 : resolveOctantAngle(absoluteY, absoluteX);
    }
    else
    {
        baseAngle = (u8)(64 - resolveOctantAngle(absoluteX, absoluteY));
    }

    if (dx >= 0)
    {
        return dy >= 0 ? baseAngle : (u8)(0 - baseAngle);
    }
    return dy >= 0 ? (u8)(128 - baseAngle) : (u8)(128 + baseAngle);
}

static void updateTurretTargetFromMouse(const HullState *hull, TurretState *turret)
{
    s16 centerX = (s16)(hull->positionX >> FIXED_SHIFT);
    s16 centerY = (s16)(hull->positionY >> FIXED_SHIFT);
    s16 dx = (s16)aim.x - centerX;
    s16 dy = (s16)aim.y - centerY;

    if (absoluteS16(dx) + absoluteS16(dy) < AIM_DEADZONE)
    {
        return;
    }
    turret->targetHeading = vectorToHeading(dx, dy);
}

static u8 resolvePadHeading(u16 pad, u8 *heading)
{
    u8 upHeld = (pad & KEY_UP) != 0;
    u8 downHeld = (pad & KEY_DOWN) != 0;
    u8 leftHeld = (pad & KEY_LEFT) != 0;
    u8 rightHeld = (pad & KEY_RIGHT) != 0;
    s8 horizontal = 0;
    s8 vertical = 0;

    if (leftHeld != rightHeld)
    {
        horizontal = leftHeld != 0 ? -1 : 1;
    }
    if (upHeld != downHeld)
    {
        vertical = upHeld != 0 ? -1 : 1;
    }

    if (horizontal > 0)
    {
        *heading = vertical < 0 ? 0xE0 : (vertical > 0 ? 0x20 : 0x00);
    }
    else if (horizontal < 0)
    {
        *heading = vertical < 0 ? 0xA0 : (vertical > 0 ? 0x60 : 0x80);
    }
    else if (vertical < 0)
    {
        *heading = 0xC0;
    }
    else if (vertical > 0)
    {
        *heading = 0x40;
    }
    else
    {
        return 0;
    }

    return 1;
}

static void updateTurretTargetFromPad2(u16 pad2, TurretState *turret)
{
    resolvePadHeading(pad2, &turret->targetHeading);
}

static void updateTurretHeading(TurretState *turret)
{
    s8 delta = (s8)(turret->targetHeading - turret->heading);

    if (delta > TURRET_TURN_RATE)
    {
        turret->heading = (u8)(turret->heading + TURRET_TURN_RATE);
    }
    else if (delta < -TURRET_TURN_RATE)
    {
        turret->heading = (u8)(turret->heading - TURRET_TURN_RATE);
    }
    else
    {
        turret->heading = turret->targetHeading;
    }
}

static u8 addProjectileDelta(u16 *position, s16 delta)
{
    u16 magnitude;

    if (delta < 0)
    {
        magnitude = (u16)(-delta);
        if (*position < magnitude)
        {
            return 0;
        }
        *position -= magnitude;
        return 1;
    }

    magnitude = (u16)delta;
    if (*position > (u16)(0xFFFF - magnitude))
    {
        return 0;
    }
    *position += magnitude;
    return 1;
}

static void initializeProjectiles(void)
{
    u8 index;

    for (index = 0; index < MAX_PLAYER_SHELLS; index++)
    {
        projectiles[index].positionX = 0;
        projectiles[index].positionY = 0;
        projectiles[index].velocityX = 0;
        projectiles[index].velocityY = 0;
        projectiles[index].heading = 0;
        projectiles[index].active = 0;
    }
}

static void initializeEnemy(void)
{
    enemy.positionX = (u16)ENEMY_INITIAL_X << FIXED_SHIFT;
    enemy.positionY = (u16)ENEMY_INITIAL_Y << FIXED_SHIFT;
    enemy.heading = ENEMY_INITIAL_HEADING;
    enemy.hp = HEAVY_TANK_TEST_MAX_HP;
    enemy.maxHp = HEAVY_TANK_TEST_MAX_HP;
    enemy.active = 1;
    enemy.hitFlashFrames = 0;
    enemy.classId = ENEMY_CLASS_HEAVY_TEST;
    lastArmorImpact.pointX = 0;
    lastArmorImpact.pointY = 0;
    lastArmorImpact.face = ARMOR_FACE_NONE;
    lastArmorImpact.normalHeading = 0;
    lastArmorImpact.impactAngle = 0;
    lastArmorImpact.result = ARMOR_RESULT_NONE;
    enemyHitCount = 0;
}

static u8 shortestHeadingDistance(u8 first, u8 second)
{
    u8 difference = (u8)(first - second);

    return difference > 128 ? (u8)(0 - difference) : difference;
}

static u8 entryFractionIsEarlier(
    u16 candidateNumerator,
    u16 candidateDenominator,
    u16 entryNumerator,
    u16 entryDenominator)
{
    return candidateNumerator * entryDenominator <
        entryNumerator * candidateDenominator;
}

static u8 resolveEnemyArmorImpact(
    u16 previousX,
    u16 previousY,
    u16 currentX,
    u16 currentY,
    u8 projectileHeading,
    const EnemyState *enemy,
    ArmorImpact *impact)
{
    s16 previousPixelX = (s16)(previousX >> FIXED_SHIFT);
    s16 previousPixelY = (s16)(previousY >> FIXED_SHIFT);
    s16 currentPixelX = (s16)(currentX >> FIXED_SHIFT);
    s16 currentPixelY = (s16)(currentY >> FIXED_SHIFT);
    s16 enemyPixelX = (s16)(enemy->positionX >> FIXED_SHIFT);
    s16 enemyPixelY = (s16)(enemy->positionY >> FIXED_SHIFT);
    s16 previousDeltaX;
    s16 previousDeltaY;
    s16 currentDeltaX;
    s16 currentDeltaY;
    s16 forwardX;
    s16 forwardY;
    s16 previousForward;
    s16 previousRight;
    s16 currentForward;
    s16 currentRight;
    s16 halfLength = HEAVY_TANK_TEST_ARMOR_HALF_LENGTH;
    s16 halfWidth = HEAVY_TANK_TEST_ARMOR_HALF_WIDTH;
    u16 entryNumerator = 0;
    u16 entryDenominator = 1;
    u16 candidateNumerator;
    u16 candidateDenominator;
    s16 segmentX;
    s16 segmentY;
    u8 face = ARMOR_FACE_NONE;
    u8 candidateFace = ARMOR_FACE_NONE;
    u8 normalHeading;
    u8 attackHeading;
    u8 impactAngle;

    if (enemy->active == 0)
    {
        return 0;
    }

    if (currentPixelX < enemyPixelX - HEAVY_TANK_TEST_ARMOR_BOUNDING_RADIUS ||
        currentPixelX > enemyPixelX + HEAVY_TANK_TEST_ARMOR_BOUNDING_RADIUS ||
        currentPixelY < enemyPixelY - HEAVY_TANK_TEST_ARMOR_BOUNDING_RADIUS ||
        currentPixelY > enemyPixelY + HEAVY_TANK_TEST_ARMOR_BOUNDING_RADIUS)
    {
        return 0;
    }

    previousDeltaX = previousPixelX - enemyPixelX;
    previousDeltaY = previousPixelY - enemyPixelY;
    currentDeltaX = currentPixelX - enemyPixelX;
    currentDeltaY = currentPixelY - enemyPixelY;
    forwardX = cos256(enemy->heading);
    forwardY = sin256(enemy->heading);
    previousForward = (previousDeltaX * forwardX + previousDeltaY * forwardY) >> FIXED_SHIFT;
    previousRight = (-previousDeltaX * forwardY + previousDeltaY * forwardX) >> FIXED_SHIFT;
    currentForward = (currentDeltaX * forwardX + currentDeltaY * forwardY) >> FIXED_SHIFT;
    currentRight = (-currentDeltaX * forwardY + currentDeltaY * forwardX) >> FIXED_SHIFT;

    if (currentForward < -halfLength || currentForward > halfLength ||
        currentRight < -halfWidth || currentRight > halfWidth)
    {
        return 0;
    }

    if (previousForward > halfLength)
    {
        candidateFace = ARMOR_FACE_FRONT;
        candidateNumerator = (u16)(previousForward - halfLength);
        candidateDenominator = (u16)(previousForward - currentForward);
    }
    else if (previousForward < -halfLength)
    {
        candidateFace = ARMOR_FACE_REAR;
        candidateNumerator = (u16)(-halfLength - previousForward);
        candidateDenominator = (u16)(currentForward - previousForward);
    }

    if (candidateFace != ARMOR_FACE_NONE)
    {
        face = candidateFace;
        entryNumerator = candidateNumerator;
        entryDenominator = candidateDenominator;
    }

    candidateFace = ARMOR_FACE_NONE;
    if (previousRight > halfWidth)
    {
        candidateFace = ARMOR_FACE_RIGHT;
        candidateNumerator = (u16)(previousRight - halfWidth);
        candidateDenominator = (u16)(previousRight - currentRight);
    }
    else if (previousRight < -halfWidth)
    {
        candidateFace = ARMOR_FACE_LEFT;
        candidateNumerator = (u16)(-halfWidth - previousRight);
        candidateDenominator = (u16)(currentRight - previousRight);
    }

    if (candidateFace != ARMOR_FACE_NONE &&
        (face == ARMOR_FACE_NONE || entryFractionIsEarlier(
            candidateNumerator,
            candidateDenominator,
            entryNumerator,
            entryDenominator) != 0))
    {
        face = candidateFace;
        entryNumerator = candidateNumerator;
        entryDenominator = candidateDenominator;
    }

    if (face == ARMOR_FACE_NONE || entryDenominator <= 0)
    {
        return 0;
    }

    if (face == ARMOR_FACE_FRONT)
    {
        normalHeading = enemy->heading;
    }
    else if (face == ARMOR_FACE_REAR)
    {
        normalHeading = (u8)(enemy->heading + 128);
    }
    else if (face == ARMOR_FACE_RIGHT)
    {
        normalHeading = (u8)(enemy->heading + 64);
    }
    else
    {
        normalHeading = (u8)(enemy->heading - 64);
    }

    attackHeading = (u8)(projectileHeading + 128);
    impactAngle = shortestHeadingDistance(attackHeading, normalHeading);
    if (impactAngle > 64)
    {
        impactAngle = 64;
    }

    segmentX = currentPixelX - previousPixelX;
    segmentY = currentPixelY - previousPixelY;
    impact->pointX = (u16)(previousPixelX + segmentX * entryNumerator / entryDenominator);
    impact->pointY = (u16)(previousPixelY + segmentY * entryNumerator / entryDenominator);
    impact->face = face;
    impact->normalHeading = normalHeading;
    impact->impactAngle = impactAngle;
    impact->result = impactAngle >= RICOCHET_THRESHOLD_HEADING_UNITS ?
        ARMOR_RESULT_RICOCHET : ARMOR_RESULT_PENETRATION;
    return 1;
}

static void damageEnemy(EnemyState *enemy)
{
    enemy->hp--;
    enemyHitCount++;
    enemy->hitFlashFrames = ENEMY_HIT_FLASH_FRAMES;
    if (enemy->hp == 0)
    {
        enemy->active = 0;
    }
}

static void updateProjectiles(EnemyState *enemy)
{
    u8 index;
    u16 pixelX;
    u16 pixelY;
    u16 previousX;
    u16 previousY;
    ArmorImpact impact;

    for (index = 0; index < MAX_PLAYER_SHELLS; index++)
    {
        if (projectiles[index].active != 1)
        {
            continue;
        }

        previousX = projectiles[index].positionX;
        previousY = projectiles[index].positionY;
        if (addProjectileDelta(&projectiles[index].positionX, projectiles[index].velocityX) == 0 ||
            addProjectileDelta(&projectiles[index].positionY, projectiles[index].velocityY) == 0)
        {
            projectiles[index].active = 0;
            continue;
        }

        pixelX = projectiles[index].positionX >> FIXED_SHIFT;
        pixelY = projectiles[index].positionY >> FIXED_SHIFT;
        if (resolveEnemyArmorImpact(
                previousX,
                previousY,
                projectiles[index].positionX,
                projectiles[index].positionY,
                projectiles[index].heading,
                enemy,
                &impact) != 0)
        {
            projectiles[index].active = 0;
            lastArmorImpact = impact;
            if (impact.result == ARMOR_RESULT_RICOCHET)
            {
                enemy->hitFlashFrames = ENEMY_RICOCHET_FLASH_FRAMES;
            }
            else
            {
                damageEnemy(enemy);
            }
            continue;
        }

        if (pixelX < PROJECTILE_MIN_X || pixelX > PROJECTILE_MAX_X ||
            pixelY < PROJECTILE_MIN_Y || pixelY > PROJECTILE_MAX_Y)
        {
            projectiles[index].active = 0;
        }
    }
}

static void updateEnemyHitFlash(EnemyState *enemy)
{
    if (enemy->hitFlashFrames > 0)
    {
        enemy->hitFlashFrames--;
    }
}

static u8 spawnProjectile(const HullState *hull, const TurretState *turret)
{
    u8 index;
    u8 shotHeading = turret->heading;
    s16 directionX = cos256(shotHeading);
    s16 directionY = sin256(shotHeading);
    s16 muzzleX = (s16)(hull->positionX >> FIXED_SHIFT) +
        (s16)((directionX * MUZZLE_DISTANCE) >> FIXED_SHIFT);
    s16 muzzleY = (s16)(hull->positionY >> FIXED_SHIFT) +
        (s16)((directionY * MUZZLE_DISTANCE) >> FIXED_SHIFT);

    if (muzzleX < 0 || muzzleX > 255 ||
        muzzleY < PROJECTILE_MIN_Y || muzzleY > PROJECTILE_MAX_Y)
    {
        return 0;
    }

    for (index = 0; index < MAX_PLAYER_SHELLS; index++)
    {
        if (projectiles[index].active != 1)
        {
            projectiles[index].positionX = (u16)muzzleX << FIXED_SHIFT;
            projectiles[index].positionY = (u16)muzzleY << FIXED_SHIFT;
            projectiles[index].velocityX = (s16)(directionX * SHELL_SPEED_PIXELS);
            projectiles[index].velocityY = (s16)(directionY * SHELL_SPEED_PIXELS);
            projectiles[index].heading = shotHeading;
            projectiles[index].active = 1;
            return 1;
        }
    }

    return 0;
}

static void resetMainGunInput(void)
{
    previousFireHeld = 0;
    fireInputArmed = 0;
}

static u8 updateMainGun(u8 fireHeld, const HullState *hull, const TurretState *turret)
{
    u8 fireWasHeld = previousFireHeld;
    u8 fired = 0;

    if (fireCooldown > 0)
    {
        fireCooldown--;
    }

    if (fireHeld == 0)
    {
        fireInputArmed = 1;
    }

    if (fireHeld != 0 && fireWasHeld == 0 && fireInputArmed != 0 && fireCooldown == 0)
    {
        fired = spawnProjectile(hull, turret);
        if (fired != 0)
        {
            fireCooldown = FIRE_COOLDOWN_FRAMES;
            fireInputArmed = 0;
        }
    }

    previousFireHeld = fireHeld;
    return fired;
}

static void resolveHullInputPcLike(u16 pad, HullState *hull)
{
    u8 forwardHeld = (pad & KEY_UP) != 0;
    u8 reverseHeld = (pad & KEY_DOWN) != 0;
    u8 leftHeld = (pad & KEY_LEFT) != 0;
    u8 rightHeld = (pad & KEY_RIGHT) != 0;

    if (forwardHeld == reverseHeld)
    {
        hull->throttle = 0;
    }
    else if (forwardHeld != 0)
    {
        hull->throttle = 1;
    }
    else
    {
        hull->throttle = -1;
    }

    if (leftHeld == rightHeld)
    {
        hull->turn = 0;
    }
    else if (leftHeld != 0)
    {
        hull->turn = -1;
    }
    else
    {
        hull->turn = 1;
    }
}

static void resolveHullInputStick(u16 pad, HullState *hull)
{
    s8 delta;

    if (resolvePadHeading(pad, &hull->targetHeading) == 0)
    {
        hull->throttle = 0;
        hull->turn = 0;
        return;
    }

    hull->throttle = 1;
    delta = (s8)(hull->targetHeading - hull->heading);
    hull->turn = delta < 0 ? -1 : (delta > 0 ? 1 : 0);
}

static void updateHullHeading(HullState *hull)
{
    s8 delta;

    if (driveMode == DRIVE_MODE_STICK)
    {
        if (hull->throttle == 0)
        {
            return;
        }

        delta = (s8)(hull->targetHeading - hull->heading);
        if (delta > TURN_RATE)
        {
            hull->heading = (u8)(hull->heading + TURN_RATE);
        }
        else if (delta < -TURN_RATE)
        {
            hull->heading = (u8)(hull->heading - TURN_RATE);
        }
        else
        {
            hull->heading = hull->targetHeading;
        }
        return;
    }

    if (hull->turn < 0)
    {
        hull->heading = (u8)(hull->heading - TURN_RATE);
    }
    else if (hull->turn > 0)
    {
        hull->heading = (u8)(hull->heading + TURN_RATE);
    }
}

static void updateHullSpeed(HullState *hull)
{
    if (hull->throttle > 0)
    {
        if (hull->speed < 0)
        {
            if (hull->speed >= -FORWARD_ACCELERATION)
            {
                hull->speed = 0;
            }
            else
            {
                hull->speed += FORWARD_ACCELERATION;
            }
        }
        else if (hull->speed >= MAX_FORWARD_SPEED - FORWARD_ACCELERATION)
        {
            hull->speed = MAX_FORWARD_SPEED;
        }
        else
        {
            hull->speed += FORWARD_ACCELERATION;
        }
    }
    else if (hull->throttle < 0)
    {
        if (hull->speed > 0)
        {
            if (hull->speed <= REVERSE_ACCELERATION)
            {
                hull->speed = 0;
            }
            else
            {
                hull->speed -= REVERSE_ACCELERATION;
            }
        }
        else if (hull->speed <= -MAX_REVERSE_SPEED + REVERSE_ACCELERATION)
        {
            hull->speed = -MAX_REVERSE_SPEED;
        }
        else
        {
            hull->speed -= REVERSE_ACCELERATION;
        }
    }
    else if (hull->speed > 0)
    {
        if (hull->speed <= COAST_DECELERATION)
        {
            hull->speed = 0;
        }
        else
        {
            hull->speed -= COAST_DECELERATION;
        }
    }
    else if (hull->speed < 0)
    {
        if (hull->speed >= -COAST_DECELERATION)
        {
            hull->speed = 0;
        }
        else
        {
            hull->speed += COAST_DECELERATION;
        }
    }
}

static void updateHullPosition(HullState *hull)
{
    s16 deltaX = (s16)(((s32)cos256(hull->heading) * hull->speed) >> 8);
    s16 deltaY = (s16)(((s32)sin256(hull->heading) * hull->speed) >> 8);
    u16 minimumX = (u16)HULL_MIN_X << FIXED_SHIFT;
    u16 maximumX = (u16)HULL_MAX_X << FIXED_SHIFT;
    u16 minimumY = (u16)HULL_MIN_Y << FIXED_SHIFT;
    u16 maximumY = (u16)HULL_MAX_Y << FIXED_SHIFT;
    u16 magnitude;

    if (deltaX < 0)
    {
        magnitude = (u16)(-deltaX);
        if (hull->positionX <= minimumX + magnitude)
        {
            hull->positionX = minimumX;
        }
        else
        {
            hull->positionX -= magnitude;
        }
    }
    else
    {
        magnitude = (u16)deltaX;
        if (hull->positionX >= maximumX - magnitude)
        {
            hull->positionX = maximumX;
        }
        else
        {
            hull->positionX += magnitude;
        }
    }

    if (deltaY < 0)
    {
        magnitude = (u16)(-deltaY);
        if (hull->positionY <= minimumY + magnitude)
        {
            hull->positionY = minimumY;
        }
        else
        {
            hull->positionY -= magnitude;
        }
    }
    else
    {
        magnitude = (u16)deltaY;
        if (hull->positionY >= maximumY - magnitude)
        {
            hull->positionY = maximumY;
        }
        else
        {
            hull->positionY += magnitude;
        }
    }
}

static void updateHullSprites(const HullState *hull)
{
    u8 visualFrame = headingVisualFrame(hull->heading);
    s16 centerX = (s16)(hull->positionX >> FIXED_SHIFT);
    s16 centerY = (s16)(hull->positionY >> FIXED_SHIFT);
    s16 markerX = centerX + ((cos256(hull->heading) * MARKER_DISTANCE) >> 8) - 8;
    s16 markerY = centerY + ((sin256(hull->heading) * MARKER_DISTANCE) >> 8) - 8;

    oamSetGfxOffset(HULL_OAM_ID, hullVisualTile(visualFrame));
    oamSetXY(HULL_OAM_ID, centerX - 16, centerY - 16);
    oamSetXY(MARKER_OAM_ID, markerX, markerY);
}

static void updateTurretAndCursorSprites(const HullState *hull, const TurretState *turret)
{
    s16 centerX = (s16)(hull->positionX >> FIXED_SHIFT);
    s16 centerY = (s16)(hull->positionY >> FIXED_SHIFT);
    s16 cursorX;
    s16 cursorY;

    if (aimMode == AIM_MODE_MOUSE)
    {
        cursorX = (s16)aim.x;
        cursorY = (s16)aim.y;
    }
    else
    {
        cursorX = centerX + ((cos256(turret->targetHeading) * PAD_AIM_INDICATOR_DISTANCE) >> FIXED_SHIFT);
        cursorY = centerY + ((sin256(turret->targetHeading) * PAD_AIM_INDICATOR_DISTANCE) >> FIXED_SHIFT);
    }

    oamSetGfxOffset(TURRET_OAM_ID, turretVisualTile(headingVisualFrame(turret->heading)));
    oamSetXY(TURRET_OAM_ID, centerX - 8, centerY - 8);
    oamSetXY(CURSOR_OAM_ID, cursorX - 8, cursorY - 8);
}

static void updateProjectileSprites(void)
{
    u8 index;
    u16 oamId;

    for (index = 0; index < MAX_PLAYER_SHELLS; index++)
    {
        oamId = (u16)(PROJECTILE_OAM_BASE + ((u16)index << 2));
        if (projectiles[index].active == 1)
        {
            oamSetXY(
                oamId,
                (s16)(projectiles[index].positionX >> FIXED_SHIFT) - 8,
                (s16)(projectiles[index].positionY >> FIXED_SHIFT) - 8);
            oamSetVisible(oamId, OBJ_SHOW);
        }
        else
        {
            oamSetVisible(oamId, OBJ_HIDE);
        }
    }
}

static void updateEnemySprite(const EnemyState *enemy)
{
    u8 visualFrame;
    s16 centerX;
    s16 centerY;
    s16 moduleOffsetX;
    s16 moduleOffsetY;

    if (enemy->active == 0 ||
        (enemy->hitFlashFrames != 0 && (enemy->hitFlashFrames & 1) == 0))
    {
        oamSetVisible(ENEMY_FRONT_OAM_ID, OBJ_HIDE);
        oamSetVisible(ENEMY_CENTER_OAM_ID, OBJ_HIDE);
        oamSetVisible(ENEMY_REAR_OAM_ID, OBJ_HIDE);
        return;
    }

    visualFrame = headingVisualFrame(enemy->heading);
    centerX = (s16)(enemy->positionX >> FIXED_SHIFT);
    centerY = (s16)(enemy->positionY >> FIXED_SHIFT);
    moduleOffsetX = (cos256(enemy->heading) * HEAVY_TANK_TEST_MODULE_OFFSET) >> FIXED_SHIFT;
    moduleOffsetY = (sin256(enemy->heading) * HEAVY_TANK_TEST_MODULE_OFFSET) >> FIXED_SHIFT;

    oamSetGfxOffset(ENEMY_FRONT_OAM_ID, HEAVY_TANK_TEST_FRONT_TILE_BASE);
    oamSetGfxOffset(ENEMY_CENTER_OAM_ID, HEAVY_TANK_TEST_CENTER_TILE_BASE);
    oamSetGfxOffset(ENEMY_REAR_OAM_ID, HEAVY_TANK_TEST_REAR_TILE_BASE);
    oamSetXY(
        ENEMY_FRONT_OAM_ID,
        centerX + moduleOffsetX - 16,
        centerY + moduleOffsetY - 16);
    oamSetXY(
        ENEMY_CENTER_OAM_ID,
        centerX - 16,
        centerY - 16);
    oamSetXY(
        ENEMY_REAR_OAM_ID,
        centerX - moduleOffsetX - 16,
        centerY - moduleOffsetY - 16);
    oamSetVisible(ENEMY_FRONT_OAM_ID, OBJ_SHOW);
    oamSetVisible(ENEMY_CENTER_OAM_ID, OBJ_SHOW);
    oamSetVisible(ENEMY_REAR_OAM_ID, OBJ_SHOW);
}

static void updateInputHudSprites(u16 pad)
{
    oamSetGfxOffset(HUD_UP_OAM_ID, inputButtonTile(0, (pad & KEY_UP) != 0));
    oamSetGfxOffset(HUD_DOWN_OAM_ID, inputButtonTile(1, (pad & KEY_DOWN) != 0));
    oamSetGfxOffset(HUD_LEFT_OAM_ID, inputButtonTile(2, (pad & KEY_LEFT) != 0));
    oamSetGfxOffset(HUD_RIGHT_OAM_ID, inputButtonTile(3, (pad & KEY_RIGHT) != 0));
}

static u8 countActiveProjectiles(void)
{
    u8 index;
    u8 count = 0;

    for (index = 0; index < MAX_PLAYER_SHELLS; index++)
    {
        if (projectiles[index].active == 1)
        {
            count++;
        }
    }
    return count;
}

static void drawFooter(void)
{
    consoleDrawText(9, 27, "CHANNEL A BNC");
}

static void drawGameplayText(void)
{
    consoleDrawText(4, 7, "                            ");
    consoleDrawText(4, 9, "                            ");
    consoleDrawText(8, 12, "                        ");
    consoleDrawText(13, 0, "DRV:P AIM:M EN:20");
    consoleDrawText(13, 1, "H:00 T:00 G:0 S:0");
    consoleDrawText(0, 2, "             AR:- A:-- ---     ");
    consoleDrawText(0, 3, "S3-02R");
    drawFooter();
}

static void drawControlMenu(void)
{
    consoleDrawText(0, 2, "                               ");
    consoleDrawText(0, 3, "                               ");
    consoleDrawText(9, 2, "RicochetAngles");
    consoleDrawText(4, 7, menuSelection == MENU_ROW_DRIVE ? "> DRIVE" : "  DRIVE");
    consoleDrawText(17, 7, driveMode == DRIVE_MODE_PC_LIKE ? "PC-LIKE" : "STICK  ");
    consoleDrawText(4, 9, menuSelection == MENU_ROW_AIM ? "> AIM" : "  AIM");
    consoleDrawText(17, 9, aimMode == AIM_MODE_MOUSE ? "MOUSE " : "P2 PAD");
    consoleDrawText(8, 12, "START : RESUME");
    drawFooter();
}

static void updateControlMenu(u16 padDown)
{
    u8 upDown = (padDown & KEY_UP) != 0;
    u8 downDown = (padDown & KEY_DOWN) != 0;

    if (upDown != downDown)
    {
        menuSelection = menuSelection == MENU_ROW_DRIVE ? MENU_ROW_AIM : MENU_ROW_DRIVE;
    }

    if ((padDown & (KEY_LEFT | KEY_RIGHT | KEY_A)) != 0)
    {
        if (menuSelection == MENU_ROW_DRIVE)
        {
            driveMode = driveMode == DRIVE_MODE_PC_LIKE ? DRIVE_MODE_STICK : DRIVE_MODE_PC_LIKE;
            hull.targetHeading = hull.heading;
        }
        else
        {
            aimMode = aimMode == AIM_MODE_MOUSE ? AIM_MODE_PAD2 : AIM_MODE_MOUSE;
            resetMainGunInput();
        }
    }

    drawControlMenu();
}

static void initializePlayerSprites(void)
{
    u8 index;
    u16 oamId;

    oamInit();
    oamInitGfxAttr(HULL_GFX_VRAM_ADDRESS, OBJ_SIZE16_L32);

    dmaCopyVram(&hullPlaceholderTiles, HULL_GFX_VRAM_ADDRESS, HULL_GFX_BYTES);
    dmaCopyVram(&inputHudTiles, INPUT_HUD_GFX_VRAM_ADDRESS, INPUT_HUD_GFX_BYTES);
    dmaCopyVram(&turretPlaceholderTiles, TURRET_GFX_VRAM_ADDRESS, TURRET_GFX_BYTES);
    loadHeavyTankTestVisualFrame(headingVisualFrame(ENEMY_INITIAL_HEADING));
    dmaCopyCGram(&hullPlaceholderPalette, 128, (&hullPlaceholderPaletteEnd - &hullPlaceholderPalette));
    dmaCopyCGram(&inputHudPalette, 144, (&inputHudPaletteEnd - &inputHudPalette));
    dmaCopyCGram(&turretPlaceholderPalette, 160, (&turretPlaceholderPaletteEnd - &turretPlaceholderPalette));
    dmaCopyCGram(&heavyTankTestPalette, 176, (&heavyTankTestPaletteEnd - &heavyTankTestPalette));

    oamSet(HULL_OAM_ID, 112, 128, 2, 0, 0, hullVisualTile(0), 0);
    oamSetEx(HULL_OAM_ID, OBJ_LARGE, OBJ_SHOW);
    oamSet(MARKER_OAM_ID, 140, 136, 3, 0, 0, INPUT_HUD_MARKER_TILE, 1);
    oamSetEx(MARKER_OAM_ID, OBJ_SMALL, OBJ_SHOW);

    oamSet(HUD_UP_OAM_ID, 8, 0, 3, 0, 0, inputButtonTile(0, 0), 1);
    oamSetEx(HUD_UP_OAM_ID, OBJ_SMALL, OBJ_SHOW);
    oamSet(HUD_DOWN_OAM_ID, 28, 0, 3, 0, 0, inputButtonTile(1, 0), 1);
    oamSetEx(HUD_DOWN_OAM_ID, OBJ_SMALL, OBJ_SHOW);
    oamSet(HUD_LEFT_OAM_ID, 56, 0, 3, 0, 0, inputButtonTile(2, 0), 1);
    oamSetEx(HUD_LEFT_OAM_ID, OBJ_SMALL, OBJ_SHOW);
    oamSet(HUD_RIGHT_OAM_ID, 76, 0, 3, 0, 0, inputButtonTile(3, 0), 1);
    oamSetEx(HUD_RIGHT_OAM_ID, OBJ_SMALL, OBJ_SHOW);

    oamSet(TURRET_OAM_ID, 120, 136, 3, 0, 0, turretVisualTile(0), 2);
    oamSetEx(TURRET_OAM_ID, OBJ_SMALL, OBJ_SHOW);
    oamSet(CURSOR_OAM_ID, 168, 136, 3, 0, 0, INPUT_HUD_CURSOR_TILE, 1);
    oamSetEx(CURSOR_OAM_ID, OBJ_SMALL, OBJ_SHOW);

    for (index = 0; index < MAX_PLAYER_SHELLS; index++)
    {
        oamId = (u16)(PROJECTILE_OAM_BASE + ((u16)index << 2));
        oamSet(oamId, 0, 0, 3, 0, 0, INPUT_HUD_SHELL_TILE, 1);
        oamSetEx(oamId, OBJ_SMALL, OBJ_HIDE);
    }

    oamSet(
        ENEMY_FRONT_OAM_ID,
        ENEMY_INITIAL_X - HEAVY_TANK_TEST_MODULE_OFFSET - 16,
        ENEMY_INITIAL_Y - 16,
        2,
        0,
        0,
        HEAVY_TANK_TEST_FRONT_TILE_BASE,
        3);
    oamSetEx(ENEMY_FRONT_OAM_ID, OBJ_LARGE, OBJ_SHOW);
    oamSet(
        ENEMY_CENTER_OAM_ID,
        ENEMY_INITIAL_X - 16,
        ENEMY_INITIAL_Y - 16,
        2,
        0,
        0,
        HEAVY_TANK_TEST_CENTER_TILE_BASE,
        3);
    oamSetEx(ENEMY_CENTER_OAM_ID, OBJ_LARGE, OBJ_SHOW);
    oamSet(
        ENEMY_REAR_OAM_ID,
        ENEMY_INITIAL_X + HEAVY_TANK_TEST_MODULE_OFFSET - 16,
        ENEMY_INITIAL_Y - 16,
        2,
        0,
        0,
        HEAVY_TANK_TEST_REAR_TILE_BASE,
        3);
    oamSetEx(ENEMY_REAR_OAM_ID, OBJ_LARGE, OBJ_SHOW);
}

int main(void)
{
    u16 pad;
    u16 padDown;
    u16 pad2;
    u16 pad2Gameplay;
    u8 mouseConnected;
    u8 mouseRawX;
    u8 mouseRawY;
    u8 mouseButtons;
    u8 fireHeld;
    hull.positionX = (u16)128 << FIXED_SHIFT;
    hull.positionY = (u16)144 << FIXED_SHIFT;
    hull.speed = 0;
    hull.heading = 0;
    hull.targetHeading = hull.heading;
    hull.throttle = 0;
    hull.turn = 0;
    aim.x = (u16)(128 + INITIAL_AIM_DISTANCE);
    aim.y = 144;
    turret.heading = hull.heading;
    turret.targetHeading = hull.heading;
    driveMode = DRIVE_MODE_PC_LIKE;
    aimMode = AIM_MODE_MOUSE;
    menuOpen = 0;
    menuSelection = MENU_ROW_DRIVE;
    initializeProjectiles();
    initializeEnemy();
    fireCooldown = 0;
    resetMainGunInput();

    consoleInitDefaultText(0);
    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    initializePlayerSprites();

    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    drawGameplayText();

    initMouse(MOUSE_SLOW);
    WaitForVBlank();
    updateHullSprites(&hull);
    updateTurretAndCursorSprites(&hull, &turret);
    updateEnemySprite(&enemy);
    setScreenOn();

    while (1)
    {
        WaitForVBlank();

        pad = padsCurrent(0);
        padDown = padsDown(0);
        pad2 = padsCurrent(1);
        mouseConnected = mouseConnect[1] != 0 ? 1 : 0;

        if (mouseConnected != 0)
        {
            mouseRawX = mouse_x[1];
            mouseRawY = mouse_y[1];
            mouseButtons = mousePressed[1];
        }
        else
        {
            mouseRawX = 0;
            mouseRawY = 0;
            mouseButtons = 0;
        }

        if ((padDown & KEY_START) != 0)
        {
            menuOpen = menuOpen == 0 ? 1 : 0;
            resetMainGunInput();
            if (menuOpen != 0)
            {
                drawControlMenu();
            }
            else
            {
                drawGameplayText();
            }
            continue;
        }

        if (menuOpen != 0)
        {
            updateControlMenu(padDown);
            continue;
        }

        if ((padDown & KEY_SELECT) != 0)
        {
            initializeProjectiles();
            initializeEnemy();
            fireCooldown = 0;
            resetMainGunInput();
        }
        pad2Gameplay = mouseConnected == 0 ? pad2 : 0;

        if (aimMode == AIM_MODE_MOUSE)
        {
            updateAimCursor(mouseConnected, mouseRawX, mouseRawY);
            fireHeld = mouseConnected != 0 && (mouseButtons & mouse_L) != 0 ? 1 : 0;
        }
        else
        {
            fireHeld = (pad2Gameplay & KEY_B) != 0 ? 1 : 0;
        }

        if (driveMode == DRIVE_MODE_PC_LIKE)
        {
            resolveHullInputPcLike(pad, &hull);
        }
        else
        {
            resolveHullInputStick(pad, &hull);
        }

        updateHullHeading(&hull);
        updateHullSpeed(&hull);
        updateHullPosition(&hull);
        if (aimMode == AIM_MODE_MOUSE)
        {
            updateTurretTargetFromMouse(&hull, &turret);
        }
        else
        {
            updateTurretTargetFromPad2(pad2Gameplay, &turret);
        }
        updateTurretHeading(&turret);
        updateProjectiles(&enemy);
        updateMainGun(fireHeld, &hull, &turret);
        updateHullSprites(&hull);
        updateTurretAndCursorSprites(&hull, &turret);
        updateProjectileSprites();
        updateEnemySprite(&enemy);
        updateInputHudSprites(pad);
        updateEnemyHitFlash(&enemy);

        consoleDrawText(17, 0, driveMode == DRIVE_MODE_PC_LIKE ? "P" : "S");
        consoleDrawText(23, 0, aimMode == AIM_MODE_MOUSE ? "M" : "P");
        formatDecimal2(enemy.hp, statusValue);
        statusValue[2] = '\0';
        consoleDrawText(28, 0, "%s", statusValue);

        formatHex8(hull.heading, statusValue);
        statusValue[2] = '\0';
        consoleDrawText(15, 1, "%s", statusValue);
        formatHex8(turret.heading, statusValue);
        consoleDrawText(20, 1, "%s", statusValue);
        statusValue[0] = fireHeld != 0 ? '1' : '0';
        statusValue[1] = '\0';
        consoleDrawText(25, 1, "%s", statusValue);
        statusValue[0] = hexDigits[countActiveProjectiles() & 0x0F];
        consoleDrawText(29, 1, "%s", statusValue);

        statusValue[0] = armorFaceLetters[lastArmorImpact.face];
        statusValue[1] = '\0';
        consoleDrawText(16, 2, "%s", statusValue);
        if (lastArmorImpact.result == ARMOR_RESULT_NONE)
        {
            consoleDrawText(20, 2, "--");
        }
        else
        {
            formatImpactDegrees(lastArmorImpact.impactAngle, statusValue);
            statusValue[2] = '\0';
            consoleDrawText(20, 2, "%s", statusValue);
        }
        consoleDrawText(
            23,
            2,
            lastArmorImpact.result == ARMOR_RESULT_RICOCHET ? "RIC" :
                (lastArmorImpact.result == ARMOR_RESULT_PENETRATION ? "PEN" : "---"));
    }

    return 0;
}
