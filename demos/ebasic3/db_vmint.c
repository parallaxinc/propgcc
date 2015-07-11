/* db_vmint.c - bytecode interpreter for a simple virtual machine
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "db_vm.h"
#include "db_vmdebug.h"

//#define DEBUG

#define MIN_STACK_SIZE  128

/* interpreter state structure */
typedef struct {
    System *sys;
    ImageHdr *image;
    jmp_buf errorTarget;
    VMVALUE *stack;
    VMVALUE *stackTop;
    uint8_t *pc;
    VMVALUE *fp;
    VMVALUE *sp;
    VMVALUE tos;
    int linePos;
} Interpreter;

/* stack manipulation macros */
#define Reserve(i, n)   do {                                    \
                            if ((i)->sp - (n) < (i)->stack)     \
                                StackOverflow(i);               \
                            else                                \
                                (i)->sp -= (n);                 \
                        } while (0)
#define CPush(i, v)     do {                                    \
                            if ((i)->sp - 1 < (i)->stack)       \
                                StackOverflow(i);               \
                            else                                \
                                Push(i, v);                     \
                        } while (0)
#define Push(i, v)      (*--(i)->sp = (v))
#define Pop(i)          (*(i)->sp++)
#define Top(i)          (*(i)->sp)
#define Drop(i, n)      ((i)->sp += (n))

/* prototypes for local functions */
static void DoTrap(Interpreter *i, int op);
static void StackOverflow(Interpreter *i);
#ifdef DEBUG
static void ShowStack(Interpreter *i);
#endif

