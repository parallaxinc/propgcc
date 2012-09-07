#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include "expr.h"

#define TRUE    1
#define FALSE   0

#define ID_MAX  32

#define TKN_NONE        0
#define TKN_EOF         -1
#define TKN_IDENTIFIER  -2
#define TKN_NUMBER      -3
#define TKN_SHL         -4
#define TKN_SHR         -5
#define TKN_LE          -6
#define TKN_EQ          -7
#define TKN_NE          -8
#define TKN_GE          -9
#define TKN_AND         -10
#define TKN_OR          -11

static void ParseExpr2(ParseContext *c, VALUE *pValue);
static void ParseExpr3(ParseContext *c, VALUE *pValue);
static void ParseExpr4(ParseContext *c, VALUE *pValue);
static void ParseExpr5(ParseContext *c, VALUE *pValue);
static void ParseExpr6(ParseContext *c, VALUE *pValue);
static void ParseExpr7(ParseContext *c, VALUE *pValue);
static void ParseExpr8(ParseContext *c, VALUE *pValue);
static void ParseExpr9(ParseContext *c, VALUE *pValue);
static void ParseExpr10(ParseContext *c, VALUE *pValue);
static void ParseExpr11(ParseContext *c, VALUE *pValue);
static void ParseExpr12(ParseContext *c, VALUE *pValue);
static void ParseExpr13(ParseContext *c, VALUE *pValue);
static void ParsePrimary(ParseContext *c, VALUE *pValue);
static int GetToken(ParseContext *c, VALUE *pValue);
static void ParseIdentifier(ParseContext *c, VALUE *pValue);
static void ParseNumber(ParseContext *c, VALUE *pValue);
static void Error(ParseContext *c, const char *fmt, ...);

/* ParseNumericExpr - parse a numeric expression */
int ParseNumericExpr(ParseContext *c, const char *str, VALUE *pValue)
{
    if (setjmp(c->errorTarget))
        return FALSE;
    c->linePtr = (char *)str;
    c->savedTkn = TKN_NONE;
    c->showErrors = TRUE;
    ParseExpr2(c, pValue);
    return TRUE;
}

/* TryParseNumericExpr - try to parse a numeric expression */
int TryParseNumericExpr(ParseContext *c, const char *str, VALUE *pValue)
{
    if (setjmp(c->errorTarget))
        return FALSE;
    c->linePtr = (char *)str;
    c->savedTkn = TKN_NONE;
    c->showErrors = FALSE;
    ParseExpr2(c, pValue);
    return TRUE;
}

/* ParseExpr2 - handle the '?:' operator */
static void ParseExpr2(ParseContext *c, VALUE *pValue)
{
    VALUE value2, value3;
    int tkn;
    ParseExpr3(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '?') {
        ParseExpr3(c, &value2);
        if (GetToken(c, &value2) != ':')
            Error(c, "expecting a colon");
        ParseExpr3(c, &value3);
        *pValue = *pValue ? value2 : value3;
    }
    c->savedTkn = tkn;
}

/* ParseExpr3 - handle the '||' operator */
static void ParseExpr3(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr4(c, pValue);
    while ((tkn = GetToken(c, &value2)) == TKN_OR) {
        ParseExpr4(c, &value2);
        *pValue = *pValue || value2;
    }
    c->savedTkn = tkn;
}

/* ParseExpr4 - handle the '&&' operator */
static void ParseExpr4(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr5(c, pValue);
    while ((tkn = GetToken(c, &value2)) == TKN_AND) {
        ParseExpr5(c, &value2);
        *pValue = *pValue && value2;
    }
    c->savedTkn = tkn;
}

/* ParseExpr5 - handle the '|' operator */
static void ParseExpr5(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr6(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '|') {
        ParseExpr6(c, &value2);
        *pValue = (VALUE)((int)*pValue | (int)value2);
    }
    c->savedTkn = tkn;
}

/* ParseExpr6 - handle the '^' operator */
static void ParseExpr6(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr7(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '^') {
        ParseExpr7(c, &value2);
        *pValue = (VALUE)((int)*pValue ^ (int)value2);
    }
    c->savedTkn = tkn;
}

/* ParseExpr7 - handle the '&' operator */
static void ParseExpr7(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr8(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '&') {
        ParseExpr8(c, &value2);
        *pValue = (VALUE)((int)*pValue & (int)value2);
    }
    c->savedTkn = tkn;
}

