#include <snes.h>
#include "hull_placeholder.inc"
#include "input_hud.inc"
#include "turret_placeholder.inc"

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

#define ENEMY_INITIAL_X 208
#define ENEMY_INITIAL_Y 144
#define ENEMY_INITIAL_HEADING 128
#define ENEMY_MAX_HP 3
#define ENEMY_HITBOX_HALF_WIDTH 13
#define ENEMY_HITBOX_HALF_HEIGHT 13
#define ENEMY_HIT_FLASH_FRAMES 6

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
#define ENEMY_OAM_ID 48
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
#define MARKER_DISTANCE 20

typedef struct
{
    u16 positionX;
    u16 positionY;
    s16 speed;
    u8 heading;
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
} EnemyState;

typedef enum
{
    AIM_MODE_MOUSE = 0,
    AIM_MODE_PAD2 = 1
} AimMode;

static const char hexDigits[] = "0123456789ABCDEF";
static HullState hull;
static TurretState turret;
static AimState aim;
static AimMode aimMode;
static ProjectileState projectiles[MAX_PLAYER_SHELLS];
static EnemyState enemy;
static u8 enemyHitCount;
static u8 fireCooldown;
static u8 previousFireHeld;
static u8 fireInputArmed;
static u8 lastShotHeading;
static char controlStatusLine[13];
static char hullStatusLine[20];
static char turretStatusLine[18];
static char aimStatusLine[20];
static char inputStatusLine[25];
static char aimInputStatusLine[9];
static char gunStatusLine[20];
static char enemyStatusLine[13];

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

static void formatHex16(u16 value, char *output)
{
    output[0] = hexDigits[(value >> 12) & 0x0F];
    output[1] = hexDigits[(value >> 8) & 0x0F];
    output[2] = hexDigits[(value >> 4) & 0x0F];
    output[3] = hexDigits[value & 0x0F];
}

static void formatHex8(u8 value, char *output)
{
    output[0] = hexDigits[(value >> 4) & 0x0F];
    output[1] = hexDigits[value & 0x0F];
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

static void updateTurretTargetFromPad2(u16 pad2, TurretState *turret)
{
    u8 upHeld = (pad2 & KEY_UP) != 0;
    u8 downHeld = (pad2 & KEY_DOWN) != 0;
    u8 leftHeld = (pad2 & KEY_LEFT) != 0;
    u8 rightHeld = (pad2 & KEY_RIGHT) != 0;
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
        turret->targetHeading = vertical < 0 ? 0xE0 : (vertical > 0 ? 0x20 : 0x00);
    }
    else if (horizontal < 0)
    {
        turret->targetHeading = vertical < 0 ? 0xA0 : (vertical > 0 ? 0x60 : 0x80);
    }
    else if (vertical < 0)
    {
        turret->targetHeading = 0xC0;
    }
    else if (vertical > 0)
    {
        turret->targetHeading = 0x40;
    }
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
    enemy.hp = ENEMY_MAX_HP;
    enemy.maxHp = ENEMY_MAX_HP;
    enemy.active = 1;
    enemy.hitFlashFrames = 0;
    enemyHitCount = 0;
}

