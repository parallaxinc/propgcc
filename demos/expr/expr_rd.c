#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "expr_rd.h"

#define TRUE    1
#define FALSE   0

#define ID_MAX  32

#define TKN_NONE        0
#define TKN_EOF         -1
#define TKN_IDENTIFIER  -2
#define TKN_NUMBER      -3
#define TKN_UNARY_MINUS -4
#define TKN_SHL         -5
#define TKN_SHR         -6
#define TKN_SIN         -7
#define TKN_COS         -8
#define TKN_TAN         -9
#define TKN_SQRT        -10
#define TKN_EXP         -11

#define TYPE_NUMBER     1
#define TYPE_VARIABLE   2

static void EvalExpr1(EvalState *c, PVAL *pval);
static void EvalExpr2(EvalState *c, PVAL *pval);
static void EvalExpr3(EvalState *c, PVAL *pval);
static void EvalExpr4(EvalState *c, PVAL *pval);
static void EvalExpr5(EvalState *c, PVAL *pval);
static void EvalExpr6(EvalState *c, PVAL *pval);
static void EvalExpr7(EvalState *c, PVAL *pval);
static void EvalExpr8(EvalState *c, PVAL *pval);
static void EvalPrimary(EvalState *c, PVAL *pval);
static void RValue(EvalState *c, PVAL *pval);
static int GetToken(EvalState *c, PVAL *pval);
static int ParseIdentifier(EvalState *c, PVAL *pval);
static void ParseNumber(EvalState *c, PVAL *pval);
static int AddVariable(EvalState *c, char *id, PVAL *pval);
static Variable *FindVariable(EvalState *c, char *id);
static void Error(EvalState *c, const char *fmt, ...);

/* InitEvalState - initialize the expression evaluator state */
void InitEvalState(EvalState *c, uint8_t *heap, size_t heapSize)
{
    memset(c, 0, sizeof(EvalState));
    c->base = heap;
    c->free = heap;
    c->top = heap + heapSize;
}

/* EvalExpr - Eval and evaluate an expression */
int EvalExpr(EvalState *c, const char *str, VALUE *pValue)
{
    PVAL pval;
    
    /* setup an error target */
    if (setjmp(c->errorTarget))
        return FALSE;
        
    /* initialize the parser */
    c->linePtr = (char *)str;
    c->savedToken = TKN_NONE;
    
    /* evaluate an expression */
    EvalExpr1(c, &pval);
    
    /* return the value */
    RValue(c, &pval);
    *pValue = pval.v.value;
    
    /* make sure there isn't any junk at the end of the line */
    if (GetToken(c, &pval) != TKN_EOF)
        Error(c, "invalid expression");
        
    /* return successfully */
    return TRUE;
}

/* EvalExpr1 - handle the '=' operator */
static void EvalExpr1(EvalState *c, PVAL *pval)
{
    PVAL pval2;
    int tkn;
    EvalExpr2(c, pval);
    while ((tkn = GetToken(c, &pval2)) == '=') {
        if (pval->type != TYPE_VARIABLE)
            Error(c, "expecting a variable to the left of '='");
        EvalExpr1(c, &pval2);
        RValue(c, &pval2);
        pval->v.var->value = pval2.v.value;
        pval->v.var->bound = TRUE;
        pval->type = TYPE_NUMBER;
        pval->v.value = pval2.v.value;
    }
    c->savedToken = tkn;
}

/* EvalExpr2 - handle the '|' operator */
static void EvalExpr2(EvalState *c, PVAL *pval)
{
    PVAL pval2;
    int tkn;
    EvalExpr3(c, pval);
    while ((tkn = GetToken(c, &pval2)) == '|') {
        RValue(c, pval);
        EvalExpr3(c, &pval2);
        RValue(c, &pval2);
        pval->v.value = (VALUE)((int)pval->v.value | (int)pval2.v.value);
    }
    c->savedToken = tkn;
}

/* EvalExpr3 - handle the '^' operator */
static void EvalExpr3(EvalState *c, PVAL *pval)
{
    PVAL pval2;
    int tkn;
    EvalExpr4(c, pval);
    while ((tkn = GetToken(c, &pval2)) == '^') {
        RValue(c, pval);
        EvalExpr4(c, &pval2);
        RValue(c, &pval2);
        pval->v.value = (VALUE)((int)pval->v.value ^ (int)pval2.v.value);
    }
    c->savedToken = tkn;
}

