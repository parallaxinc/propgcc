#include <stdio.h>
#include "db_compiler.h"
#include "db_vm.h"
#include "db_edit.h"

static DATA_SPACE uint8_t space[HEAPSIZE];

int main(int argc, char *argv[])
{
    System *sys;

    VM_sysinit(argc, argv);

    sys = InitSystem(space, sizeof(space));

    EditWorkspace(sys);
    
    return 0;
}
