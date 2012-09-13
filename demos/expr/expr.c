#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "expr.h"

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
#define TKN_FCALL       -7
#define TKN_FCALL_ARGS  -8

/* operator associativities */
#define ASSOC_LEFT      1
#define ASSOC_RIGHT     2

/* operator stack macros */
#define oStackIsEmpty(c)    ((c)->oStackPtr < (c)->oStack)
#define oStackPush(c,v)     do {                                            \
                                if ((c)->oStackPtr >= (c)->oStackTop)       \
                                    Error(c, "operator stack overflow");    \
                                ++(c)->oStackPtr;                           \
                                (c)->oStackPtr->op = (v);                   \
                            } while (0)
#define oStackPushData(c,v) do {                                            \
                                if ((c)->oStackPtr >= (c)->oStackTop)       \
                                    Error(c, "operator stack overflow");    \
                                ++(c)->oStackPtr;                           \
                                (c)->oStackPtr->data = (v);                 \
                            } while (0)
#define oStackTop(c)        ((c)->oStackPtr->op)
#define oStackTopData(c)    ((c)->oStackPtr->data)
#define oStackDrop(c)       (--(c)->oStackPtr)
                        
/* operand stack macros */
#define rStackIsEmpty(c)    ((c)->rStackPtr < (c)->rStack)
#define rStackPush(c,v)     do {                                            \
                                if ((c)->rStackPtr >= (c)->rStackTop)       \
                                    Error(c, "operand stack overflow");     \
                                *++(c)->rStackPtr = (v);                    \
                            } while (0)
#define rStackCount(c)      (((c)->rStackPtr - (c)->rStack) + 1)
#define rStackDrop(c)       ((c)->rStackPtr--)

static int PopAndEvaluate(EvalState *c);
static int Prec(EvalState *c, int op);
static int Assoc(int op);
static int Unary(int op);
static void Apply(EvalState *c, int op);
static void ApplyUnary(EvalState *c, int op, PVAL *pval);
static void ApplyBinary(EvalState *c, int op, PVAL *left, PVAL *right);
static void CallFunction(EvalState *c);
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
    c->oStackTop = (oEntry *)((char *)c->oStack + sizeof(c->oStack));
    c->rStackTop = (PVAL *)((char *)c->rStack + sizeof(c->rStack));
    c->base = heap;
    c->free = heap;
    c->top = heap + heapSize;
}

/* EvalExpr - Eval and evaluate an expression using the shunting yard algorithm */
int EvalExpr(EvalState *c, const char *str, VALUE *pValue)
{
    int unaryPossible = TRUE;
    int tkn, count, prec, op;
    PVAL pval;
    
    /* setup an error target */
    if (setjmp(c->errorTarget))
        return FALSE;
        
    /* initialize the parser */
    c->linePtr = (char *)str;
    c->savedToken = TKN_NONE;
    
    /* initialize the operator and operand stacks */
    c->oStackPtr = c->oStack - 1;
    c->rStackPtr = c->rStack - 1;
    
    /* handle each input token */
    while ((tkn = GetToken(c, &pval)) != TKN_EOF) {
        switch (tkn) {
        case TKN_IDENTIFIER:
        case TKN_NUMBER:
            rStackPush(c, pval);
            unaryPossible = FALSE;
            break;
        case TKN_FCALL:
            oStackPush(c, c->argc);
            oStackPushData(c, c->fcn);
            oStackPush(c, tkn);
            c->fcn = pval.v.fcn;
            c->argc = 0;
            break;
        case '(':
            if (oStackTop(c) == TKN_FCALL)
                c->oStackPtr->op = TKN_FCALL_ARGS;
            else
                oStackPush(c, tkn);
            unaryPossible = TRUE;
            break;
        case ',':
            if (PopAndEvaluate(c) != TKN_FCALL_ARGS)
                Error(c, "argument list outside of a function call");
            unaryPossible = FALSE;
            break;
        case ')':
            tkn = PopAndEvaluate(c);
            oStackDrop(c);
            if (tkn == TKN_FCALL || tkn == TKN_FCALL_ARGS)
                CallFunction(c);
            unaryPossible = FALSE;
            break;
        default:
            if (unaryPossible && tkn == '-')
                tkn = TKN_UNARY_MINUS;
            prec = Prec(c, tkn);
            while (!oStackIsEmpty(c)) {
                int stackPrec = Prec(c, oStackTop(c));
                if ((Assoc(tkn) == ASSOC_LEFT && prec > stackPrec) || prec >= stackPrec)
                    break;
                op = oStackTop(c);
                oStackDrop(c);
                if (op == TKN_FCALL)
                    CallFunction(c);
                else
                    Apply(c, op);
            }
            oStackPush(c, tkn);
            unaryPossible = TRUE;
            break;
        }
    }
    
    /* apply all of the remaining operands on the operator stack */
    while (!oStackIsEmpty(c)) {
        int op = oStackTop(c);
        oStackDrop(c);
        if (op == '(')
            Error(c, "mismatched parens - 1");
        if (op == TKN_FCALL)
            CallFunction(c);
        else
            Apply(c, op);
    }
    
    /* if the operand stack is empty then there was no expression to parse */
    if ((count = rStackCount(c)) == 0)
        return FALSE;
        
    /* otherwise, make sure there is only one entry left on the operand stack */
    else if (count != 1)
        Error(c, "syntax error - 1");
    
    /* return the expression value */
    RValue(c, &c->rStackPtr[0]);
    *pValue = c->rStackPtr[0].v.value;
    
    /* return successfully */
    return TRUE;
}

