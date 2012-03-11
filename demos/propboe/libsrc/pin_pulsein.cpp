#include "Pin.h"

HUBTEXT int Pin::pulseIn(int state)
{
    uint32_t data = state ? m_mask : 0;
    uint32_t ticks;
    DIRA &= ~m_mask;
    waitpeq(data, m_mask);
    ticks = CNT;
    waitpne(data, m_mask);
    ticks = CNT - ticks;
    return ticks / (CLKFREQ / 1000000);
}
