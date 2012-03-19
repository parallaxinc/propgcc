/* config.c - an elf and spin binary loader for the Parallax Propeller microcontroller

Copyright (c) 2011 David Michael Betz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "config.h"
#include "system.h"

#ifdef NEED_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif

#define MAXLINE 128

typedef struct Field Field;
struct Field {
    char *tag;
    char *value;
    Field *next;
};

struct BoardConfig {
    BoardConfig *parent;
    BoardConfig *sibling;
    BoardConfig *child;
    BoardConfig **pNextChild;
    Field *fields;
    char name[1];
};

typedef struct {
    char lineBuf[MAXLINE];  /* line buffer */
    char *linePtr;          /* pointer to the current character */
    int lineNumber;         /* current line number */
} LineBuf;

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
{   "RCFAST",       0x00        },
{   "RCSLOW",       0x01        },
{   "XINPUT",       0x20        },
{   "XTAL1",        0x28        },
{   "XTAL2",        0x30        },
{   "XTAL3",        0x38        },
{   "PLL1X",        0x43        },
{   "PLL2X",        0x44        },
{   "PLL4X",        0x45        },
{   "PLL8X",        0x46        },
{   "PLL16X",       0x47        },
{   "K",            1024        },
{   "M",            1024*1024   },
{   "MHZ",          1000*1000   },
{   "TRUE",         TRUE        },
{   "FALSE",        FALSE       },
{   NULL,           0           }
};

static BoardConfig *GetDefaultConfiguration(void);
static int SkipSpaces(LineBuf *buf);
static char *NextToken(LineBuf *buf, const char *termSet, int *pTerm);
static int ParseNumericExpr(char *token, int *pValue);
static int DoOp(int *pValue, int op, int left, int right);
static void ParseError(LineBuf *buf, const char *fmt, ...);
static int Error(const char *fmt, ...);
static void Fatal(const char *fmt, ...);

BoardConfig *NewBoardConfig(BoardConfig *parent, const char *name)
{
    BoardConfig *config;
    if (!(config = (BoardConfig *)malloc(sizeof(BoardConfig) + strlen(name))))
        Fatal("insufficient memory");
    memset(config, 0, sizeof(BoardConfig));
    strcpy(config->name, name);
    config->parent = parent;
    config->pNextChild = &config->child;
    if (parent) {
        *parent->pNextChild = config;
        parent->pNextChild = &config->sibling;
    }
    return config;
}

/* ParseConfigurationFile - parse a configuration file */
BoardConfig *ParseConfigurationFile(System *sys, const char *name)
{
    char path[PATH_MAX];
    BoardConfig *baseConfig, *config;
    char *tag, *value, *dst;
    const char *src;
    LineBuf buf;
    FILE *fp;
    int ch;
    
    /* make a local copy of the name in lowercase */
    src = name; dst = path;
    while ((*dst++ = tolower(*src++)) != '\0')
        ;
    
    /* check for a request for the default configuration */
    if (strcmp(path, DEF_BOARD) == 0)
        return GetDefaultConfiguration();
    
    /* make the configuration file name */
    strcat(path, ".cfg");

    /* open the configuration file */
    if (!(fp = xbOpenFileInPath(sys, path, "r")))
        return NULL;

    /* create a new board configuration */
    baseConfig = config = NewBoardConfig(NULL, name);
        
    /* process each line in the configuration file */
    while (fgets(buf.lineBuf, sizeof(buf.lineBuf), fp)) {
    
        /* initialize token parser */
        buf.linePtr = buf.lineBuf;
        ++buf.lineNumber;
        
        /* look for the first token on the line */
        switch (SkipSpaces(&buf)) {
        
        case '\n':  /* blank line */
        case '#':   /* comment */
            // ignore blank lines and comments
            break;
            
        case '[':   /* configuration tag */
        
            /* get the configuration name */
            ++buf.linePtr;
            if (!(tag = NextToken(&buf, "]", &ch)))
                ParseError(&buf, "missing configuration tag");
            if (ch != ']') {
                if (SkipSpaces(&buf) != ']')
                    ParseError(&buf, "missing close bracket after configuration tag");
                ++buf.linePtr;
            }
            if (SkipSpaces(&buf) != '\n')
                ParseError(&buf, "missing end of line");
                
            /* add a new board configuration */
            config = NewBoardConfig(baseConfig, tag);
            break;

        default:    /* tag:value pair */
        
            /* get the tag */
            if (!(tag = NextToken(&buf, ":", &ch)))
                ParseError(&buf, "missing tag");
                
            /* check for the colon separator */
            if (ch != ':') {
                if (SkipSpaces(&buf) != ':')
                    ParseError(&buf, "missing colon");
                ++buf.linePtr;
            }
            
            /* get the value */
            if (!(value = NextToken(&buf, "", &ch)))
                ParseError(&buf, "missing value");
                
            /* make sure this is the end of line */
            if (ch != '\n') {
                if ((ch = SkipSpaces(&buf) != '\n') && ch != '#')
                    ParseError(&buf, "missing end of line");
                ++buf.linePtr;
            }
            
            /* set the configuration value */
            SetConfigField(config, tag, value);
            break;
        }
    }

    /* close the board configuration file */
    fclose(fp);
    
    /* return the board configuration */
    return config;
}

/* GetConfigSubtype - get a subtype of a board configuration */
BoardConfig *GetConfigSubtype(BoardConfig *config, const char *name)
{
    BoardConfig *subconfig;
    for (subconfig = config->child; subconfig != NULL; subconfig = subconfig->sibling)
        if (strcasecmp(name, subconfig->name) == 0)
            return subconfig;
    return strcasecmp(name, DEF_SUBTYPE) == 0 ? config : NULL;
}

