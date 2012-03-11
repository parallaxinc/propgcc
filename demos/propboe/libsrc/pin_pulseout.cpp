#include "Pin.h"

HUBTEXT void Pin::pulseOut(int duration)
{
    uint32_t ticks = duration * (CLKFREQ / 1000000);
    OUTA ^= m_mask;
    DIRA |= m_mask;
    waitcnt(CNT + ticks);
    OUTA ^= m_mask;
}
