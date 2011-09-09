/*
 * some common definitions for cog code
 */
#ifndef PROPELLER_COG_H_
#define PROPELLER_COG_H_

/* for variables that should go in cog memory */
#define _COGMEM __attribute__((cogmem))

/* for functions that use cog "call/ret" calling (nonrecursive) */
#define _NATIVE __attribute__((native))

/* for functions with no epilogue or prologue: these should never return */
#define _NAKED  __attribute__((naked))

/* useful variables */
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

/* boot parameters */
extern unsigned int _clkfreq; /* in the spin boot code */
#define _CLKFREQ _clkfreq


#endif
