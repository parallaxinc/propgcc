#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "db_edit.h"
#include "db_compiler.h"
#include "db_vm.h"

#ifdef WIN32
#define strcasecmp  _stricmp
#endif

/* command handlers */
static void DoNew(System *sys);
#ifdef LOAD_SAVE
static void DoLoad(System *sys);
static void DoSave(System *sys);
#endif
static void DoList(System *sys);
static void DoRun(System *sys);

/* command table */
static struct {
    char *name;
    void (*handler)(System *sys);
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
static char *NextToken(System *sys);
static int ParseNumber(char *token, VMVALUE *pValue);
static int IsBlank(char *p);

void EditWorkspace(System *sys)
{
    VMVALUE lineNumber;
    char *token;

    BufInit();
    
    VM_printf("ebasic 0.001\n");

    for (;; ) {
        
        VM_getline(sys->lineBuf, sizeof(sys->lineBuf));

        sys->linePtr = sys->lineBuf;

        if ((token = NextToken(sys)) != NULL) {
            if (ParseNumber(token, &lineNumber)) {
                if (IsBlank(sys->linePtr)) {
                    if (!BufDeleteLineN(lineNumber))
                        VM_printf("no line %d\n", lineNumber);
                }
                else if (!BufAddLineN(lineNumber, sys->linePtr))
                    VM_printf("out of edit buffer space\n");
            }

            else {
                int i;
                for (i = 0; cmds[i].name != NULL; ++i)
                    if (strcasecmp(token, cmds[i].name) == 0)
                        break;
                    if (cmds[i].handler) {
                        (*cmds[i].handler)(sys);
                        VM_printf("OK\n");
                    }
                else
                    VM_printf("unknown command: %s\n", token);
            }
        }
    }
}

#ifdef LOAD_SAVE
static int SetProgramName(System *sys)
{
    char *name;
    if ((name = NextToken(sys)) != NULL) {
        strncpy(programName, name, MAX_PROG_NAME - 1);
        programName[MAX_PROG_NAME - 1] = '\0';
    }
    return programName[0] != '\0';
}
#endif

static void DoNew(System *sys)
{
    /* check for a program name on the command line */
#ifdef LOAD_SAVE
    SetProgramName(sys);
#endif
    BufInit();
}

#ifdef LOAD_SAVE

static void DoLoad(System *sys)
{
    VMFILE *fp;
    
    /* check for a program name on the command line */
    if (!SetProgramName(sys)) {
        VM_printf("expecting a file name\n");
        return;
    }
    
    /* load the program */
    if (!(fp = VM_fopen(programName, "r")))
        VM_printf("error loading '%s'\n", programName);
    else {
        VM_printf("Loading '%s'\n", programName);
        BufInit();
        while (VM_fgets(sys->lineBuf, sizeof(sys->lineBuf), fp) != NULL) {
            int len = strlen(sys->lineBuf);
            VMVALUE lineNumber;
            char *token;
            sys->linePtr = sys->lineBuf;
            if ((token = NextToken(sys)) != NULL) {
                if (ParseNumber(token, &lineNumber))
                    BufAddLineN(lineNumber, sys->linePtr);
                else
                    VM_printf("expecting a line number: %s\n", token);
            }
        }
        VM_fclose(fp);
    }
}

static void DoSave(System *sys)
{
    VMFILE *fp;
    
    /* check for a program name on the command line */
    if (!SetProgramName(sys)) {
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
        while (BufGetLine(&lineNumber, sys->lineBuf)) {
            char buf[32];
            sprintf(buf, "%d ", lineNumber);
            VM_fputs(buf, fp);
            VM_fputs(sys->lineBuf, fp);
        }
        VM_fclose(fp);
    }
}

#endif

static void DoList(System *sys)
{
    VMVALUE lineNumber;
    BufSeekN(0);
    while (BufGetLine(&lineNumber, sys->lineBuf))
        VM_printf("%d %s", lineNumber, sys->lineBuf);
}

static int EditGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber)
{
    return BufGetLine(pLineNumber, buf);
}

static void DoRun(System *sys)
{
    ParseContext *c;
    sys->freeNext = sys->freeSpace;
    if (!(c = InitCompiler(sys, MAXOBJECTS)))
        VM_printf("insufficient memory\n");
    else {
        ImageHdr *image;
        BufSeekN(0);
        c->getLine = EditGetLine;
        if ((image = Compile(c)) != NULL) {
            Interpreter *i = (Interpreter *)sys->freeNext;
            size_t stackSize = (sys->freeTop - sys->freeNext - sizeof(Interpreter)) / sizeof(VMVALUE);
            if (stackSize <= 0)
                VM_printf("insufficient memory\n");
            else {
                InitInterpreter(i, stackSize);
                Execute(i, image);
            }
        }
    }
}

static char *NextToken(System *sys)
{
    char *token;
    int ch;
    while ((ch = *sys->linePtr) != '\0' && isspace(ch))
        ++sys->linePtr;
    token = sys->linePtr;
    while ((ch = *sys->linePtr) != '\0' && !isspace(ch))
        ++sys->linePtr;
    if (*sys->linePtr != '\0')
        *sys->linePtr++ = '\0';
    return *token == '\0' ? NULL : token;
}

static int ParseNumber(char *token, VMVALUE *pValue)
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
