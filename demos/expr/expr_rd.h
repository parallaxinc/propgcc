#ifndef __EXPR_H__
#define __EXPR_H__

#include <stdint.h>
#include <setjmp.h>

/* forward type definitions */
typedef struct EvalState EvalState;
typedef struct Variable Variable;
typedef struct Function Function;

/* value type */
typedef double VALUE;

#define TYPE_NUMBER     1
#define TYPE_VARIABLE   2
#define TYPE_FUNCTION   3

/* partial value structure */
typedef struct {
    int type;
    union {
        VALUE value;
        Variable *var;
        Function *fcn;
    } v;
} PVAL;

/* variable structure */
struct Variable {
    Variable *next;
    int bound;
    VALUE value;
    char name[1];
};

/* function structure */
struct Function {
    char *name;
    int argc;
    void (*fcn)(EvalState *c);
};

/* operator stack size */
#define OSTACK_SIZE 16

/* operand stack size */
#define RSTACK_SIZE 16

/* parse context */
struct EvalState {
    int (*findSymbol)(void *cookie, const char *name, VALUE *pValue);
    void *cookie;
    jmp_buf errorTarget;
    char *linePtr;
    int savedToken;
    Variable *variables;
    uint8_t *base;      /* base of heap data */
    uint8_t *free;      /* next free heap location */
    uint8_t *top;       /* top of heap */
};

/* defined in expr.c */
void InitEvalState(EvalState *c, uint8_t *heap, size_t heapSize);
int EvalExpr(EvalState *c, const char *token, VALUE *pValue);

#endif
