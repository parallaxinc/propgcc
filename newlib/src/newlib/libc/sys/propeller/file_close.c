#include <errno.h>
#include <reent.h>
#include "propdev.h"

extern _fs_state_t _fs_state;

int _file_close(int fd)
{
    int i = fd - _FS_BASE_FD;
    if (!_fs_state.initialized || i < 0 || i >= _FS_MAX_FILES)
        return -1;
    if (!_fs_state.files[i].inUse)
        return -1;
    _fs_state.files[i].inUse = FALSE;
    return 0;
}
