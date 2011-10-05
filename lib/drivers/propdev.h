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

#define coginit(id, code, param)    __builtin_propeller_coginit(                        \
                                            (((uint32_t)(param) << 16) & 0xfffc0000)    \
                                        |   (((uint32_t)(code)  <<  2) & 0x0003fff0)    \
                                        |   (((id)                   ) & 0x0000000f))
                                    
#define cognew(code, param)         coginit(0x8, (code), (param))
                                    
#define cogstop(a)                  __builtin_propeller_cogstop((a))

#define waitcnt(a)                  __builtin_propeller_waitcnt((a), 0)

#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)

#define use_cog_driver(id)      extern uint32_t _load_start_##id[], _load_stop_##id[]

#define load_cog_driver(code, id, param)            \
            load_cog_driver_xmm(                    \
                code,                               \
                _load_stop_##id - _load_start_##id, \
                (uint32_t *)(param))
    
int load_cog_driver_xmm(uint32_t *code, uint32_t codelen, uint32_t *params);

static __inline__ void copy_from_xmm(uint32_t *dst, uint32_t *src, int count)
{
    uint32_t _dx, _sx, _cx;
    __asm__ volatile (
        "mov __TMP0, %[_src]\n\t"
        "call #__LMM_RDLONG\n\t"
        "wrlong __TMP1, %[_dst]\n\t"
        "add %[_src], #4\n\t"
        "add %[_dst], #4\n\t"
        "sub %[_count], #1 wz\n\t"
        "if_nz sub pc, #7*4"
    : /* outputs */
      [_dst] "=&r" (_dx),
      [_src] "=&r" (_sx),
      [_count] "=&r" (_cx)
    : /* inputs */
      "[_dst]" (dst),
      "[_src]" (src),
      "[_count]" (count)
    : /* clobbered registers */
      "cc" );
}

static __inline__ uint32_t rdlong_xmm(uint32_t *addr)
{
    uint32_t value;
    __asm__ volatile (
        "mov __TMP0, %[_addr]\n\t"
        "call #__LMM_RDLONG\n\t"
        "mov %[_value], __TMP1"
    : /* outputs */
        [_value] "=r" (value)
    : /* inputs */
        [_addr] "r" (addr)
    : /* no clobbered registers */
    );
    return value;
}

static __inline__ void wrlong_xmm(uint32_t *addr, uint32_t value)
{
    __asm__ volatile (
        "mov __TMP0, %[_addr]\n\t"
        "mov __TMP1, %[_value]\n\t"
        "call #__LMM_WRLONG"
    : /* no outputs */
    : /* inputs */
        [_addr] "r" (addr),
        [_value] "r" (value)
    : /* no clobbered registers */
    );
}

#else
    
#define use_cog_driver(id)

#define load_cog_driver(code, id, param) cognew(code, (uint32_t *)(param))
    
#endif

#ifdef __cplusplus
}
#endif

#endif
