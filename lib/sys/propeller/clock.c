#include "cog.h"

#ifdef __PROPELLER2__
__attribute__((native)) unsigned int _GETCNT(void);
#endif

unsigned int
clock(void)
{
#ifdef __PROPELLER2__
    return _GETCNT();
#else
    return _CNT;
#endif
}
