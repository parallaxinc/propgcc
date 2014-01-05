#include <stdio.h>
#include "db_edit.h"
#include "db_compiler.h"
#include "db_vm.h"

static DATA_SPACE uint8_t space[WORKSPACESIZE];

DefIntrinsic(dump);
DefIntrinsic(gc);

/* command handlers */
static void DoRun(void *cookie);

/* command table */
UserCmd userCmds[] = {
{   "RUN",      DoRun   },
{   NULL,       NULL    }
};

void CompileAndExecute(ObjHeap *heap);

static int TermGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber);

int main(int argc, char *argv[])
{
    VMVALUE lineNumber = 0;
    ObjHeap *heap;
    System *sys;
    
    VM_sysinit(argc, argv);

    VM_printf("pico-basic 0.001\n");

    if (!(sys = InitSystem(space, sizeof(space))))
        return 1;
    sys->getLine = TermGetLine;
    sys->getLineCookie = &lineNumber;
    
    /* setup an initialization error target */
    if (setjmp(sys->errorTarget) != 0)
        return 1;

    heap = InitHeap(sys, HEAPSIZE, MAXOBJECTS);                     
        
    AddIntrinsic(heap, "DUMP",          dump,       "=i")
    AddIntrinsic(heap, "GC",            gc,         "=i")

    sys->freeMark = sys->freeNext;
     
    EditWorkspace(sys, userCmds, (Handler *)CompileAndExecute, heap);
    
    return 0;
}

static int EditGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber)
{
    return BufGetLine(pLineNumber, buf);
}

static void DoRun(void *cookie)
{
    ObjHeap *heap = (ObjHeap *)cookie;
    System *sys = heap->sys;
    GetLineHandler *getLine;
    void *getLineCookie;
    VMHANDLE code;

    getLine = sys->getLine;
    getLineCookie = sys->getLineCookie;
    
    sys->getLine = EditGetLine;

    BufSeekN(0);

    sys->freeNext = sys->freeMark;
    
    ResetHeap(heap);

    if ((code = Compile(sys, heap, VMFALSE)) != NULL) {
        sys->freeNext = sys->freeMark;
        Execute(sys, heap, code);
    }

    sys->getLine = getLine;
    sys->getLineCookie = getLineCookie;
}

void CompileAndExecute(ObjHeap *heap)
{
    System *sys = heap->sys;
    VMHANDLE code;
    
    sys->freeNext = sys->freeMark;
    
    if ((code = Compile(sys, heap, VMTRUE)) != NULL) {
        sys->freeNext = sys->freeMark;
        Execute(sys, heap, code);
    }
}

static int TermGetLine(void *cookie, char *buf, int len, VMVALUE *pLineNumber)
{
    VMVALUE *pLine = (VMVALUE *)cookie;
    *pLineNumber = ++(*pLine);
    return VM_getline(buf, len) != NULL;
}

void fcn_dump(Interpreter *i)
{
    DumpHeap(i->heap);
}

void fcn_gc(Interpreter *i)
{
    CompactHeap(i->heap);
}

