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
static void fcn_left(Interpreter *i);
static void fcn_right(Interpreter *i);
static void fcn_mid(Interpreter *i);
static void fcn_chr(Interpreter *i);
static void fcn_str(Interpreter *i);
static void fcn_val(Interpreter *i);
static void fcn_asc(Interpreter *i);
static void fcn_len(Interpreter *i);
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

/* this table must be in the same order as the FN_xxx macros */
IntrinsicFcn * FLASH_SPACE Intrinsics[] = {
    fcn_abs,
    fcn_rnd,
    fcn_left,
    fcn_right,
    fcn_mid,
    fcn_chr,
    fcn_str,
    fcn_val,
    fcn_asc,
    fcn_len,
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

/* local functions */
static VMVALUE GetStringVal(uint8_t *str, int len);

/* fcn_abs - ABS(n): return the absolute value of a number */
static void fcn_abs(Interpreter *i)
{
    CheckArgCountEq(i, 1);
    i->sp[1] = abs(i->sp[1]);
}

/* fcn_rnd - RND(n): return a random number between 0 and n-1 */
static void fcn_rnd(Interpreter *i)
{
    CheckArgCountEq(i, 1);
    i->sp[1] = rand() % i->sp[1];
}

/* fcn_left - LEFT$(str$, n): return the leftmost n characters of a string */
static void fcn_left(Interpreter *i)
{
    uint8_t *str;
    size_t len;
    VMVALUE n;
    CheckArgCountEq(i, 2);
    str = GetStringPtr((VMHANDLE)i->sp[1]);
    len = GetHeapObjSize((VMHANDLE)i->sp[1]);
    n = i->sp[2];
    if (n > len)
        n = len;
    i->sp[2] = (VMVALUE)StoreByteVector(i->heap, ObjTypeString, str, n);
}

/* fcn_right - RIGHT$(str$, n): return the rightmost n characters of a string */
static void fcn_right(Interpreter *i)
{
    uint8_t *str;
    size_t len;
    VMVALUE n;
    CheckArgCountEq(i, 2);
    str = GetStringPtr((VMHANDLE)i->sp[1]);
    len = GetHeapObjSize((VMHANDLE)i->sp[1]);
    n = i->sp[2];
    if (n > len)
        n = len;
    i->sp[2] = (VMVALUE)StoreByteVector(i->heap, ObjTypeString, str + len - n, n);
    Drop(i, 2);
}

/* fcn_mid - MID$(str$, start, n): return n characters from the middle of a string */
static void fcn_mid(Interpreter *i)
{
    VMVALUE start, n;
    uint8_t *str;
    size_t len;
    CheckArgCountEq(i, 3);
    str = GetStringPtr((VMHANDLE)i->sp[1]);
    len = GetHeapObjSize((VMHANDLE)i->sp[1]);
    start = i->sp[2];
    n = i->sp[3];
    if (start < 0 || start >= len)
        Abort(i, str_string_index_range_err, start + 1);
    if (start + n > len)
        n = len - start;
    i->sp[3] = (VMVALUE)StoreByteVector(i->heap, ObjTypeString, str + start, n);
}

/* fcn_chr - CHR$(n): return a one character string with the specified character code */
static void fcn_chr(Interpreter *i)
{
    uint8_t buf[1];
    CheckArgCountEq(i, 1);
    buf[0] = (char)i->sp[1];
    i->sp[1] = (VMVALUE)StoreByteVector(i->heap, ObjTypeString, buf, 1);
}

/* fcn_str - STR$(n): return n converted to a string */
static void fcn_str(Interpreter *i)
{
    char buf[16];
    CheckArgCountEq(i, 1);
    sprintf(buf, "%d", i->sp[1]);
    i->sp[1] = (VMVALUE)StoreByteVector(i->heap, ObjTypeString, (uint8_t *)buf, strlen(buf));
}

/* fcn_val - VAL(str$): return the numeric value of a string */
static void fcn_val(Interpreter *i)
{
    uint8_t *str;
    size_t len;
    CheckArgCountEq(i, 1);
    str = GetStringPtr((VMHANDLE)i->sp[1]);
    len = GetHeapObjSize((VMHANDLE)i->sp[1]);
    i->sp[1] = GetStringVal(str, len);
}

/* fcn_asc - ASC(str$): return the character code of the first character of a string */
static void fcn_asc(Interpreter *i)
{
    uint8_t *str;
    size_t len;
    CheckArgCountEq(i, 1);
    str = GetStringPtr((VMHANDLE)i->sp[1]);
    len = GetHeapObjSize((VMHANDLE)i->sp[1]);
    i->sp[1] = len > 0 ? *str : 0;
}

/* fcn_len - LEN(str$): return length of a string */
static void fcn_len(Interpreter *i)
{
    CheckArgCountEq(i, 1);
    i->sp[1] = GetHeapObjSize((VMHANDLE)i->sp[1]);
}

/* GetStringVal - get the numeric value of a string */
static VMVALUE GetStringVal(uint8_t *str, int len)
{
    VMVALUE val = 0;
    int radix = 'd';
    int sign = 1;
    int ch;

    /* handle non-empty strings */
    if (--len >= 0) {

        /* check for a sign or a radix indicator */
        switch (ch = *str++) {
        case '-':
            sign = -1;
            break;
        case '+':
            /* sign is 1 by default */
            break;
        case '0':
            if (--len < 0)
                return 0;
            switch (ch = *str++) {
            case 'b':
            case 'B':
                radix = 'b';
                break;
            case 'x':
            case 'X':
                radix = 'x';
                break;
            default:
                ++len;
                --str;
                radix = 'o';
                break;
            }
            break;
        default:
            if (!isdigit(ch))
                return 0;
            val = ch - '0';
            break;
        }


        /* get the number */
        switch (radix) {
        case 'b':
            while (--len >= 0) {
                ch = *str++;
                if (ch == '0' || ch == '1')
                    val = val * 2 + ch - '0';
                else if (ch != '_')
                    break;
            }
            break;
        case 'd':
            while (--len >= 0) {
                ch = *str++;
                if (isdigit(ch))
                    val = val * 10 + ch - '0';
                else if (ch != '_')
                    break;
            }
            break;
        case 'x':
            while (--len >= 0) {
                ch = *str++;
                if (isxdigit(ch)) {
                    int lowerch = tolower(ch);
                    val = val * 16 + (isdigit(lowerch) ? lowerch - '0' : lowerch - 'a' + 16);
                }
                else if (ch != '_')
                    break;
            }
            break;
        case 'o':
            while (--len >= 0) {
                ch = *str++;
                if (ch >= '0' && ch <= '7')
                    val = val * 8 + ch - '0';
                else if (ch != '_')
                    break;
            }
            break;
        }
    }
    
    /* return the numeric value */
    return sign * val;
}

/* fcn_printStr - printStr(n): print a string */
static void fcn_printStr(Interpreter *i)
{
    VMHANDLE string;
    uint8_t *str;
    size_t size;
    CheckArgCountEq(i, 1);
    string = (VMHANDLE)i->sp[1];
    str = GetByteVectorBase(string);
    size = GetHeapObjSize(string);
    while (size > 0) {
        VM_putchar(*str++);
        --size;
    }
}

/* fcn_printInt - printInt(n): print an integer */
static void fcn_printInt(Interpreter *i)
{
    CheckArgCountEq(i, 1);
    VM_printf("%d", i->sp[1]);
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
        uint32_t pin_mask = 1 << i->sp[1];
        DIRA &= ~pin_mask;
        i->sp[1] = (INA & pin_mask) ? 1 : 0;
    }
    else {
        int high = i->sp[1], low = i->sp[2];
        uint32_t pin_mask = ((1 << (high - low + 1)) - 1) << low;
        DIRA &= ~pin_mask;
        i->sp[2] = (INA & pin_mask) >> low;
    }
}

