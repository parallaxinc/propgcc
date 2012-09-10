#ifndef __EXPR_H__
#define __EXPR_H__

#include <stdint.h>
#include <setjmp.h>

/* forward type definitions */
typedef struct Variable Variable;

/* value type */
typedef double VALUE;

typedef struct {
    int type;
    union {
        VALUE value;
        Variable *var;
    } v;
} PVAL;

/* variable structure */
struct Variable {
    Variable *next;
    int bound;
    VALUE value;
    char name[1];
};

/* operator stack size */
#define OSTACK_SIZE 16

/* operand stack size */
#define RSTACK_SIZE 16

/* parse context */
typedef struct {
    int (*findSymbol)(void *cookie, const char *name, VALUE *pValue);
    void *cookie;
    jmp_buf errorTarget;
    char *linePtr;
    int savedToken;
#ifdef USE_SHUNTING_YARD_ALGORITHM
    int oStack[OSTACK_SIZE];
    int *oStackPtr;
    int *oStackTop;
    PVAL rStack[RSTACK_SIZE];
    PVAL *rStackPtr;
    PVAL *rStackTop;
#endif
    Variable *variables;
    uint8_t *base;      /* base of heap data */
    uint8_t *free;      /* next free heap location */
    uint8_t *top;       /* top of heap */
} EvalState;

/* defined in expr.c */
void InitEvalState(EvalState *c, uint8_t *heap, size_t heapSize);
int EvalExpr(EvalState *c, const char *token, VALUE *pValue);

#endif
