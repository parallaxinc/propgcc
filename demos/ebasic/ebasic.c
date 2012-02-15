#include <stdio.h>
#include "db_compiler.h"
#include "db_vm.h"
#include "db_edit.h"

#ifdef CHAMELEONPIC
uint8_t __attribute__((far)) space[HEAPSIZE];
#else
uint8_t space[HEAPSIZE];
#endif

#if defined(PROPELLER_GCC) && defined(USE_FDS)
/* list of drivers we can use */
extern _Driver _FullDuplexSerialDriver;
_Driver *_driverlist[] = {
    &_FullDuplexSerialDriver,
    NULL
};
#endif

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