/* PopAndEvaluate - pop operators and apply them until a '(' or TKN_FCALL token is found */
static int PopAndEvaluate(EvalState *c)
{
    int tkn;
    for (;;) {
        if (oStackIsEmpty(c))
            Error(c, "mismatched parens - 2");
        if ((tkn = oStackTop(c)) == '(' || tkn == TKN_FCALL_ARGS)
            break;
        oStackDrop(c);
        Apply(c, tkn);
    }
    RValue(c, c->rStackPtr);
    if (tkn == TKN_FCALL_ARGS)
        ++c->argc;
    return tkn;
}

/* Prec - return the precedence of an operator */
static int Prec(EvalState *c, int op)
{
    int precedence;
    switch (op) {
    case TKN_FCALL_ARGS:
        precedence = 0;
        break;
    case '(':
        precedence = 1;
        break;
    case '=':
        precedence = 2;
        break;
    case '|':
        precedence = 3;
        break;
    case '^':
        precedence = 4;
        break;
    case '&':
        precedence = 5;
        break;
    case TKN_SHL:
    case TKN_SHR:
        precedence = 6;
        break;
    case '+':
    case '-':
        precedence = 7;
        break;
    case '*':
    case '/':
    case '%':
        precedence = 8;
        break;
    case TKN_UNARY_MINUS:
    case '~':
        precedence = 9;
        break;
    case TKN_FCALL:
        precedence = 10;
        break;
    default:
        Error(c, "unknown operator -- '%c' (%d)", op, op);
        break;
    }
    return precedence;
}

/* Assoc - return the associativity of an operator */
static int Assoc(int op)
{
    int associativity;
    switch (op) {
    case '=':
    case TKN_UNARY_MINUS:
    case '~':
    case TKN_FCALL:
    case TKN_FCALL_ARGS:
        associativity = ASSOC_RIGHT;
        break;
    default:
        associativity = ASSOC_LEFT;
        break;
    }
    return associativity;
}

/* Unary - determine if a unary operator */
static int Unary(int op)
{
    int unary;
    switch (op) {
    case TKN_UNARY_MINUS:
    case '~':
    case TKN_FCALL:
    case TKN_FCALL_ARGS:
        unary = TRUE;
        break;
    default:
        unary = FALSE;
    }
    return unary;
}

/* Apply - apply an operator to operands */
static void Apply(EvalState *c, int op)
{
    if (Unary(op)) {
        PVAL *pval;
        if (rStackCount(c) < 1)
            Error(c, "syntax error - 2");
        pval = &c->rStackPtr[0];
        ApplyUnary(c, op, pval);
    }
    else {
        PVAL *left, *right;
        if (rStackCount(c) < 2)
            Error(c, "syntax error - 3");
        left = &c->rStackPtr[-1];
        right = &c->rStackPtr[0];
        ApplyBinary(c, op, left, right);
        rStackDrop(c);
    }
}

/* ApplyUnary - apply a unary operator to an operand */
static void ApplyUnary(EvalState *c, int op, PVAL *pval)
{
    RValue(c, pval);
    switch (op) {
    case TKN_UNARY_MINUS:
        pval->v.value = -pval->v.value;
        break;
    case '~':
        pval->v.value = (VALUE)~((int)pval->v.value);
        break;
    case TKN_FCALL:
    case TKN_FCALL_ARGS:
        break;
    default:
        Error(c, "internal error in UnaryApply -- '%c' (%d)", op, op);
        break;
    }
}

