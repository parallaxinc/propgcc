#include <stdio.h>
#include "db_compiler.h"
#include "db_image.h"
#include "db_vm.h"
#include "db_edit.h"

static DATA_SPACE uint8_t space[HEAPSIZE];

/* command handlers */
static void DoRun(void *cookie);

/* command table */
UserCmd userCmds[] = {
{   "RUN",      DoRun   },
{   NULL,       NULL    }
};

void CompileAndExecute(ImageHdr *image);

static int TermGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber);

int main(int argc, char *argv[])
{
    VMVALUE lineNumber = 0;
    ImageHdr *image;
    System *sys;

    VM_sysinit(argc, argv);

    sys = InitSystem(space, sizeof(space));
    sys->getLine = TermGetLine;
    sys->getLineCookie = &lineNumber;

    if (!(image = AllocateImage(sys, IMAGESIZE)))
        return 1;

    sys->freeMark = sys->freeNext;
     
    EditWorkspace(sys, userCmds, (Handler *)CompileAndExecute, image);
    
    return 0;
}

static int EditGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber)
{
    return BufGetLine(pLineNumber, buf);
}

static void DoRun(void *cookie)
{
    ImageHdr *image = (ImageHdr *)cookie;
    System *sys = image->sys;
    GetLineHandler *getLine;
    void *getLineCookie;
    VMVALUE code;

    getLine = sys->getLine;
    getLineCookie = sys->getLineCookie;
    
    sys->getLine = EditGetLine;

    BufSeekN(0);

    sys->freeNext = sys->freeMark;
    
    InitImage(image);

    if ((code = Compile(sys, image, VMFALSE)) != NULL) {
        sys->freeNext = sys->freeMark;
        Execute(sys, image, code);
    }

    sys->getLine = getLine;
    sys->getLineCookie = getLineCookie;
}

void CompileAndExecute(ImageHdr *image)
{
    System *sys = image->sys;
    VMVALUE code;
    
    sys->freeNext = sys->freeMark;
    
    if ((code = Compile(sys, image, VMTRUE)) != NULL) {
        sys->freeNext = sys->freeMark;
        Execute(sys, image, code);
    }
}

static int TermGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber)
{
    VMVALUE *pLine = (VMVALUE *)cookie;
    *pLineNumber = ++(*pLine);
    return VM_getline(buf, len) != NULL;
}
