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
static void fcn_printFlush(Interpreter *i);

#ifdef PROPELLER

#include <unistd.h>
#include <propeller.h>

static void fcn_IN(Interpreter *i);
static void fcn_OUT(Interpreter *i);
static void fcn_HIGH(Interpreter *i);
static void fcn_LOW(Interpreter *i);
static void fcn_TOGGLE(Interpreter *i);
static void fcn_DIR(Interpreter *i);
static void fcn_GETDIR(Interpreter *i);
static void fcn_CNT(Interpreter *i);
static void fcn_PAUSE(Interpreter *i);
static void fcn_PULSEIN(Interpreter *i);
static void fcn_PULSEOUT(Interpreter *i);

#endif // PROPELLER

uint8_t *GetArgBVector(Interpreter *i, VMVALUE object, VMVALUE *pSize);

/* this table must be in the same order as the FN_xxx macros */
IntrinsicFcn * FLASH_SPACE Intrinsics[] = {
    fcn_abs,
    fcn_rnd,
    fcn_printStr,
    fcn_printInt,
    fcn_printTab,
    fcn_printNL,
    fcn_printFlush,
#ifdef PROPELLER
    fcn_IN,
    fcn_OUT,
    fcn_HIGH,
    fcn_LOW,
    fcn_TOGGLE,
    fcn_DIR,
    fcn_GETDIR,
    fcn_CNT,
    fcn_PAUSE,
    fcn_PULSEIN,
    fcn_PULSEOUT
#endif
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
    VMVALUE size;
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

/* fcn_printFlush - printFlush(): flush the output buffer */
static void fcn_printFlush(Interpreter *i)
{
    VM_flush();
}

#ifdef PROPELLER

static void fcn_IN(Interpreter *i)
{
    CheckArgCountBt(i, 1, 2);
    if (i->argc == 1) {
        uint32_t pin_mask = 1 << i->sp[0];
        DIRA &= ~pin_mask;
        i->sp[-1] = (INA & pin_mask) ? 1 : 0;
    }
    else {
        int high = i->sp[-1], low = i->sp[0];
        uint32_t pin_mask = ((1 << (high - low + 1)) - 1) << low;
        DIRA &= ~pin_mask;
        i->sp[-2] = (INA & pin_mask) >> low;
    }
}

static void fcn_OUT(Interpreter *i)
{
    CheckArgCountBt(i, 2, 3);
    if (i->argc == 2) {
        uint32_t pin_mask = 1 << i->sp[-1];
        OUTA = (OUTA & ~pin_mask) | (i->sp[0] ? pin_mask : 0);
        DIRA |= pin_mask;
    }
    else {
        int high = i->sp[-2], low = i->sp[-1];
        uint32_t pin_mask = ((1 << (high - low + 1)) - 1) << low;
        DIRA |= pin_mask;
        OUTA = (OUTA & ~pin_mask) | ((i->sp[0] << low) & pin_mask);
    }
}

static void fcn_HIGH(Interpreter *i)
{
    uint32_t pin_mask;
    CheckArgCountEq(i, 1);
    pin_mask = 1 << i->sp[0];
    OUTA |= pin_mask;
    DIRA |= pin_mask;
}

static void fcn_LOW(Interpreter *i)
{
    uint32_t pin_mask;
    CheckArgCountEq(i, 1);
    pin_mask = 1 << i->sp[0];
    OUTA &= ~pin_mask;
    DIRA |= pin_mask;
}

static void fcn_TOGGLE(Interpreter *i)
{
    uint32_t pin_mask;
    CheckArgCountEq(i, 1);
    pin_mask = 1 << i->sp[0];
    OUTA ^= pin_mask;
    DIRA |= pin_mask;
}

static void fcn_DIR(Interpreter *i)
{
    CheckArgCountBt(i, 2, 3);
    if (i->argc == 2) {
        uint32_t pin_mask = 1 << i->sp[-1];
        DIRA = (DIRA & ~pin_mask) | (i->sp[0] ? pin_mask : 0);
    }
    else {
        int high = i->sp[-2], low = i->sp[-1];
        uint32_t pin_mask = ((1 << (high - low + 1)) - 1) << low;
        DIRA = (DIRA & ~pin_mask) | ((i->sp[0] << low) & pin_mask);
    }
}

static void fcn_GETDIR(Interpreter *i)
{
    CheckArgCountBt(i, 1, 2);
    if (i->argc == 1) {
        uint32_t pin_mask = 1 << i->sp[0];
        i->sp[-1] = (DIRA & pin_mask) ? 1 : 0;
    }
    else {
        int high = i->sp[-1], low = i->sp[0];
        uint32_t pin_mask = ((1 << (high - low + 1)) - 1) << low;
        i->sp[-2] = (DIRA & pin_mask) >> low;
    }
}

static HUBTEXT int pulseIn(int pin, int state)
{
    uint32_t mask = 1 << pin;
    uint32_t data = state << pin;
    uint32_t ticks;
    DIRA &= ~mask;
    waitpeq(data, mask);
    ticks = CNT;
    waitpne(data, mask);
    ticks = CNT - ticks;
    return ticks / (CLKFREQ / 1000000);
}

static void fcn_PULSEIN(Interpreter *i)
{
    i->sp[-2] = pulseIn(i->sp[-1], i->sp[0]);
}

static HUBTEXT void pulseOut(int pin, int duration)
{
    uint32_t mask = 1 << pin;
    uint32_t ticks = duration * (CLKFREQ / 1000000);
    OUTA ^= mask;
    DIRA |= mask;
    waitcnt(CNT + ticks);
    OUTA ^= mask;
}

static void fcn_PULSEOUT(Interpreter *i)
{
    pulseOut(i->sp[-1], i->sp[0]);
}

static void fcn_CNT(Interpreter *i)
{
    CheckArgCountEq(i, 0);
    i->sp[0] = CNT;
}

static void fcn_PAUSE(Interpreter *i)
{
    CheckArgCountEq(i, 1);
    usleep(i->sp[0] * 1000);
}

#endif // PROPELLER

/* GetArgBVector - get a byte vector argument */
uint8_t *GetArgBVector(Interpreter *i, VMVALUE object, VMVALUE *pSize)
{
    VectorObjectHdr *hdr;
    CheckObjNumber(i, object);
    hdr = (VectorObjectHdr *)GetObjHdr(i->image, object);
    CheckObjType(i, hdr, PROTO_BVECTOR);
    *pSize = (VMVALUE)GetVectorSize(hdr);
    return GetBVectorBase(hdr);
}
