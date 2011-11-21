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

#include "config.h"
#include "system.h"

#define MAXLINE 128

typedef struct {
    char lineBuf[MAXLINE];  /* line buffer */
    char *linePtr;          /* pointer to the current character */
    int lineNumber;         /* current line number */
} LineBuf;

typedef struct {
    char *name;
    int value;
} ConfigSymbol;

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

/* board configurations */
static BoardConfig *boardConfigs = NULL;

static BoardConfig *SetupDefaultConfiguration(void);
static int SkipSpaces(LineBuf *buf);
static char *NextToken(LineBuf *buf, const char *termSet, int *pTerm);
static int ParseNumericExpr(char *token, int *pValue);
static int DoOp(int *pValue, int op, int left, int right);
static char *CopyString(const char *str);
static void ParseError(LineBuf *buf, const char *fmt, ...);
static int Error(const char *fmt, ...);
static void Fatal(const char *fmt, ...);

BoardConfig *NewBoardConfig(const char *name)
{
    BoardConfig *config;
    if (!(config = (BoardConfig *)malloc(sizeof(BoardConfig) + strlen(name))))
        Fatal("insufficient memory");
    memset(config, 0, sizeof(BoardConfig));
    strcpy(config->name, name);
    return config;
}

BoardConfig *GetBoardConfig(const char *name)
{
    BoardConfig *config;
    for (config = boardConfigs; config != NULL; config = config->next)
        if (strcasecmp(name, config->name) == 0)
            return config;
    return NULL;
}

/* ParseConfigurationFile - parse a configuration file */
void ParseConfigurationFile(System *sys, const char *path)
{
    BoardConfig *config = NULL;
    BoardConfig **pNextConfig;
    char *tag, *value;
    LineBuf buf;
    FILE *fp;
    int ch;

    boardConfigs = SetupDefaultConfiguration();
    pNextConfig = &boardConfigs->next;
        
    if (!(fp = xbOpenFileInPath(sys, path, "r")))
        return;

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
            
        case '[':   /* board tag */
        
            /* get the board name */
            ++buf.linePtr;
            if (!(tag = NextToken(&buf, "]", &ch)))
                ParseError(&buf, "missing board tag");
            if (ch != ']') {
                if (SkipSpaces(&buf) != ']')
                    ParseError(&buf, "missing close bracket after board tag");
                ++buf.linePtr;
            }
            if (SkipSpaces(&buf) != '\n')
                ParseError(&buf, "missing end of line");
                
            /* add a new board configuration */
            if (!(config = NewBoardConfig(tag)))
                ParseError(&buf, "insufficient memory");
            *pNextConfig = config;
            pNextConfig = &config->next;
            break;
            
        default:    /* tag:value pair */
        
            /* make sure we're in a board configuration */
            if (!config)
                ParseError(&buf, "not in a board configuration");
                
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

    fclose(fp);
}

static int chktag(char *tag, char *chktag)
{
    return strcasecmp(tag, chktag) == 0;
}

static void SetNumericField(BoardConfig *config, char *value, uint32_t *pValue, int flag)
{
    int iValue;
    if (ParseNumericExpr(value, &iValue)) {
        *pValue = (uint32_t)iValue;
        config->validMask |= flag;
    }
}

static void SetStringField(BoardConfig *config, char *value, char **pValue, int flag)
{
    if (*pValue)
        free(*pValue);
    *pValue = CopyString(value);
    config->validMask |= flag;
}

