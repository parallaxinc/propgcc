#include <propeller.h>
#include "launch.h"

#define EXTERNAL_START  0x20000000

static void longcopy(uint32_t *dst, uint32_t *src, int len)
{
    while (--len >= 0)
        *dst++ = *src++;
}

int launchcog(uint32_t *code, uint32_t *end, void *param)
{
    int id;
    
    // handle code in hub memory
    if ((uint32_t)code < EXTERNAL_START)
        id = cognew((uint32_t)code, (uint32_t)param);
    
    // handle code in external memory
    else {
        uint32_t buffer[496]; // this assumes the stack is always in hub memory!!
        longcopy(buffer, code, end - code);
        id = cognew((uint32_t)buffer, (uint32_t)param);
    }
    
    return id;
}
