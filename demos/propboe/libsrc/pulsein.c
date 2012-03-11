#include <propeller.h>
#include "boe.h"

HUBTEXT int pulseIn(int pin, int state)
{
    uint32_t mask = 1 << pin;
    uint32_t data = state << pin;
    uint32_t ticks;
    DIRA &= ~mask;
    waitpeq(data, mask);
    ticks = CNT;
    waitpne(data, mask);
    ticks = CNT - ticks;
    return ticks / (CLKFREQ / 1000000);
}
