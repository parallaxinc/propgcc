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

/* prototypes for local functions */
static void StartCode(Interpreter *i, int16_t object);

/* InitInterpreter - initialize the interpreter */
uint8_t *InitInterpreter(Interpreter *i, size_t stackSize)
{
    i->stack = (int16_t *)((uint8_t *)i + sizeof(Interpreter));
    i->stackTop = i->stack + stackSize;
    return (uint8_t *)i->stackTop;
}

/* Execute - execute the main code */
int Execute(Interpreter *i, ImageHdr *image)
{
    ObjectHdr *hdr;
    int16_t tmp, tmp2, ind;
    int8_t tmpb;

    /* setup the new image */
    i->image = image;

    /* initialize */
    CheckObjNumber(i, i->image->mainCode);
    hdr = GetObjHdr(i->image, i->image->mainCode);
    CheckObjType(i, hdr, PROTO_CODE);
    i->pc = GetBVectorBase(hdr);
    i->sp = i->fp = i->stack - 1;

    if (setjmp(i->errorTarget))
        return VMFALSE;

    for (;;) {
#if 0
        //ShowStack(i);
        DecodeInstruction(0, (uint8_t *)i->image->objectData, i->pc);
#endif
        switch (VMCODEBYTE(i->pc++)) {
        case OP_HALT:
            return VMTRUE;
        case OP_BRT:
            tmp = VMCODEBYTE(i->pc++);
            tmp |= (int16_t)(VMCODEBYTE(i->pc++) << 8);
            if (Pop(i))
                i->pc += tmp;
            break;
        case OP_BRTSC:
            tmp = VMCODEBYTE(i->pc++);
            tmp |= (int16_t)(VMCODEBYTE(i->pc++) << 8);
            if (*i->sp)
                i->pc += tmp;
            else
                Drop(i, 1);
            break;
        case OP_BRF:
            tmp = VMCODEBYTE(i->pc++);
            tmp |= (int16_t)(VMCODEBYTE(i->pc++) << 8);
            if (!Pop(i))
                i->pc += tmp;
            break;
        case OP_BRFSC:
            tmp = VMCODEBYTE(i->pc++);
            tmp |= (int16_t)(VMCODEBYTE(i->pc++) << 8);
            if (!*i->sp)
                i->pc += tmp;
            else
                Drop(i, 1);
            break;
        case OP_BR:
            tmp = VMCODEBYTE(i->pc++);
            tmp |= (int16_t)(VMCODEBYTE(i->pc++) << 8);
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
            tmp = VMCODEBYTE(i->pc++);
            CPush(i, (int16_t)(VMCODEBYTE(i->pc++) << 8) | tmp);
            break;
        case OP_GREF:
            tmp = VMCODEBYTE(i->pc++);
            tmp |= (int16_t)VMCODEBYTE(i->pc++) << 8;
            CPush(i, i->image->variables[tmp]);
            break;
        case OP_GSET:
            tmp = VMCODEBYTE(i->pc++);
            tmp |= (int16_t)VMCODEBYTE(i->pc++) << 8;
            i->image->variables[tmp] = Pop(i);
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
            tmp = i->sp[0];
            CheckObjNumber(i, tmp);
            hdr = GetObjHdr(i->image, tmp);
            CheckObjType(i, hdr, PROTO_VECTOR);
            if (ind < 0 || ind >= GetVectorSize(hdr))
                Abort(i, str_array_subscript_err, ind);
            i->sp[0] = GetVectorBase(hdr)[ind];
            break;
        case OP_VSET:
            tmp2 = Pop(i);
            ind = Pop(i);
            tmp = i->sp[0];
            CheckObjNumber(i, tmp);
            hdr = GetObjHdr(i->image, tmp);
            CheckObjType(i, hdr, PROTO_VECTOR);
            if (ind < 0 || ind >= GetVectorSize(hdr))
                Abort(i, str_array_subscript_err, ind);
            GetVectorBase(hdr)[ind] = tmp2;
            Drop(i, 1);
            break;
        case OP_RESERVE:
            tmp = VMCODEBYTE(i->pc++);
            Reserve(i, tmp);
            break;
        case OP_CALL:
            i->argc = VMCODEBYTE(i->pc++);
            StartCode(i, i->sp[-i->argc]);
            break;
        case OP_RETURN:
            tmp = *i->sp;
            i->pc = (uint8_t *)i->image->objectData + i->fp[F_PC];
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

static void StartCode(Interpreter *i, int16_t object)
{
    if (object & INTRINSIC_FLAG) {
        uint16_t index = object & ~INTRINSIC_FLAG;
        if (index < IntrinsicCount)
            (*VMINTRINSIC(index))(i);
        else
            Abort(i, str_not_code_object_err, object);
    }
    else {
        ObjectHdr *hdr;
        CheckObjNumber(i, object);
        hdr = GetObjHdr(i->image, object);
        if (GetObjPrototype(hdr) == PROTO_CODE) {
            int16_t tmp = (int16_t)(i->fp - i->stack);
            i->fp = i->sp;
            Reserve(i, F_SIZE);
            i->fp[F_FP] = tmp;
            i->fp[F_PC] = (int16_t)(i->pc - (uint8_t *)i->image->objectData);
            i->pc = GetBVectorBase(hdr);
        }
        else
            Abort(i, str_not_code_object_err, object);
    }
}

void ShowStack(Interpreter *i)
{
    int16_t *p;
    for (p = i->stack; p <= i->sp; ++p) {
        if (p == i->fp)
            VM_printf(" <fp>");
        VM_printf(" %d", *p);
    }
    VM_putchar('\n');
}

void StackOverflow(Interpreter *i)
{
    Abort(i, str_stack_overflow_err);
}

void Warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    VM_printf(str_warn_prefix);
    VM_vprintf(fmt, ap);
    VM_putchar('\n');
    va_end(ap);
}

void Abort(Interpreter *i, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    VM_printf(str_abort_prefix);
    VM_vprintf(fmt, ap);
    VM_putchar('\n');
    va_end(ap);
    if (i)
        longjmp(i->errorTarget, 1);
    else
        exit(1);
}
