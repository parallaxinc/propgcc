/* db_image.h - compiled image file definitions
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_IMAGE_H__
#define __DB_IMAGE_H__

/* stack frame offsets */
#define F_FP    -1
#define F_HFP   -2
#define F_PC    -3
#define F_SIZE  3

/* handle stack frame offsets */
#define HF_CODE 1
#define HF_SIZE 1

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
#define OP_LIT          0x19    /* load literal */
#define OP_GREF         0x1a    /* load a global variable */
#define OP_GSET         0x1b    /* set a global variable */
#define OP_LREF         0x1c    /* load a local variable relative to the frame pointer */
#define OP_LSET         0x1d    /* set a local variable relative to the frame pointer */
#define OP_VREF         0x1e    /* load an element of a vector */
#define OP_VSET         0x1f    /* set an element of a vector */
#define OP_RESERVE      0x20    /* reserve space on the stack */
#define OP_CALL         0x21    /* call a function */
#define OP_RETURN       0x22    /* return from a function leaving an integer result on the stack */
#define OP_RETURNV      0x23    /* return from a function leaving no result on the stack */
#define OP_DROP         0x24    /* drop the top element of the stack */

#define OP_LITH         0x40    /* literal handle */
#define OP_GREFH        0x41    /* load a handle global variable */
#define OP_GSETH        0x42    /* set a handle global variable */
#define OP_LREFH        0x43    /* load a handle local variable relative to the frame pointer */
#define OP_LSETH        0x44    /* set a handlelocal variable relative to the frame pointer */
#define OP_VREFH        0x45    /* load an element of a vector */
#define OP_VSETH        0x46    /* set an element of a vector */
#define OP_RETURNH      0x47    /* return from a function leaving a handle result on the stack */
#define OP_DROPH        0x48    /* drop the top element of the handle stack */

#define OP_CAT          0x49    /* concatenate two strings */

#if ALIGN_MASK == 1
#define get_VMVALUE(var, getbyte)               \
            var =  (VMVALUE) (getbyte);         \
            var |= (VMVALUE)((getbyte) << 8);
#elif ALIGN_MASK == 3
#define get_VMVALUE(var, getbyte)               \
            var =  (VMVALUE) (getbyte);         \
            var |= (VMVALUE)((getbyte) << 8);   \
            var |= (VMVALUE)((getbyte) << 16);  \
            var |= (VMVALUE)((getbyte) << 24);
#else
#error Only 16 bit and 32 bit alignment is currently supported.
#endif

#endif
