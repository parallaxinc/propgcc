/*
 * some common definitions for cog code
 */
#ifndef PROPELLER_COG_H_
#define PROPELLER_COG_H_

#define _COGMEM __attribute__((cogmem))
#define _NATIVE __attribute__((native))

extern _COGMEM volatile unsigned int _CNT __asm__("CNT");
extern _COGMEM volatile unsigned int _OUTA __asm__("OUTA");
extern _COGMEM volatile unsigned int _INA __asm__("INA");
extern _COGMEM volatile unsigned int _DIRA __asm__("DIRA");

extern unsigned int _clkfreq; /* in the spin boot code */
#define _CLKFREQ _clkfreq


#endif