void SetConfigField(BoardConfig *config, char *tag, char *value)
{
    Field **pNext, *field;
    int taglen;
    for (pNext = &config->fields; (field = *pNext) != NULL; pNext = &field->next)
        if (strcasecmp(tag, field->tag) == 0) {
            *pNext = field->next;
            free(field);
            break;
        }
    taglen = strlen(tag) + 1;
    if (!(field = (Field *)malloc(sizeof(Field) + taglen + strlen(value) + 1)))
        Fatal("insufficient memory");
    field->tag = (char *)field + sizeof(Field);
    field->value = field->tag + taglen;
    strcpy(field->tag, tag);
    strcpy(field->value, value);
    field->next = *pNext;
    *pNext = field;
}

char *GetConfigField(BoardConfig *config, char *tag)
{
    while (config != NULL) {
        Field *field;
        for (field = config->fields; field != NULL; field = field->next)
            if (strcasecmp(tag, field->tag) == 0)
                return field->value;
        config = config->parent;
    }
    return NULL;
}

int GetNumericConfigField(BoardConfig *config, char *tag, int *pValue)
{
    char *value;
    if (!(value = GetConfigField(config, tag)))
        return FALSE;
    return ParseNumericExpr(value, pValue);
}

BoardConfig *MergeConfigs(BoardConfig *parent, BoardConfig *child)
{
    child->parent = parent;
    return child;
}

static BoardConfig *GetDefaultConfiguration(void)
{
    static BoardConfig *defaultConfig = NULL;
    if (!defaultConfig) {
        defaultConfig = NewBoardConfig(NULL, DEF_BOARD);
        SetConfigField(defaultConfig, "clkfreq",  "80000000");
        SetConfigField(defaultConfig, "clkmode",  "XTAL1+PLL16X");
        SetConfigField(defaultConfig, "baudrate", "115200");
        SetConfigField(defaultConfig, "rxpin",    "31");
        SetConfigField(defaultConfig, "txpin",    "30");
    }
    return defaultConfig;
}

static int SkipSpaces(LineBuf *buf)
{
    int ch;
    while ((ch = *buf->linePtr) != '\0' && ch != '#' && ch != '\n' && isspace(ch))
         ++buf->linePtr;
    if (ch == '#') {
        while ((ch = *++buf->linePtr) != '\0' && ch != '\n')
            ;
    }
    return *buf->linePtr;
}

static char *NextToken(LineBuf *buf, const char *termSet, int *pTerm)
{
    char *token;
    int ch;

    /* skip leading spaces */
    if (!SkipSpaces(buf))
        return NULL;

    /* collect the token */
    token = buf->linePtr;
    while ((ch = *buf->linePtr) != '\0' && ch != '\n' && !isspace(ch) && !strchr(termSet, ch))
        ++buf->linePtr;

    /* return the terminator character */
    *pTerm = ch;

    /* terminate the token */
    if (*buf->linePtr != '\0')
        *buf->linePtr++ = '\0';

    /* return the token or NULL if at the end of the line */
    return *token == '\0' ? NULL : token;
}

static int SkipSpacesStr(char **pp)
{
    char *p = *pp;
    while (*p != '\0' && isspace(*p))
        ++p;
    *pp = p;
    return *p;
}

static int ParseNumericExpr(char *token, int *pValue)
{
    char *p = token;
    int value = 0;
    int op = -1;
    *pValue = 0; // makes unary + and - work
    while (SkipSpacesStr(&p)) {
        if (isdigit(*p)) {
            value = (int)strtol(p, &p, 0);
            switch (*p) {
            case 'k':
            case 'K':
                value *= 1024;
                ++p;
                break;
            case 'm':
            case 'M':
                if (strcasecmp(p, "MHZ") == 0) {
                    value *= 1000 * 1000;
                    p += 3;
                }
                else {
                    value *= 1024 * 1024;
                    ++p;
                }
                break;
            default:
                // nothing to do
                break;
            }
            if (!DoOp(pValue, op, *pValue, value))
                return FALSE;
            op = -1;
        }
        else if (isalpha(*p)) {
            char id[32], *p2 = id;
            ConfigSymbol *sym;
            while (*p != '\0' && isalnum(*p)) {
                if (p2 < id + sizeof(id) - 1)
                    *p2++ = *p;
                ++p;
            }
            *p2 = '\0';
            for (sym = configSymbols; sym->name != NULL; ++sym)
                if (strcasecmp(id, sym->name) == 0) {
                    if (!DoOp(pValue, op, *pValue, sym->value))
                        return FALSE;
                    op = -1;
                    break;
                }
            if (!sym->name) {
                Error("undefined symbol: %s", id);
                return FALSE;
            }
        }
        else {
            switch (*p) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '&':
            case '|':
                op = *p;
                break;
            default:
                Error("unknown operator: %c", *p);
                return FALSE;
            }
            ++p;
        }
    }
    return *p == '\0';
}

static int DoOp(int *pValue, int op, int left, int right)
{
    switch (op) {
    case -1:
        left = right;
        break;
    case '+':
        left += right;
        break;
    case '-':
        left -= right;
        break;
    case '*':
        left *= right;
        break;
    case '/':
        left /= right;
        break;
    case '%':
        left %= right;
        break;
    case '&':
        left &= right;
        break;
    case '|':
        left |= right;
        break;
    default:
        Error("unknown operator: %c", op);
        return FALSE;
    }
    *pValue = left;
    return TRUE;
}

static void ParseError(LineBuf *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    if (buf)
        fprintf(stderr, "  on line number %d\n", buf->lineNumber);
    va_end(ap);
    exit(1);
}

static int Error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
    return FALSE;
}

static void Fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
    exit(1);
}

