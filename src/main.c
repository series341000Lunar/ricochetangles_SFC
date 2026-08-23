#include <snes.h>
#include "hull_placeholder.inc"
#include "input_hud.inc"

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
#define HUD_UP_OAM_ID 8
#define HUD_DOWN_OAM_ID 12
#define HUD_LEFT_OAM_ID 16
#define HUD_RIGHT_OAM_ID 20
#define HULL_GFX_VRAM_ADDRESS 0x0000
#define HULL_GFX_BYTES 8192
#define INPUT_HUD_GFX_VRAM_ADDRESS 0x1000
#define INPUT_HUD_GFX_BYTES 3072
#define INPUT_HUD_TILE_BASE 256
#define INPUT_HUD_MARKER_TILE (INPUT_HUD_TILE_BASE + 64)
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

static const char hexDigits[] = "0123456789ABCDEF";
static HullState hull;
static char controlStatusLine[13];
static char hullStatusLine[24];
static char inputPositionLine[18];
static char mouseStatusLine[20];

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
    return (u16)(((u16)(visualFrame >> 2) << 6) + ((u16)(visualFrame & 0x03) << 2));
}

static u16 inputButtonTile(u8 buttonIndex, u8 pressed)
{
    return (u16)(INPUT_HUD_TILE_BASE + ((u16)buttonIndex << 2) + (pressed != 0 ? 2 : 0));
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
    u8 visualFrame = hullVisualFrame(hull->heading);
    s16 centerX = (s16)(hull->positionX >> FIXED_SHIFT);
    s16 centerY = (s16)(hull->positionY >> FIXED_SHIFT);
    s16 markerX = centerX + ((cos256(hull->heading) * MARKER_DISTANCE) >> 8) - 8;
    s16 markerY = centerY + ((sin256(hull->heading) * MARKER_DISTANCE) >> 8) - 8;

    oamSetGfxOffset(HULL_OAM_ID, hullVisualTile(visualFrame));
    oamSetXY(HULL_OAM_ID, centerX - 16, centerY - 16);
    oamSetXY(MARKER_OAM_ID, markerX, markerY);
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
    output[1] = 'D';
    output[2] = 'G';
    output[3] = ':';
    formatHex8(hull->heading, &output[4]);
    output[6] = ' ';
    output[7] = 'F';
    output[8] = 'R';
    output[9] = 'M';
    output[10] = ':';
    formatHex8(hullVisualFrame(hull->heading), &output[11]);
    output[13] = ' ';
    output[14] = 'S';
    output[15] = 'P';
    output[16] = 'D';
    output[17] = ':';
    output[18] = hull->speed < 0 ? '-' : '+';
    speedMagnitude = hull->speed < 0 ? (u16)(-hull->speed) : (u16)hull->speed;
    formatHex16(speedMagnitude, &output[19]);
    output[23] = '\0';
}

static void formatInputPositionLine(u16 pad, const HullState *hull, char *output)
{
    output[0] = 'P';
    output[1] = '1';
    output[2] = ':';
    formatHex16(pad, &output[3]);
    output[7] = ' ';
    output[8] = 'X';
    output[9] = ':';
    formatHex8((u8)(hull->positionX >> FIXED_SHIFT), &output[10]);
    output[12] = ' ';
    output[13] = 'Y';
    output[14] = ':';
    formatHex8((u8)(hull->positionY >> FIXED_SHIFT), &output[15]);
    output[17] = '\0';
}

static void formatMouseStatusLine(
    u8 mouseConnected,
    u8 mouseRawX,
    u8 mouseRawY,
    u8 mouseButtons,
    u8 mouseSensitivityValue,
    char *output)
{
    output[0] = 'P';
    output[1] = '2';
    output[2] = ':';
    output[3] = 'C';
    output[4] = mouseConnected != 0 ? '1' : '0';
    output[5] = ' ';
    output[6] = 'U';
    formatHex8(mouseRawX, &output[7]);
    output[9] = ' ';
    output[10] = 'V';
    formatHex8(mouseRawY, &output[11]);
    output[13] = ' ';
    output[14] = 'B';
    output[15] = hexDigits[mouseButtons & 0x03];
    output[16] = ' ';
    output[17] = 'S';
    output[18] = hexDigits[mouseSensitivityValue & 0x0F];
    output[19] = '\0';
}

static void initializeHullSprites(void)
{
    oamInit();
    oamInitGfxAttr(HULL_GFX_VRAM_ADDRESS, OBJ_SIZE16_L32);

    dmaCopyVram(&hullPlaceholderTiles, HULL_GFX_VRAM_ADDRESS, HULL_GFX_BYTES);
    dmaCopyVram(&inputHudTiles, INPUT_HUD_GFX_VRAM_ADDRESS, INPUT_HUD_GFX_BYTES);
    dmaCopyCGram(&hullPlaceholderPalette, 128, (&hullPlaceholderPaletteEnd - &hullPlaceholderPalette));
    dmaCopyCGram(&inputHudPalette, 144, (&inputHudPaletteEnd - &inputHudPalette));

    oamSet(HULL_OAM_ID, 112, 128, 3, 0, 0, hullVisualTile(0), 0);
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
}

int main(void)
{
    u16 pad;
    u8 mouseConnected;
    u8 mouseRawX;
    u8 mouseRawY;
    u8 mouseButtons;
    u8 mouseSensitivityValue;
    hull.positionX = (u16)128 << FIXED_SHIFT;
    hull.positionY = (u16)144 << FIXED_SHIFT;
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

    consoleDrawText(13, 0, "THR:N TURN:N");
    consoleDrawText(13, 1, "S1-02R");
    consoleDrawText(0, 2, "HDG:00 FRM:00 SPD:+0000");
    consoleDrawText(0, 3, "P1:0000 X:80 Y:90");
    consoleDrawText(0, 4, "P2:C0 U00 V00 B0 S0");

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
        updateInputHudSprites(pad);

        formatControlStatusLine(&hull, controlStatusLine);
        consoleDrawText(13, 0, "%s", controlStatusLine);

        formatHullStatusLine(&hull, hullStatusLine);
        consoleDrawText(0, 2, "%s", hullStatusLine);

        formatInputPositionLine(pad, &hull, inputPositionLine);
        consoleDrawText(0, 3, "%s", inputPositionLine);

        formatMouseStatusLine(
            mouseConnected,
            mouseRawX,
            mouseRawY,
            mouseButtons,
            mouseSensitivityValue,
            mouseStatusLine);
        consoleDrawText(0, 4, "%s", mouseStatusLine);
    }

    return 0;
}
