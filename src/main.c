#include <snes.h>
#include "hull_placeholder.inc"

#define FIXED_SHIFT 8

#define MAX_FORWARD_SPEED 0x0180
#define MAX_REVERSE_SPEED 0x00C0
#define FORWARD_ACCELERATION 0x0008
#define REVERSE_ACCELERATION 0x0008
#define COAST_DECELERATION 0x0006
#define TURN_RATE 2

#define HULL_MIN_X 16
#define HULL_MAX_X 239
#define HULL_MIN_Y 80
#define HULL_MAX_Y 207

#define HULL_OAM_ID 0
#define MARKER_OAM_ID 4
#define HULL_GFX_VRAM_ADDRESS 0x0000
#define HULL_GFX_BYTES 2048
#define MARKER_GFX_VRAM_ADDRESS 0x0400
#define MARKER_TILE 64
#define MARKER_DISTANCE 12

typedef struct
{
    s32 positionX;
    s32 positionY;
    s16 speed;
    u8 heading;
    s8 throttle;
    s8 turn;
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

static const u8 markerTileData[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x3C,
    0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
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

static u8 hullVisualFrame(u8 heading)
{
    return (u8)((((u16)heading + 8) >> 4) & 0x0F);
}

static u16 hullVisualTile(u8 visualFrame)
{
    return (u16)(((u16)(visualFrame >> 3) << 5) + ((u16)(visualFrame & 0x07) << 1));
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
    u8 visualFrame = hullVisualFrame(hull->heading);
    s16 centerX = (s16)(hull->positionX >> FIXED_SHIFT);
    s16 centerY = (s16)(hull->positionY >> FIXED_SHIFT);
    s16 markerX = centerX + ((cos256(hull->heading) * MARKER_DISTANCE) >> 8) - 4;
    s16 markerY = centerY + ((sin256(hull->heading) * MARKER_DISTANCE) >> 8) - 4;

    oamSetGfxOffset(HULL_OAM_ID, hullVisualTile(visualFrame));
    oamSetXY(HULL_OAM_ID, centerX - 8, centerY - 8);
    oamSetXY(MARKER_OAM_ID, markerX, markerY);
}

static void formatStatusLine(
    u16 pad,
    const HullState *hull,
    u8 mouseConnected,
    char *output)
{
    u16 speedMagnitude;

    output[0] = 'P';
    formatHex16(pad, &output[1]);
    output[5] = 'T';
    output[6] = hull->throttle > 0 ? 'F' : (hull->throttle < 0 ? 'R' : 'N');
    output[7] = 'R';
    output[8] = hull->turn < 0 ? 'L' : (hull->turn > 0 ? 'R' : 'N');
    output[9] = 'X';
    formatHex8((u8)(hull->positionX >> FIXED_SHIFT), &output[10]);
    output[12] = 'Y';
    formatHex8((u8)(hull->positionY >> FIXED_SHIFT), &output[13]);
    output[15] = 'H';
    formatHex8(hull->heading, &output[16]);
    output[18] = 'S';
    output[19] = hull->speed < 0 ? '-' : '+';
    speedMagnitude = hull->speed < 0 ? (u16)(-hull->speed) : (u16)hull->speed;
    formatHex16(speedMagnitude, &output[20]);
    output[24] = 'C';
    output[25] = mouseConnected != 0 ? '1' : '0';
    output[26] = 'F';
    formatHex8(hullVisualFrame(hull->heading), &output[27]);
    output[29] = '\0';
}

static void formatMouseStatusLine(
    u8 mouseRawX,
    u8 mouseRawY,
    u8 mouseButtons,
    u8 mouseSensitivityValue,
    char *output)
{
    output[0] = 'U';
    formatHex8(mouseRawX, &output[1]);
    output[3] = 'V';
    formatHex8(mouseRawY, &output[4]);
    output[6] = 'B';
    output[7] = hexDigits[mouseButtons & 0x03];
    output[8] = 'M';
    output[9] = hexDigits[mouseSensitivityValue & 0x0F];
    output[10] = '\0';
}

static void initializeHullSprites(void)
{
    oamInit();
    oamInitGfxAttr(HULL_GFX_VRAM_ADDRESS, OBJ_SIZE8_L16);

    dmaCopyVram(&hullPlaceholderTiles, HULL_GFX_VRAM_ADDRESS, HULL_GFX_BYTES);
    dmaCopyVram((u8 *)markerTileData, MARKER_GFX_VRAM_ADDRESS, sizeof(markerTileData));
    dmaCopyCGram(&hullPlaceholderPalette, 128, (&hullPlaceholderPaletteEnd - &hullPlaceholderPalette));

    oamSet(HULL_OAM_ID, 120, 136, 3, 0, 0, hullVisualTile(0), 0);
    oamSetEx(HULL_OAM_ID, OBJ_LARGE, OBJ_SHOW);
    oamSet(MARKER_OAM_ID, 136, 140, 3, 0, 0, MARKER_TILE, 0);
    oamSetEx(MARKER_OAM_ID, OBJ_SMALL, OBJ_SHOW);
}

int main(void)
{
    HullState hull;
    u16 pad;
    u8 mouseConnected;
    u8 mouseRawX;
    u8 mouseRawY;
    u8 mouseButtons;
    u8 mouseSensitivityValue;
    char statusLine[30];
    char mouseStatusLine[11];

    hull.positionX = (s32)128 << FIXED_SHIFT;
    hull.positionY = (s32)144 << FIXED_SHIFT;
    hull.speed = 0;
    hull.heading = 0;
    hull.throttle = 0;
    hull.turn = 0;

    consoleInitDefaultText(0);
    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    initializeHullSprites();

    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    consoleDrawText(5, 0, "S1-02 HULL 16-DIR V0");
    consoleDrawText(0, 2, "PRAW T R X  Y  H  SPEED C FRM");
    consoleDrawText(0, 3, "P0000TNRNX80Y90H00S+0000C0F00");
    consoleDrawText(0, 5, "P2 RAW U/V  B BTN  M SENS");
    consoleDrawText(0, 6, "U00V00B0M0");

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

        resolveHullInput(pad, &hull);

        updateHullHeading(&hull);
        updateHullSpeed(&hull);
        updateHullPosition(&hull);
        updateHullSprites(&hull);

        formatStatusLine(
            pad,
            &hull,
            mouseConnected,
            statusLine);
        consoleDrawText(0, 3, "%s", statusLine);

        formatMouseStatusLine(
            mouseRawX,
            mouseRawY,
            mouseButtons,
            mouseSensitivityValue,
            mouseStatusLine);
        consoleDrawText(0, 6, "%s", mouseStatusLine);
    }

    return 0;
}
