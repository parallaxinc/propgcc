/**
 * @file include/propeller2.h
 * @brief Provides Propeller 2 specific functions.
 *
 * @details Propeller 2 (P2) provides hardware registers that may be accessed differently from P8x32a (P1).
 * This header provides access to the P2 hardware resources.
 * Similar functions are provided for P1 users for portability.
 *
 * Common registers are DIRA and DIRB (only DIRA is used in P1). To access other pin hardware on P2
 * requires the use of DIRA, PINA, and other variations for more than 32 pins.
 *
 * The P2 Propeller-GCC compiler/assembler provides direct access to PINx and DIRx registers.
 * Pins of P2 are also accessible individually by ASM instructions getpin, setpc, and notp.
 * The ASM code is encapsulated by inline functions getpin, setpin, and togglepin.
 *
 * The CNT value in P2 is not a register as in P1. The CNT value can be read by the getcnt instruction.
 * The getcnt inline function provides access to the ASM for C users.
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
 * @param image Start address of PASM code to load.
 * @param par address
 *
 * @returns COG ID provided by the builtin function or -1 on failure.
 */
static __inline__ int coginit(int id, void *image, void *par)
{
    __asm__ volatile (
        "setcog %[_id]\n\t"
        "coginit %[_image], %[_par] wc\n\t"
        "if_c neg %[_value], #1"
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

/**
 * @brief CNT register accessor.
 *
 * @details P2 provides a shared CNT register, but it is not register memory mapped as in P1.
 * The CNT is only readable with the getcnt instruction.
 *
 * @returns the global CNT value.
 */
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

/**
 * @brief getpin accessor used to read the state of a pin.
 *
 * @details P2 provides pin registers and instructions.
 * This inline macro provides access to read a given pin.
 *
 * @param pin Pin to read in the range 0:127.
 * @returns State of the requested pin with range 0:1.
 */
static __inline__ int getpin(int pin)
{
    uint32_t value;
    __asm__ volatile (
        "getp %[_pin] wc\n\t"
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

/**
 * @brief setpin accessor used to write the state of a pin.
 *
 * @details P2 provides pin registers and instructions.
 * This inline macro provides access to write the value to a given pin.
 *
 * @param pin Pin to read in the range 0:127.
 * @param value The value to set to the pin 0:1
 * @returns Nothing.
 */
static __inline__ void setpin(int pin, int value)
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

/**
 * @brief togglepin accessor used to toggle the state of a pin.
 *
 * @details P2 provides pin registers and instructions.
 * This inline macro provides access to toggle the value of a given pin.
 * Toggle means to set the opposite of the existing state.
 *
 * @param pin Pin to read in the range 0:127.
 * @returns Nothing.
 */
static __inline__ void togglepin(int pin)
{
    __asm__ volatile (
        "notp %[_pin]"
    : /* no outputs */
    : /* inputs */
        [_pin] "r" (pin)
    : /* no clobbered registers */
    );
}

/**
 * PINx definitions are variables which map to the register defaults in the P2 COG.
 * They are used to read the actual pin states if set to input or output by DIRx.
 * They are used to write the pin states if enabled as outputs by DIRx.
 * PINx means PINA, PINB, PINC, or PIND.
 * The PINx registers may be remapped by instructions to point to each-other.
 */
/// PINA by default allows reading or writing P2 P[00:31] values.
#define PINA    _PINA
/// PINB by default allows reading or writing P2 P[32:63] values.
#define PINB    _PINB
/// PINC by default allows reading or writing P2 P[64:91] values.
#define PINC    _PINC
/// PIND by default allows reading or writing P2 PIND shadow register.
#define PIND    _PIND

/**
 * DIRx definitions are variables which map to the register defaults in the P2 COG.
 * They are used to to set PINx pins as inputs only or outputs which can be read.
 * DIRx means DIRA, DIRB, DIRC, or DIRD.
 * The DIRx registers may be remapped by instructions to point to each-other.
 */
/// DIRA by default allows setting P2 P[00:31] to input/output or input only.
#define DIRA    _DIRA
/// DIRB by default allows setting P2 P[32:63] to input/output or input only.
#define DIRB    _DIRB
/// DIRC by default allows setting P2 P[64:91] to input/output or input only.
#define DIRC    _DIRC
/// DIRD by default allows setting P2 PIND shadow register to input/output or input only.
#define DIRD    _DIRD

#ifdef __cplusplus
}
#endif

#endif
// _PROPELLER2_H_
