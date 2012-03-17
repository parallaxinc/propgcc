#include <machine/cog.h>

unsigned int
clock(void)
{
    return _CNT;
}