/* Execute - execute the main code */
int Execute(System *sys, ImageHdr *image, VMVALUE main)
{
    size_t stackSize;
    Interpreter *i;
    VMVALUE tmp;
    int8_t tmpb;
    int cnt;

    /* allocate the interpreter state */
    if (!(i = (Interpreter *)AllocateFreeSpace(sys, sizeof(Interpreter))))
        return VMFALSE;

    /* make sure there is space left for the stack */
    if ((stackSize = sys->freeTop - sys->freeNext) < MIN_STACK_SIZE)
        return VMFALSE;

	/* setup the new image */
    i->sys = sys;
	i->image = image;
    i->stack = (VMVALUE *)((uint8_t *)i + sizeof(Interpreter));
    i->stackTop = (VMVALUE *)((uint8_t *)i->stack + stackSize);

    /* initialize */    
    i->pc = (uint8_t *)main;
    i->sp = i->fp = i->stackTop;
    i->linePos = 0;

    if (setjmp(i->errorTarget))
        return VMFALSE;

    for (;;) {
#ifdef DEBUG
        ShowStack(i);
        DecodeInstruction(i->pc, i->pc);
#endif
        switch (VMCODEBYTE(i->pc++)) {
        case OP_HALT:
            return VMTRUE;
        case OP_BRT:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            if (i->tos)
                i->pc += tmp;
            i->tos = Pop(i);
            break;
        case OP_BRTSC:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            if (i->tos)
                i->pc += tmp;
            else
                i->tos = Pop(i);
            break;
        case OP_BRF:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            if (!i->tos)
                i->pc += tmp;
            i->tos = Pop(i);
            break;
        case OP_BRFSC:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            if (!i->tos)
                i->pc += tmp;
            else
                i->tos = Pop(i);
            break;
        case OP_BR:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            i->pc += tmp;
            break;
        case OP_NOT:
            i->tos = (i->tos ? VMFALSE : VMTRUE);
            break;
        case OP_NEG:
            i->tos = -i->tos;
            break;
        case OP_ADD:
            tmp = Pop(i);
            i->tos = tmp + i->tos;
            break;
        case OP_SUB:
            tmp = Pop(i);
            i->tos = tmp - i->tos;
            break;
        case OP_MUL:
            tmp = Pop(i);
            i->tos = tmp * i->tos;
            break;
        case OP_DIV:
            tmp = Pop(i);
            i->tos = (i->tos == 0 ? 0 : tmp / i->tos);
            break;
        case OP_REM:
            tmp = Pop(i);
            i->tos = (i->tos == 0 ? 0 : tmp % i->tos);
            break;
        case OP_BNOT:
            i->tos = ~i->tos;
            break;
        case OP_BAND:
            tmp = Pop(i);
            i->tos = tmp & i->tos;
            break;
        case OP_BOR:
            tmp = Pop(i);
            i->tos = tmp | i->tos;
            break;
        case OP_BXOR:
            tmp = Pop(i);
            i->tos = tmp ^ i->tos;
            break;
        case OP_SHL:
            tmp = Pop(i);
            i->tos = tmp << i->tos;
            break;
        case OP_SHR:
            tmp = Pop(i);
            i->tos = tmp >> i->tos;
            break;
        case OP_LT:
            tmp = Pop(i);
            i->tos = (tmp < i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_LE:
            tmp = Pop(i);
            i->tos = (tmp <= i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_EQ:
            tmp = Pop(i);
            i->tos = (tmp == i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_NE:
            tmp = Pop(i);
            i->tos = (tmp != i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_GE:
            tmp = Pop(i);
            i->tos = (tmp >= i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_GT:
            tmp = Pop(i);
            i->tos = (tmp > i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_LIT:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            CPush(i, i->tos);
            i->tos = tmp;
            break;
        case OP_SLIT:
            tmpb = (int8_t)VMCODEBYTE(i->pc++);
            CPush(i, i->tos);
            i->tos = tmpb;
            break;
        case OP_LOAD:
            i->tos = *(VMVALUE *)i->tos;
            break;
        case OP_LOADB:
            i->tos = *(uint8_t *)i->tos;
            break;
        case OP_STORE:
            tmp = Pop(i);
            *(VMVALUE *)i->tos = tmp;
            i->tos = Pop(i);
            break;
        case OP_STOREB:
            tmp = Pop(i);
            *(uint8_t *)i->tos = tmp;
            i->tos = Pop(i);
            break;
        case OP_LREF:
            tmpb = (int8_t)VMCODEBYTE(i->pc++);
            CPush(i, i->tos);
            i->tos = i->fp[(int)tmpb];
            break;
        case OP_LSET:
            tmpb = (int8_t)VMCODEBYTE(i->pc++);
            i->fp[(int)tmpb] = i->tos;
            i->tos = Pop(i);
            break;
        case OP_INDEX:
            tmp = Pop(i);
            i->tos = tmp + i->tos * sizeof (VMVALUE);
            break;
        case OP_CALL:
            ++i->pc; // skip over the argument count
            tmp = i->tos;
            i->tos = (VMVALUE)i->pc;
            i->pc = (uint8_t *)tmp;
            break;
        case OP_FRAME:
            cnt = VMCODEBYTE(i->pc++);
            tmp = (VMVALUE)i->fp;
            i->fp = i->sp;
            Reserve(i, cnt);
            i->sp[0] = i->tos;
            i->sp[1] = tmp;
            break;
        case OP_RETURN:
            i->pc = (uint8_t *)Top(i);
            i->sp = i->fp;
            Drop(i, i->pc[-1]);
            i->fp = (VMVALUE *)i->fp[-1];
            break;
        case OP_DROP:
            i->tos = Pop(i);
            break;
        case OP_DUP:
            CPush(i, i->tos);
            break;
        case OP_NATIVE:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            break;
        case OP_TRAP:
            DoTrap(i, VMCODEBYTE(i->pc++));
            break;
        default:
            Abort(i->sys, "undefined opcode 0x%02x", VMCODEBYTE(i->pc - 1));
            break;
        }
    }
}

static void DoTrap(Interpreter *i, int op)
{
    switch (op) {
    case TRAP_GetChar:
        Push(i, i->tos);
        i->tos = VM_getchar();
        break;
    case TRAP_PutChar:
        VM_putchar(i->tos);
        i->tos = Pop(i);
        break;
    case TRAP_PrintStr:
        VM_printf("%s", (char *)i->tos);
        i->tos = *i->sp++;
        break;
    case TRAP_PrintInt:
        VM_printf("%d", i->tos);
        i->tos = *i->sp++;
        break;
    case TRAP_PrintTab:
        VM_putchar('\t');
        break;
    case TRAP_PrintNL:
        VM_putchar('\n');
        break;
    case TRAP_PrintFlush:
        VM_flush();
        break;
    default:
        Abort(i->sys, "undefined trap %d", op);
        break;
    }
}

static void StackOverflow(Interpreter *i)
{
    Abort(i->sys, "stack overflow");
}

#ifdef DEBUG
static void ShowStack(Interpreter *i)
{
    VMVALUE *p;
    if (i->sp < i->stackTop) {
        VM_printf(" %d", i->tos);
        for (p = i->sp; p < i->stackTop - 1; ++p) {
            if (p == i->fp)
                VM_printf(" <fp>");
            VM_printf(" %d", *p);
        }
        VM_printf("\n");
    }
}
#endif
