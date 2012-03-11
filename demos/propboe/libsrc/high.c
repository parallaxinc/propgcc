#include <propeller.h>
#include "boe.h"

void high(int pin)
{
    uint32_t mask = 1 << pin;
    OUTA |= mask;
    DIRA |= mask;
}
