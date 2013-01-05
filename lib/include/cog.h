/**
 * @file cog.h
 * @brief Inludes common API definitions for COG programming.
 *
 * @details  Each COG includes 16 32 bit special purpose registers.
 * The purposes of the registers are to provide control over user I/O
 * input and output. Some registers like INB, OUTB, DIRB are not used
 * with P8X32A.
 *
 * The state of each physical input pin is available to any COG via INA.
 * Output pin values are the bitwise "wire OR" of all the COGs at the
 * physical output pins when the DIRA bits are set high (1).
 *
 * OUTA bits control the state of the physical output pins.
 * If one COG sets a pin to output high (1), and another COG sets the same pin
 * to output low (0), the high (1) will be set.
 *
 * The per COG special purpose register summary:
 *
 * @verbatim

 Register Description

 PAR      Parameter register is used for sharing HUB RAM address info with the COG.
 CNT      The system clock count
 INA      Use to read the pins when corresponding DIRA bits are 0.
 INB      Unused in P8X32A
 OUTA     Use to set pin states when corresponding DIRA bits are 1.
 OUTB     Unused in P8X32A
 DIRA     Use to set pins to input (0) or output (1).
 DIRB     Unused in P8X32A
 CTRA     Counter A control register.
 CTRB     Counter B control register.
 FRQA     Counter A frequency register.
 FRQB     Counter B frequency register.
 PHSA     Counter A phase accumulation register.
 PHSB     Counter B phase accumulation register.
 VCFG     Video Configuration register can be used for other special output.
 VSCL     Video Scale register for setting pixel and frame clocks.
 @endverbatim
 */
#ifndef PROPELLER_COG_H_
#define PROPELLER_COG_H_

/** @brief Can be used in per-variable declarations to tell compiler that a variable should go in COG memory. */

/* for variables that should go in cog memory */
#define _COGMEM __attribute__((cogmem))

/** @brief Can be used in per-function declarations to tell compiler that function will use cog "call/ret" calling (nonrecursive). */

/* for functions that use cog "call/ret" calling (nonrecursive) */
#define _NATIVE __attribute__((native))

/** @brief Can be used in per-function declarations to tell compiler that function will not have an epilogue or prologue: these should never return. */

/* for functions with no epilogue or prologue: these should never return */
#define _NAKED  __attribute__((naked))

/** @brief Can be used in per-function declarations to tell compiler that function should be compiled to load into fcache; useful for guaranteeing timing of small functions */
#define _FCACHE __attribute__((fcache))

/* useful variables */

#ifdef __PROPELLER2__
extern _COGMEM volatile unsigned int _PINA __asm__("PINA");
extern _COGMEM volatile unsigned int _PINB __asm__("PINB");
extern _COGMEM volatile unsigned int _PINC __asm__("PINC");
extern _COGMEM volatile unsigned int _PIND __asm__("PIND");
extern _COGMEM volatile unsigned int _DIRA __asm__("DIRA");
extern _COGMEM volatile unsigned int _DIRB __asm__("DIRB");
extern _COGMEM volatile unsigned int _DIRC __asm__("DIRC");
extern _COGMEM volatile unsigned int _DIRD __asm__("DIRD");
#else
extern _COGMEM volatile unsigned int _PAR __asm__("PAR");
extern _COGMEM volatile unsigned int _CNT __asm__("CNT");
extern _COGMEM volatile unsigned int _INA __asm__("INA");
extern _COGMEM volatile unsigned int _INB __asm__("INB");
extern _COGMEM volatile unsigned int _OUTA __asm__("OUTA");
extern _COGMEM volatile unsigned int _OUTB __asm__("OUTB");
extern _COGMEM volatile unsigned int _DIRA __asm__("DIRA");
extern _COGMEM volatile unsigned int _DIRB __asm__("DIRB");
extern _COGMEM volatile unsigned int _CTRA __asm__("CTRA");
extern _COGMEM volatile unsigned int _CTRB __asm__("CTRB");
extern _COGMEM volatile unsigned int _FRQA __asm__("FRQA");
extern _COGMEM volatile unsigned int _FRQB __asm__("FRQB");
extern _COGMEM volatile unsigned int _PHSA __asm__("PHSA");
extern _COGMEM volatile unsigned int _PHSB __asm__("PHSB");
extern _COGMEM volatile unsigned int _VCFG __asm__("VCFG");
extern _COGMEM volatile unsigned int _VSCL __asm__("VSCL");
#endif

/* boot parameters */

/** 32 bit system startup clock frequency variable */
extern unsigned int _clkfreq; /* in the spin boot code */

/** @brief This is an alias for the 32 bit clock frequency which is kept in address 0. */
#define _CLKFREQ _clkfreq

/** System startup clock mode */
extern unsigned char _clkmode; /* in the spin boot code */

/** @brief This is an alias for the 8 bit clock mode which is kept in address 4.
 *  @details This is not automatically updated by the clkset macro.
 */
#define _CLKMODE _clkmode


#endif
