#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "db_edit.h"

#ifdef WIN32
#define strcasecmp  _stricmp
#endif

#define MAXTOKEN    32

/* command handlers */
static void DoNew(System *sys);
#ifdef LOAD_SAVE
static void DoLoad(System *sys);
static void DoSave(System *sys);
static void DoCat(System *sys);
#endif
static void DoList(System *sys);

/* command table */
static struct {
    char *name;
    void (*handler)(System *sys);
} cmds[] = {
{   "NEW",      DoNew   },
#ifdef LOAD_SAVE
{   "LOAD",     DoLoad  },
{   "SAVE",     DoSave  },
{   "CAT",      DoCat   },
#endif
{   "LIST",     DoList  },
{   NULL,       NULL    }
};

/* memory of the last filename */
#ifdef LOAD_SAVE
static DATA_SPACE char programName[MAX_PROG_NAME] = "";
#endif

/* prototypes */
static char *NextToken(System *sys);
static int ParseNumber(char *token, VMVALUE *pValue);
static int IsBlank(char *p);

void EditWorkspace(System *sys, UserCmd *userCmds, Handler *evalHandler, void *cookie)
{
    VMVALUE lineNumber;
    char *token;

    BufInit();
    
    while (GetLine(sys)) {

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
                else {
                    for (i = 0; userCmds[i].name != NULL; ++i)
                        if (strcasecmp(token, userCmds[i].name) == 0)
                            break;
                    if (userCmds[i].handler) {
                        (*userCmds[i].handler)(cookie);
                        VM_printf("OK\n");
                    }
                    else {
                        sys->linePtr = sys->lineBuf;
                        (*evalHandler)(cookie);
                    }
                }
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

static void DoCat(System *sys)
{
#if 0
    VMDIRENT entry;
    VMDIR dir;    
    if (VM_opendir(".", &dir) == 0) {
        while (VM_readdir(&dir, &entry) == 0) {
            int len = strlen(entry.name);
            if (len >= 4 && strcasecmp(&entry.name[len - 4], ".bas") == 0)
                VM_printf("  %s\n", entry.name);
        }
        VM_closedir(&dir);
    }
#endif
}

#endif

static void DoList(System *sys)
{
    VMVALUE lineNumber;
    BufSeekN(0);
    while (BufGetLine(&lineNumber, sys->lineBuf))
        VM_printf("%d %s", lineNumber, sys->lineBuf);
}

static char *NextToken(System *sys)
{
    static char token[MAXTOKEN];
    int ch, i;
    
    /* skip leading spaces */
    while ((ch = *sys->linePtr) != '\0' && isspace(ch))
        ++sys->linePtr;
        
    /* collect a token until the next non-space */
    for (i = 0; (ch = *sys->linePtr) != '\0' && !isspace(ch); ++sys->linePtr)
        if (i < sizeof(token) - 1)
            token[i++] = ch;
    token[i] = '\0';
    
    return token[0] == '\0' ? NULL : token;
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
