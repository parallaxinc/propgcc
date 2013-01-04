/**
 * @file include/propeller1.h
 * @brief Provides Propeller 1 specific functions.
 *
 * Copyright (c) 2011-2013 by Parallax, Inc.
 * MIT Licensed
 */

#ifndef _PROPELLER1_H_
#define _PROPELLER1_H_

#ifdef __cplusplus
extern "C" 
{
#endif

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
#define coginit(id, code, param)    __builtin_propeller_coginit( \
                                     (((uint32_t)(param) << 16) & 0xfffc0000) \
                                    |(((uint32_t)(code) <<  2) & 0x0003fff0) \
                                    |(((id)                  ) & 0x0000000f))

/**
 * @brief CNT register accessor.
 *
 * @details P1 provides a COG accessible CNT register.
 *
 * This macro is a convenience for portability between P1/P2 code.
 *
 * @returns the global CNT value.
 */
#define getcnt()                    _CNT

/**
 * @brief getpin accessor used to read the state of a pin.
 *
 * @details P1 provides pin access via registers only.
 * This inline macro provides access to read a given pin.
 *
 * This macro is a convenience for portability between P1/P2 code.
 *
 * @param pin Pin to read in the range 0:31.
 * @returns State of the requested pin with range 0:1.
 */
static __inline__ int getpin(int pin)
{
    uint32_t mask = 1 << pin;
    _OUTA &= ~mask;
    return _INA & mask ? 1 : 0;
}

/**
 * @brief setpin accessor used to write the state of a pin.
 *
 * @details P1 provides pin access via registers only.
 * This inline macro provides access to write the value to a given pin.
 *
 * This macro is a convenience for portability between P1/P2 code.
 *
 * @param pin Pin to read in the range 0:31.
 * @param value The value to set to the pin 0:1
 * @returns Nothing.
 */
static __inline__ void setpin(int pin, int value)
{
    uint32_t mask = 1 << pin;
    if (value)
        _OUTA |= mask;
    else
        _OUTA &= ~mask;
    _DIRA |= mask;
}

/**
 * @brief togglepin accessor used to toggle the state of a pin.
 *
 * @details P1 provides pin access via registers only.
 * This inline macro provides access to toggle the value of a given pin.
 * Toggle means to set the opposite of the existing state.
 *
 * This macro is a convenience for portability between P1/P2 code.
 *
 * @param pin Pin to read in the range 0:31.
 * @returns Nothing.
 */
static __inline__ void togglepin(int pin)
{
    uint32_t mask = 1 << pin;
    _OUTA ^= mask;
    _DIRA |= mask;
}

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

#ifdef __cplusplus
}
#endif

#endif
// _PROPELLER1_H_
