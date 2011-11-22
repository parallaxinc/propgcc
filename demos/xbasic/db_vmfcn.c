/* db_vmfcn.c - intrinsic functions
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdlib.h>
#include <ctype.h>
#include "db_vm.h"

static void fcn_abs(Interpreter *i);
static void fcn_rnd(Interpreter *i);
static void fcn_printf(Interpreter *i);
uint8_t *GetArgBVector(Interpreter *i, int16_t object, int16_t *pSize);

/* this table must be in the same order as the FN_xxx macros */
IntrinsicFcn * FLASH_SPACE Intrinsics[] = {
    fcn_abs,
    fcn_rnd,
    fcn_printf
};

int IntrinsicCount = sizeof(Intrinsics) / sizeof(IntrinsicFcn *);

/* fcn_abs - ABS(n): return the absolute value of a number */
static void fcn_abs(Interpreter *i)
{
    int n = i->sp[0];
    i->sp[-1] = abs(n);
    Drop(i, 1);
}

/* fcn_rnd - RND(n): return a random number between 0 and n-1 */
static void fcn_rnd(Interpreter *i)
{
    int n = i->sp[0];
    i->sp[-1] = rand() % n;
    Drop(i, 1);
}

/* fcn_printf - PRINTF(str$, ...): formatted print */
static void fcn_printf(Interpreter *i)
{
    int16_t *argv = GetArgPointer(i);
    uint8_t *fmt;
    int16_t size;
    int j;

    CheckArgCountGe(i, 1);
    fmt = GetArgBVector(i, argv[1], &size);
    j = 2;

    while (--size > 0) {
        int ch = VMCODEBYTE(fmt++);
        if (ch == '%' && --size > 0) {
            switch (ch = VMCODEBYTE(fmt++)) {
            case 'd':
                if (j <= i->argc)
                    VM_printf("%d", argv[j++]);
                break;
            case '%':
                VM_putchar(ch);
                break;
            }
        }
        else
            VM_putchar(ch);
    }

    Drop(i, i->argc);
}

/* GetArgBVector - get a byte vector argument */
uint8_t *GetArgBVector(Interpreter *i, int16_t object, int16_t *pSize)
{
    VectorObjectHdr *hdr;
    CheckObjNumber(i, object);
    hdr = (VectorObjectHdr *)GetObjHdr(i->image, object);
    CheckObjType(i, hdr, PROTO_BVECTOR);
    *pSize = (int16_t)GetVectorSize(hdr);
    return GetBVectorBase(hdr);
}