int SetConfigField(BoardConfig *config, char *tag, char *value)
{
         if (chktag(tag, "clkfreq"))       SetNumericField(config, value, &config->clkfreq,     VALID_CLKFREQ);
    else if (chktag(tag, "clkmode"))       SetNumericField(config, value, &config->clkmode,     VALID_CLKMODE);
    else if (chktag(tag, "baudrate"))      SetNumericField(config, value, &config->baudrate,    VALID_BAUDRATE);
    else if (chktag(tag, "rxpin"))         SetNumericField(config, value, &config->rxpin,       VALID_RXPIN);
    else if (chktag(tag, "txpin"))         SetNumericField(config, value, &config->txpin,       VALID_TXPIN);
    else if (chktag(tag, "tvpin"))         SetNumericField(config, value, &config->tvpin,       VALID_TVPIN);
    else if (chktag(tag, "cache-driver"))  SetStringField (config, value, &config->cacheDriver, VALID_CACHEDRIVER);
    else if (chktag(tag, "cache-size"))    SetNumericField(config, value, &config->cacheSize,   VALID_CACHESIZE);
    else if (chktag(tag, "cache-param1"))  SetNumericField(config, value, &config->cacheParam1, VALID_CACHEPARAM1);
    else if (chktag(tag, "cache-param2"))  SetNumericField(config, value, &config->cacheParam2, VALID_CACHEPARAM2);
    else if (chktag(tag, "sd-driver"))     SetStringField (config, value, &config->sdDriver,    VALID_SDDRIVER);
    else if (chktag(tag, "sdspi-do"))      SetNumericField(config, value, &config->sdspiDO,     VALID_SDSPIDO);
    else if (chktag(tag, "sdspi-clk"))     SetNumericField(config, value, &config->sdspiClk,    VALID_SDSPICLK);
    else if (chktag(tag, "sdspi-di"))      SetNumericField(config, value, &config->sdspiDI,     VALID_SDSPIDI);
    else if (chktag(tag, "sdspi-cs"))      SetNumericField(config, value, &config->sdspiCS,     VALID_SDSPICS);
    else if (chktag(tag, "sdspi-clr"))     SetNumericField(config, value, &config->sdspiClr,    VALID_SDSPICLR);
    else if (chktag(tag, "sdspi-inc"))     SetNumericField(config, value, &config->sdspiInc,    VALID_SDSPIINC);
    else if (chktag(tag, "sdspi-sel"))     SetNumericField(config, value, &config->sdspiSel,    VALID_SDSPISEL);
    else if (chktag(tag, "sdspi-msk"))     SetNumericField(config, value, &config->sdspiMsk,    VALID_SDSPIMSK);
    else if (chktag(tag, "eeprom-first"))  SetNumericField(config, value, &config->eepromFirst, VALID_EEPROMFIRST);
    else {
        Error("unknown board configuration variable: %s", tag);
        return FALSE;
    }
    return TRUE;
}

