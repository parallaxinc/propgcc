#include <stdio.h>
#include "vmimage.h"
#include <propeller.h>

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

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

/* VM trap codes */
enum {
    TRAP_GetChar      = 0,
    TRAP_PutChar      = 1,
    TRAP_PrintStr     = 2,
    TRAP_PrintInt     = 3,
    TRAP_PrintTab     = 4,
    TRAP_PrintNL      = 5,
    TRAP_PrintFlush   = 6
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

uint8_t image[] = {
    OP_SLIT, 'H',
    OP_TRAP, TRAP_PutChar,
    OP_SLIT, 'e',
    OP_TRAP, TRAP_PutChar,
    OP_SLIT, 'l',
    OP_TRAP, TRAP_PutChar,
    OP_SLIT, 'l',
    OP_TRAP, TRAP_PutChar,
    OP_SLIT, 'o',
    OP_TRAP, TRAP_PutChar,
    OP_SLIT, '!',
    OP_TRAP, TRAP_PutChar,
    OP_TRAP, TRAP_PrintNL,
    OP_SLIT, 123,
    OP_TRAP, TRAP_PrintInt,
    OP_TRAP, TRAP_PrintNL,
    OP_HALT
};

uint32_t stack[32];

int main(void)
{
    extern uint32_t binary_ebasic_vm_dat_start[];
    int running, cog;
    VM_Init init;

    init.mailbox = &mailbox;
    init.state = &state;
    init.stack = stack;
    init.stackSize = sizeof(stack);
    
    mailbox.cmd = VM_Continue;
    

    if ((cog = cognew(binary_ebasic_vm_dat_start, &init)) < 0)
        return -1;
        
    while (mailbox.cmd != VM_Done)
        ;

    state.pc = image;
    state.stepping = 0;
    mailbox.cmd = VM_Continue;
        
    running = TRUE;
    while (running) {
    
        while (mailbox.cmd != VM_Done)
            ;
        
        switch (mailbox.arg_sts) {
        case STS_Fail:
            printf("Fail\n");
            running = FALSE;
            break;
        case STS_Halt:
            printf("Halt\n");
            running = FALSE;
            break;
        case STS_Step:
            printf("Step\n");
            running = FALSE;
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
                running = FALSE;
                break;
            }
            break;
        case STS_Success:
            printf("Success\n");
            running = FALSE;
            break;
        case STS_StackOver:
            printf("Stack overflow\n");
            running = FALSE;
            break;
        case STS_DivideZero:
            printf("Divide by zero\n");
            running = FALSE;
            break;
        case STS_IllegalOpcode:
            printf("Illegal opcode\n");
            running = FALSE;
            break;
        default:
            printf("Unknown status\n");
            running = FALSE;
            break;
        }
    }

    return 0;
}
