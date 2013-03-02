#ifndef __PROPDEV_H__
#define __PROPDEV_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdint.h>

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#include "propeller.h"
                                    
#if defined(__PROPELLER_USE_XMM__)

#define use_cog_driver(id)      extern uint32_t _load_start_##id[], _load_stop_##id[]

#define load_cog_driver(code, id, param)            \
            load_cog_driver_xmm(                    \
                code,                               \
                _load_stop_##id - _load_start_##id, \
                (uint32_t *)(param))
    
int load_cog_driver_xmm(uint32_t *code, uint32_t codelen, uint32_t *params);

#else
    
#define use_cog_driver(id)

#define load_cog_driver(code, id, param) cognew(code, (uint32_t *)(param))
    
#endif

#ifdef __cplusplus
}
#endif

#endif