static u8 projectileHitsEnemy(u16 pixelX, u16 pixelY, const EnemyState *enemy)
{
    u16 enemyX;
    u16 enemyY;

    if (enemy->active == 0)
    {
        return 0;
    }

    enemyX = enemy->positionX >> FIXED_SHIFT;
    enemyY = enemy->positionY >> FIXED_SHIFT;
    return pixelX >= enemyX - ENEMY_HITBOX_HALF_WIDTH &&
        pixelX <= enemyX + ENEMY_HITBOX_HALF_WIDTH &&
        pixelY >= enemyY - ENEMY_HITBOX_HALF_HEIGHT &&
        pixelY <= enemyY + ENEMY_HITBOX_HALF_HEIGHT;
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

    for (index = 0; index < MAX_PLAYER_SHELLS; index++)
    {
        if (projectiles[index].active != 1)
        {
            continue;
        }

        if (addProjectileDelta(&projectiles[index].positionX, projectiles[index].velocityX) == 0 ||
            addProjectileDelta(&projectiles[index].positionY, projectiles[index].velocityY) == 0)
        {
            projectiles[index].active = 0;
            continue;
        }

        pixelX = projectiles[index].positionX >> FIXED_SHIFT;
        pixelY = projectiles[index].positionY >> FIXED_SHIFT;
        if (projectileHitsEnemy(pixelX, pixelY, enemy) != 0)
        {
            projectiles[index].active = 0;
            damageEnemy(enemy);
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
            lastShotHeading = shotHeading;
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

static void updateAimMode(u16 padDown)
{
    if ((padDown & KEY_SELECT) == 0)
    {
        return;
    }

    aimMode = aimMode == AIM_MODE_MOUSE ? AIM_MODE_PAD2 : AIM_MODE_MOUSE;
    resetMainGunInput();
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

static void resolveHullInput(u16 pad, HullState *hull)
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

static void updateHullHeading(HullState *hull)
{
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
    if (enemy->active == 0 ||
        (enemy->hitFlashFrames != 0 && (enemy->hitFlashFrames & 1) == 0))
    {
        oamSetVisible(ENEMY_OAM_ID, OBJ_HIDE);
        return;
    }

    oamSetGfxOffset(ENEMY_OAM_ID, hullVisualTile(headingVisualFrame(enemy->heading)));
    oamSetXY(
        ENEMY_OAM_ID,
        (s16)(enemy->positionX >> FIXED_SHIFT) - 16,
        (s16)(enemy->positionY >> FIXED_SHIFT) - 16);
    oamSetVisible(ENEMY_OAM_ID, OBJ_SHOW);
}

static void updateInputHudSprites(u16 pad)
{
    oamSetGfxOffset(HUD_UP_OAM_ID, inputButtonTile(0, (pad & KEY_UP) != 0));
    oamSetGfxOffset(HUD_DOWN_OAM_ID, inputButtonTile(1, (pad & KEY_DOWN) != 0));
    oamSetGfxOffset(HUD_LEFT_OAM_ID, inputButtonTile(2, (pad & KEY_LEFT) != 0));
    oamSetGfxOffset(HUD_RIGHT_OAM_ID, inputButtonTile(3, (pad & KEY_RIGHT) != 0));
}

static void formatControlStatusLine(const HullState *hull, char *output)
{
    output[0] = 'T';
    output[1] = 'H';
    output[2] = 'R';
    output[3] = ':';
    output[4] = hull->throttle > 0 ? 'F' : (hull->throttle < 0 ? 'R' : 'N');
    output[5] = ' ';
    output[6] = 'T';
    output[7] = 'U';
    output[8] = 'R';
    output[9] = 'N';
    output[10] = ':';
    output[11] = hull->turn < 0 ? 'L' : (hull->turn > 0 ? 'R' : 'N');
    output[12] = '\0';
}

static void formatHullStatusLine(const HullState *hull, char *output)
{
    u16 speedMagnitude;

    output[0] = 'H';
    output[1] = ':';
    formatHex8(hull->heading, &output[2]);
    output[4] = ' ';
    output[5] = 'H';
    output[6] = 'F';
    output[7] = ':';
    formatHex8(headingVisualFrame(hull->heading), &output[8]);
    output[10] = ' ';
    output[11] = 'S';
    output[12] = ':';
    output[13] = hull->speed < 0 ? '-' : '+';
    speedMagnitude = hull->speed < 0 ? (u16)(-hull->speed) : (u16)hull->speed;
    formatHex16(speedMagnitude, &output[14]);
    output[18] = '\0';
}

static void formatTurretStatusLine(const TurretState *turret, char *output)
{
    output[0] = 'T';
    output[1] = ':';
    formatHex8(turret->heading, &output[2]);
    output[4] = ' ';
    output[5] = 'T';
    output[6] = 'G';
    output[7] = ':';
    formatHex8(turret->targetHeading, &output[8]);
    output[10] = ' ';
    output[11] = 'T';
    output[12] = 'F';
    output[13] = ':';
    formatHex8(headingVisualFrame(turret->heading), &output[14]);
    output[16] = '\0';
}

static void formatAimStatusLine(const TurretState *turret, char *output)
{
    output[0] = 'A';
    output[1] = 'I';
    output[2] = 'M';
    output[3] = ':';
    output[4] = aimMode == AIM_MODE_MOUSE ? 'M' : 'P';
    output[5] = ' ';

    if (aimMode == AIM_MODE_MOUSE)
    {
        output[6] = 'A';
        output[7] = 'X';
        output[8] = ':';
        formatHex8((u8)aim.x, &output[9]);
        output[11] = ' ';
        output[12] = 'A';
        output[13] = 'Y';
        output[14] = ':';
        formatHex8((u8)aim.y, &output[15]);
        output[17] = ' ';
        output[18] = ' ';
        output[19] = '\0';
    }
    else
    {
        output[6] = 'I';
        output[7] = 'H';
        output[8] = ':';
        formatHex8(turret->targetHeading, &output[9]);
        output[11] = ' ';
        output[12] = ' ';
        output[13] = ' ';
        output[14] = ' ';
        output[15] = ' ';
        output[16] = ' ';
        output[17] = ' ';
        output[18] = ' ';
        output[19] = '\0';
    }
}

static void formatInputStatusLine(
    u16 pad,
    u16 pad2,
    u8 mouseConnected,
    u8 mouseRawX,
    u8 mouseRawY,
    char *output)
{
    output[0] = 'P';
    output[1] = '1';
    output[2] = ':';
    formatHex16(pad, &output[3]);
    output[7] = ' ';
    output[8] = 'P';
    output[9] = '2';
    output[10] = ':';

    if (aimMode == AIM_MODE_MOUSE)
    {
        output[11] = 'M';
        output[12] = mouseConnected != 0 ? '1' : '0';
        output[13] = ' ';
        output[14] = 'U';
        formatHex8(mouseRawX, &output[15]);
        output[17] = ' ';
        output[18] = 'V';
        formatHex8(mouseRawY, &output[19]);
        output[21] = ' ';
        output[22] = ' ';
        output[23] = ' ';
        output[24] = '\0';
    }
    else
    {
        output[11] = 'P';
        output[12] = ' ';
        formatHex16(pad2, &output[13]);
        output[17] = ' ';
        output[18] = ' ';
        output[19] = ' ';
        output[20] = ' ';
        output[21] = ' ';
        output[22] = ' ';
        output[23] = ' ';
        output[24] = '\0';
    }
}

static void formatAimInputStatusLine(u8 mouseButtons, u8 mouseSensitivityValue, u16 pad2, char *output)
{
    if (aimMode == AIM_MODE_MOUSE)
    {
        output[0] = 'M';
        output[1] = 'B';
        output[2] = hexDigits[mouseButtons & 0x03];
        output[3] = ' ';
        output[4] = 'S';
        output[5] = hexDigits[mouseSensitivityValue & 0x0F];
        output[6] = ' ';
        output[7] = ' ';
        output[8] = '\0';
    }
    else
    {
        output[0] = 'P';
        output[1] = 'B';
        output[2] = (pad2 & KEY_B) != 0 ? '1' : '0';
        output[3] = ' ';
        output[4] = ' ';
        output[5] = ' ';
        output[6] = ' ';
        output[7] = ' ';
        output[8] = '\0';
    }
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

static void formatGunStatusLine(u8 fireHeld, char *output)
{
    output[0] = 'G';
    output[1] = 'U';
    output[2] = 'N';
    output[3] = ' ';
    output[4] = 'F';
    output[5] = fireHeld != 0 ? '1' : '0';
    output[6] = ' ';
    output[7] = 'C';
    output[8] = 'D';
    formatHex8(fireCooldown, &output[9]);
    output[11] = ' ';
    output[12] = 'S';
    output[13] = 'H';
    output[14] = hexDigits[countActiveProjectiles() & 0x0F];
    output[15] = ' ';
    output[16] = 'H';
    formatHex8(lastShotHeading, &output[17]);
    output[19] = '\0';
}

static void formatEnemyStatusLine(const EnemyState *enemy, char *output)
{
    output[0] = 'E';
    output[1] = 'N';
    output[2] = ' ';
    output[3] = 'H';
    output[4] = 'P';
    output[5] = hexDigits[enemy->hp & 0x0F];
    output[6] = ' ';
    if (enemy->active != 0)
    {
        output[7] = 'H';
        output[8] = 'I';
        output[9] = 'T';
        formatHex8(enemyHitCount, &output[10]);
    }
    else
    {
        output[7] = 'D';
        output[8] = 'E';
        output[9] = 'S';
        output[10] = 'T';
        output[11] = ' ';
    }
    output[12] = '\0';
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
    dmaCopyCGram(&hullPlaceholderPalette, 128, (&hullPlaceholderPaletteEnd - &hullPlaceholderPalette));
    dmaCopyCGram(&inputHudPalette, 144, (&inputHudPaletteEnd - &inputHudPalette));
    dmaCopyCGram(&turretPlaceholderPalette, 160, (&turretPlaceholderPaletteEnd - &turretPlaceholderPalette));

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
        ENEMY_OAM_ID,
        ENEMY_INITIAL_X - 16,
        ENEMY_INITIAL_Y - 16,
        2,
        0,
        0,
        hullVisualTile(headingVisualFrame(ENEMY_INITIAL_HEADING)),
        0);
    oamSetEx(ENEMY_OAM_ID, OBJ_LARGE, OBJ_SHOW);
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
    u8 mouseSensitivityValue;
    u8 fireHeld;
    hull.positionX = (u16)128 << FIXED_SHIFT;
    hull.positionY = (u16)144 << FIXED_SHIFT;
    hull.speed = 0;
    hull.heading = 0;
    hull.throttle = 0;
    hull.turn = 0;
    aim.x = (u16)(128 + INITIAL_AIM_DISTANCE);
    aim.y = 144;
    turret.heading = hull.heading;
    turret.targetHeading = hull.heading;
    aimMode = AIM_MODE_MOUSE;
    initializeProjectiles();
    initializeEnemy();
    fireCooldown = 0;
    resetMainGunInput();
    lastShotHeading = turret.heading;

    consoleInitDefaultText(0);
    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    initializePlayerSprites();

    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    consoleDrawText(13, 0, "THR:N TURN:N");
    consoleDrawText(13, 1, "S3-01 ");
    consoleDrawText(0, 2, "H:00 HF:00 S:+0000");
    consoleDrawText(0, 3, "T:00 TG:00 TF:00");
    consoleDrawText(0, 4, "AIM:M AX:B0 AY:90");
    consoleDrawText(0, 5, "P1:0000 P2:M0 U00 V00");
    consoleDrawText(0, 6, "MB0 S0  ");
    consoleDrawText(0, 7, "GUN F0 CD00 SH0 H00");
    consoleDrawText(0, 8, "EN HP3 HIT00");

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
            mouseSensitivityValue = mouseSensitivity[1];
        }
        else
        {
            mouseRawX = 0;
            mouseRawY = 0;
            mouseButtons = 0;
            mouseSensitivityValue = 0;
        }

        updateAimMode(padDown);
        if ((padDown & KEY_START) != 0)
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

        resolveHullInput(pad, &hull);

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

        formatControlStatusLine(&hull, controlStatusLine);
        consoleDrawText(13, 0, "%s", controlStatusLine);

        formatHullStatusLine(&hull, hullStatusLine);
        consoleDrawText(0, 2, "%s", hullStatusLine);

        formatTurretStatusLine(&turret, turretStatusLine);
        consoleDrawText(0, 3, "%s", turretStatusLine);

        formatAimStatusLine(&turret, aimStatusLine);
        consoleDrawText(0, 4, "%s", aimStatusLine);

        formatInputStatusLine(
            pad,
            pad2,
            mouseConnected,
            mouseRawX,
            mouseRawY,
            inputStatusLine);
        consoleDrawText(0, 5, "%s", inputStatusLine);

        formatAimInputStatusLine(mouseButtons, mouseSensitivityValue, pad2, aimInputStatusLine);
        consoleDrawText(0, 6, "%s", aimInputStatusLine);

        formatGunStatusLine(fireHeld, gunStatusLine);
        consoleDrawText(0, 7, "%s", gunStatusLine);

        formatEnemyStatusLine(&enemy, enemyStatusLine);
        consoleDrawText(0, 8, "%s", enemyStatusLine);
    }

    return 0;
}
