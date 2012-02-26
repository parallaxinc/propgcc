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
#ifdef LOAD_SAVE
static void DoLoad(ParseContext *e);
static void DoSave(ParseContext *e);
#endif
static void DoList(ParseContext *e);
static void DoRun(ParseContext *e);

/* command table */
static struct {
    char *name;
    void (*handler)(ParseContext *e);
} cmds[] = {
    {   "NEW",   DoNew   },
#ifdef LOAD_SAVE
    {   "LOAD",  DoLoad  },
    {   "SAVE",  DoSave  },
#endif
    {   "LIST",  DoList  },
    {   "RUN",   DoRun   },
    {   NULL,    NULL    }
};

/* memory of the last filename */
#ifdef LOAD_SAVE
static DATA_SPACE char programName[MAX_PROG_NAME] = "";
#endif

/* prototypes */
static int EditGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber);
static char *NextToken(ParseContext *e);
static int ParseNumber(ParseContext *e, char *token, VMVALUE *pValue);
static int IsBlank(char *p);

void EditWorkspace(ParseContext *c)
{
    VMVALUE lineNumber;
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

static int SetProgramName(ParseContext *c)
{
    char *name;
    if ((name = NextToken(c)) != NULL) {
        strncpy(programName, name, MAX_PROG_NAME - 1);
        programName[MAX_PROG_NAME - 1] = '\0';
    }
    return programName[0] != '\0';
}

static void DoNew(ParseContext *c)
{
    /* check for a program name on the command line */
    SetProgramName(c);
    BufInit();
}

#ifdef LOAD_SAVE

static void DoLoad(ParseContext *c)
{
    VMFILE *fp;
    
    /* check for a program name on the command line */
    if (!SetProgramName(c)) {
        VM_printf("expecting a file name\n");
        return;
    }
    
    /* load the program */
    if (!(fp = VM_fopen(programName, "r")))
        VM_printf("error loading '%s'\n", programName);
    else {
        VM_printf("Loading '%s'\n", programName);
        BufInit();
        while (VM_fgets(c->lineBuf, sizeof(c->lineBuf), fp) != NULL) {
            int len = strlen(c->lineBuf);
            int16_t lineNumber;
            char *token;
            c->linePtr = c->lineBuf;
            if ((token = NextToken(c)) != NULL) {
                if (ParseNumber(c, token, &lineNumber))
                    BufAddLineN(lineNumber, c->linePtr);
                else
                    VM_printf("expecting a line number: %s\n", token);
            }
        }
        VM_fclose(fp);
    }
}

static void DoSave(ParseContext *c)
{
    VMFILE *fp;
    
    /* check for a program name on the command line */
    if (!SetProgramName(c)) {
        VM_printf("expecting a file name\n");
        return;
    }
    
    /* save the program */
    if (!(fp = VM_fopen(programName, "w")))
        VM_printf("error saving '%s'\n", programName);
    else {
        VMVALUE lineNumber;
        VM_printf("Saving '%s'\n", programName);
        BufSeekN(0);
        while (BufGetLine(&lineNumber, c->lineBuf)) {
            char buf[32];
            sprintf(buf, "%d ", lineNumber);
            VM_fputs(buf, fp);
            VM_fputs(c->lineBuf, fp);
        }
        VM_fclose(fp);
    }
}

#endif

static void DoList(ParseContext *c)
{
    VMVALUE lineNumber;
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

static int EditGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber)
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
        size_t stackSize = (c->freeTop - c->freeNext - sizeof(Interpreter)) / sizeof(VMVALUE);
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

static int ParseNumber(ParseContext *c, char *token, VMVALUE *pValue)
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