void MergeConfigs(BoardConfig *dst, BoardConfig *src)
{
    if (src->validMask & VALID_CLKFREQ) {
        dst->validMask |= VALID_CLKFREQ;
        dst->clkfreq = src->clkfreq;
    }
    if (src->validMask & VALID_CLKMODE) {
        dst->validMask |= VALID_CLKMODE;
        dst->clkmode = src->clkmode;
    }
    if (src->validMask & VALID_BAUDRATE) {
        dst->validMask |= VALID_BAUDRATE;
        dst->baudrate = src->baudrate;
    }
    if (src->validMask & VALID_RXPIN) {
        dst->validMask |= VALID_RXPIN;
        dst->rxpin = src->rxpin;
    }
    if (src->validMask & VALID_TXPIN) {
        dst->validMask |= VALID_TXPIN;
        dst->txpin = src->txpin;
    }
    if (src->validMask & VALID_TVPIN) {
        dst->validMask |= VALID_TVPIN;
        dst->tvpin = src->tvpin;
    }
    if (src->validMask & VALID_CACHEDRIVER) {
        dst->validMask |= VALID_CACHEDRIVER;
        if (dst->cacheDriver)
            free(dst->cacheDriver);
        dst->cacheDriver = CopyString(src->cacheDriver);
    }
    if (src->validMask & VALID_CACHESIZE) {
        dst->validMask |= VALID_CACHESIZE;
        dst->cacheSize = src->cacheSize;
    }
    if (src->validMask & VALID_CACHEPARAM1) {
        dst->validMask |= VALID_CACHEPARAM1;
        dst->cacheParam1 = src->cacheParam1;
    }
    if (src->validMask & VALID_CACHEPARAM2) {
        dst->validMask |= VALID_CACHEPARAM2;
        dst->cacheParam2 = src->cacheParam2;
    }
    if (src->validMask & VALID_SDDRIVER) {
        dst->validMask |= VALID_SDDRIVER;
        if (dst->sdDriver)
            free(dst->sdDriver);
        dst->sdDriver = CopyString(src->sdDriver);
    }
    if (src->validMask & VALID_SDSPIDO) {
        dst->validMask |= VALID_SDSPIDO;
        dst->sdspiDO = src->sdspiDO;
    }
    if (src->validMask & VALID_SDSPICLK) {
        dst->validMask |= VALID_SDSPICLK;
        dst->sdspiClk = src->sdspiClk;
    }
    if (src->validMask & VALID_SDSPIDI) {
        dst->validMask |= VALID_SDSPIDI;
        dst->sdspiDI = src->sdspiDI;
    }
    if (src->validMask & VALID_SDSPICS) {
        dst->validMask &= ~(VALID_SDSPICLR | VALID_SDSPIINC);
        dst->validMask |= VALID_SDSPICS;
        dst->sdspiCS = src->sdspiCS;
    }
    if (src->validMask & VALID_SDSPICLR) {
        dst->validMask &= ~VALID_SDSPICS;
        dst->validMask |= VALID_SDSPICLR;
        dst->sdspiClr = src->sdspiClr;
    }
    if (src->validMask & VALID_SDSPIINC) {
        dst->validMask &= ~(VALID_SDSPISEL | VALID_SDSPIMSK);
        dst->validMask |= VALID_SDSPIINC;
        dst->sdspiInc = src->sdspiInc;
    }
    if (src->validMask & VALID_SDSPISEL) {
        dst->validMask &= ~(VALID_SDSPICLR | VALID_SDSPIINC);
        dst->validMask |= VALID_SDSPISEL;
        dst->sdspiSel = src->sdspiSel;
    }
    if (src->validMask & VALID_SDSPIMSK) {
        dst->validMask &= ~(VALID_SDSPICLR | VALID_SDSPIINC);
        dst->validMask |= VALID_SDSPIMSK;
        dst->sdspiMsk = src->sdspiMsk;
    }
    if (src->validMask & VALID_EEPROMFIRST) {
        dst->validMask |= VALID_EEPROMFIRST;
        dst->eepromFirst = src->eepromFirst;
    }
}

static BoardConfig *SetupDefaultConfiguration(void)
{
    BoardConfig *config;
    
    if (!(config = (BoardConfig *)malloc(sizeof(BoardConfig) + strlen(DEF_NAME))))
        Fatal("insufficient memory");
        
    memset(config, 0, sizeof(BoardConfig));
    config->validMask = VALID_CLKFREQ | VALID_CLKMODE | VALID_BAUDRATE | VALID_RXPIN | VALID_TXPIN | VALID_TVPIN
                      | VALID_SDDRIVER | VALID_SDSPIDO | VALID_SDSPICLK | VALID_SDSPIDI | VALID_SDSPICS;
    config->clkfreq = DEF_CLKFREQ;
    config->clkmode = DEF_CLKMODE;
    config->baudrate = DEF_BAUDRATE;
    config->rxpin = DEF_RXPIN;
    config->txpin = DEF_TXPIN;
    config->tvpin = DEF_TVPIN;
    config->sdDriver = CopyString("sd_driver.dat");
    config->sdspiDO = DEF_SDSPIDO;
    config->sdspiClk = DEF_SDSPICLK;
    config->sdspiDI = DEF_SDSPIDI;
    config->sdspiCS = DEF_SDSPICS;
    strcpy(config->name, DEF_NAME);
    
    return config;
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
            if (!sym) {
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

static char *CopyString(const char *str)
{
    char *copy = (char *)malloc(strlen(str) + 1);
    if (!copy)
        Fatal("insufficient memory");
    strcpy(copy, str);
    return copy;
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

