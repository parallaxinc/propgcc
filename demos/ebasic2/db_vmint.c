/* db_vmint.c - bytecode interpreter for a simple virtual machine
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "db_vm.h"

#if ALIGN_MASK == 1
#define VMVALUE_to_tmp                                      \
            tmp =  VMCODEBYTE(i->pc++);                     \
            tmp |= (VMVALUE)(VMCODEBYTE(i->pc++) << 8)
#else
#define VMVALUE_to_tmp                                      \
            tmp =  VMCODEBYTE(i->pc++);                     \
            tmp |= (VMVALUE)(VMCODEBYTE(i->pc++) << 8);     \
            tmp |= (VMVALUE)(VMCODEBYTE(i->pc++) << 16);    \
            tmp |= (VMVALUE)(VMCODEBYTE(i->pc++) << 24)
#endif

#if ALIGN_MASK == 1
#define get_VMVALUE(var, type)                              \
            tmp =  VMCODEBYTE(i->pc++);                     \
            tmp |= (VMVALUE)(VMCODEBYTE(i->pc++) << 8);     \
            var = (type)tmp
#else
#define get_VMVALUE(var, type)                              \
            tmp =  VMCODEBYTE(i->pc++);                     \
            tmp |= (VMVALUE)(VMCODEBYTE(i->pc++) << 8);     \
            tmp |= (VMVALUE)(VMCODEBYTE(i->pc++) << 16);    \
            tmp |= (VMVALUE)(VMCODEBYTE(i->pc++) << 24);    \
            var = (type)tmp
#endif

/* prototypes for local functions */
static void StartCode(Interpreter *i, VMVALUE object);
static void StringCat(Interpreter *i);

/* InitInterpreter - initialize the interpreter */
uint8_t *InitInterpreter(Interpreter *i, ObjHeap *heap, size_t stackSize)
{
    i->heap = heap;
    i->stack = (VMVALUE *)((uint8_t *)i + sizeof(Interpreter));
    i->stackTop = i->stack + stackSize;
    return (uint8_t *)i->stackTop;
}

/* Execute - execute the main code */
int Execute(Interpreter *i, VMHANDLE main)
{
    VMVALUE tmp, tmp2, ind;
    VMHANDLE obj;
    int8_t tmpb;

    /* initialize */
    i->pc = GetCodePtr(main);
    i->sp = i->fp = i->stackTop;

    if (setjmp(i->errorTarget))
        return VMFALSE;

    for (;;) {
#if 0
        ShowStack(i);
        DecodeInstruction(0, 0, i->pc);
#endif
        switch (VMCODEBYTE(i->pc++)) {
        case OP_HALT:
            return VMTRUE;
        case OP_BRT:
            VMVALUE_to_tmp;
            if (Pop(i))
                i->pc += tmp;
            break;
        case OP_BRTSC:
            VMVALUE_to_tmp;
            if (*i->sp)
                i->pc += tmp;
            else
                Drop(i, 1);
            break;
        case OP_BRF:
            VMVALUE_to_tmp;
            if (!Pop(i))
                i->pc += tmp;
            break;
        case OP_BRFSC:
            VMVALUE_to_tmp;
            if (!*i->sp)
                i->pc += tmp;
            else
                Drop(i, 1);
            break;
        case OP_BR:
            VMVALUE_to_tmp;
            i->pc += tmp;
            break;
        case OP_NOT:
            *i->sp = (*i->sp ? VMFALSE : VMTRUE);
            break;
        case OP_NEG:
            *i->sp = -*i->sp;
            break;
        case OP_ADD:
            tmp = Pop(i);
            *i->sp += tmp;
            break;
        case OP_SUB:
            tmp = Pop(i);
            *i->sp -= tmp;
            break;
        case OP_MUL:
            tmp = Pop(i);
            *i->sp *= tmp;
            break;
        case OP_DIV:
            tmp = Pop(i);
            *i->sp = (tmp == 0 ? 0 : *i->sp / tmp);
            break;
        case OP_REM:
            tmp = Pop(i);
            *i->sp = (tmp == 0 ? 0 : *i->sp % tmp);
            break;
        case OP_CAT:
            StringCat(i);
            break;
        case OP_BNOT:
            *i->sp = ~*i->sp;
            break;
        case OP_BAND:
            tmp = Pop(i);
            *i->sp &= tmp;
            break;
        case OP_BOR:
            tmp = Pop(i);
            *i->sp |= tmp;
            break;
        case OP_BXOR:
            tmp = Pop(i);
            *i->sp ^= tmp;
            break;
        case OP_SHL:
            tmp = Pop(i);
            *i->sp <<= tmp;
            break;
        case OP_SHR:
            tmp = Pop(i);
            *i->sp >>= tmp;
            break;
        case OP_LT:
            tmp = Pop(i);
            *i->sp = (*i->sp < tmp ? VMTRUE : VMFALSE);
            break;
        case OP_LE:
            tmp = Pop(i);
            *i->sp = (*i->sp <= tmp ? VMTRUE : VMFALSE);
            break;
        case OP_EQ:
            tmp = Pop(i);
            *i->sp = (*i->sp == tmp ? VMTRUE : VMFALSE);
            break;
        case OP_NE:
            tmp = Pop(i);
            *i->sp = (*i->sp != tmp ? VMTRUE : VMFALSE);
            break;
        case OP_GE:
            tmp = Pop(i);
            *i->sp = (*i->sp >= tmp ? VMTRUE : VMFALSE);
            break;
        case OP_GT:
            tmp = Pop(i);
            *i->sp = (*i->sp > tmp ? VMTRUE : VMFALSE);
            break;
        case OP_LIT:
        case OP_LITH:
            VMVALUE_to_tmp;
            CPush(i, tmp);
            break;
        case OP_GREF:
            get_VMVALUE(obj, VMHANDLE);
            CPush(i, GetSymbolPtr(obj)->v.iValue);
            break;
        case OP_GSET:
            get_VMVALUE(obj, VMHANDLE);
            GetSymbolPtr(obj)->v.iValue = Pop(i);
            break;
        case OP_LREF:
            tmpb = (int8_t)VMCODEBYTE(i->pc++);
            CPush(i, i->fp[(int)tmpb]);
            break;
        case OP_LSET:
            tmpb = (int8_t)VMCODEBYTE(i->pc++);
            i->fp[(int)tmpb] = Pop(i);
            break;
        case OP_VREF:
            ind = Pop(i);
            obj = (VMHANDLE)i->sp[0];
            if (ind < 0 || ind >= GetHeapObjSize(obj))
                Abort(i, str_array_subscript_err, ind);
            i->sp[0] = GetIntegerVectorBase(obj)[ind];
            break;
        case OP_VSET:
            tmp2 = Pop(i);
            ind = Pop(i);
            obj = (VMHANDLE)i->sp[0];
            if (ind < 0 || ind >= GetHeapObjSize(obj))
                Abort(i, str_array_subscript_err, ind);
            GetIntegerVectorBase(obj)[ind] = tmp2;
            Drop(i, 1);
            break;
        case OP_RESERVE:
            tmp = VMCODEBYTE(i->pc++);
            Reserve(i, tmp);
            break;
        case OP_CALL:
            i->argc = VMCODEBYTE(i->pc++);
            StartCode(i, *i->sp);
            break;
        case OP_RETURN:
            tmp = *i->sp;
            i->pc = i->heap->data + i->fp[F_PC];
            i->sp = i->fp;
            Drop(i, VMCODEBYTE(i->pc - 1));
            i->fp = i->stack + i->fp[F_FP];
            *i->sp = tmp;
            break;
        case OP_DROP:
            Drop(i, 1);
            break;
        default:
            Abort(i, str_opcode_err, VMCODEBYTE(i->pc - 1));
            break;
        }
    }
}

