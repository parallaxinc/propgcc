#include <propeller.h>
#include "boe.h"

void setPin(int pin, int value)
{
    uint32_t mask = 1 << pin;
    uint32_t data = (value & 1) << pin;
    OUTA = (OUTA & ~mask) | data;
    DIRA |= mask;
}
