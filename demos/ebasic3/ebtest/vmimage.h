/* vmimage.h - compiled image file definitions
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __VMIMAGE_H__
#define __VMIMAGE_H__

#include <stdint.h>
typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;
#define VMCODEBYTE(p)	*p

#define IMAGE_TAG       "XLOD"
#define IMAGE_VERSION   0x0100

/* image file section */
typedef struct {
    VMUVALUE base;
    VMUVALUE offset;
    VMUVALUE size;
} ImageFileSection;

/* image file header */
typedef struct {
    uint8_t tag[4];     /* should be 'XLOD' */
    uint16_t version;   /* version number */
    uint16_t unused;    /* (unused at present) */
    VMUVALUE mainCode;
    VMUVALUE stackSize;
    VMUVALUE sectionCount;
    ImageFileSection sections[1];
} ImageFileHdr;

/* stack frame offsets */
#define F_FP    -1
#define F_SIZE  1

/* opcodes */
#define OP_HALT         0x00    /* halt */
#define OP_BRT          0x01    /* branch on true */
#define OP_BRTSC        0x02    /* branch on true (for short circuit booleans) */
#define OP_BRF          0x03    /* branch on false */
#define OP_BRFSC        0x04    /* branch on false (for short circuit booleans) */
#define OP_BR           0x05    /* branch unconditionally */
#define OP_NOT          0x06    /* logical negate top of stack */
#define OP_NEG          0x07    /* negate */
#define OP_ADD          0x08    /* add two numeric expressions */
#define OP_SUB          0x09    /* subtract two numeric expressions */
#define OP_MUL          0x0a    /* multiply two numeric expressions */
#define OP_DIV          0x0b    /* divide two numeric expressions */
#define OP_REM          0x0c    /* remainder of two numeric expressions */
#define OP_BNOT         0x0d    /* bitwise not of two numeric expressions */
#define OP_BAND         0x0e    /* bitwise and of two numeric expressions */
#define OP_BOR          0x0f    /* bitwise or of two numeric expressions */
#define OP_BXOR         0x10    /* bitwise exclusive or */
#define OP_SHL          0x11    /* shift left */
#define OP_SHR          0x12    /* shift right */
#define OP_LT           0x13    /* less than */
#define OP_LE           0x14    /* less than or equal to */
#define OP_EQ           0x15    /* equal to */
#define OP_NE           0x16    /* not equal to */
#define OP_GE           0x17    /* greater than or equal to */
#define OP_GT           0x18    /* greater than */
#define OP_LIT          0x19    /* load a literal */
#define OP_SLIT         0x1a    /* load a short literal (-128 to 127) */
#define OP_LOAD         0x1b    /* load a long from memory */
#define OP_LOADB        0x1c    /* load a byte from memory */
#define OP_STORE        0x1d    /* store a long into memory */
#define OP_STOREB       0x1e    /* store a byte into memory */
#define OP_LREF         0x1f    /* load a local variable relative to the frame pointer */
#define OP_LSET         0x20    /* set a local variable relative to the frame pointer */
#define OP_INDEX        0x21    /* index into a vector of longs */
#define OP_PUSHJ        0x22    /* push the pc and jump to a function */
#define OP_POPJ         0x23    /* return to the address on the stack */
#define OP_CLEAN        0x24    /* clean arguments off the stack after a function call */
#define OP_FRAME        0x25    /* create a stack frame */
#define OP_RETURN       0x26    /* remove a stack frame and return from a function call */
#define OP_RETURNZ      0x27    /* remove a stack frame and return zero from a function call */
#define OP_DROP         0x28    /* drop the top element of the stack */
#define OP_DUP          0x29    /* duplicate the top element of the stack */
#define OP_NATIVE       0x2a    /* execute native code */
#define OP_TRAP         0x2b    /* trap to handler */

/* OP_TRAP functions */
enum {
    TRAP_GETCHAR = 0x00,
    TRAP_PUTCHAR,
    TRAP_PRINTSTR,
    TRAP_PRINTINT,
    TRAP_PRINTTAB,
    TRAP_PRINTNL
};

#endif
