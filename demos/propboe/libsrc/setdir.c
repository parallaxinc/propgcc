#include <propeller.h>
#include "boe.h"

void setDir(int pin, int dir)
{
    uint32_t mask = 1 << pin;
    uint32_t data = (dir & 1) << pin;
    DIRA = (DIRA & ~mask) | data;
}
