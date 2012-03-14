#include "PinGroup.h"

void PinGroup::set(uint32_t value)
{
    OUTA = (OUTA & ~m_mask) | ((value << m_shift) & m_mask);
    DIRA |= m_mask;
}
