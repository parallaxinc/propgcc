/* db_image.h - compiled image file definitions
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_IMAGE_H__
#define __DB_IMAGE_H__

#include <stdarg.h>
#include "db_types.h"

/* target runtimes */
#define TARGET_UNKNOWN      0
#define TARGET_XGS          1
#define TARGET_CHAMELEON    2

/* in-memory image header */
typedef struct {
    int16_t     mainCode;           /* main code object number */
    int16_t     *objects;           /* object table */
    uint16_t    objectCount;        /* object count */
    int16_t     *objectData;        /* object data space */
    uint16_t    objectDataSize;     /* object data space size in words */
    int16_t     *variables;         /* variable table */
    uint16_t    variableCount;      /* variable count */
} ImageHdr;

/* stack frame offsets */
#define F_FP    1
#define F_PC    2
#define F_SIZE  2

/* intrinsic function flag */
#define INTRINSIC_FLAG  0x8000

/* primitive prototypes */
#define PROTO_CODE      0x7fff
#define PROTO_VECTOR    0x7ffe
#define PROTO_BVECTOR   0x7ffd

/* get the size of an object in words */
#define GetObjSizeInWords(s)    (((s) + sizeof(int16_t) - 1) / sizeof(int16_t))

/* object header structure */
typedef struct {
    int16_t prototype;
} ObjectHdr;

/* vector or byte vector object header structure */
typedef struct {
    int16_t prototype;  // must be PROTO_CODE, PROTO_VECTOR, or PROTO_BVECTOR
    int16_t size;
} VectorObjectHdr;

/* get a pointer to an object header */
#define GetObjHdr(i, n)         ((ObjectHdr *)&(i)->objectData[(i)->objects[(n) - 1]])

/* get the prototype of an object */
#define GetObjPrototype(h)      ((int16_t)(h)->prototype)

/* get the size of a vector */
#define GetVectorSize(h)        ((VectorObjectHdr *)(h))->size

/* get the base of a word vector */
#define GetVectorBase(h)        ((int16_t *)((uint8_t *)(h) + sizeof(VectorObjectHdr)))

/* get the base of a byte vector */
#define GetBVectorBase(h)       ((uint8_t *)(h) + sizeof(VectorObjectHdr))

/* get the index of an intrinsic function */
#define GetIntrinsicIndex(h)    ((IntrinsicObjectHdr *)(h))->index

/* check an object number */
#define CheckObjNumber(i, n)    do {                                                \
                                    if ((n) <= 0 || (n) > (i)->image->objectCount)  \
                                        Abort(i, str_object_number_err, (n));       \
                                } while (0)

/* check the type of an object */
#define CheckObjType(i, h, t)   do {                                            \
                                    if (GetObjPrototype(h) != (t))              \
                                        Abort(i, str_wrong_type_err);           \
                                } while (0)

/* check for a specified number of arguments */
#define CheckArgCountEq(i, n)   do {                                            \
                                    if ((i)->argc != (n))                       \
                                        Abort(i, str_argument_count_err);       \
                                } while (0)
/* check for at least a specified number of arguments */
#define CheckArgCountGe(i, n)   do {                                            \
                                    if ((i)->argc < (n))                        \
                                        Abort(i, str_argument_count_err);       \
                                } while (0)

/* get the argument pointer */
#define GetArgPointer(i)        &i->sp[-i->argc]

/* intrinsic function numbers */
enum {
    FN_ABS              = 0x00,
    FN_RND,
    FN_printStr,
    FN_printInt,
    FN_printTab,
    FN_printNL
};

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
#define OP_DROP         0x23    /* drop the top element of the stack */

void VM_printf(const char *fmt, ...);           /* fmt in FLASH_SPACE */
void VM_vprintf(const char *fmt, va_list ap);   /* fmt in FLASH_SPACE */
void VM_putchar(int ch);

#endif