/* EvalExpr4 - handle the '&' operator */
static void EvalExpr4(EvalState *c, PVAL *pval)
{
    PVAL pval2;
    int tkn;
    EvalExpr5(c, pval);
    while ((tkn = GetToken(c, &pval2)) == '&') {
        RValue(c, pval);
        EvalExpr5(c, &pval2);
        RValue(c, &pval2);
        pval->v.value = (VALUE)((int)pval->v.value & (int)pval2.v.value);
    }
    c->savedToken = tkn;
}

/* EvalExpr5 - handle the '<<' and '>>' operators */
static void EvalExpr5(EvalState *c, PVAL *pval)
{
    PVAL pval2;
    int tkn;
    EvalExpr6(c, pval);
    while ((tkn = GetToken(c, &pval2)) == TKN_SHL || tkn == TKN_SHR) {
        RValue(c, pval);
        EvalExpr6(c, &pval2);
        RValue(c, &pval2);
        switch (tkn) {
        case TKN_SHL:
            pval->v.value = (VALUE)((int)pval->v.value << (int)pval2.v.value);
            break;
        case TKN_SHR:
            pval->v.value = (VALUE)((int)pval->v.value >> (int)pval2.v.value);
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedToken = tkn;
}

/* EvalExpr6 - handle the '+' and '-' operators */
static void EvalExpr6(EvalState *c, PVAL *pval)
{
    PVAL pval2;
    int tkn;
    EvalExpr7(c, pval);
    while ((tkn = GetToken(c, &pval2)) == '+' || tkn == '-') {
        RValue(c, pval);
        EvalExpr7(c, &pval2);
        RValue(c, &pval2);
        switch (tkn) {
        case '+':
            pval->v.value += pval2.v.value;
            break;
        case '-':
            pval->v.value -= pval2.v.value;
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedToken = tkn;
}

/* EvalExpr7 - handle the '*', '/', and '%' operators */
static void EvalExpr7(EvalState *c, PVAL *pval)
{
    PVAL pval2;
    int tkn;
    EvalExpr8(c, pval);
    while ((tkn = GetToken(c, &pval2)) == '*' || tkn == '/' || tkn == '%') {
        RValue(c, pval);
        EvalExpr8(c, &pval2);
        RValue(c, &pval2);
        switch (tkn) {
        case '*':
            pval->v.value *= pval2.v.value;
            break;
        case '/':
            if (pval2.v.value == 0)
                Error(c, "division by zero");
            pval->v.value /= pval2.v.value;
            break;
        case '%':
            if ((int)pval2.v.value == 0)
                Error(c, "division by zero");
            pval->v.value = (VALUE)((int)pval->v.value % (int)pval2.v.value);
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedToken = tkn;
}

/* EvalExpr8 - handle unary operators */
static void EvalExpr8(EvalState *c, PVAL *pval)
{
    int tkn;
    switch (tkn = GetToken(c, pval)) {
    case '-':
        EvalPrimary(c, pval);
        RValue(c, pval);
        pval->v.value = -pval->v.value;
        break;
    case '~':
        EvalPrimary(c, pval);
        RValue(c, pval);
        pval->v.value = (VALUE)~(int)pval->v.value;
        break;
    default:
        c->savedToken = tkn;
        EvalPrimary(c, pval);
        break;
    }
}

/* EvalPrimary - Eval a primary expression */
static void EvalPrimary(EvalState *c, PVAL *pval)
{
    PVAL pval2;
    switch (GetToken(c, pval)) {
    case '(':
        EvalExpr2(c, pval);
        if (GetToken(c, &pval2) != ')')
            Error(c, "expecting a right paren");
        break;
    case TKN_NUMBER:
    case TKN_IDENTIFIER:
        /* nothing else to do */
        break;
    default:
        Error(c, "expecting a primary expression");
        break;
    }
}

static void RValue(EvalState *c, PVAL *pval)
{
    switch (pval->type) {
    case TYPE_NUMBER:
        /* nothing to do */
        break;
    case TYPE_VARIABLE:
        if (!pval->v.var->bound)
            Error(c, "%s has no value", pval->v.var->name);
        pval->type = TYPE_NUMBER;
        pval->v.value = pval->v.var->value;
        break;
    default:
        Error(c, "internal error - RValue");
    }
}

static struct {
    char *name;
    int tkn;
} keywords[] = {
{   "sin",      TKN_SIN     },
{   "cos",      TKN_COS     },
{   "tan",      TKN_TAN     },
{   "sqrt",     TKN_SQRT    },
{   "exp",      TKN_EXP     },
{   NULL,       TKN_NONE    }
};

static int GetToken(EvalState *c, PVAL *pval)
{
    int tkn;
    
    if ((tkn = c->savedToken) != TKN_NONE) {
        c->savedToken = TKN_NONE;
        return tkn;
    }
    
    /* skip leading spaces */
    while (*c->linePtr != '\0' && isspace(*c->linePtr))
        ++c->linePtr;
        
    /* check for end of file (string) */
    if (*c->linePtr == '\0')
        return TKN_EOF;
        
    /* check for a number */
    if (*c->linePtr == '.' || isdigit(*c->linePtr)) {
        ParseNumber(c, pval);
        tkn = TKN_NUMBER;
    }
    
    /* check for an identifier */
    else if (isalpha(*c->linePtr))
        tkn = ParseIdentifier(c, pval);
    
    /* handle operators */
    else {
        switch (tkn = *c->linePtr) {
        case '<':
            if (c->linePtr[1] == '<') {
                tkn = TKN_SHL;
                ++c->linePtr;
            }
            break;
        case '>':
            if (c->linePtr[1] == '>') {
                tkn = TKN_SHR;
                ++c->linePtr;
            }
            break;
        default:
            /* nothing to do */
            break;
        }
        ++c->linePtr;
    }
    
    return tkn;
}

static void ParseNumber(EvalState *c, PVAL *pval)
{
    pval->type = TYPE_NUMBER;
    pval->v.value = strtod(c->linePtr, (char **)&c->linePtr);
}

static int ParseIdentifier(EvalState *c, PVAL *pval)
{
    char id[ID_MAX];
    char *p = id;
    int i;
    
    /* parse the identifier */
    while (*c->linePtr != '\0' && (isalnum(*c->linePtr) || *c->linePtr == '_')) {
        if (p < id + ID_MAX - 1)
            *p++ = *c->linePtr;
        ++c->linePtr;
    }
    *p = '\0';
    
    /* check for a function keyword */
    for (i = 0; keywords[i].name != NULL; ++i)
        if (strcasecmp(id, keywords[i].name) == 0)
             return keywords[i].tkn;
        
    /* check for an application symbol reference */
    if ((*c->findSymbol)(c->cookie, id, &pval->v.value))
        pval->type = TYPE_NUMBER;
    
    /* check for a variable reference */
    else if (!AddVariable(c, id, pval))
        Error(c, "insufficient variable space");
        
    /* return an identifier */
    return TKN_IDENTIFIER;
}

static int AddVariable(EvalState *c, char *id, PVAL *pval)
{
    Variable *var;
    
    /* find or add a variable */
    if (!(var = FindVariable(c, id))) {
        size_t size = sizeof(Variable) + strlen(id);
        if (c->free + size > c->top)
            return FALSE;
        var = (Variable *)c->free;
        c->free += size;
        strcpy(var->name, id);
        var->next = c->variables;
        var->bound = FALSE;
        c->variables = var;
    }

    /* return the successfully */
    pval->type = TYPE_VARIABLE;
    pval->v.var = var;
    return TRUE;
}

static Variable *FindVariable(EvalState *c, char *id)
{
    Variable *var;
    for (var = c->variables; var != NULL; var = var->next)
        if (strcmp(id, var->name) == 0)
            return var;
    return NULL;
}

static void Error(EvalState *c, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("error: ");
    vprintf(fmt, ap);
    putchar('\n');
    va_end(ap);
    longjmp(c->errorTarget, 1);
}

#ifdef MAIN

typedef struct {
    char *name;
    VALUE value;
} Symbol;

static Symbol symbols[] = {
{   "PI",           3.141592653589      },
{   "E",            2.718281828459      },
{   NULL,           0                   }
};

static int FindSymbol(void *cookie, const char *name, VALUE *pValue)
{    
    Symbol *sym;
    for (sym = symbols; sym->name != NULL; ++sym)
        if (strcasecmp(name, sym->name) == 0) {
            *pValue = sym->value;
            return TRUE;
        }
    return FALSE;
}

int main(int argc, char *argv[])
{
    EvalState c;
    uint8_t heap[512];
    char buf[100];
    VALUE value;

    InitEvalState(&c, heap, sizeof(heap));
    c.findSymbol = FindSymbol;
    c.cookie = NULL;

    for (;;) {
        printf("expr> ");
        if (!gets(buf))
            break;
        if (EvalExpr(&c, buf, &value))
            printf(" --> %g\n", value);
    }
    
    return 0;
}

#endif
