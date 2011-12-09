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
static void fcn_printStr(Interpreter *i);
static void fcn_printInt(Interpreter *i);
static void fcn_printTab(Interpreter *i);
static void fcn_printNL(Interpreter *i);
uint8_t *GetArgBVector(Interpreter *i, int16_t object, int16_t *pSize);

/* this table must be in the same order as the FN_xxx macros */
IntrinsicFcn * FLASH_SPACE Intrinsics[] = {
    fcn_abs,
    fcn_rnd,
    fcn_printStr,
    fcn_printInt,
    fcn_printTab,
    fcn_printNL
};

int IntrinsicCount = sizeof(Intrinsics) / sizeof(IntrinsicFcn *);

/* fcn_abs - ABS(n): return the absolute value of a number */
static void fcn_abs(Interpreter *i)
{
    int n;
    CheckArgCountEq(i, 1);
    n = i->sp[0];
    i->sp[-1] = abs(n);
    Drop(i, 1);
}

/* fcn_rnd - RND(n): return a random number between 0 and n-1 */
static void fcn_rnd(Interpreter *i)
{
    int n;
    CheckArgCountEq(i, 1);
    n = i->sp[0];
    i->sp[-1] = rand() % n;
    Drop(i, 1);
}

/* fcn_printStr - printStr(n): print a string */
static void fcn_printStr(Interpreter *i)
{
    uint8_t *str;
    int16_t size;
    CheckArgCountEq(i, 1);
    str = GetArgBVector(i, i->sp[0], &size);
    while (--size >= 0)
        VM_putchar(*str++);
    Drop(i, 1);
}

/* fcn_printInt - printInt(n): print an integer */
static void fcn_printInt(Interpreter *i)
{
    CheckArgCountEq(i, 1);
    VM_printf("%d", i->sp[0]);
    Drop(i, 1);
}

/* fcn_printTab - printTab(): print a tab */
static void fcn_printTab(Interpreter *i)
{
    CheckArgCountEq(i, 0);
    VM_putchar('\t');
}

/* fcn_printNL - printNL(): print a newline */
static void fcn_printNL(Interpreter *i)
{
    CheckArgCountEq(i, 0);
    VM_putchar('\n');
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
