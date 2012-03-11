#include <propeller.h>
#include "boe.h"

int getPin(int pin)
{
    uint32_t mask = 1 << pin;
    DIRA &= ~mask;
    return (INA & mask) ? 1 : 0;
}
