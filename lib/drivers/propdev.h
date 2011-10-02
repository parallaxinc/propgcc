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

#define coginit(id, code, param)    __builtin_propeller_coginit(                                  \
                                            (((uint32_t)(param) << 16) & 0xfffc0000)    \
                                        |   (((uint32_t)(code)  <<  2) & 0x0003fff0)    \
                                        |   (((id)                   ) & 0x0000000f))
                                    
#define cognew(code, param)         coginit(0x8, (code), (param))
                                    
#define cogstop(a)                  __builtin_propeller_cogstop((a))

#define waitcnt(a)                  __builtin_propeller_waitcnt((a), 0)


#ifdef __cplusplus
}
#endif

#endif
