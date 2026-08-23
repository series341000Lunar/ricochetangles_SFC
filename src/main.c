#include <snes.h>

int main(void)
{
    consoleInitDefaultText(0);

    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    consoleDrawText(6, 10, "RICOCHETANGLES SFC");
    consoleDrawText(11, 13, "S0 HELLO");
    consoleDrawText(8, 17, "PVSNESLIB 4.6.0");
    consoleDrawText(10, 19, "BUILD S0-01");

    setScreenOn();

    while (1)
    {
        WaitForVBlank();
    }

    return 0;
}

