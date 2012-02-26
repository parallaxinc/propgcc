#include <stdio.h>
#include "db_compiler.h"
#include "db_vm.h"
#include "db_edit.h"

static DATA_SPACE uint8_t space[HEAPSIZE];

int main(int argc, char *argv[])
{
    ParseContext *c = (ParseContext *)space;
    uint8_t *freeSpace = space + sizeof(ParseContext);
    size_t freeSize = sizeof(space) - sizeof(ParseContext);

    VM_sysinit(argc, argv);

    InitCompiler(c, freeSpace, freeSize);

    EditWorkspace(c);
    
    return 0;
}
