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
#include <propeller.h>
#include "db_vm.h"

/* VM commands */
enum {
    VM_Done           = 0,
    VM_Continue       = 1,
    VM_ReadLong       = 2,
    VM_WriteLong      = 3,
    VM_ReadByte       = 4
};

/* VM status codes */
enum {
    STS_Fail          = 0,
    STS_Halt          = 1,
    STS_Step          = 2,
    STS_Trap          = 3,
    STS_Success       = 4,
    STS_StackOver     = 5,
    STS_DivideZero    = 6,
    STS_IllegalOpcode = 7
};

/* VM mailbox structure */
typedef struct {
    volatile uint32_t cmd;
    volatile uint32_t arg_sts;
    volatile uint32_t arg2_fcn;
} VM_Mailbox;

/* VM state structure */
typedef struct {
    volatile uint32_t *fp;
    volatile uint32_t *sp;
    volatile uint32_t tos;
    volatile uint8_t *pc;
    volatile uint32_t stepping;
} VM_State;

/* VM initialization structure */
typedef struct {
    VM_Mailbox *mailbox;
    VM_State *state;
    uint32_t *stack;
    uint32_t stackSize;
} VM_Init;

VM_Mailbox mailbox;
VM_State state;
uint32_t stack[32];

/* InitInterpreter - initialize the interpreter */
int InitInterpreter(Interpreter *i, size_t stackSize)
{
    extern uint32_t binary_ebasic_vm_dat_start[];
    VM_Init init;
    int cog;

    init.mailbox = &mailbox;
    init.state = &state;
    init.stack = stack;
    init.stackSize = sizeof(stack);
    
    mailbox.cmd = VM_Continue;
    
    if ((cog = cognew(binary_ebasic_vm_dat_start, &init)) < 0)
        return VMFALSE;
        
    while (mailbox.cmd != VM_Done)
        ;
        
    return VMTRUE;
}

/* Execute - execute the main code */
int Execute(Interpreter *i, ImageHdr *image)
{
    int running;
    
    state.pc = (uint8_t *)image->mainCode;
    state.stepping = 0;
    mailbox.cmd = VM_Continue;
        
    running = VMTRUE;
    while (running) {
    
        while (mailbox.cmd != VM_Done)
            ;
        
        switch (mailbox.arg_sts) {
        case STS_Fail:
            printf("Fail\n");
            running = VMFALSE;
            break;
        case STS_Halt:
            printf("Halt\n");
            running = VMFALSE;
            break;
        case STS_Step:
            printf("Step\n");
            running = VMFALSE;
            break;
        case STS_Trap:
            switch (mailbox.arg2_fcn) {
            case TRAP_GetChar:
                *--state.sp = state.tos;
                state.tos = getchar();
                mailbox.cmd = VM_Continue;
                break;
            case TRAP_PutChar:
                putchar(state.tos);
                state.tos = *state.sp++;
                mailbox.cmd = VM_Continue;
                break;
            case TRAP_PrintStr:
                puts((char *)state.tos);
                state.tos = *state.sp++;
                mailbox.cmd = VM_Continue;
                break;
            case TRAP_PrintInt:
                printf("%d", state.tos);
                state.tos = *state.sp++;
                mailbox.cmd = VM_Continue;
                break;
            case TRAP_PrintTab:
                putchar('\t');
                mailbox.cmd = VM_Continue;
                break;
            case TRAP_PrintNL:
                putchar('\n');
                mailbox.cmd = VM_Continue;
                break;
            case TRAP_PrintFlush:
                fflush(stdout);
                mailbox.cmd = VM_Continue;
                break;
            default:
                printf("Unknown trap\n");
                running = VMFALSE;
                break;
            }
            break;
        case STS_Success:
            printf("Success\n");
            running = VMFALSE;
            break;
        case STS_StackOver:
            printf("Stack overflow\n");
            running = VMFALSE;
            break;
        case STS_DivideZero:
            printf("Divide by zero\n");
            running = VMFALSE;
            break;
        case STS_IllegalOpcode:
            printf("Illegal opcode\n");
            running = VMFALSE;
            break;
        default:
            printf("Unknown status\n");
            running = VMFALSE;
            break;
        }
    }

    return 0;
}

void Abort(Interpreter *i, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    VM_printf("abort: ");
    VM_vprintf(fmt, ap);
    VM_putchar('\n');
    va_end(ap);
    if (i)
        longjmp(i->errorTarget, 1);
    else
        VM_sysexit();
}