static void fcn_OUT(Interpreter *i)
{
    CheckArgCountBt(i, 2, 3);
    if (i->argc == 2) {
        uint32_t pin_mask = 1 << i->sp[1];
        OUTA = (OUTA & ~pin_mask) | (i->sp[2] ? pin_mask : 0);
        DIRA |= pin_mask;
    }
    else {
        int high = i->sp[1], low = i->sp[2];
        uint32_t pin_mask = ((1 << (high - low + 1)) - 1) << low;
        DIRA |= pin_mask;
        OUTA = (OUTA & ~pin_mask) | ((i->sp[3] << low) & pin_mask);
    }
}

static void fcn_HIGH(Interpreter *i)
{
    uint32_t pin_mask;
    CheckArgCountEq(i, 1);
    pin_mask = 1 << i->sp[1];
    OUTA |= pin_mask;
    DIRA |= pin_mask;
}

static void fcn_LOW(Interpreter *i)
{
    uint32_t pin_mask;
    CheckArgCountEq(i, 1);
    pin_mask = 1 << i->sp[1];
    OUTA &= ~pin_mask;
    DIRA |= pin_mask;
}

static void fcn_TOGGLE(Interpreter *i)
{
    uint32_t pin_mask;
    CheckArgCountEq(i, 1);
    pin_mask = 1 << i->sp[1];
    OUTA ^= pin_mask;
    DIRA |= pin_mask;
}

static void fcn_DIR(Interpreter *i)
{
    CheckArgCountBt(i, 2, 3);
    if (i->argc == 2) {
        uint32_t pin_mask = 1 << i->sp[1];
        DIRA = (DIRA & ~pin_mask) | (i->sp[2] ? pin_mask : 0);
    }
    else {
        int high = i->sp[1], low = i->sp[2];
        uint32_t pin_mask = ((1 << (high - low + 1)) - 1) << low;
        DIRA = (DIRA & ~pin_mask) | ((i->sp[3] << low) & pin_mask);
    }
}

static void fcn_GETDIR(Interpreter *i)
{
    CheckArgCountBt(i, 1, 2);
    if (i->argc == 1) {
        uint32_t pin_mask = 1 << i->sp[1];
        i->sp[1] = (DIRA & pin_mask) ? 1 : 0;
    }
    else {
        int high = i->sp[1], low = i->sp[2];
        uint32_t pin_mask = ((1 << (high - low + 1)) - 1) << low;
        i->sp[2] = (DIRA & pin_mask) >> low;
    }
}

/* move pulseIn and pulseOut to a library at some point */

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

static HUBTEXT void pulseOut(int pin, int duration)
{
    uint32_t mask = 1 << pin;
    uint32_t ticks = duration * (CLKFREQ / 1000000);
    OUTA ^= mask;
    DIRA |= mask;
    waitcnt(CNT + ticks);
    OUTA ^= mask;
}

static void fcn_PULSEIN(Interpreter *i)
{
    CheckArgCountEq(i, 2);
    i->sp[2] = pulseIn(i->sp[1], i->sp[2]);
}

static void fcn_PULSEOUT(Interpreter *i)
{
    CheckArgCountEq(i, 2);
    pulseOut(i->sp[1], i->sp[2]);
}

static void fcn_CNT(Interpreter *i)
{
    CheckArgCountEq(i, 0);
    i->sp[0] = CNT;
}

static void fcn_PAUSE(Interpreter *i)
{
    CheckArgCountEq(i, 1);
    usleep(i->sp[1] * 1000);
}

#endif // PROPELLER