/* ApplyBinary - apply a binary operator to operands */
static void ApplyBinary(EvalState *c, int op, PVAL *left, PVAL *right)
{
    RValue(c, right);
    if (op == '=') {
        if (left->type != TYPE_VARIABLE)
            Error(c, "expecting a variable to the left of '='");
        left->v.var->value = right->v.value;
        left->v.var->bound = TRUE;
        left->type = TYPE_NUMBER;
        left->v.value = right->v.value;
    }
    else {
        RValue(c, left);
        switch (op) {
        case '|':
            left->v.value = (VALUE)((int)left->v.value | (int)right->v.value);
            break;
        case '^':
            left->v.value = (VALUE)((int)left->v.value ^ (int)right->v.value);
            break;
        case '&':
            left->v.value = (VALUE)((int)left->v.value & (int)right->v.value);
            break;
        case TKN_SHL:
            left->v.value = (VALUE)((int)left->v.value << (int)right->v.value);
            break;
        case TKN_SHR:
            left->v.value = (VALUE)((int)left->v.value >> (int)right->v.value);
            break;
        case '+':
            left->v.value += right->v.value;
            break;
        case '-':
            left->v.value -= right->v.value;
            break;
        case '*':
            left->v.value *= right->v.value;
            break;
        case '/':
            if ((int)right->v.value == 0)
                Error(c, "division by zero");
            left->v.value /= right->v.value;
            break;
        case '%':
            if ((int)left->v.value == 0)
                Error(c, "division by zero");
            left->v.value = (VALUE)((int)left->v.value % (int)right->v.value);
            break;
        default:
            Error(c, "internal error in ApplyFunction -- '%c' (%d)", op, op);
            break;
        }
    }
}

/* CallFunction - call the function on the top of the operator stack */
static void CallFunction(EvalState *c)
{
    Function *fcn;
    int argc;
    fcn = oStackTopData(c);
    oStackDrop(c);
    argc = oStackTop(c);
    oStackDrop(c);
    if (c->argc < c->fcn->argc)
        Error(c, "too few arguments");
    else if (c->argc > c->fcn->argc)
        Error(c, "too many arguments");
    (*c->fcn->fcn)(c);
    c->fcn = fcn;
    c->argc = argc;
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

static int GetToken(EvalState *c, PVAL *pval)
{
    int tkn;
    
    /* check for a saved token */
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

static void fcn_sin(EvalState *c)
{
    c->rStackPtr->v.value = sin(c->rStackPtr->v.value);
}

static void fcn_cos(EvalState *c)
{
    c->rStackPtr->v.value = cos(c->rStackPtr->v.value);
}

static void fcn_tan(EvalState *c)
{
    c->rStackPtr->v.value = tan(c->rStackPtr->v.value);
}

static void fcn_asin(EvalState *c)
{
    c->rStackPtr->v.value = asin(c->rStackPtr->v.value);
}

static void fcn_acos(EvalState *c)
{
    c->rStackPtr->v.value = acos(c->rStackPtr->v.value);
}

static void fcn_atan(EvalState *c)
{
    c->rStackPtr->v.value = atan(c->rStackPtr->v.value);
}

static void fcn_sqrt(EvalState *c)
{
    c->rStackPtr->v.value = sqrt(c->rStackPtr->v.value);
}

static void fcn_exp(EvalState *c)
{
    c->rStackPtr->v.value = exp(c->rStackPtr->v.value);
}

static void fcn_ln(EvalState *c)
{
    c->rStackPtr->v.value = log(c->rStackPtr->v.value);
}

static void fcn_roll(EvalState *c)
{
    PVAL pval;
    pval.type = TYPE_NUMBER;
    pval.v.value = (double)(rand() % 6 + 1);
    rStackPush(c, pval);
}

static Function functions[] = {
{   "sin",      1,  fcn_sin     },
{   "cos",      1,  fcn_cos     },
{   "tan",      1,  fcn_tan     },
{   "asin",     1,  fcn_asin    },
{   "acos",     1,  fcn_acos    },
{   "atan",     1,  fcn_atan    },
{   "sqrt",     1,  fcn_sqrt    },
{   "exp",      1,  fcn_exp     },
{   "ln",       1,  fcn_ln      },
{   "roll",     0,  fcn_roll    },
{   NULL,       0,  NULL        }
};

static int ParseIdentifier(EvalState *c, PVAL *pval)
{
    char id[ID_MAX];
    char *p = id;
    Function *fcn;
    
    /* parse the identifier */
    while (*c->linePtr != '\0' && (isalnum(*c->linePtr) || *c->linePtr == '_')) {
        if (p < id + ID_MAX - 1)
            *p++ = *c->linePtr;
        ++c->linePtr;
    }
    *p = '\0';
    
    /* check for a function name */
    for (fcn = functions; fcn->name != NULL; ++fcn)
        if (strcasecmp(id, fcn->name) == 0) {
            pval->type = TYPE_FUNCTION;
            pval->v.fcn = fcn;
            return TKN_FCALL;
        }
        
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
