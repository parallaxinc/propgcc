#include <propeller.h>
#include "boe.h"

HUBTEXT void pulseOut(int pin, int duration)
{
    uint32_t mask = 1 << pin;
    uint32_t ticks = duration * (CLKFREQ / 1000000);
    OUTA ^= mask;
    DIRA |= mask;
    waitcnt(CNT + ticks);
    OUTA ^= mask;
}
