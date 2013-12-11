/**
 * @file include/propeller.h
 * @brief Provides Propeller specific functions.
 *
 * Copyright (c) 2011-2012 by Parallax, Inc.
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

/**
 * @brief HUBDATA tells compiler to put data into HUB RAM section.
 * This is mostly useful in XMM modes where data must be shared in hub.
 */
#define HUBDATA __attribute__((section(".hub")))

/**
 * @brief HUBTEXT tells compiler to put code into HUB RAM section.
 * @details This is a GCC super-power. Put code in HUB RAM even in XMM modes.
 * Sometimes code in XMM programs is time sensitive.
 * @details Use HUBTEXT before a function declaration to make
 * sure code is run from HUB instead of external memory.
 * Performance of code run from external memory is
 * unpredictable across platforms.
 */
#define HUBTEXT __attribute__((section(".hubtext")))

/// Parameter register is used for sharing HUB RAM address info with the COG.
#define PAR     _PAR
/// The system clock count
#define CNT     _CNT
/// Use to read the pins when corresponding DIRA bits are 0.
#define INA     _INA
/// Unused in P8X32A
#define INB     _INB
/// Use to set output pin states when corresponding DIRA bits are 1.
#define OUTA    _OUTA
/// Unused in P8X32A
#define OUTB    _OUTB
/// Use to set pins to input (0) or output (1).
#define DIRA    _DIRA
/// Unused in P8X32A
#define DIRB    _DIRB
/// Counter A control register.
#define CTRA    _CTRA
/// Counter B control register.
#define CTRB    _CTRB
/// Counter A frequency register.
#define FRQA    _FRQA
/// Counter B frequency register.
#define FRQB    _FRQB
/// Counter A phase accumulation register.
#define PHSA    _PHSA
/// Counter B phase accumulation register.
#define PHSB    _PHSB
/// Video Configuration register can be used for other special output.
#define VCFG    _VCFG
/// Video Scale register for setting pixel and frame clocks.
#define VSCL    _VSCL

/** @brief Returns the current clock frequency */
#define CLKFREQ _CLKFREQ

/** @brief Returns the current clock mode */
#define CLKMODE _CLKMODE

/**
 * @brief Set clock mode and frequency.
 * @details This macro is used to set the run-time clock mode and frequency.
 * The clock mode and frequency are normally configured by the loader
 * based on the user selected board type.
 *
 * Please see the Propeller Data Sheet for more clock information.
 *
 * @param mode The 8 bit clock mode
 * @param frequency The 32 bit clock frequency
 * @returns This macro will not return a value.
 */
#define clkset(mode, frequency) \
do { \
  _CLKFREQ = (frequency); \
  _CLKMODE = (mode); \
  __builtin_propeller_clkset(mode); \
} while(0)

/**
 * @brief Return the id of the current cog
 * @details Sometimes we need to know which COG is running the program.
 * cogid returns that value.
 * @returns ID number of current COG
 */
#define cogid()                  __builtin_propeller_cogid()

/**
 * @brief Start a cog with a parameter
 *
 * @details
 * The fields in parameters are:
 *
 * @li 31:18   = 14-bit Long address for PAR Register
 * @li 17:4    = 14-bit Long address of CODE to load
 * @li 3       = New bit
 * @li 2:0     = Cog ID. New bit 3 is 0.
 *
 * It is important to realize that a 14 bit address means that
 * long aligned addresses or pointers should be use.
 * That is if you pass a value to the PAR such as 3, the value
 * will be truncated to 0. A value 5 will be interpreted as 4.
 *
 * @param id The COG id to initialize
 * @param code Start address of PASM code to load.
 * @param param PAR address
 *
 * @returns COG ID provided by the builtin function or -1 on failure.
 */
#define coginit(id, code, param) __builtin_propeller_coginit( \
                                 (((uint32_t)(param) << 16) & 0xfffc0000) \
                                 |(((uint32_t)(code) <<  2) & 0x0003fff0) \
                                 |(((id)                  ) & 0x0000000f) )

/**
 * @brief Start a new Propeller PASM COG
 *
 * @details This is use to start a COG with code compiled as PASM, AS, or COG-C.
 * PASM can be any Spin/PASM code that is self-contained. That is, all data
 * for initialization and mailbox use are passed via the par parameter.
 * Changing PASM variables from SPIN code will not work with this method.
 *
 * @details GAS and COG-C programs have similar restrictions.
 * COG-C programs should not use any stack or variables in HUB memory that
 * are not accessed via PAR mailbox or pointers.
 *
 * @param code Address of PASM to load
 * @param param Value of par parameter usually an address
 * @returns COG ID provided by the builtin function or -1 on failure.
 */
#define cognew(code, param) coginit(0x8, (code), (param))

/**
 * @brief Stop a COG
 * @param a The COG ID
 */
#define cogstop(a) __builtin_propeller_cogstop((a))

/**
 * @brief Start a new propeller LMM function/thread in another COG.
 *
 * @details This function starts a new LMM VM kernel in a new COG with
 * func as the start function. The stack size must be big enough to hold
 * the struct _thread_state_t, the initial stack frame, and other stack
 * frames used by called functions.
 *
 * @details This function can be used instead of _start_cog_thread.
 *
 * @param func LMM start function
 * @param par Value of par parameter usually an address
 * @param stack Address of user defined stack space.
 * @param stacksize Size of user defined stack space.
 * @returns COG ID allocated by the function or -1 on failure.
 */
