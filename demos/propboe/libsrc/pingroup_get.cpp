#include "PinGroup.h"

uint32_t PinGroup::get()
{
    DIRA &= ~m_mask;
    return (INA & m_mask) >> m_shift;
}
