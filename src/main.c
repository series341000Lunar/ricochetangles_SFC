#include <snes.h>

static const char hexDigits[] = "0123456789ABCDEF";

static void formatHex16(u16 value, char *output)
{
    output[0] = hexDigits[(value >> 12) & 0x0F];
    output[1] = hexDigits[(value >> 8) & 0x0F];
    output[2] = hexDigits[(value >> 4) & 0x0F];
    output[3] = hexDigits[value & 0x0F];
    output[4] = '\0';
}

static char bitCharacter(u16 pad, u16 key)
{
    return (pad & key) != 0 ? '1' : '0';
}

static void formatPadLine(u16 pad, char *output)
{
    formatHex16(pad, output);
    output[4] = ' ';
    output[5] = ' ';
    output[6] = ' ';
    output[7] = bitCharacter(pad, KEY_UP);
    output[8] = bitCharacter(pad, KEY_DOWN);
    output[9] = bitCharacter(pad, KEY_LEFT);
    output[10] = bitCharacter(pad, KEY_RIGHT);
    output[11] = ' ';
    output[12] = bitCharacter(pad, KEY_A);
    output[13] = bitCharacter(pad, KEY_B);
    output[14] = bitCharacter(pad, KEY_X);
    output[15] = bitCharacter(pad, KEY_Y);
    output[16] = ' ';
    output[17] = bitCharacter(pad, KEY_L);
    output[18] = bitCharacter(pad, KEY_R);
    output[19] = ' ';
    output[20] = bitCharacter(pad, KEY_START);
    output[21] = bitCharacter(pad, KEY_SELECT);
    output[22] = '\0';
}

int main(void)
{
    u16 pad;
    u16 previousPad = 0;
    char padLine[23];

    consoleInitDefaultText(0);

    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    consoleDrawText(6, 1, "RICOCHETANGLES SFC");
    consoleDrawText(5, 3, "S0-02 PAD RAW INPUT");

    consoleDrawText(2, 5, "RAW    UDLR ABXY LR TS");
    consoleDrawText(2, 6, "0000   0000 0000 00 00");

    consoleDrawText(2, 8, "D-PAD   UP DOWN LEFT RIGHT");
    consoleDrawText(2, 10, "BUTTON  A B X Y");
    consoleDrawText(2, 12, "SHOULDER L R");
    consoleDrawText(2, 14, "SYSTEM  START SELECT");

    consoleDrawText(2, 18, "P1 INDEX 0 - HELD/CURRENT");
    consoleDrawText(2, 20, "BUILD S0-02");

    setScreenOn();

    while (1)
    {
        WaitForVBlank();

        pad = padsCurrent(0);
        if (pad != previousPad)
        {
            formatPadLine(pad, padLine);
            consoleDrawText(2, 6, "%s", padLine);
            previousPad = pad;
        }
    }

    return 0;
}
