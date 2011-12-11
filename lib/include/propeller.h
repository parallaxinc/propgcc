/**
 * @file propeller.h
 * This is part of the propeller library. ... this is a work in progress.
 *
 * Copyright (c) 2011 by Parallax, Inc.
 * MIT Licensed
 */

#ifndef _PROPELLER_H_
#define _PROPELLER_H_

#ifdef __cplusplus
extern "C" 
{
#endif

#include "cog.h"
#include <stdint.h>
#include <string.h>

/*
 * Use these defines to tell compiler linker to put
 * data and code (.text) into HUB RAM.
 * This is mostly useful in XMM and XMMC modes
 * where data must be shared in hub or
 * where code must execute as fast as possible
 */
#define HUBDATA __attribute__((section(".hub")))

/* This is a GCC super-power. Put code in HUB RAM.
 * Sometimes code in XMM programs is time sensitive.
 * Use HUBTEXT before a function declaration to make
 * sure code is run from HUB instead of external memory.
 * Performance of code run from external memory is
 * unpredictable across platforms.
 */
#define HUBTEXT __attribute__((section(".hubtext")))

/* some defines for special purpose COG registers */
#define PAR     _PAR
#define CNT     _CNT
#define INA     _INA
#define INB     _INB
#define OUTA    _OUTA
#define OUTB    _OUTB
#define DIRA    _DIRA
#define DIRB    _DIRB
#define CTRA    _CTRA
#define CTRB    _CTRB
#define FRQA    _FRQA
#define FRQB    _FRQB
#define PHSA    _PHSA
#define PHSB    _PHSB
#define VCFG    _VCFG
#define VSCL    _VSCL

/* return clock frequency */
#define CLKFREQ _CLKFREQ

/* return clock mode */
#define CLKMODE _CLKMODE

/**
 * Set clock mode and frequency.
 * This macro will not return a value.
 */
#define clkset(mode, frequency) \
do { \
  _CLKFREQ = (frequency); \
  _CLKMODE = (mode); \
  __builtin_propeller_clkset(mode); \
} while(0)

/**
 * return the id of the current cog
 */
#define cogid()                  __builtin_propeller_cogid()

/**
 * start a cog with a parameter
 * the fields in parameter are:
 * 31:18   = 14-bit Long address for PAR Register
 * 17:4    = 14-bit Long address of code to load
 * 3       = New bit
 * 2:0     = Cog ID if New bit is 0
 * @param a - parameter value
 */
#define coginit(id, code, param) __builtin_propeller_coginit( \
                                 (((uint32_t)(param) << 16) & 0xfffc0000) \
                                 |(((uint32_t)(code) <<  2) & 0x0003fff0) \
                                 |(((id)                  ) & 0x0000000f) )

/**
 * start a new propeller cog 
 * @param code - address of PASM to load
 * @param par  - value of par parameter usually an address
 * @returns COG ID provided by the builtin function.
 */
#define cognew(code, param) coginit(0x8, (code), (param))

/**
 * stop a cog
 * @param a - cog value
 */
#define cogstop(a) __builtin_propeller_cogstop((a))

/**
 * get a new lock
 * @returns new lockid
 */
#define locknew() __builtin_propeller_locknew()

/**
 * return lock to pool
 * @param lockid
 */
#define lockret(lockid) __builtin_propeller_lockret((lockid))

/**
 * set a lock
 * @param lockid
 * @returns true on success
 */
#define lockset(lockid) __builtin_propeller_lockset((lockid))

/**
 * clear lock
 * @param lockid
 */
#define lockclr(lockid) __builtin_propeller_lockclr((lockid))

/*
 * wait until system counter reaches a value
 * @param a - target value
 */
#define waitcnt(a) __builtin_propeller_waitcnt((a),_CNT)

/*
 * wait until system counter reaches a value
 * @param a - target value
 * @param b - adjust value
 */
#define waitcnt2(a,b) __builtin_propeller_waitcnt((a),(b))

/**
 * wait until INA equal state & mask
 * @param state - target value
 * @param mask - ignore masked 0 bits in state
 */
#define waitpeq(state, mask) __builtin_propeller_waitpeq((state), (mask))

/**
 * wait until INA not equal state & mask
 * @param state - target value
 * @param mask - ignore masked 0 bits in state
 */
#define waitpne(state, mask) __builtin_propeller_waitpne((state), (mask))

/**
 * wait for video generator to accept pixel info
 *
 * @param colors - a long containing four byte-sized color values, each describing the four possible colors of the pixel patterns in Pixels.
 * @param pixels - the next 16-pixel by 2-bit (or 32-pixel by 1-bit) pixel pattern to display.
 */
#define waitvid(colors, pixels) __builtin_propeller_waitvid((colors), (pixels))

#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)

/**
 * copy longs from external memory to hub memory
 * @param dst - destination address in hub memory
 * @param src - source address in external memory
 * @param count - number of longs to copy
 *
 * these days memcpy knows how to copy from XMM to hub memory
 *
 */
static __inline__ void copy_from_xmm(uint32_t *dst, uint32_t *src, int count)
{
  memcpy(dst, src, count*4);
}

/**
 * read a long from external memory
 * @param addr - address in external memory to read
 * @returns the long at that address
 */
static __inline__ uint32_t rdlong_xmm(uint32_t *addr)
{
    uint32_t value;
    __asm__ volatile (
        "xmmio rdlong,%[_value],%[_addr]"
    : /* outputs */
        [_value] "=r" (value)
    : /* inputs */
        [_addr] "r" (addr)
    : /* no clobbered registers */
    );
    return value;
}

/**
 * write a long to external memory
 * @param addr - address in external memory to write
 * @param value - the value to write
 */
static __inline__ void wrlong_xmm(uint32_t *addr, uint32_t value)
{
    __asm__ volatile (
        "xmmio wrlong,%[_value],%[_addr]"
    : /* no outputs */
    : /* inputs */
        [_addr] "r" (addr),
        [_value] "r" (value)
    : /* no clobbered registers */
    );
}

#endif

#ifdef __cplusplus
}
#endif

#endif
// _PROPELLER_H_