static void StartCode(Interpreter *i, VMVALUE object)
{
    if (object & INTRINSIC_FLAG) {
        VMUVALUE index = object & ~INTRINSIC_FLAG;
        if (index < IntrinsicCount) {
            (*VMINTRINSIC(index))(i);
            Drop(i, i->argc);
        }
        else
            Abort(i, str_not_code_object_err, object);
    }
    else if (object) {
        VMVALUE tmp;
        tmp = (VMVALUE)(i->fp - i->stack);
        i->fp = i->sp;
        Reserve(i, F_SIZE);
        i->fp[F_FP] = tmp;
        i->fp[F_PC] = (VMVALUE)(i->pc - i->heap->data);
        i->pc = GetCodePtr((VMHANDLE)object);
    }
    else
        Abort(i, str_not_code_object_err, object);
}

static void StringCat(Interpreter *i)
{
    VMHANDLE hStr2 = (VMHANDLE)Pop(i);
    VMHANDLE hStr1 = (VMHANDLE)i->sp[0];
    uint8_t *str1, *str2, *str;
    size_t len1, len2;

    /* get the first string pointer and length */
    str1 = GetStringPtr(hStr1);
    len1 = GetHeapObjSize(hStr1);

    /* get the second string pointer and length */
    str2 = GetStringPtr(hStr2);
    len2 = GetHeapObjSize(hStr2);

    /* allocate the result string */
    i->sp[0] = (VMVALUE)NewString(i->heap, len1 + len2);
    str = GetStringPtr((VMHANDLE)i->sp[0]);

    /* copy the source strings into the result string */
    memcpy(str, str1, len1);
    memcpy(str + len1, str2, len2);
}

void ShowStack(Interpreter *i)
{
    VMVALUE *p;
    for (p = i->sp; p < i->stackTop; ++p) {
        if (p == i->fp)
            VM_printf(" <fp>");
        VM_printf(" %d", *p);
    }
    VM_putchar('\n');
}

void StackOverflow(Interpreter *i)
{
    Abort(i, "stack overflow");
}

void Abort(Interpreter *i, const char *fmt, ...)
{
    char buf[100], *p = buf;
    va_list ap;
    va_start(ap, fmt);
    VM_printf(str_abort_prefix);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    while (*p != '\0')
        VM_putchar(*p++);
    VM_putchar('\n');
    va_end(ap);
    if (i)
        longjmp(i->errorTarget, 1);
    else
        exit(1);
}
