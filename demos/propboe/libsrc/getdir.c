#include <propeller.h>
#include "boe.h"

int getDir(int pin)
{
    uint32_t mask = 1 << pin;
    return (DIRA & mask) ? 1 : 0;
}
