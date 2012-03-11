#include <propeller.h>
#include "boe.h"

void output(int pin)
{
    uint32_t mask = 1 << pin;
    DIRA |= mask;
}
