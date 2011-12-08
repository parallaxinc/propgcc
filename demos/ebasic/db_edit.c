#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "db_edit.h"
#include "db_vm.h"

#ifdef WIN32
#define strcasecmp  _stricmp
#endif

/* command handlers */
static void DoNew(ParseContext *e);
static void DoList(ParseContext *e);
static void DoRun(ParseContext *e);

/* command table */
static struct {
    char *name;
    void (*handler)(ParseContext *e);
} cmds[] = {
    {   "NEW",   DoNew   },
    {   "LIST",  DoList  },
    {   "RUN",   DoRun   },
    {   NULL,    NULL    }
};

/* prototypes */
static int EditGetLine(void *cookie, char *buf, int len, int16_t *pLineNumber);
static char *NextToken(ParseContext *e);
static int ParseNumber(ParseContext *e, char *token, int16_t *pValue);
static int IsBlank(char *p);

void EditWorkspace(ParseContext *c)
{
    int16_t lineNumber;
    char *token;

    BufInit();

    c->getLine = EditGetLine;
    c->getLineCookie = c;
    
    VM_printf("ebasic 0.001\n");

    for (;; ) {
        
        VM_getline(c->lineBuf, sizeof(c->lineBuf));

        c->linePtr = c->lineBuf;

        if ((token = NextToken(c)) != NULL) {
            if (ParseNumber(c, token, &lineNumber)) {
                if (IsBlank(c->linePtr)) {
                    if (!BufDeleteLineN(lineNumber))
                        VM_printf("no line %d\n", lineNumber);
                }
                else if (!BufAddLineN(lineNumber, c->linePtr))
                    VM_printf("out of edit buffer space\n");
            }

            else {
                int i;
                for (i = 0; cmds[i].name != NULL; ++i)
                    if (strcasecmp(token, cmds[i].name) == 0)
                        break;
                    if (cmds[i].handler) {
                        (*cmds[i].handler)(c);
                        VM_printf("OK\n");
                    }
                else
                    VM_printf("unknown command: %s\n", token);
            }
        }
    }
}

static void DoNew(ParseContext *c)
{
    BufInit();
}

static void DoList(ParseContext *c)
{
    int16_t lineNumber;
    BufSeekN(0);
    while (BufGetLine(&lineNumber, c->lineBuf))
        VM_printf("%d %s", lineNumber, c->lineBuf);
}

char *prog[] = {
//  "for x=1 to 10\n",
//  "printf(\"%d %d\n\", x, x*x)\n",
//  "next x\n",
    "for\n",
    0
};
int prog_i;

static int EditGetLine(void *cookie, char *buf, int len, int16_t *pLineNumber)
{
#if 0
    if (!prog[prog_i])
        return VMFALSE;
    strcpy(buf, prog[prog_i++]);
    *pLineNumber = prog_i;
    return VMTRUE;
#else
    return BufGetLine(pLineNumber, buf);
#endif
}

static void DoRun(ParseContext *c)
{
    BufSeekN(0);
prog_i = 0;
    if (Compile(c, MAXOBJECTS)) {
        Interpreter *i = (Interpreter *)c->freeNext;
        size_t stackSize = (c->freeTop - c->freeNext - sizeof(Interpreter)) / sizeof(int16_t);
        if (stackSize <= 0)
            VM_printf("insufficient memory\n");
        else {
            InitInterpreter(i, stackSize);
            Execute(i, c->image);
        }
    }
}

static char *NextToken(ParseContext *c)
{
    char *token;
    int ch;
    while ((ch = *c->linePtr) != '\0' && isspace(ch))
        ++c->linePtr;
    token = c->linePtr;
    while ((ch = *c->linePtr) != '\0' && !isspace(ch))
        ++c->linePtr;
    if (*c->linePtr != '\0')
        *c->linePtr++ = '\0';
    return *token == '\0' ? NULL : token;
}

static int ParseNumber(ParseContext *c, char *token, int16_t *pValue)
{
    int ch;
    *pValue = 0;
    while ((ch = *token++) != '\0' && isdigit(ch))
        *pValue = *pValue * 10 + ch - '0';
    return ch == '\0';
}

static int IsBlank(char *p)
{
    int ch;
    while ((ch = *p++) != '\0')
        if (!isspace(ch))
            return VMFALSE;
    return VMTRUE;
}


