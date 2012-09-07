#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include "expr.h"

#define TRUE    1
#define FALSE   0

#define ID_MAX      32

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

static void ParseExpr2(ParseContext *c, int *pValue);
static void ParseExpr3(ParseContext *c, int *pValue);
static void ParseExpr4(ParseContext *c, int *pValue);
static void ParseExpr5(ParseContext *c, int *pValue);
static void ParseExpr6(ParseContext *c, int *pValue);
static void ParseExpr7(ParseContext *c, int *pValue);
static void ParseExpr8(ParseContext *c, int *pValue);
static void ParseExpr9(ParseContext *c, int *pValue);
static void ParseExpr10(ParseContext *c, int *pValue);
static void ParseExpr11(ParseContext *c, int *pValue);
static void ParseExpr12(ParseContext *c, int *pValue);
static void ParseExpr13(ParseContext *c, int *pValue);
static void ParsePrimary(ParseContext *c, int *pValue);
static int GetToken(ParseContext *c, int *pValue);
static void ParseIdentifier(ParseContext *c, int *pValue);
static void ParseConfigVariable(ParseContext *c, int *pValue);
static void ParseNumber(ParseContext *c, int *pValue);
static void Error(ParseContext *c, const char *fmt, ...);

/* ParseNumericExpr - parse a numeric expression */
int ParseNumericExpr(ParseContext *c, const char *str, int *pValue)
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
int TryParseNumericExpr(ParseContext *c, const char *str, int *pValue)
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
static void ParseExpr2(ParseContext *c, int *pValue)
{
    int value2, value3, tkn;
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
static void ParseExpr3(ParseContext *c, int *pValue)
{
    int value2, tkn;
    ParseExpr4(c, pValue);
    while ((tkn = GetToken(c, &value2)) == TKN_OR) {
        ParseExpr4(c, &value2);
        *pValue = *pValue || value2;
    }
    c->savedTkn = tkn;
}

/* ParseExpr4 - handle the '&&' operator */
static void ParseExpr4(ParseContext *c, int *pValue)
{
    int value2, tkn;
    ParseExpr5(c, pValue);
    while ((tkn = GetToken(c, &value2)) == TKN_AND) {
        ParseExpr5(c, &value2);
        *pValue = *pValue && value2;
    }
    c->savedTkn = tkn;
}

/* ParseExpr5 - handle the '|' operator */
static void ParseExpr5(ParseContext *c, int *pValue)
{
    int value2, tkn;
    ParseExpr6(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '|') {
        ParseExpr6(c, &value2);
        *pValue |= value2;
    }
    c->savedTkn = tkn;
}

/* ParseExpr6 - handle the '^' operator */
static void ParseExpr6(ParseContext *c, int *pValue)
{
    int value2, tkn;
    ParseExpr7(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '^') {
        ParseExpr7(c, &value2);
        *pValue ^= value2;
    }
    c->savedTkn = tkn;
}

/* ParseExpr7 - handle the '&' operator */
static void ParseExpr7(ParseContext *c, int *pValue)
{
    int value2, tkn;
    ParseExpr8(c, pValue);
    while ((tkn = GetToken(c, &value2)) == '&') {
        ParseExpr8(c, &value2);
        *pValue &= value2;
    }
    c->savedTkn = tkn;
}

/* ParseExpr8 - handle the '==' and '!=' operators */
static void ParseExpr8(ParseContext *c, int *pValue)
{
    int value2, tkn;
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
static void ParseExpr9(ParseContext *c, int *pValue)
{
    int value2, tkn;
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
static void ParseExpr10(ParseContext *c, int *pValue)
{
    int value2, tkn;
    ParseExpr11(c, pValue);
    while ((tkn = GetToken(c, &value2)) == TKN_SHL || tkn == TKN_SHR) {
        ParseExpr11(c, &value2);
        switch (tkn) {
        case TKN_SHL:
            *pValue <<= value2;
            break;
        case TKN_SHR:
            *pValue >>= value2;
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedTkn = tkn;
}

/* ParseExpr11 - handle the '+' and '-' operators */
static void ParseExpr11(ParseContext *c, int *pValue)
{
    int value2, tkn;
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
static void ParseExpr12(ParseContext *c, int *pValue)
{
    int value2, tkn;
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
            *pValue %= value2;
            break;
        default:
            /* never reached */
            break;
        }
    }
    c->savedTkn = tkn;
}

/* ParseExpr13 - handle unary operators */
static void ParseExpr13(ParseContext *c, int *pValue)
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
        *pValue = ~*pValue;
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
static void ParsePrimary(ParseContext *c, int *pValue)
{
    int value2;
    switch (GetToken(c, pValue)) {
    case '(':
        ParseExpr2(c, pValue);
        if (GetToken(c, &value2) != ')')
            Error(c, "expecting a right paren");
        break;
    case '{':
        ParseConfigVariable(c, pValue);
        if (GetToken(c, &value2) != '}')
            Error(c, "expecting a right brace");
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

static int GetToken(ParseContext *c, int *pValue)
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
    if (isdigit(*c->linePtr)) {
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

static void ParseNumber(ParseContext *c, int *pValue)
{
    int value = (int)strtol(c->linePtr, (char **)&c->linePtr, 0);
    switch (*c->linePtr) {
    case 'k':
    case 'K':
        value *= 1024;
        ++c->linePtr;
        break;
    case 'm':
    case 'M':
        if (strncasecmp(c->linePtr, "mhz", 3) == 0) {
            value *= 1000 * 1000;
            c->linePtr += 3;
        }
        else {
            value *= 1024 * 1024;
            ++c->linePtr;
        }
        break;
    default:
        // nothing to do
        break;
    }
    *pValue = value;
}

static void ParseIdentifier(ParseContext *c, int *pValue)
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

static void ParseConfigVariable(ParseContext *c, int *pValue)
{
    char id[ID_MAX];
    char *p = id;
    
    /* skip leading spaces */
    while (*c->linePtr != '\0' && isspace(*c->linePtr))
        ++c->linePtr;

    while (*c->linePtr != '\0' && !isspace(*c->linePtr) && *c->linePtr != '}') {
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
#ifdef MAIN

typedef struct {
    char *name;
    int value;
} ConfigSymbol;

#define RCFAST      0x00
#define RCSLOW      0x01
#define XINPUT      0x22
#define XTAL1       0x2a
#define XTAL2       0x32
#define XTAL3       0x3a
#define PLL1X       0x41
#define PLL2X       0x42
#define PLL4X       0x43
#define PLL8X       0x44
#define PLL16X      0x45

static ConfigSymbol configSymbols[] = {
{   "RCFAST",       RCFAST      },
{   "RCSLOW",       RCSLOW      },
{   "XINPUT",       XINPUT      },
{   "XTAL1",        XTAL1       },
{   "XTAL2",        XTAL2       },
{   "XTAL3",        XTAL3       },
{   "PLL1X",        PLL1X       },
{   "PLL2X",        PLL2X       },
{   "PLL4X",        PLL4X       },
{   "PLL8X",        PLL8X       },
{   "PLL16X",       PLL16X      },
{   "K",            1024        },
{   "M",            1024*1024   },
{   "MHZ",          1000*1000   },
{   "TRUE",         TRUE        },
{   "FALSE",        FALSE       },
{   "SDSPI-DI",     12          },
{   "SDSPI-CLK",    13          },
{   "SDSPI-DO",     14          },
{   "SDSPI-CS",     15          },
{   NULL,           0           }
};

int FindSymbol(void *cookie, const char *name, int *pValue)
{    
    ConfigSymbol *sym;
    for (sym = configSymbols; sym->name != NULL; ++sym)
        if (strcasecmp(name, sym->name) == 0) {
            *pValue = sym->value;
            return TRUE;
        }
    return FALSE;
}

int main(int argc, char *argv[])
{
    ParseContext c;
    char buf[100];
    int value;

    while (gets(buf)) {
        c.findSymbol = FindSymbol;
        c.cookie = NULL;
        if (ParseNumericExpr(&c, buf, &value))
            printf("%s -> %d\n", buf, value);
    }
    
    return 0;
}

#endif
