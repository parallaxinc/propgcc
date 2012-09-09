#ifdef __PROPELLER_USE_XMM__

#include <propeller.h>
#include "launch.h"

#define EXTERNAL_START  0x20000000

int launchcog(uint32_t *code, uint32_t *end, void *param)
{
    int id;
    
    // handle code in hub memory
    if ((uint32_t)code < EXTERNAL_START)
        id = cognew((uint32_t)code, (uint32_t)param);
    
    // handle code in external memory
    else {
        uint32_t buffer[496]; // this assumes the stack is always in hub memory!!
        copy_from_xmm(buffer, code, end - code);
        id = cognew((uint32_t)buffer, (uint32_t)param);
    }
    
    return id;
}

#endif