/* ParseExpr8 - handle the '==' and '!=' operators */
static void ParseExpr8(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr9(c, pValue);
    while ((tkn = GetToken(c, &value2)) == TKN_EQ || tkn == TKN_NE) {
        ParseExpr9(c, &value2);
        switch (tkn) {
        case TKN_EQ:
            *pValue = *pValue == value2;
            break;
        case TKN_NE:
            *pValue = *pValue != value2;
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedTkn = tkn;
}

/* ParseExpr9 - handle the '<', '<=', '>', and '>=' operators */
static void ParseExpr9(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr10(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '<' || tkn == TKN_LE || tkn == TKN_GE || tkn == '>') {
        ParseExpr10(c, &value2);
        switch (tkn) {
        case '<':
            *pValue = *pValue < value2;
            break;
        case TKN_LE:
            *pValue = *pValue <= value2;
            break;
        case TKN_GE:
            *pValue = *pValue >= value2;
            break;
        case '>':
            *pValue = *pValue > value2;
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedTkn = tkn;
}

/* ParseExpr10 - handle the '<<' and '>>' operators */
static void ParseExpr10(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr11(c, pValue);
    while ((tkn = GetToken(c, &value2)) == TKN_SHL || tkn == TKN_SHR) {
        ParseExpr11(c, &value2);
        switch (tkn) {
        case TKN_SHL:
            *pValue = (VALUE)((int)*pValue << (int)value2);
            break;
        case TKN_SHR:
            *pValue = (VALUE)((int)*pValue >> (int)value2);
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedTkn = tkn;
}

/* ParseExpr11 - handle the '+' and '-' operators */
static void ParseExpr11(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr12(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '+' || tkn == '-') {
        ParseExpr12(c, &value2);
        switch (tkn) {
        case '+':
            *pValue += value2;
            break;
        case '-':
            *pValue -= value2;
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedTkn = tkn;
}

/* ParseExpr12 - handle the '*', '/', and '%' operators */
static void ParseExpr12(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    int tkn;
    ParseExpr13(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '*' || tkn == '/' || tkn == '%') {
        ParseExpr13(c, &value2);
        switch (tkn) {
        case '*':
            *pValue *= value2;
            break;
        case '/':
            if (value2 == 0)
                Error(c, "division by zero");
            *pValue /= value2;
            break;
        case '%':
            if (value2 == 0)
                Error(c, "division by zero");
            *pValue = (VALUE)((int)*pValue % (int)value2);
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedTkn = tkn;
}

/* ParseExpr13 - handle unary operators */
static void ParseExpr13(ParseContext *c, VALUE *pValue)
{
    int tkn;
    switch (tkn = GetToken(c, pValue)) {
    case '+':
        ParsePrimary(c, pValue);
        break;
    case '-':
        ParsePrimary(c, pValue);
        *pValue = -*pValue;
        break;
    case '~':
        ParsePrimary(c, pValue);
        *pValue = (VALUE)~(int)*pValue;
        break;
    case '!':
        ParsePrimary(c, pValue);
        *pValue = !*pValue;
        break;
    default:
        c->savedTkn = tkn;
        ParsePrimary(c, pValue);
        break;
    }
}

/* ParsePrimary - parse a primary expression */
static void ParsePrimary(ParseContext *c, VALUE *pValue)
{
    VALUE value2;
    switch (GetToken(c, pValue)) {
    case '(':
        ParseExpr2(c, pValue);
        if (GetToken(c, &value2) != ')')
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

static int GetToken(ParseContext *c, VALUE *pValue)
{
    int tkn;
    
    if ((tkn = c->savedTkn) != TKN_NONE) {
        c->savedTkn = TKN_NONE;
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
        ParseNumber(c, pValue);
        tkn = TKN_NUMBER;
    }
    
    /* check for an identifier */
    else if (isalpha(*c->linePtr)) {
        ParseIdentifier(c, pValue);
        tkn = TKN_IDENTIFIER;
    }
    
    /* handle operators */
    else {
        switch (tkn = *c->linePtr) {
        case '<':
            if (c->linePtr[1] == '<') {
                tkn = TKN_SHL;
                ++c->linePtr;
            }
            else if (c->linePtr[1] == '=') {
                tkn = TKN_LE;
                ++c->linePtr;
            }
            break;
        case '>':
            if (c->linePtr[1] == '>') {
                tkn = TKN_SHR;
                ++c->linePtr;
            }
            else if (c->linePtr[1] == '=') {
                tkn = TKN_GE;
                ++c->linePtr;
            }
            break;
        case '!':
            if (c->linePtr[1] == '=') {
                tkn = TKN_NE;
                ++c->linePtr;
            }
            break;
        case '=':
            if (c->linePtr[1] == '=') {
                tkn = TKN_EQ;
                ++c->linePtr;
            }
            break;
        case '&':
            if (c->linePtr[1] == '&') {
                tkn = TKN_AND;
                ++c->linePtr;
            }
            break;
        case '|':
            if (c->linePtr[1] == '|') {
                tkn = TKN_OR;
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

static void ParseNumber(ParseContext *c, VALUE *pValue)
{
    *pValue = strtod(c->linePtr, (char **)&c->linePtr);
}

static void ParseIdentifier(ParseContext *c, VALUE *pValue)
{
    char id[ID_MAX];
    char *p = id;
    
    while (*c->linePtr != '\0' && (isalnum(*c->linePtr) || *c->linePtr == '_')) {
        if (p < id + ID_MAX - 1)
            *p++ = *c->linePtr;
        ++c->linePtr;
    }
    *p = '\0';
        
    if (!(*c->findSymbol)(c->cookie, id, pValue))
        Error(c, "undefined symbol: %s", id);
}

static void Error(ParseContext *c, const char *fmt, ...)
{
    if (c->showErrors) {
        va_list ap;
        va_start(ap, fmt);
        printf("error: ");
        vprintf(fmt, ap);
        putchar('\n');
        va_end(ap);
    }
    longjmp(c->errorTarget, 1);
}
typedef struct {
    char *name;
    VALUE value;
} Symbol;

static Symbol symbols[] = {
{   "PI",           3.14159     },
{   NULL,           0           }
};

int FindSymbol(void *cookie, const char *name, VALUE *pValue)
{    
    Symbol *sym;
    for (sym = symbols; sym->name != NULL; ++sym)
        if (strcasecmp(name, sym->name) == 0) {
            *pValue = sym->value;
            return TRUE;
        }
    return FALSE;
}

#ifdef MAIN

int main(int argc, char *argv[])
{
    ParseContext c;
    char buf[100];
    VALUE value;

    c.findSymbol = FindSymbol;
    c.cookie = NULL;

    for (;;) {
        printf("expr> ");
	if (!gets(buf))
            break;
        if (ParseNumericExpr(&c, buf, &value))
            printf(" --> %g\n", value);
    }
    
    return 0;
}

#endif
