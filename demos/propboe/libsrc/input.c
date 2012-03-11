#include <propeller.h>
#include "boe.h"

void input(int pin)
{
    uint32_t mask = 1 << pin;
    DIRA &= ~mask;
}
