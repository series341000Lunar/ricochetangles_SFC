#include <snes.h>

static const char hexDigits[] = "0123456789ABCDEF";

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

static int signedMouseDelta(u8 rawValue)
{
    int magnitude = rawValue & 0x7F;

    return (rawValue & 0x80) != 0 ? -magnitude : magnitude;
}

static void formatSignedDelta(int value, char *output)
{
    int magnitude = value;

    if (value < 0)
    {
        output[0] = '-';
        magnitude = -value;
    }
    else
    {
        output[0] = '+';
    }

    output[1] = '0' + (magnitude / 100);
    output[2] = '0' + ((magnitude / 10) % 10);
    output[3] = '0' + (magnitude % 10);
}

static void formatStatusLine(
    u16 pad,
    u8 connected,
    u8 rawX,
    u8 rawY,
    u8 buttons,
    u8 sensitivity,
    char *output)
{
    output[0] = 'P';
    formatHex16(pad, &output[1]);
    output[5] = ' ';
    output[6] = 'C';
    output[7] = connected != 0 ? '1' : '0';
    output[8] = ' ';
    output[9] = 'X';
    formatHex8(rawX, &output[10]);
    formatSignedDelta(signedMouseDelta(rawX), &output[12]);
    output[16] = ' ';
    output[17] = 'Y';
    formatHex8(rawY, &output[18]);
    formatSignedDelta(signedMouseDelta(rawY), &output[20]);
    output[24] = ' ';
    output[25] = 'L';
    output[26] = (buttons & mouse_L) != 0 ? '1' : '0';
    output[27] = 'R';
    output[28] = (buttons & mouse_R) != 0 ? '1' : '0';
    output[29] = 'S';
    output[30] = hexDigits[sensitivity & 0x0F];
    output[31] = '\0';
}

int main(void)
{
    u16 pad;
    u16 previousPad = 0;
    u8 connected;
    u8 rawX;
    u8 rawY;
    u8 buttons;
    u8 sensitivity;
    u8 previousConnected = 0;
    u8 previousRawX = 0;
    u8 previousRawY = 0;
    u8 previousButtons = 0;
    u8 previousSensitivity = 0;
    u8 firstStatus = 1;
    char statusLine[32];

    consoleInitDefaultText(0);

    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    consoleDrawText(6, 1, "RICOCHETANGLES SFC");
    consoleDrawText(3, 3, "S0-03 P1 PAD + P2 MOUSE");

    consoleDrawText(0, 5, "PRAW  C  XRAW/DX  YRAW/DY L R S");
    consoleDrawText(0, 6, "P0000 C0 X00+000 Y00+000 L0R0S0");

    consoleDrawText(2, 9, "P = P1 CURRENT RAW MASK");
    consoleDrawText(2, 11, "C = P2 MOUSE CONNECTED");
    consoleDrawText(2, 13, "X/Y = RAW HEX + SIGNED DEC");
    consoleDrawText(2, 15, "L/R = HELD   S = SENS");

    consoleDrawText(2, 18, "P1 PAD INDEX 0");
    consoleDrawText(2, 20, "P2 MOUSE INDEX 1");
    consoleDrawText(2, 22, "INIT MOUSE_SLOW");
    consoleDrawText(2, 24, "BUILD S0-03");

    initMouse(MOUSE_SLOW);
    WaitForVBlank();
    setScreenOn();

    while (1)
    {
        WaitForVBlank();

        pad = padsCurrent(0);
        connected = mouseConnect[1] != 0 ? 1 : 0;

        if (connected != 0)
        {
            rawX = mouse_x[1];
            rawY = mouse_y[1];
            buttons = mousePressed[1];
            sensitivity = mouseSensitivity[1];
        }
        else
        {
            rawX = 0;
            rawY = 0;
            buttons = 0;
            sensitivity = 0;
        }

        if (firstStatus != 0 ||
            pad != previousPad ||
            connected != previousConnected ||
            rawX != previousRawX ||
            rawY != previousRawY ||
            buttons != previousButtons ||
            sensitivity != previousSensitivity)
        {
            formatStatusLine(
                pad,
                connected,
                rawX,
                rawY,
                buttons,
                sensitivity,
                statusLine);
            consoleDrawText(0, 6, "%s", statusLine);

            previousPad = pad;
            previousConnected = connected;
            previousRawX = rawX;
            previousRawY = rawY;
            previousButtons = buttons;
            previousSensitivity = sensitivity;
            firstStatus = 0;
        }
    }

    return 0;
}