int cogstart(void (*func)(void *), void *par, void *stack, size_t stacksize);

/**
 * @brief Get a new lock
 * @returns new lockid
 */
#define locknew() __builtin_propeller_locknew()

/**
 * @brief Return lock to pool
 * @param lockid
 */
#define lockret(lockid) __builtin_propeller_lockret((lockid))

/**
 * @brief Set a lock
 * @param lockid
 * @returns true on success
 */
#define lockset(lockid) __builtin_propeller_lockset((lockid))

/**
 * @brief Clear lock
 * @param lockid
 */
#define lockclr(lockid) __builtin_propeller_lockclr((lockid))

/**
 * @brief Wait until system counter reaches a value
 * @param a Target value
 */
#define waitcnt(a) __builtin_propeller_waitcnt((a),0)

/**
 * @brief Wait until system counter reaches a value
 * @param a Target value
 * @param b Adjust value
 */
#define waitcnt2(a,b) __builtin_propeller_waitcnt((a),(b))

/**
 * @brief Wait until INA equal state & mask
 * @param state Target value
 * @param mask Ignore masked 0 bits in state
 */
#define waitpeq(state, mask) __builtin_propeller_waitpeq((state), (mask))

/**
 * @brief Wait until INA not equal state & mask
 * @param state Target value
 * @param mask Ignore masked 0 bits in state
 */
#define waitpne(state, mask) __builtin_propeller_waitpne((state), (mask))

/**
 * @brief Wait for video generator to accept pixel info
 *
 * @param colors A long containing four byte-sized color values, each describing the four possible colors of the pixel patterns in Pixels.
 * @param pixels The next 16-pixel by 2-bit (or 32-pixel by 1-bit) pixel pattern to display.
 */
#define waitvid(colors, pixels) __builtin_propeller_waitvid((colors), (pixels))

/*
 * Most of these are only used for __PROPELLER_XMM__
 */
#if defined(__PROPELLER_USE_XMM__)

/**
 * @brief Copy longs from external memory to hub memory
 *
 * @details Used for XMM-SINGLE or XMM-SPLIT to copy longs from
 * external memory to HUB memory primarily for the purpose
 * of starting a COG with copied code.
 * Can be used for other copy too.
 *
 * @details It is not necessary to use this for XMMC mode as
 * XMMC data will already be in HUB memory.
 *
 * @param dst Destination address in hub memory
 * @param src Source address in external memory
 * @param count Number of longs to copy
 *
 * these days memcpy knows how to copy from XMM to hub memory
 *
 */
static __inline__ void copy_from_xmm(uint32_t *dst, uint32_t *src, int count)
{
  memcpy(dst, src, count*4);
}

/**
 * @brief Read a long from external memory
 * @param addr Address in external memory to read
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
 * @brief Write a long to external memory
 * @param addr Address in external memory to write
 * @param value The value to write
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

/**
 * @brief Enable bus locking in the kernel's cache driver
 * @param lockId The lock that the cache driver should use
 */
void kernel_use_lock(uint32_t lockId);

#endif

#if defined(__PROPELLER_USE_XMM__)

/**
 * @brief Make the load symbols available for a driver
 * @param id The COG driver name
 */
#define use_cog_driver(id)     extern uint32_t binary_##id##_dat_start[], binary_##id##_dat_end[]

/**
 * @brief Get a hub memory buffer containing a driver image
 * @param id The COG driver name
 */
#define get_cog_driver(id)                                              \
            get_cog_driver_xmm(                                         \
                binary_##id##_dat_start,                                \
                binary_##id##_dat_end - binary_##id##_dat_start)
                
/**
 * @brief Load a COG driver
 * @param id The COG driver name
 * @param param Parameter to pass to the driver
 * @returns the id of the COG that was loaded
 */
#define load_cog_driver(id, param)                                      \
            load_cog_driver_xmm(                                        \
                binary_##id##_dat_start,                                \
                binary_##id##_dat_end - binary_##id##_dat_start,        \
                (uint32_t *)(param))
    
uint32_t *get_cog_driver_xmm(uint32_t *code, uint32_t codelen);
int load_cog_driver_xmm(uint32_t *code, uint32_t codelen, uint32_t *params);

#else
    
/**
 * @brief Make the load symbols available for a driver
 * @param id The COG driver name
 */
#define use_cog_driver(id)     extern uint32_t binary_##id##_dat_start[]

/**
 * @brief Get a hub memory buffer containing a driver image
 * @param id The COG driver name
 */
#define get_cog_driver(id)     (binary_##id##_dat_start)                              \
                
/**
 * @brief Load a COG driver
 * @param id The COG driver name
 * @param param Parameter to pass to the driver
 * @returns the id of the COG that was loaded
 */
#define load_cog_driver(id, param) cognew(binary_##id##_dat_start, (uint32_t *)(param))
    
#endif

#ifdef __cplusplus
}
#endif

#endif
// _PROPELLER_H_
