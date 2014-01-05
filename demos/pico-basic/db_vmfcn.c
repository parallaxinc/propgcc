/* db_vmfcn.c - intrinsic functions
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdlib.h>
#include <ctype.h>
#include "db_vm.h"

/* local functions */
static VMVALUE GetStringVal(uint8_t *str, int len);

/* fcn_abs - ABS(n): return the absolute value of a number */
void fcn_abs(Interpreter *i)
{
    *i->sp = abs(*i->sp);
}

/* fcn_rnd - RND(n): return a random number between 0 and n-1 */
void fcn_rnd(Interpreter *i)
{
    *i->sp = rand() % *i->sp;
}

/* fcn_left - LEFT$(str$, n): return the leftmost n characters of a string */
void fcn_left(Interpreter *i)
{
    VMHANDLE hstr;
    uint8_t *str;
    size_t len;
    VMVALUE n;
    str = GetStringPtr(*i->hsp);
    len = GetHeapObjSize(*i->hsp);
    n = *i->sp;
    if (n < 0)
        n = 0;
    else if (n > len)
        n = len;
    hstr = StoreByteVector(i->heap, ObjTypeString, str, n);
    ObjRelease(i->heap, *i->hsp);
    *i->hsp = hstr;
    Drop(i, 1);
}

/* fcn_right - RIGHT$(str$, n): return the rightmost n characters of a string */
void fcn_right(Interpreter *i)
{
    VMHANDLE hstr;
    uint8_t *str;
    size_t len;
    VMVALUE n;
    str = GetStringPtr(*i->hsp);
    len = GetHeapObjSize(*i->hsp);
    n = *i->sp;
    if (n < 0)
        n = 0;
    else if (n > len)
        n = len;
    hstr = StoreByteVector(i->heap, ObjTypeString, str + len - n, n);
    ObjRelease(i->heap, *i->hsp);
    *i->hsp = hstr;
    Drop(i, 1);
}

/* fcn_mid - MID$(str$, start, n): return n characters from the middle of a string */
void fcn_mid(Interpreter *i)
{
    VMHANDLE hstr;
    VMVALUE start, n;
    uint8_t *str;
    size_t len;
    str = GetStringPtr(*i->hsp);
    len = GetHeapObjSize(*i->hsp);
    start = *i->sp;
    n = i->sp[1];
    if (start < 0 || start >= len)
        Abort(i->sys, str_subscript_err, start + 1);
    if (start + n > len)
        n = len - start;
    hstr = StoreByteVector(i->heap, ObjTypeString, str + start, n);
    ObjRelease(i->heap, *i->hsp);
    *i->hsp = hstr;
    Drop(i, 2);
}

/* fcn_chr - CHR$(n): return a one character string with the specified character code */
void fcn_chr(Interpreter *i)
{
    uint8_t buf[1];
    buf[0] = (char)Pop(i);
    CPushH(i, StoreByteVector(i->heap, ObjTypeString, buf, 1));
}

/* fcn_str - STR$(n): return n converted to a string */
void fcn_str(Interpreter *i)
{
    char buf[32];
    snprintf(buf, sizeof(buf), str_value_fmt, Pop(i));
    CPushH(i, StoreByteVector(i->heap, ObjTypeString, (uint8_t *)buf, strlen(buf)));
}

/* fcn_val - VAL(str$): return the numeric value of a string */
void fcn_val(Interpreter *i)
{
    uint8_t *str;
    size_t len;
    str = GetStringPtr(*i->hsp);
    len = GetHeapObjSize(*i->hsp);
    CPush(i, GetStringVal(str, len));
    ObjRelease(i->heap, PopH(i));
}

/* fcn_asc - ASC(str$): return the character code of the first character of a string */
void fcn_asc(Interpreter *i)
{
    uint8_t *str;
    size_t len;
    str = GetStringPtr(*i->hsp);
    len = GetHeapObjSize(*i->hsp);
    CPush(i, len > 0 ? *str : 0);
    ObjRelease(i->heap, PopH(i));
}

/* fcn_len - LEN(str$): return length of a string */
void fcn_len(Interpreter *i)
{
    CPush(i,GetHeapObjSize(*i->hsp));
    ObjRelease(i->heap, PopH(i));
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
void fcn_printStr(Interpreter *i)
{
    VMHANDLE string;
    uint8_t *str;
    size_t size;
    string = i->hsp[0];
    str = GetByteVectorBase(string);
    size = GetHeapObjSize(string);
    while (size > 0) {
        VM_putchar(*str++);
        --size;
    }
    ObjRelease(i->heap, *i->hsp);
    DropH(i, 1);
}

/* fcn_printInt - printInt(n): print an integer */
void fcn_printInt(Interpreter *i)
{
    VM_printf(str_value_fmt, i->sp[0]);
    Drop(i, 1);
}

/* fcn_printTab - printTab(): print a tab */
void fcn_printTab(Interpreter *i)
{
    VM_putchar('\t');
}

/* fcn_printNL - printNL(): print a newline */
void fcn_printNL(Interpreter *i)
{
    VM_putchar('\n');
}

/* fcn_printFlush - printFlush(): flush the output buffer */
void fcn_printFlush(Interpreter *i)
{
    VM_flush();
}
