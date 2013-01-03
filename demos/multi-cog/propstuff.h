#ifndef __PROPSTUFF_H__
#define __PROPSTUFF_H__

#ifdef __PROPELLER2__

static __inline__ uint32_t p2_cogid(void)
{
    uint32_t value;
    __asm__ volatile (
        "cogid %[_value]"
    : /* outputs */
        [_value] "=r" (value)
    : /* no inputs */
    : /* no clobbered registers */
    );
    return value;
}

static __inline__ void p2_setcog(uint32_t n)
{
    __asm__ volatile (
        "setcog %[_n]"
    : /* no outputs */
    : /* inputs */
        [_n] "r" (n)
    : /* no clobbered registers */
    );
}

static __inline__ uint32_t p2_coginit(uint32_t n, void *image, void *par)
{
    p2_setcog(n);
    __asm__ volatile (
        "coginit %[_image], %[_par] wc\n\t"
        "if_c mov %[_value], #0\n\t"
        "if_c not %[_value]"
    : /* outputs */
        [_value] "=r" (image)
    : /* inputs */
        [_image] "0" (image),
        [_par] "r" (par)
    : /* no clobbered registers */
    );
    return (uint32_t)image;
}

static __inline__ uint32_t p2_cognew(void *image, void *par)
{
    return p2_coginit(8, image, par);
}

static __inline__ void p2_cogstop(uint32_t n)
{
    __asm__ volatile (
        "cogstop %[_n]"
    : /* no outputs */
    : /* inputs */
        [_n] "r" (n)
    : /* no clobbered registers */
    );
}

static __inline__ uint32_t p2_getcnt(void)
{
    uint32_t value;
    __asm__ volatile (
        "getcnt %[_value]"
    : /* outputs */
        [_value] "=r" (value)
    : /* no inputs */
    : /* no clobbered registers */
    );
    return value;
}

static __inline__ uint32_t p2_getpin(uint32_t pin)
{
    uint32_t value;
    __asm__ volatile (
        "getpin %[_pin] wc\n\t"
        "mov %[_value], #0\n\t"
        "rcl %[_value], #1"
    : /* outputs */
        [_value] "=r" (value)
    : /* inputs */
        [_pin] "r" (pin)
    : /* no clobbered registers */
    );
    return value;
}

static __inline__ void p2_setpin(uint32_t pin, uint32_t value)
{
    __asm__ volatile (
        "rcr %[_value], #1 wc\n\t"
        "setpc %[_pin]"
    : /* outputs */
        "=r" (value)
    : /* inputs */
        [_pin]"r" (pin),
        [_value] "0" (value)
    : /* no clobbered registers */
    );
}

static __inline__ void p2_togglepin(uint32_t pin)
{
    __asm__ volatile (
        "notp %[_pin]"
    : /* no outputs */
    : /* inputs */
        [_pin] "r" (pin)
    : /* no clobbered registers */
    );
}

static __inline__ uint32_t p2_waitcnt2(uint32_t target, uint32_t delta)
{
    __asm__ volatile (
        "waitcnt %[_target], %[_delta]"
    : /* outputs */
        [_value] "=r" (target)
    : /* inputs */
        [_target] "0" (target),
        [_delta] "r" (delta)
    : /* no clobbered registers */
    );
    return target;
}

#define CLKFREQ_P                   ((uint32_t *)0x0e80)

#define prop_cogid()                p2_cogid()
#define prop_coginit(n, image, par) p2_coginit(n, image, par)
#define prop_cognew(image, par)     p2_cognew(image, par)
#define prop_cogstop()              p2_cogstop()
#define prop_getcnt()               p2_getcnt()
#define prop_getpin(n)              p2_getpin(n)
#define prop_setpin(n, v)           p2_setpin(n, v)
#define prop_togglepin(n)           p2_togglepin(n)
#define prop_waitcnt(t)             p2_waitcnt2(t, 0)
#define prop_waitcnt2(t, d)         p2_waitcnt2(t, d)

#else

#include <propeller.h>

static __inline__ uint32_t p1_getpin(uint32_t pin)
{
    uint32_t mask = 1 << pin;
    _OUTA &= ~mask;
    return _INA & mask ? 1 : 0;
}

static __inline__ void p1_setpin(uint32_t pin, uint32_t value)
{
    uint32_t mask = 1 << pin;
    if (value)
        _OUTA |= mask;
    else
        _OUTA &= ~mask;
    _DIRA |= mask;
}

static __inline__ void p1_togglepin(uint32_t pin)
{
    uint32_t mask = 1 << pin;
    _OUTA ^= mask;
    _DIRA |= mask;
}

#define CLKFREQ_P                   ((uint32_t *)0)

#define prop_cogid()                cogid()
#define prop_coginit(n, image, par) coginit(n, image, par)
#define prop_cognew(image, par)     cognew(image, par)
#define prop_cogstop()              cogstop()
#define prop_getcnt()               _CNT
#define prop_getpin(n)              p1_getpin(n)
#define prop_setpin(n, v)           p1_setpin(n, v)
#define prop_togglepin(n)           p1_togglepin(n)
#define prop_waitcnt(t)             waitcnt(t)
#define prop_waitcnt2(t, d)         waitcnt2(t, d)

#endif

#endif
