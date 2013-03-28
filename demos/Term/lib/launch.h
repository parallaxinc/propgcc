#ifndef __LAUNCH_H__
#define __LAUNCH_H__

#include <stdint.h>

#ifndef __PROPELLER_USE_XMM__

#define USEDRIVER(n)                                    \
    extern uint32_t binary_ ## n ## _dat_start[]; 

#define LAUNCHCOG(n, p)                                 \
    cognew((uint32_t)binary_ ## n ## _dat_start, (uint32_t)p)

#else // xmm and xmmc

#define USEDRIVER(n)                                    \
    extern uint32_t binary_ ## n ## _dat_start[];       \
    extern uint32_t binary_ ## n ## _dat_end[];
    
#define LAUNCHCOG(n, p)                                 \
    launchcog(binary_ ## n ## _dat_start, binary_ ## n ## _dat_end, p)
    
int launchcog(uint32_t *code, uint32_t *end, void *param);

#endif // __PROPELLER_LMM__

#endif // __LAUNCH_H__
