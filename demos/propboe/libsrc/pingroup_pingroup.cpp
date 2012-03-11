#include "PinGroup.h"

PinGroup::PinGroup(int high, int low)
{
    if (high < low) {
        int swap = high;
        high = low;
        low = swap;
    }
    m_mask = ((1 << (high - low + 1)) - 1) << low;
    m_shift = low;
}
