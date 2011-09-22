#include <reent.h>
#include <errno.h>
#include "propdev.h"

extern _fs_state_t _fs_state;

int _file_unlink(const char *pathname)
{
    return DFS_UnlinkFile(&_fs_state.vinfo, pathname, _fs_state.scratch) == DFS_OK ? 0 : -1;
}

