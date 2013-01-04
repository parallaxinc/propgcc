/**
 * @file include/propeller2.h
 * @brief Provides Propeller 2 specific functions.
 *
 * Copyright (c) 2011-2013 by Parallax, Inc.
 * MIT Licensed
 */

#ifndef _PROPELLER2_H_
#define _PROPELLER2_H_

#ifdef __cplusplus
extern "C" 
{
#endif

/**
 * @brief Start a cog with a parameter
 *
 * @details
 * @param id The COG id to initialize
 * @param code Start address of PASM code to load.
 * @param param address
 *
 * @returns COG ID provided by the builtin function or -1 on failure.
 */
static __inline__ uint32_t coginit(uint32_t id, void *image, void *par)
{
    __asm__ volatile (
        "setcog %[_id]\n\t"
        "coginit %[_image], %[_par] wc\n\t"
        "if_c mov %[_value], #0\n\t"
        "if_c not %[_value]"
    : /* outputs */
        [_value] "=r" (image)
    : /* inputs */
        [_id] "r" (id),
        [_image] "0" (image),
        [_par] "r" (par)
    : /* no clobbered registers */
    );
    return (uint32_t)image;
}

static __inline__ uint32_t getcnt(void)
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

static __inline__ uint32_t getpin(uint32_t pin)
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

static __inline__ void setpin(uint32_t pin, uint32_t value)
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

static __inline__ void togglepin(uint32_t pin)
{
    __asm__ volatile (
        "notp %[_pin]"
    : /* no outputs */
    : /* inputs */
        [_pin] "r" (pin)
    : /* no clobbered registers */
    );
}

/// Use to read the pins.
#define PINA    _PINA
#define PINB    _PINB
#define PINC    _PINC
#define PIND    _PIND
/// Use to enable output on pins.
#define DIRA    _DIRA
#define DIRB    _DIRB
#define DIRC    _DIRC
#define DIRD    _DIRD

#ifdef __cplusplus
}
#endif

#endif
// _PROPELLER2_H_
