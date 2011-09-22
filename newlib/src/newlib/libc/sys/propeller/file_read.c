#include <errno.h>
#include <reent.h>
#include <stdio.h>
#include "propdev.h"

extern _fs_state_t _fs_state;

_ssize_t _file_read(int fd, void *buf, size_t bytes)
{
    int i = fd - _FS_BASE_FD;
    uint32_t count;
    int err;
    if (!_fs_state.initialized || i < 0 || i >= _FS_MAX_FILES)
        return -1;
    if (!_fs_state.files[i].inUse)
        return -1;
    err = DFS_ReadFile(&_fs_state.files[i].file, _fs_state.scratch, buf, &count, bytes);
    return count > 0 ? count : EOF;
}
