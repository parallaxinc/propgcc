#ifndef __EXPR_H__
#define __EXPR_H__

#include <setjmp.h>

/* value type */
typedef double VALUE;

/* parse context */
typedef struct {
    int (*findSymbol)(void *cookie, const char *name, VALUE *pValue);
    void *cookie;
    jmp_buf errorTarget;
    char *linePtr;
    int savedTkn;
    int showErrors;
} ParseContext;

/* defined in expr.c */
int ParseNumericExpr(ParseContext *c, const char *token, VALUE *pValue);
int TryParseNumericExpr(ParseContext *c, const char *str, VALUE *pValue);
int FindSymbol(void *cookie, const char *name, VALUE *pValue);

#endif
