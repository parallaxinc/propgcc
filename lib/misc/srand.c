#include <stdlib.h>

extern long _rseed;

void srand(unsigned int seed)
{
    _rseed = seed;
}

