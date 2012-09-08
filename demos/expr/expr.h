#ifndef __EXPR_H__
#define __EXPR_H__

#include <stdint.h>
#include <setjmp.h>

/* value type */
typedef double VALUE;

/* variable structure */
typedef struct Variable Variable;
struct Variable {
    Variable *next;
    int bound;
    VALUE value;
    char name[1];
};

/* parse context */
typedef struct {
    int (*findSymbol)(void *cookie, const char *name, VALUE *pValue);
    void *cookie;
    jmp_buf errorTarget;
    char *linePtr;
    int savedToken;
    Variable *variables;
    uint8_t *base;      /* base of heap data */
    uint8_t *free;      /* next free heap location */
    uint8_t *top;       /* top of heap */
} EvalState;

/* defined in expr.c */
void InitEvalState(EvalState *c, uint8_t *heap, size_t heapSize);
int EvalExpr(EvalState *c, const char *token, VALUE *pValue);

#endif
