#include <snes.h>

#define FIXED_SHIFT 8

#define MAX_SPEED 0x0180
#define ACCELERATION 0x0008
#define DECELERATION 0x0006
#define TURN_RATE 2

#define HULL_MIN_X 16
#define HULL_MAX_X 239
#define HULL_MIN_Y 80
#define HULL_MAX_Y 207

#define HULL_OAM_ID 0
#define MARKER_OAM_ID 4
#define HULL_TILE 0
#define MARKER_TILE 2
#define MARKER_DISTANCE 12

typedef struct
{
    s32 positionX;
    s32 positionY;
    u16 speed;
    u8 heading;
    u8 targetHeading;
    u8 throttle;
} HullState;

static const char hexDigits[] = "0123456789ABCDEF";

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

static const u8 hullTileData[32] = {
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const u8 markerTileData[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x3C,
    0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const u16 hullPalette[16] = {
    0x0000,
    RGB5(8, 23, 10),
    RGB5(31, 29, 4),
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
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

static u8 resolveDesiredHeading(u16 pad, u8 *desiredHeading)
{
    s8 inputX = 0;
    s8 inputY = 0;

    if ((pad & KEY_LEFT) != 0)
    {
        inputX--;
    }
    if ((pad & KEY_RIGHT) != 0)
    {
        inputX++;
    }
    if ((pad & KEY_UP) != 0)
    {
        inputY--;
    }
    if ((pad & KEY_DOWN) != 0)
    {
        inputY++;
    }

    if (inputX == 0 && inputY == 0)
    {
        return 0;
    }

    if (inputY < 0)
    {
        if (inputX < 0)
        {
            *desiredHeading = 160;
        }
        else if (inputX > 0)
        {
            *desiredHeading = 224;
        }
        else
        {
            *desiredHeading = 192;
        }
    }
    else if (inputY > 0)
    {
        if (inputX < 0)
        {
            *desiredHeading = 96;
        }
        else if (inputX > 0)
        {
            *desiredHeading = 32;
        }
        else
        {
            *desiredHeading = 64;
        }
    }
    else if (inputX < 0)
    {
        *desiredHeading = 128;
    }
    else
    {
        *desiredHeading = 0;
    }

    return 1;
}

static void updateHullHeading(HullState *hull)
{
    s8 angularDelta;

    if (hull->throttle == 0)
    {
        hull->targetHeading = hull->heading;
        return;
    }

    angularDelta = (s8)(hull->targetHeading - hull->heading);
    if (angularDelta > TURN_RATE)
    {
        hull->heading = (u8)(hull->heading + TURN_RATE);
    }
    else if (angularDelta < -TURN_RATE)
    {
        hull->heading = (u8)(hull->heading - TURN_RATE);
    }
    else
    {
        hull->heading = hull->targetHeading;
    }
}

static void updateHullSpeed(HullState *hull)
{
    if (hull->throttle != 0)
    {
        if (hull->speed >= MAX_SPEED - ACCELERATION)
        {
            hull->speed = MAX_SPEED;
        }
        else
        {
            hull->speed += ACCELERATION;
        }
    }
    else if (hull->speed <= DECELERATION)
    {
        hull->speed = 0;
    }
    else
    {
        hull->speed -= DECELERATION;
    }
}

static void updateHullPosition(HullState *hull)
{
    s32 deltaX = ((s32)cos256(hull->heading) * hull->speed) >> 8;
    s32 deltaY = ((s32)sin256(hull->heading) * hull->speed) >> 8;
    s32 minimumX = (s32)HULL_MIN_X << FIXED_SHIFT;
    s32 maximumX = (s32)HULL_MAX_X << FIXED_SHIFT;
    s32 minimumY = (s32)HULL_MIN_Y << FIXED_SHIFT;
    s32 maximumY = (s32)HULL_MAX_Y << FIXED_SHIFT;

    hull->positionX += deltaX;
    hull->positionY += deltaY;

    if (hull->positionX < minimumX)
    {
        hull->positionX = minimumX;
    }
    else if (hull->positionX > maximumX)
    {
        hull->positionX = maximumX;
    }

    if (hull->positionY < minimumY)
    {
        hull->positionY = minimumY;
    }
    else if (hull->positionY > maximumY)
    {
        hull->positionY = maximumY;
    }
}

static void updateHullSprites(const HullState *hull)
{
    s16 centerX = (s16)(hull->positionX >> FIXED_SHIFT);
    s16 centerY = (s16)(hull->positionY >> FIXED_SHIFT);
    s16 markerX = centerX + ((cos256(hull->heading) * MARKER_DISTANCE) >> 8) - 4;
    s16 markerY = centerY + ((sin256(hull->heading) * MARKER_DISTANCE) >> 8) - 4;

    oamSetXY(HULL_OAM_ID, centerX - 8, centerY - 8);
    oamSetXY(MARKER_OAM_ID, markerX, markerY);
}

static void formatStatusLine(
    u16 pad,
    const HullState *hull,
    u8 mouseConnected,
    u8 mouseRawX,
    u8 mouseRawY,
    u8 mouseButtons,
    char *output)
{
    output[0] = 'P';
    formatHex16(pad, &output[1]);
    output[5] = 'T';
    output[6] = hull->throttle != 0 ? '1' : '0';
    output[7] = 'X';
    formatHex8((u8)(hull->positionX >> FIXED_SHIFT), &output[8]);
    output[10] = 'Y';
    formatHex8((u8)(hull->positionY >> FIXED_SHIFT), &output[11]);
    output[13] = 'H';
    formatHex8(hull->heading, &output[14]);
    output[16] = 'G';
    formatHex8(hull->targetHeading, &output[17]);
    output[19] = 'S';
    formatHex8((u8)(hull->speed >> 4), &output[20]);
    output[22] = 'C';
    output[23] = mouseConnected != 0 ? '1' : '0';
    output[24] = 'U';
    formatHex8(mouseRawX, &output[25]);
    output[27] = 'V';
    formatHex8(mouseRawY, &output[28]);
    output[30] = 'B';
    output[31] = hexDigits[mouseButtons & 0x03];
    output[32] = '\0';
}

static void initializeHullSprites(void)
{
    oamInit();
    oamInitGfxAttr(0x0000, OBJ_SIZE8_L16);

    dmaCopyVram((u8 *)hullTileData, 0x0000, sizeof(hullTileData));
    dmaCopyVram((u8 *)hullTileData, 0x0010, sizeof(hullTileData));
    dmaCopyVram((u8 *)markerTileData, 0x0020, sizeof(markerTileData));
    dmaCopyVram((u8 *)hullTileData, 0x0100, sizeof(hullTileData));
    dmaCopyVram((u8 *)hullTileData, 0x0110, sizeof(hullTileData));
    dmaCopyCGram((u8 *)hullPalette, 128, sizeof(hullPalette));

    oamSet(HULL_OAM_ID, 120, 136, 3, 0, 0, HULL_TILE, 0);
    oamSetEx(HULL_OAM_ID, OBJ_LARGE, OBJ_SHOW);
    oamSet(MARKER_OAM_ID, 136, 140, 3, 0, 0, MARKER_TILE, 0);
    oamSetEx(MARKER_OAM_ID, OBJ_SMALL, OBJ_SHOW);
}

int main(void)
{
    HullState hull;
    u16 pad;
    u8 desiredHeading;
    u8 mouseConnected;
    u8 mouseRawX;
    u8 mouseRawY;
    u8 mouseButtons;
    u8 mouseSensitivityValue;
    u8 previousMouseSensitivity = 0;
    char statusLine[33];

    hull.positionX = (s32)128 << FIXED_SHIFT;
    hull.positionY = (s32)144 << FIXED_SHIFT;
    hull.speed = 0;
    hull.heading = 0;
    hull.targetHeading = 0;
    hull.throttle = 0;

    consoleInitDefaultText(0);
    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    initializeHullSprites();

    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    consoleDrawText(4, 0, "S1-01 HULL MOVEMENT V0");
    consoleDrawText(0, 2, "P T X Y H G S | C U V B");
    consoleDrawText(0, 3, "P0000T0X80Y90H00G00S00C0U00V00B0");
    consoleDrawText(0, 5, "X/Y PIX H/G HDG/TGT S Q4.4");
    consoleDrawText(0, 6, "C/U/V/B P2 CONN/RAWX/RAWY/BTN");

    initMouse(MOUSE_SLOW);
    WaitForVBlank();
    updateHullSprites(&hull);
    setScreenOn();

    while (1)
    {
        WaitForVBlank();

        pad = padsCurrent(0);
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

        hull.throttle = resolveDesiredHeading(pad, &desiredHeading);
        if (hull.throttle != 0)
        {
            hull.targetHeading = desiredHeading;
        }

        updateHullHeading(&hull);
        updateHullSpeed(&hull);
        updateHullPosition(&hull);
        updateHullSprites(&hull);

        formatStatusLine(
            pad,
            &hull,
            mouseConnected,
            mouseRawX,
            mouseRawY,
            mouseButtons,
            statusLine);
        consoleDrawText(0, 3, "%s", statusLine);

        previousMouseSensitivity = mouseSensitivityValue;
    }

    return previousMouseSensitivity;
}
